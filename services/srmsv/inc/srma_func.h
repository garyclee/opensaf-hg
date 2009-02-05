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

  MODULE NAME: SRMA_FUNC.H

..............................................................................

  DESCRIPTION: This file comtains the MACRO definitions and enum definitions
               defined for SRMSv service, used by both SRMA and SRMND.

******************************************************************************/

#ifndef SRMA_FUNC_H
#define SRMA_FUNC_H

/****************************************************************************
 *                            srma_cbk.c 
 ***************************************************************************/
EXTERN_C uns32 srma_update_sync_cbk_info(SRMA_CB *srma,
                                         MDS_DEST *srmnd_dest);
EXTERN_C void srma_hdl_cbk_dispatch_one(SRMA_CB *srma,
                                        SRMA_USR_APPL_NODE *appl);
EXTERN_C void srma_hdl_cbk_dispatch_all(SRMA_CB *srma, 
                                        SRMA_USR_APPL_NODE *appl,
                                        NCS_BOOL finalize);
EXTERN_C uns32 srma_hdl_cbk_dispatch_blocking(SRMA_CB *srma, SRMA_USR_APPL_NODE *appl);
EXTERN_C uns32 srma_update_appl_cbk_info(SRMA_CB *srma,
                                         NCS_SRMSV_RSRC_CBK_INFO *cbk_info,
                                         SRMA_USR_APPL_NODE *appl);
EXTERN_C void srma_free_cbk_rec(SRMA_RSRC_MON *rsrc_mon);

/****************************************************************************
 *                            srma_tmr.c 
 ***************************************************************************/
EXTERN_C uns32 srma_timer_init(SRMA_CB *srma);
EXTERN_C void srma_timeout_handler (void *arg);
EXTERN_C void srma_del_rsrc_from_monitoring(SRMA_CB *srma, SRMA_RSRC_MON *rsrc,
                                            NCS_BOOL modif_flag);
EXTERN_C uns32 srma_add_rsrc_for_monitoring(SRMA_CB *srma,
                                            SRMA_RSRC_MON *rsrc,
                                            uns32 period,
                                            NCS_BOOL create_flg);
EXTERN_C uns32 srma_timer_destroy(SRMA_CB *srma);
EXTERN_C void srma_rsrc_mon_timeout_handler(NCSCONTEXT arg);

/****************************************************************************
 *                          srma_db.c
 ***************************************************************************/
EXTERN_C NCS_SRMSV_ERR srma_usr_appl_create(SRMA_CB *srma,
                                            NCS_SRMSV_CALLBACK srmsv_cbk,
                                            NCS_SRMSV_HANDLE *srmsv_hdl);

EXTERN_C NCS_SRMSV_ERR srma_hdl_cbk_dispatch(SRMA_CB *srma,
                                             NCS_SRMSV_HANDLE srmsv_hdl,
                                             SaDispatchFlagsT dispatch_flags);

EXTERN_C NCS_SRMSV_ERR srma_usr_appl_delete(SRMA_CB *srma,
                                            NCS_SRMSV_HANDLE srmsv_hdl,
                                            NCS_BOOL appl_del);

EXTERN_C NCS_SRMSV_ERR srma_appl_start_mon(SRMA_CB *srma,
                                           NCS_SRMSV_HANDLE srmsv_hdl);

EXTERN_C NCS_SRMSV_ERR srma_appl_stop_mon(SRMA_CB *srma,
                                          NCS_SRMSV_HANDLE srmsv_hdl);

EXTERN_C NCS_SRMSV_ERR srma_appl_create_rsrc_mon(SRMA_CB *srma, 
                                          uns32 srmsv_hdl,
                                          NCS_SRMSV_MON_INFO *mon_info,
                                          NCS_SRMSV_RSRC_HANDLE *rsrc_mon_hdl);

EXTERN_C NCS_SRMSV_ERR srma_appl_modify_rsrc_mon(SRMA_CB *srma, 
                                           NCS_SRMSV_MON_INFO *mon_info,
                                           NCS_SRMSV_RSRC_HANDLE rsrc_mon_hdl);

EXTERN_C NCS_SRMSV_ERR srma_appl_delete_rsrc_mon(SRMA_CB *srma,
                                           NCS_SRMSV_RSRC_HANDLE rsrc_mon_hdl,
                                           NCS_BOOL flg);

EXTERN_C NCS_SRMSV_ERR srma_appl_subscr_rsrc_mon(SRMA_CB *srma,
                                       uns32 srmsv_hdl,
                                       NCS_SRMSV_RSRC_INFO *rsrc_info,
                                       NCS_SRMSV_SUBSCR_HANDLE *subscr_handle);

EXTERN_C NCS_SRMSV_ERR srma_appl_unsubscr_rsrc_mon(SRMA_CB *srma,
                                        NCS_SRMSV_SUBSCR_HANDLE subscr_handle);

EXTERN_C NCS_SRMSV_ERR srma_appl_stop_rsrc_mon(SRMA_CB *srma,
                                           NCS_SRMSV_RSRC_HANDLE rsrc_mon_hdl);

EXTERN_C NCS_SRMSV_ERR srma_appl_start_rsrc_mon(SRMA_CB *srma,
                                           NCS_SRMSV_RSRC_HANDLE rsrc_mon_hdl);

EXTERN_C NCS_SRMSV_ERR srma_appl_start_rsrc_mon(SRMA_CB *srma,
                                          NCS_SRMSV_RSRC_HANDLE rsrc_mon_hdl);

EXTERN_C NCS_SRMSV_ERR srma_get_watermark_val(SRMA_CB *srma, 
                                              NCS_SRMSV_HANDLE srmsv_hdl,
                                              NCS_SRMSV_RSRC_INFO *rsrc_info,
                                              NCS_SRMSV_WATERMARK_TYPE wm_type);

/****************************************************************************
 *                          srma_proc.c
 ***************************************************************************/
EXTERN_C void srma_del_rsrc_mon_from_appl_list(SRMA_CB *srma, SRMA_RSRC_MON *rsrc_mon);
EXTERN_C void srma_del_rsrc_mon_from_srmnd_list(SRMA_CB *srma, SRMA_RSRC_MON *rsrc_mon);
EXTERN_C void srma_add_usr_appl_node(SRMA_CB *srma, SRMA_USR_APPL_NODE *appl);
EXTERN_C void srma_add_rsrc_mon_to_bcast_list(SRMA_RSRC_MON *rsrc, SRMA_USR_APPL_NODE *appl);
EXTERN_C void srma_del_rsrc_mon_from_bcast_list(SRMA_RSRC_MON *rsrc_mon,
                                                SRMA_USR_APPL_NODE *appl);
EXTERN_C uns32 srma_validate_mon_info(NCS_SRMSV_MON_INFO *mon_info, 
                                      NODE_ID  node_num);
EXTERN_C uns32 srma_validate_rsrc_info(NCS_SRMSV_RSRC_INFO *rsrc, NODE_ID node_num);
EXTERN_C uns32 srma_validate_value(NCS_SRMSV_VALUE *val, NCS_SRMSV_RSRC_TYPE rsrc_type);
EXTERN_C uns32 srma_validate_mon_data(NCS_SRMSV_MON_DATA *mon_data, 
                                      NCS_SRMSV_RSRC_TYPE rsrc_type);
EXTERN_C uns32 srma_add_rsrc_mon_to_appl_list(SRMA_CB *srma,
                                     SRMA_RSRC_MON *rsrc,
                                     SRMA_USR_APPL_NODE *appl);
EXTERN_C uns32 srma_add_rsrc_mon_to_srmnd_list(SRMA_CB *srma,
                                      SRMA_RSRC_MON *rsrc,
                                      SRMA_USR_APPL_NODE *appl,
                                      NODE_ID node_id);
EXTERN_C SRMA_RSRC_MON *srma_check_duplicate_mon_info(SRMA_USR_APPL_NODE *appl,
                                             NCS_SRMSV_MON_INFO *info,
                                             NCS_BOOL only_rsrc_info);

EXTERN_C SRMA_SRMND_USR_NODE *srma_create_srmnd_usr_add_rsrc(NCSCONTEXT srmnd_or_appl,
                                                    SRMA_RSRC_MON *rsrc,
                                           SRMA_SRMND_USR_NODE_TYPE node_type);

EXTERN_C SRMA_SRMND_INFO *srma_check_srmnd_exists(SRMA_CB *srma, NODE_ID node_id);

EXTERN_C void srma_send_srmnd_rsrc_mon_data(SRMA_CB *srma, MDS_DEST *srmnd_dest, uns32 usr_hdl);
EXTERN_C SRMA_SRMND_INFO *srma_add_srmnd_node(SRMA_CB *srma, NODE_ID node_id);
EXTERN_C uns32 srma_inform_appl_rsrc_expiry(SRMA_CB *srma, SRMA_RSRC_MON *rsrc);
EXTERN_C void srma_del_srmnd_node(SRMA_CB *srma, SRMA_SRMND_INFO *srmnd);
EXTERN_C void srma_del_usr_appl_node(SRMA_CB *srma, SRMA_USR_APPL_NODE *appl);

/****************************************************************************
 *                          srma_evt.c
 ***************************************************************************/
EXTERN_C uns32 srma_send_rsrc_msg(SRMA_CB *srma,
                                  SRMA_USR_APPL_NODE *appl,
                                  MDS_DEST *srmnd_dest,
                                  SRMA_MSG_TYPE msg_type,
                                  SRMA_RSRC_MON *rsrc);
EXTERN_C uns32 srma_send_appl_msg(SRMA_CB *srma, 
                                  SRMA_MSG_TYPE msg_type,
                                  uns32 appl_hdl,
                                  SRMA_SRMND_INFO *srmnd_info);
EXTERN_C uns32 srma_process_srmnd_msg(SRMA_CB   *srma,
                                      SRMND_MSG *srmnd_msg,
                                      MDS_DEST  *srmnd_dest);
EXTERN_C void srma_del_srmnd_msg(SRMND_MSG *msg);


#endif /* SRMA_FUNC_H */



