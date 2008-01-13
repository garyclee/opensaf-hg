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

  MODULE NAME: IFSV_MDS.H

$Header: 
..............................................................................

  DESCRIPTION: Prototypes definations for IFSV-MDS functions


******************************************************************************/

#ifndef IFSV_MDS_H
#define IFSV_MDS_H

/* Time out value for sync requests */
#define IFSV_MDS_SYNC_TIME    1000   

uns32
ifsv_mds_bcast_send (MDS_HDL mds_hdl,                       
                      NCSMDS_SVC_ID from_svc, 
                      NCSCONTEXT evt, 
                      NCSMDS_SVC_ID to_svc);

uns32
ifsv_mds_scoped_send (MDS_HDL mds_hdl, 
                      NCSMDS_SCOPE_TYPE scope,
                      NCSMDS_SVC_ID from_svc, 
                      NCSCONTEXT evt, 
                      NCSMDS_SVC_ID to_svc);

uns32 
ifsv_mds_msg_sync_send (MDS_HDL mds_hdl, 
                        uns32       from_svc, 
                        uns32       to_svc, 
                        MDS_DEST    to_dest, 
                        IFSV_EVT    *i_evt, 
                        IFSV_EVT    **o_evt,
                        uns32       timeout);

uns32
ifsv_mds_normal_send (MDS_HDL mds_hdl, 
                     NCSMDS_SVC_ID from_svc, 
                     NCSCONTEXT evt, MDS_DEST dest, 
                     NCSMDS_SVC_ID to_svc);

uns32 ifsv_edp_ncs_svdest(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err);

uns32 ifsv_edp_svcd_upd(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err);

uns32 ifsv_edp_svcd_get(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len,
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err);

uns32 ifsv_edp_ifa_intf_create_rsp_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

int ifd_a2s_evt_type_test_fnc (NCSCONTEXT arg);

uns32 ifsv_edp_ifd_a2s_ifindex_spt_map_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err);

uns32 ifsv_edp_ifd_a2s_intf_data_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err);

uns32 ifsv_edp_ifd_a2s_svc_dest_upd_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err);

uns32 ifsv_edp_ifd_a2s_ifindex_upd_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err);

uns32 ifsv_edp_ifd_a2s_ifnd_up_down_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err);

uns32 ifsv_mds_send_rsp(MDS_HDL mds_hdl, 
                        NCSMDS_SVC_ID  from_svc, 
                        IFSV_SEND_INFO *s_info, 
                        IFSV_EVT       *evt);

uns32 ifsv_edp_intf_data(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                         NCSCONTEXT ptr, uns32 *ptr_data_len, 
                         EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                         EDU_ERR *o_err);
uns32 ifsv_edp_spt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                   NCSCONTEXT ptr, uns32 *ptr_data_len, 
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                   EDU_ERR *o_err);
uns32 ifsv_edp_intf_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                         NCSCONTEXT ptr, uns32 *ptr_data_len, 
                         EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                         EDU_ERR *o_err);
uns32 ifsv_edp_spt_map(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                       NCSCONTEXT ptr, uns32 *ptr_data_len, 
                       EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                       EDU_ERR *o_err);
uns32 ifsv_edp_stats_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                          NCSCONTEXT ptr, uns32 *ptr_data_len, 
                          EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                          EDU_ERR *o_err);
uns32 ifsv_edp_create_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err);
uns32 ifsv_edp_destroy_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err);
uns32 ifsv_edp_init_done_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                              NCSCONTEXT ptr, uns32 *ptr_data_len, 
                              EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                              EDU_ERR *o_err);
uns32 ifsv_edp_age_tmr_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err);
uns32 ifsv_edp_rec_sync_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err);
uns32 ifsv_edp_intf_get_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err);
uns32 ifsv_edp_stats_get_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err);
uns32 ifsv_edp_idim_port_stats(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                                NCSCONTEXT ptr, uns32 *ptr_data_len, 
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                                EDU_ERR *o_err);
uns32 ifsv_edp_idim_port_status(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                                NCSCONTEXT ptr, uns32 *ptr_data_len, 
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                                EDU_ERR *o_err);
uns32 ifsv_edp_idim_port_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                              NCSCONTEXT ptr, uns32 *ptr_data_len, 
                              EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                              EDU_ERR *o_err);
uns32 ifsv_edp_idim_port_type(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                              NCSCONTEXT ptr, uns32 *ptr_data_len, 
                              EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                              EDU_ERR *o_err);
uns32 ifsv_edp_idim_hw_rcv_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                                NCSCONTEXT ptr, uns32 *ptr_data_len, 
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                                EDU_ERR *o_err);
uns32 ifsv_edp_idim_hw_req_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                                NCSCONTEXT ptr, uns32 *ptr_data_len, 
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                                EDU_ERR *o_err);
uns32 ifsv_edp_idim_ifnd_up_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                                 NCSCONTEXT ptr, uns32 *ptr_data_len, 
                                 EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                                 EDU_ERR *o_err);

uns32 ifsv_edp_ifsv_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                   NCSCONTEXT ptr, uns32 *ptr_data_len, 
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                   EDU_ERR *o_err);

uns32 ifsv_edp_ifd_a2s_msg_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err);

uns32 ifsv_edp_ifd_a2s_iaps_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err);

uns32 ifsv_edp_nodeid_ifname(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                   NCSCONTEXT ptr, uns32 *ptr_data_len,
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                   EDU_ERR *o_err);

#if (NCS_VIP == 1)

uns32 ifsv_edp_ifsv_vip_hdl(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                   NCSCONTEXT ptr, uns32 *ptr_data_len, 
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                   EDU_ERR *o_err);
uns32 ifsv_edp_ifa_vip_add(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                   NCSCONTEXT ptr, uns32 *ptr_data_len, 
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                   EDU_ERR *o_err);
uns32 ifsv_edp_ifnd_vip_add(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                   NCSCONTEXT ptr, uns32 *ptr_data_len, 
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                   EDU_ERR *o_err);
uns32 ifsv_edp_ifa_vip_free(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                   NCSCONTEXT ptr, uns32 *ptr_data_len, 
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                   EDU_ERR *o_err);
uns32 ifsv_edp_ifnd_vip_free(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                   NCSCONTEXT ptr, uns32 *ptr_data_len, 
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                   EDU_ERR *o_err);


uns32 ifsv_edp_ifnd_info_add_resp(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                   NCSCONTEXT ptr, uns32 *ptr_data_len, 
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                   EDU_ERR *o_err);
uns32 ifsv_edp_ifd_info_add_resp(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                   NCSCONTEXT ptr, uns32 *ptr_data_len, 
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                   EDU_ERR *o_err);

uns32 ifsv_edp_ifnd_free_resp(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                   NCSCONTEXT ptr, uns32 *ptr_data_len, 
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                   EDU_ERR *o_err);

uns32 ifsv_edp_ifd_free_resp(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                   NCSCONTEXT ptr, uns32 *ptr_data_len, 
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                   EDU_ERR *o_err);

uns32 ifsv_edp_vip_common_event(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ifsv_edp_ip_from_stale_entry(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ifsv_edp_vip_err_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                   NCSCONTEXT ptr, uns32 *ptr_data_len, 
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                   EDU_ERR *o_err);

uns32 ifsv_edp_vip_chk_pt_full_rec(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ifsv_edp_ifd_a2s_vip_rec_info_evt(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                   NCSCONTEXT ptr, uns32 *ptr_data_len, 
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                   EDU_ERR *o_err);

uns32 ifsv_edp_vip_chk_pt_ip_data(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ifsv_edp_vip_chk_pt_intf_data(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ifsv_edp_vip_chk_pt_owner_data(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ifsv_edp_vip_chk_pt_data_base(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

uns32 ifsv_edp_vip_chk_pt_partial_rec(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);





#endif

int ifsv_ifd_evt_test_type_fnc(NCSCONTEXT arg);
int ifsv_ifnd_evt_test_type_fnc(NCSCONTEXT arg);
int ifsv_hw_resp_evt_test_type_fnc(NCSCONTEXT arg);
int ifsv_hw_req_evt_test_type_fnc(NCSCONTEXT arg);

/* This needs to be put in a Proper file */
uns32 ifsv_ifa_app_if_info_indicate(IFSV_INTF_DATA *actual_data, 
                            IFSV_INTF_REC_EVT intf_evt, uns32 attr,
                            IFSV_CB *cb);
uns32 ifsv_edp_idim_hw_req(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err);
int ifa_evt_test_type_fnc (NCSCONTEXT arg);
uns32 ifsv_edp_spt_map_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err);
int ifsv_ifkey_type_test_fnc (NCSCONTEXT arg);
uns32 ifsv_edp_ifa_intf_create_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err);
uns32 ifsv_edp_ifa_intf_destroy_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err);
uns32 ifsv_edp_ifa_ifget_info_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err);
uns32 ifsv_edp_ifa_ifget_rsp_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                             NCSCONTEXT ptr, uns32 *ptr_data_len, 
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                             EDU_ERR *o_err);

uns32 ifsv_edp_ifkey_info (EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                           EDU_ERR *o_err);

uns32 ifsv_edp_ifa_ifrec_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn,
                             NCSCONTEXT ptr, uns32 *ptr_data_len,
                             EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
                             EDU_ERR *o_err);

void ifsv_evt_ptrs_free(IFSV_EVT *evt, IFSV_EVT_TYPE type);

int ifsv_evt_test_type_fnc (NCSCONTEXT arg);

#endif /* IFSV_MDS_H */

