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

  MODULE NAME: SRMND_FUNC.H

..............................................................................

  DESCRIPTION: This file comtains the MACRO definitions and enum definitions
               defined for SRMSv service, used by both SRMA and SRMND.

******************************************************************************/

#ifndef SRMND_FUNC_H
#define SRMND_FUNC_H

/****************************************************************************
 *                            srmnd_db.c 
 ***************************************************************************/
EXTERN_C uns32 srmnd_get_watermark(SRMND_CB *srmnd,
                                   NCS_BOOL bcast,
                                   SRMND_SRMA_USR_KEY *usr_key,
                                   SRMA_GET_WATERMARK *get_wm);
EXTERN_C uns32 srmnd_create_rsrc_mon(SRMND_CB *srmnd,
                            NCS_BOOL bcast,
                            SRMND_SRMA_USR_KEY *usr_key,
                            SRMA_CREATE_RSRC_MON *create_mon,
                            NCS_BOOL stop_monitoring, 
                            NCS_BOOL bulk_create);
EXTERN_C void srmnd_del_srma_rsrc_mon(SRMND_CB *srmnd, 
                                      NCS_BOOL bcast,
                                      SRMND_SRMA_USR_KEY *usr_key,
                                      uns32 rsrc_hdl);
EXTERN_C void srmnd_del_rsrc_mon(SRMND_CB *srmnd, uns32 rsrc_hdl);
EXTERN_C uns32 srmnd_start_stop_appl_mon(SRMND_CB *srmnd,
                                         SRMND_SRMA_USR_KEY *usr_key,
                                         NCS_BOOL start_flg);
EXTERN_C void srmnd_del_appl_node(SRMND_CB *srmnd, SRMND_MON_SRMA_USR_NODE *user_node);
EXTERN_C uns32 srmnd_modify_rsrc_mon(SRMND_CB *srmnd,
                                     NCS_BOOL bcast,
                                     SRMND_SRMA_USR_KEY *usr_key,
                                     SRMA_MODIFY_RSRC_MON *modify_mon);
EXTERN_C uns32 srmnd_bulk_create_rsrc_mon(SRMND_CB *srmnd,
                                 NCS_BOOL bcast,
                                 SRMND_SRMA_USR_KEY *usr_key,
                                 SRMA_BULK_CREATE_RSRC_MON *bulk_create_mon);
EXTERN_C void srmnd_del_appl_rsrc(SRMND_CB *srmnd, 
                                  SRMND_MON_SRMA_USR_NODE *user_node);
EXTERN_C uns32 srmnd_unreg_appl(SRMND_CB *srmnd,
                                SRMND_SRMA_USR_KEY *usr_key);

/****************************************************************************
 *                            srmnd_tmr.c 
 ***************************************************************************/
EXTERN_C uns32 srmnd_timer_init(SRMND_CB *srmnd_cb);
EXTERN_C uns32 srmnd_add_rsrc_for_monitoring(SRMND_CB *srmnd,
                                             SRMND_RSRC_MON_NODE *rsrc,
                                             NCS_BOOL create_flg);
EXTERN_C void srmnd_del_rsrc_from_monitoring(NCS_RP_TMR_CB *srmnd,
                                             NCS_RP_TMR_HDL *rsrc);
EXTERN_C void srmnd_timeout_handler (void *arg);
EXTERN_C uns32 srmnd_timer_destroy(SRMND_CB *srmnd);
EXTERN_C void srmnd_rsrc_mon_timeout_handler(NCSCONTEXT arg);

/****************************************************************************
 *                            srmnd_proc.c 
 ***************************************************************************/
EXTERN_C void srmnd_db_dump(SRMND_CB *srmnd);
EXTERN_C SRMND_MON_SRMA_USR_NODE  *srmnd_get_appl_node(SRMND_CB *srmnd, 
                                              SRMND_SRMA_USR_KEY *usr_key);
EXTERN_C void srmnd_remove_rsrc_pid_data(SRMND_RSRC_MON_NODE *rsrc);
EXTERN_C SRMND_RSRC_TYPE_NODE *srmnd_check_rsrc_type_node(SRMND_CB *srmnd,
                                                 NCS_SRMSV_RSRC_TYPE rsrc_type,
                                                 uns32 pid);
EXTERN_C void srmnd_del_rsrc_mon_from_rsrc_type_list(SRMND_CB *srmnd,
                                            SRMND_RSRC_MON_NODE *rsrc_mon);
EXTERN_C void srmnd_del_rsrc_mon_from_user_list(SRMND_CB *srmnd,
                                       SRMND_RSRC_MON_NODE *rsrc_mon);
EXTERN_C SRMND_PID_DATA *srmnd_get_pid_node(SRMND_RSRC_MON_NODE *rsrc, uns32 pid);
EXTERN_C void srmnd_remove_pid_data(SRMND_RSRC_MON_NODE *rsrc,
                           uns32 pid,
                           SRMND_PID_DATA *pid_data);
EXTERN_C SRMND_SAMPLE_DATA *srmnd_get_sample_record(SRMND_RSRC_MON_NODE *rsrc);
EXTERN_C void srmnd_delete_rsrc_sample_data(SRMND_RSRC_MON_NODE *rsrc);
EXTERN_C void srmnd_del_srma(SRMND_CB *srmnd, MDS_DEST *srma_dest);
EXTERN_C void srmnd_main_process (void *arg);

/****************************************************************************
 *                          srmnd_stat.c
 ***************************************************************************/
EXTERN_C uns32 srmnd_get_cpu_utilization_stats(SRMND_RSRC_MON_NODE *rsrc);
EXTERN_C uns32 srmnd_get_memory_utilization_stats(SRMND_RSRC_MON_NODE *rsrc);
EXTERN_C uns32 srmnd_update_pid_data(SRMND_RSRC_MON_NODE *rsrc);
EXTERN_C uns32 srmnd_get_cpu_ldavg_stats(SRMND_RSRC_MON_NODE *rsrc);
EXTERN_C uns32 srmnd_get_proc_memory_util_stats(SRMND_RSRC_MON_NODE *rsrc);
EXTERN_C uns32 srmnd_get_proc_cpu_util_stats(SRMND_RSRC_MON_NODE *rsrc);

/****************************************************************************
 *                          srmnd_msg.c
 ***************************************************************************/
EXTERN_C uns32 srmnd_process_srma_msg(SRMND_CB *srmnd,
                             SRMA_MSG *srma_msg,
                             MDS_DEST *srma_dest);
EXTERN_C uns32 srmnd_send_msg(SRMND_CB *srmnd, 
                              NCSCONTEXT data,
                              SRMND_MSG_TYPE msg_type);
EXTERN_C uns32 srmnd_check_rsrc_stats(SRMND_CB *srmnd, 
                                      SRMND_RSRC_MON_NODE *rsrc,
                                      NCS_BOOL *rsrc_delete);
EXTERN_C void srmnd_del_srma_msg(SRMA_MSG *msg);
EXTERN_C void srmnd_process_tmr_msg(uns32 rsrc_mon_hdl);

#endif /* SRMND_FUNC_H */
