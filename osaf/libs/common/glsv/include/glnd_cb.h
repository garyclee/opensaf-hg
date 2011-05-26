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

#ifndef GLND_CB_H
#define GLND_CB_H

#include <saCkpt.h>

#include "glnd_tmr.h"
#include "glnd_evt.h"

/* global variables */
uns32 gl_glnd_hdl;
NCSCONTEXT gl_glnd_task_hdl;

/* macros for the global varibales */
#define m_GLND_RETRIEVE_GLND_CB_HDL      gl_glnd_hdl
#define m_GLND_RETRIEVE_GLND_TASK_HDL    gl_glnd_task_hdl

#define GLSV_GLND_MASTER_REELECTION_WAIT_TIME 10000000

/* macros for the CB handle */
#define m_GLND_TAKE_GLND_CB      ncshm_take_hdl(NCS_SERVICE_ID_GLND, gl_glnd_hdl)
#define m_GLND_GIVEUP_GLND_CB    ncshm_give_hdl(gl_glnd_hdl)

#define m_GLND_IS_LOCAL_NODE(m,n)   memcmp(m,n,sizeof(MDS_DEST))

#define GLSV_GLND_SHM_VERSION    1

typedef enum {
	GLND_LOCK_MASTER_NODE = 1,
	GLND_LOCK_NON_MASTER_NODE = 2
} GLND_NODE_TYPE;

typedef struct glnd_res_lock_list_info_tag {
	uns32 lck_info_hdl_id;	/* maintained for the validity */
	GLSV_LOCK_REQ_INFO lock_info;
	GLND_TMR timeout_tmr;
	MDS_DEST req_mdest_id;	/* requesting node info */
	SaLckResourceIdT lcl_resource_id;
	MDS_SYNC_SND_CTXT glnd_res_lock_mds_ctxt;	/* to store the mds context */
	NCS_BOOL unlock_req_sent;	/* To take care of unlock requests that are
					   GLSV_CALL_TYPE          unlock_call_type;
					   lost during mastership transistion */
	GLSV_CALL_TYPE unlock_call_type;
	uns32 non_master_status;
	uns32 shm_index;
	struct glnd_resource_info_tag *res_info;	/* back pointer */
	struct glnd_res_lock_list_info_tag *prev, *next;
} GLND_RES_LOCK_LIST_INFO;

typedef struct glnd_lock_master_info_tag {
	GLND_RES_LOCK_LIST_INFO *grant_list;
	GLND_RES_LOCK_LIST_INFO *wait_exclusive_list;
	GLND_RES_LOCK_LIST_INFO *wait_read_list;
	uns32 pr_orphan_req_count;	/* local lock ref count */
	uns32 ex_orphan_req_count;	/* local lock ref count */
	NCS_BOOL pr_orphaned;
	NCS_BOOL ex_orphaned;
} GLND_LOCK_MASTER_INFO;

typedef struct glnd_agent_info_tag {
	NCS_PATRICIA_NODE patnode;
	MDS_DEST agent_mds_id;	/* handle to the component */
	uns32 process_id;
} GLND_AGENT_INFO;

typedef struct glnd_resource_req_list_tag {
	uns32 res_req_hdl_id;	/* maintained for the validity */
	SaLckHandleT client_handle_id;
	MDS_DEST agent_mds_dest;
	SaNameT resource_name;
	SaInvocationT invocation;
	SaLckResourceIdT lcl_resource_id;
	GLSV_CALL_TYPE call_type;
	GLND_TMR timeout;
	MDS_SYNC_SND_CTXT glnd_res_mds_ctxt;
	struct glnd_resource_req_list_tag *prev, *next;
} GLND_RESOURCE_REQ_LIST;

typedef struct glnd_restart_res_info_tag {
	SaLckResourceIdT resource_id;	/* index for identifying the resource */
	SaNameT resource_name;
	uns32 lcl_ref_cnt;
	MDS_DEST master_mds_dest;	/* master vcard id */

	GLND_RESOURCE_STATUS status;	/* bit-wise data */
	GLND_NODE_STATUS master_status;
	tmr_t time_duration;
	time_t time_stamp;
	uns32 pr_orphan_req_count;	/* local lock ref count */
	uns32 ex_orphan_req_count;	/* local lock ref count */
	NCS_BOOL pr_orphaned;
	NCS_BOOL ex_orphaned;
	uns32 shm_index;	/*Index of queue info in shared memory */
	uns32 valid;		/* To verify whether the checkpoint info is valid or not */
} GLND_RESTART_RES_INFO;

typedef struct glnd_restart_res_lock_list_info_tag {
	SaLckHandleT app_handle_id;	/* index for the client tree */
	SaLckResourceIdT resource_id;	/* index for identifying the resource */
	SaLckResourceIdT lcl_resource_id;	/* index for identifying the resource */
	uint8_t to_which_list;	/* identies whether to update res_lock_list */
	/* To which list */
	uns32 lck_info_hdl_id;	/* maintained for the validity */
	GLSV_LOCK_REQ_INFO lock_info;
	tmr_t time_duration;
	time_t time_stamp;
	MDS_DEST req_mdest_id;	/* requesting node info */
	MDS_SYNC_SND_CTXT glnd_res_lock_mds_ctxt;	/* to store the mds context */
	NCS_BOOL unlock_req_sent;	/* To take care of unlock requests */
	GLSV_CALL_TYPE unlock_call_type;
	uns32 non_master_status;
	uns32 shm_index;	/*Index of queue info in shared memory */
	uns32 valid;		/* To verify whether the checkpoint info is valid or not */
} GLND_RESTART_RES_LOCK_LIST_INFO;

/* Back_up events  to be checkpointed */

typedef struct glsv_glnd_restart_backup_evt_info_tag {
	GLSV_GLND_EVT_TYPE type;
	SaLckResourceIdT resource_id;
	SaLckHandleT client_handle_id;
	MDS_DEST mds_dest;
	SaInvocationT invocation;
	SaLckLockIdT lockid;
	SaLckLockModeT lock_type;
	SaLckLockFlagsT lockFlags;
	SaLckLockStatusT lockStatus;
	SaLckLockModeT mode_held;
	SaAisErrorT error;
	SaTimeT timeout;
	SaTimeT time_stamp;
	uns32 shm_index;	/*Index of queue info in shared memory */
	uns32 valid;		/* To verify whether the checkpoint info is valid or not */
} GLSV_RESTART_BACKUP_EVT_INFO;

typedef struct glnd_shm_version_tag {
	uint16_t shm_version;	/* Added to provide support for SAF Inservice upgrade facilty */
	uint16_t dummy_version1;	/* Not in use */
	uint16_t dummy_version2;	/* Not in use */
	uint16_t dummy_version3;	/* Not in use */
} GLND_SHM_VERSION;

/*****************************************************************************
* Data Structure used to hold GLND control block
*****************************************************************************/
typedef struct glnd_cb_tag {
	/* Identification Information about the GLND */
	MDS_DEST glnd_mdest_id;
	uns32 glnd_node_id;
	uns32 cb_hdl_id;
	MDS_HDL glnd_mds_hdl;
	uns32 pool_id;
	GLND_NODE_STATUS node_state;
	GLND_TMR agent_info_get_tmr;
	SYSF_MBX glnd_mbx;	/* mailbox */
	SaCkptCheckpointHandleT resource_ckpt_handle;
	SaCkptCheckpointHandleT res_lock_ckpt_handle;
	SaCkptCheckpointHandleT backup_event_ckpt_handle;
	SaNameT comp_name;

	/* Information about the GLD */
	MDS_DEST gld_mdest_id;
	NCS_BOOL gld_card_up;

	/* GLND data */
	NCS_PATRICIA_TREE glnd_client_tree;	/* GLND_CLIENT_INFO - node */
	NCS_PATRICIA_TREE glnd_res_tree;	/* GLND_RESOURCE_INFO - node */
	NCS_PATRICIA_TREE glnd_agent_tree;	/* GLND_AGENT_INFO - node */

	GLND_RESOURCE_REQ_LIST *res_req_list;
	GLND_RES_LOCK_LIST_INFO *non_mst_orphan_list;
	struct glsv_glnd_evt *evt_bckup_q;	/* backup the events incase of mastership change */

	SaAmfHandleT amf_hdl;	/* AMF handle, obtained thru AMF init        */
	SaAmfHAStateT ha_state;	/* present AMF HA state of the component     */
	EDU_HDL glnd_edu_hdl;	/* edu handle used for encode/decode         */
	void *shm_base_addr;	/* Stores shared memory starting address, which contains shared memory version */
	GLND_RESTART_RES_INFO *glnd_res_shm_base_addr;
	GLND_RESTART_RES_LOCK_LIST_INFO *glnd_lck_shm_base_addr;
	GLSV_RESTART_BACKUP_EVT_INFO *glnd_evt_shm_base_addr;
} GLND_CB;

/* prototypes */
GLND_AGENT_INFO *glnd_agent_node_find(GLND_CB *glnd_cb, MDS_DEST agent_mds_dest);
GLND_AGENT_INFO *glnd_agent_node_add(GLND_CB *glnd_cb, MDS_DEST agent_mds_dest, uns32 process_id);
void glnd_agent_node_del(GLND_CB *glnd_cb, GLND_AGENT_INFO *agent_info);

/* CB prototypes */
GLND_CB *glnd_cb_create(uns32 pool_id);
NCS_BOOL glnd_cleanup_mbx(NCSCONTEXT arg, NCSCONTEXT msg);
uns32 glnd_cb_destroy(GLND_CB *glnd_cb);
void glnd_dump_cb(void);

struct glsv_evt_rsc_info_tag;	/* forward declaration required. */
GLND_RESOURCE_REQ_LIST *glnd_resource_req_node_add(GLND_CB *glnd_cb,
							    struct glsv_evt_rsc_info_tag *rsc_info,
							    MDS_SYNC_SND_CTXT *mds_ctxt,
							    SaLckResourceIdT lcl_resource_id);
GLND_RESOURCE_REQ_LIST *glnd_resource_req_node_find(GLND_CB *glnd_cb, SaNameT *resource_name);
void glnd_resource_req_node_del(GLND_CB *glnd_cb, uns32 res_req_hdl);

/* Amf prototypes */
uns32 glnd_amf_init(GLND_CB *glnd_cb);
void glnd_amf_de_init(GLND_CB *glnd_cb);
uns32 glnd_amf_register(GLND_CB *glnd_cb);
uns32 glnd_amf_deregister(GLND_CB *glnd_cb);

/* Queue Prototypes */
void glnd_evt_backup_queue_add(GLND_CB *glnd_cb, struct glsv_glnd_evt *glnd_evt);
uns32 glnd_evt_backup_queue_delete_lock_req(GLND_CB *glnd_cb,
						     SaLckHandleT hldId, SaLckResourceIdT resId, SaLckLockModeT type);
void glnd_evt_backup_queue_delete_unlock_req(GLND_CB *glnd_cb,
						      SaLckLockIdT lockid, SaLckHandleT hldId, SaLckResourceIdT resId);
void glnd_evt_backup_queue_destroy(GLND_CB *glnd_cb);
void glnd_re_send_evt_backup_queue(GLND_CB *glnd_cb);
uint8_t glnd_cpsv_initilize(GLND_CB *glnd_cb);
uns32 glnd_shm_create(GLND_CB *cb);
uns32 glnd_shm_destroy(GLND_CB *cb, char shm_name[]);

#endif
