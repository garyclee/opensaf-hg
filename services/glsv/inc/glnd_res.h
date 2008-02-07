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

#ifndef GLND_RES_H
#define GLND_RES_H

#if 0

/* typedef enums */
typedef enum
{
   GLND_RESOURCE_NOT_INITIALISED = 0,
   GLND_RESOURCE_ACTIVE_MASTER,
   GLND_RESOURCE_ACTIVE_NON_MASTER,
   GLND_RESOURCE_ELECTION_IN_PROGESS
   GLND_RESOURCE_MASTER_RESTARTED,
   GLND_RESOURCE_MASTER_OPERATIONAL,
   GLND_RESOURCE_ELECTION_COMPLETED
}GLND_RESOURCE_STATUS;
#endif


/* main resource data structure */

typedef struct glnd_resource_info_tag
{
   NCS_PATRICIA_NODE       patnode;
   SaLckResourceIdT        resource_id;   /* index for identifying the resource */
   SaNameT                 resource_name;
   uns32                   lcl_ref_cnt;
   MDS_DEST                master_mds_dest; /* master vcard id */
   GLND_NODE_STATUS        master_status;
     
   GLND_LOCK_MASTER_INFO   lck_master_info;
   GLND_RES_LOCK_LIST_INFO *lcl_lck_req_info;  /* local lock info */
   GLND_RESOURCE_STATUS    status;        /* bit-wise data */
   uns32                   shm_index;
}GLND_RESOURCE_INFO;


/* prototypes */

EXTERN_C GLND_RESOURCE_INFO *glnd_resource_node_find_by_name(GLND_CB *glnd_cb, SaNameT res_name);
#if 0
EXTERN_C GLND_RESOURCE_INFO *glnd_resource_node_find(GLND_CB *glnd_cb,SaLckResourceIdT res_id); 
#endif
EXTERN_C GLND_RESOURCE_INFO *glnd_resource_node_add(GLND_CB *glnd_cb, 
                                                    SaLckResourceIdT res_id,
                                                    SaNameT resource_name,
                                                    NCS_BOOL  is_master,
                                                    MDS_DEST master_mds_dest);

EXTERN_C uns32 glnd_set_orphan_state(GLND_CB            *glnd_cb,GLND_RESOURCE_INFO *res_info);


EXTERN_C void  glnd_resource_lock_req_set_orphan(GLND_CB *glnd_cb, 
                                                 GLND_RESOURCE_INFO *res_info,
                                                 SaLckLockModeT   type);

EXTERN_C void  glnd_resource_lock_req_unset_orphan(GLND_CB *glnd_cb, 
                                                   GLND_RESOURCE_INFO *res_info,
                                                   SaLckLockModeT   type);

EXTERN_C uns32 glnd_resource_node_destroy(GLND_CB *glnd_cb, 
                                          GLND_RESOURCE_INFO *res_info);

EXTERN_C GLND_RES_LOCK_LIST_INFO* glnd_resource_grant_lock_req_find(GLND_RESOURCE_INFO  *res_info,
                                                          GLSV_LOCK_REQ_INFO res_lock_info,
                                                          MDS_DEST            req_mds_dest,
                                                          SaLckResourceIdT  lcl_resource_id);


EXTERN_C GLND_RES_LOCK_LIST_INFO* glnd_resource_pending_lock_req_find(GLND_RESOURCE_INFO  *res_info,
                                                          GLSV_LOCK_REQ_INFO res_lock_info,
                                                          MDS_DEST            req_mds_dest,
                                                          SaLckResourceIdT  lcl_resource_id);


EXTERN_C GLND_RES_LOCK_LIST_INFO* glnd_resource_remote_lock_req_find(GLND_RESOURCE_INFO  *res_info, 
                                                                     SaLckLockIdT        lockid,
                                                                     SaLckHandleT        handleId,
                                                                     MDS_DEST            req_mds_dest,
                                                                      SaLckResourceIdT        lcl_resource_id);

EXTERN_C GLND_RES_LOCK_LIST_INFO* glnd_resource_local_lock_req_find(GLND_RESOURCE_INFO  *res_info, 
                                                                    SaLckLockIdT        lockid,
                                                                    SaLckHandleT        handleId,
                                                                      SaLckResourceIdT        lcl_resource_id);

EXTERN_C void glnd_resource_lock_req_delete(GLND_RESOURCE_INFO      *res_info, 
                                            GLND_RES_LOCK_LIST_INFO *lck_list_info);

EXTERN_C void glnd_resource_lock_req_destroy(GLND_RESOURCE_INFO      *res_info, 
                                             GLND_RES_LOCK_LIST_INFO *lck_list_info);

EXTERN_C GLND_RES_LOCK_LIST_INFO *glnd_resource_master_process_lock_req(GLND_CB             *cb,
                                                                        GLND_RESOURCE_INFO  *res_info, 
                                                                        GLSV_LOCK_REQ_INFO  lock_info,
                                                                        MDS_DEST            req_node_mds_dest,
                                                                        SaLckResourceIdT        lcl_resource_id,
SaLckLockIdT        lockid);

EXTERN_C GLND_RES_LOCK_LIST_INFO *glnd_resource_non_master_lock_req(GLND_CB  *cb,
                                                                    GLND_RESOURCE_INFO  *res_info, 
                                                                    GLSV_LOCK_REQ_INFO  lock_info,
                                                                    SaLckResourceIdT        lcl_resource_id,SaLckLockIdT        lockid);

EXTERN_C GLND_RES_LOCK_LIST_INFO *glnd_resource_master_unlock_req(GLND_CB             *cb,
                                                                  GLND_RESOURCE_INFO  *res_info,
                                                                  GLSV_LOCK_REQ_INFO  lock_info,
                                                                  MDS_DEST            req_mds_dest,
                                                                  SaLckResourceIdT  lcl_resource_id);

EXTERN_C GLND_RES_LOCK_LIST_INFO *glnd_resource_non_master_unlock_req(GLND_CB             *cb,
                                                                      GLND_RESOURCE_INFO  *res_info,
                                                                      GLSV_LOCK_REQ_INFO  lock_info,
                                                                      SaLckResourceIdT  lcl_resource_id,SaLckLockIdT        lockid);

EXTERN_C NCS_BOOL glnd_resource_grant_list_orphan_locks(GLND_RESOURCE_INFO  *res_info,
                                                        SaLckLockModeT      *mode) ;


EXTERN_C void glnd_resource_master_lock_purge_req(GLND_CB  *glnd_cb, GLND_RESOURCE_INFO  *res_info,NCS_BOOL is_local);

EXTERN_C void glnd_resource_master_lock_resync_grant_list(GLND_CB *glnd_cb,GLND_RESOURCE_INFO  *res_info);

EXTERN_C void glnd_resource_convert_nonmaster_to_master(GLND_CB *glnd_cb,GLND_RESOURCE_INFO  *res_node);

EXTERN_C void glnd_resource_resend_nonmaster_info_to_newmaster(GLND_CB *glnd_cb,GLND_RESOURCE_INFO  *res_node);

EXTERN_C void glnd_resource_master_process_resend_lock_req(GLND_CB             *glnd_cb,
                                                           GLND_RESOURCE_INFO  *res_node,
                                                           GLSV_LOCK_REQ_INFO  lock_info,
                                                           MDS_DEST            req_node_mds_id);

struct glsv_evt_glnd_dd_probe_info_tag; /* forward declaration required. */

EXTERN_C NCS_BOOL glnd_deadlock_detect(GLND_CB                                 *glnd_cb,
                                       GLND_CLIENT_INFO                        *client_info,
                                       struct glsv_evt_glnd_dd_probe_info_tag  *dd_probe);


EXTERN_C void glnd_resource_check_lost_unlock_requests(GLND_CB             *glnd_cb,
                                                       GLND_RESOURCE_INFO  *res_node);
#endif
