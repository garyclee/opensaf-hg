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

  MODULE NAME:        vds_def.h


  DESCRIPTION:

******************************************************************************
*/

#ifndef VDS_DEFS_H
#define VDS_DEFS_H

struct vds_evt;

EXTERN_C uns32 gl_vds_hdl;
EXTERN_C uns32 vds_role_ack_to_state;
EXTERN_C uns32 vds_destroying;

#ifndef VDS_TRACE_LEVEL
#define VDS_TRACE_LEVEL 0
#endif

#if (VDS_TRACE_LEVEL == 1)
#define VDS_TRACE1_ARG1(x)    m_NCS_CONS_PRINTF(x)
#define VDS_TRACE1_ARG2(x,y)  m_NCS_CONS_PRINTF(x,y)
#define VDS_TRACE1_ARG3(x,y,z) m_NCS_CONS_PRINTF(x,y,z)
#else
#define VDS_TRACE1_ARG1(x)
#define VDS_TRACE1_ARG2(x,y)
#define VDS_TRACE1_ARG3(x,y,z)
#endif

#define ID_PAT_NODE_OFFSET \
   ((long)&((VDS_VDEST_DB_INFO*)(NULL))->id_pat_node)


/***************************************************************************
                         Function Prototypes
***************************************************************************/
/* vds_main.c */
EXTERN_C void vds_main_process (void *arg);
EXTERN_C NCS_BOOL vds_mbx_clean (NCSCONTEXT arg, NCSCONTEXT msg);
EXTERN_C uns32 vds_lib_req(NCS_LIB_REQ_INFO *req);
EXTERN_C void vds_db_scavanger(struct vds_cb *cb);
EXTERN_C uns32  vds_destroy(NCS_BOOL cancel);
EXTERN_C void vds_debug_dump(void);
/* vds_db.c */
EXTERN_C uns32 vds_process_vdest_create(struct vds_cb *vds,
                                        MDS_DEST  *src_adest,
                                        NCSVDA_VDEST_CREATE_INFO *info);
EXTERN_C uns32 vds_process_vdest_lookup(struct vds_cb *vds,
                                        NCSVDA_VDEST_LOOKUP_INFO *info);
EXTERN_C uns32 vds_process_vdest_destroy(struct vds_cb *vds,
                                         MDS_DEST   *src_adest,
                                         NCSCONTEXT info,
                                         NCS_BOOL external);
EXTERN_C void vds_clean_non_persistent_vdests(VDS_CB *vds_cb,
                                              MDS_DEST *adest_down);
EXTERN_C VDS_VDEST_ADEST_INFO *vds_new_vdest_instance(VDS_VDEST_DB_INFO *db_node,
                                                      MDS_DEST *adest);

/* vds_mds.c */
typedef uns32 (*vds_mds_cbk_routine)(struct vds_cb *cb, 
                                     struct ncsmds_callback_info *info);
EXTERN_C uns32 vds_role_agent_callback(struct vds_cb *my_hdl, V_DEST_RL role);
EXTERN_C uns32 vds_mds_send(struct vds_cb *vds, struct vds_evt *vds_evt);
EXTERN_C uns32  vds_mds_unreg(struct vds_cb *vds);

/* vds_log.c */
EXTERN_C uns32 vds_log_reg(void);
EXTERN_C uns32 vds_log_unreg(void);

#endif

