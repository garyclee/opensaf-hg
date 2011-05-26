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

#ifndef CPND_CB_H
#define CPND_CB_H

#include <saClm.h>

#include "ncs_queue.h"

/* global variables */
uns32 gl_cpnd_cb_hdl;

/* macros for the CB handle */
#define m_CPND_TAKE_CPND_CB      ncshm_take_hdl(NCS_SERVICE_ID_CPND, gl_cpnd_cb_hdl)
#define m_CPND_GIVEUP_CPND_CB    ncshm_give_hdl(gl_cpnd_cb_hdl)

#define CPND_MAX_REPLICAS 1000
#define CPSV_GEN_SECTION_ID_SIZE 4
#define CPSV_WAIT_TIME  100

#define CPSV_MIN_DATA_SIZE  10000
#define CPSV_AVG_DATA_SIZE  1000000
#define CPND_WAIT_TIME(datasize) ((datasize<CPSV_MIN_DATA_SIZE)?300:500+((datasize/CPSV_AVG_DATA_SIZE)*200))

#define m_CPND_IS_LOCAL_NODE(m,n)   memcmp(m,n,sizeof(MDS_DEST))

#define m_CPND_IS_ALL_REPLICA_ATTR_SET(attr)   \
            (((attr & SA_CKPT_WR_ALL_REPLICAS) != 0)?TRUE:FALSE)
#define m_CPND_IS_ACTIVE_REPLICA_ATTR_SET(attr)   \
            (((attr & SA_CKPT_WR_ACTIVE_REPLICA) != 0)?TRUE:FALSE)
#define m_CPND_IS_ACTIVE_REPLICA_WEAK_ATTR_SET(attr)   \
            (((attr & SA_CKPT_WR_ACTIVE_REPLICA_WEAK) != 0)?TRUE:FALSE)
#define m_CPND_IS_COLLOCATED_ATTR_SET(attr)   \
            (((attr & SA_CKPT_CHECKPOINT_COLLOCATED) != 0)?TRUE:FALSE)

#define m_CPND_IS_CHECKPOINT_READ_SET(attr)   \
            (((attr & SA_CKPT_CHECKPOINT_READ) != 0)?TRUE:FALSE)
#define m_CPND_IS_CHECKPOINT_WRITE_SET(attr)   \
            (((attr & SA_CKPT_CHECKPOINT_WRITE) != 0)?TRUE:FALSE)
#define m_CPND_IS_CHECKPOINT_CREATE_SET(attr)   \
            (((attr & SA_CKPT_CHECKPOINT_CREATE) != 0)?TRUE:FALSE)

/* attribute check */

#define m_CPND_IS_CREAT_ATTRIBUTE_EQUAL(x_attr1,x_attr2) \
       (((x_attr1.creationFlags == x_attr2.creationFlags)&& \
            (x_attr1.checkpointSize == x_attr2.checkpointSize ) && \
            (x_attr1.maxSections == x_attr2.maxSections) && \
            (x_attr1.maxSectionSize == x_attr2.maxSectionSize ) && \
            (x_attr1.maxSectionIdSize == x_attr2.maxSectionIdSize )!= 0)?TRUE:FALSE)
#define m_CPND_IS_CREAT_ATTRIBUTE_EQUAL_B_1_1(x_attr1,x_attr2) \
       (((x_attr1.creationFlags == x_attr2.creationFlags)&& \
            (x_attr1.checkpointSize == x_attr2.checkpointSize ) && \
            (x_attr1.retentionDuration == x_attr2.retentionDuration ) && \
            (x_attr1.maxSections == x_attr2.maxSections) && \
            (x_attr1.maxSectionSize == x_attr2.maxSectionSize ) && \
            (x_attr1.maxSectionIdSize == x_attr2.maxSectionIdSize )!= 0)?TRUE:FALSE)

#define m_CPND_FREE_CKPT_SECTION(p) \
        do{ \
            if (p->sec_id.id != NULL) \
               m_MMGR_FREE_CPND_DEFAULT(p->sec_id.id); \
            m_MMGR_FREE_CPND_CPND_CKPT_SECTION_INFO(p); \
        }while(0)

/*** Macro used to get the AMF version used ****/
#define m_CPSV_GET_AMF_VER(amf_ver) amf_ver.releaseCode='B'; amf_ver.majorVersion=0x01; amf_ver.minorVersion=0x01;

#define m_CPND_IS_ON_SCXB(m,n) ((m==n)?1:0)

/*30B Versioning Changes */
#define CPND_MDS_PVT_SUBPART_VERSION 3

/*CPND - CPA communication */
#define CPND_WRT_CPA_SUBPART_VER_MIN 1
#define CPND_WRT_CPA_SUBPART_VER_MAX 2

#define CPND_WRT_CPA_SUBPART_VER_RANGE \
        (CPND_WRT_CPA_SUBPART_VER_MAX - \
         CPND_WRT_CPA_SUBPART_VER_MIN + 1 )

/*CPND - CPND communication */
#define CPND_WRT_CPND_SUBPART_VER_MIN 1
#define CPND_WRT_CPND_SUBPART_VER_MAX 2

#define CPND_WRT_CPND_SUBPART_VER_RANGE \
        (CPND_WRT_CPND_SUBPART_VER_MAX - \
         CPND_WRT_CPND_SUBPART_VER_MIN + 1 )

/*CPND - CPD communication */
#define CPND_WRT_CPD_SUBPART_VER_MIN 1
#define CPND_WRT_CPD_SUBPART_VER_MAX 3

#define CPND_WRT_CPD_SUBPART_VER_RANGE \
        (CPND_WRT_CPD_SUBPART_VER_MAX - \
         CPND_WRT_CPD_SUBPART_VER_MIN + 1 )

typedef enum cpnd_slot_id {
	ACTIVE_SLOT_PC = 1,
	STANDBY_SLOT_PC,
	ACTIVE_SLOT_CH = 7,
	STANDBY_SLOT_CH
} CPND_SLOT_ID;

typedef enum cpnd_state_info {
	CPND_LOCAL_NODE = 1,
	CPND_REMOTE_NODE = 2,
	CPND_OTHER = 3
} CPND_STATE_INFO;

typedef struct cpnd_ckpt_section_info {
	uns32 lcl_sec_id;	/* index to shm segment(index == sec) */
	SaCkptSectionIdT sec_id;	/* section Id */
	CPND_TMR ckpt_sec_exptmr;	/* timer to delete the section */
	SaCkptSectionStateT sec_state;	/* state of the checkpoint section -                                               * will be used for marking deletion */
	SaSizeT sec_size;
	SaTimeT exp_tmr;
	SaTimeT lastUpdate;
	struct cpnd_ckpt_section_info *prev, *next;
} CPND_CKPT_SECTION_INFO;

#define CPND_CKPT_SECTION_INFO_NULL ((CPND_CKPT_SECTION_INFO *)0)

struct cpnd_ckpt_cllist_node;

typedef struct cpnd_ckpt_replica_info_tag {

	SaUint32T n_secs;	/* Used for status */
	SaUint32T mem_used;	/* Used for status */
	NCS_OS_POSIX_SHM_REQ_INFO open;	/* for shm open */
	uns32 *shm_sec_mapping;	/* for validity of sec */
	CPND_CKPT_SECTION_INFO *section_info;	/* Sections in the shared memory */
} CPND_CKPT_REPLICA_INFO;

/*Structure to store info for ALL_REPL_WRITE EVT processing*/
typedef struct cpnd_all_repl_write_evt_node {
	NCS_PATRICIA_NODE patnode;
	SaCkptCheckpointHandleT ckpt_id;
	uns32 write_rsp_cnt;	/*Keeps track of the responses awaited during ALL_REPL_WRITE */
	CPND_TMR write_rsp_tmr;	/*Used in ALL_REPL_WRITE to await rsp from remote nodes */
	CPSV_CPND_UPDATE_DEST *cpnd_update_dest_list;
	CPSV_SEND_INFO sinfo;
/* struct cpnd_all_repl_write_evt_node *next;*/
} CPSV_CPND_ALL_REPL_EVT_NODE;

/******************************************************************************
 The checkpoint node that goes into particia tree cpnd_ckpt_info of CPND_CB 
 *****************************************************************************/
typedef struct cpnd_ckpt_node {
	NCS_PATRICIA_NODE patnode;
	SaCkptCheckpointHandleT ckpt_id;	/* index for identifying the checkpoint */
	SaNameT ckpt_name;
	SaCkptCheckpointCreationAttributesT create_attrib;
	SaCkptCheckpointOpenFlagsT open_flags;
	uns32 ckpt_lcl_ref_cnt;
	MDS_DEST active_mds_dest;	/* Active Replica vcard id */
	uns32 is_active_exist;

	CPND_CKPT_REPLICA_INFO replica_info;	/* Replica info about the Shared memory */
	struct cpnd_ckpt_cllist_node *clist;	/* List of the clients */
	CPSV_CPND_DEST_INFO *cpnd_dest_list;	/* List maintans remote client MDEST informations */

	NCS_BOOL cpnd_rep_create;

	NCS_BOOL is_unlink;	/* Set to TRUE if it is already closed */
	NCS_BOOL is_close;	/* Set to TRUE if it is already closed */
	CPND_TMR ret_tmr;
	NCS_BOOL is_restart;
	NCS_BOOL is_ckpt_onscxb;
	uns32 cur_state;
	uns32 oth_state;
/*   uns32                               read_lck_cnt; */
	CPSV_CPND_DEST_INFO *agent_dest_list;
	SaTimeT close_time;
	NCS_BOOL is_rdset;

	int32 offset;		/* for restart mechanism - shared memory offset */
	SaNameT node_name;	

	NCS_BOOL is_cpa_created_ckpt_replica;	/* Flag to indicate whether replica created by CPA/CPD */
	CPSV_EVT *evt_bckup_q;
	CPSV_SEND_INFO cpa_sinfo;	/* Used in unlink flow while sending response to CPA */
	NCS_BOOL cpa_sinfo_flag;
	CPND_TMR open_active_sync_tmr;
} CPND_CKPT_NODE;

#define CPND_CKPT_NODE_NULL  ((CPND_CKPT_NODE *)0)
/******************************************************************************
 Used to maintain the linked list of client information in ckpt_node
 ******************************************************************************/
typedef struct cpnd_ckpt_ckpt_list_node {
	CPND_CKPT_NODE *cnode;
	struct cpnd_ckpt_ckpt_list_node *next;
} CPND_CKPT_CKPT_LIST_NODE;

/*****************************************************************************
 Client  information 
 *****************************************************************************/
typedef struct cpnd_ckpt_client_node {
	NCS_PATRICIA_NODE patnode;
	SaCkptHandleT ckpt_app_hdl;	/* index for the client tree */
	MDS_DEST agent_mds_dest;	/* mds dest of the agent */
	uns32 proc_id;		/* TBD Delete this */
	SaVersionT version;
	uns16 cbk_reg_info;	/* bit-wise data */
	NCS_BOOL arrival_cb_flag;

	CPND_CKPT_CKPT_LIST_NODE *ckpt_list;	/* List of ckpts opened by this client */
	uns32 offset;		/* shared memory offset */
	NCS_BOOL app_status;
	NCS_BOOL upd_shm;
} CPND_CKPT_CLIENT_NODE;

/******************************************************************************
 Used to maintain the linked list of client information in ckpt_node
 ******************************************************************************/
typedef struct cpnd_ckpt_cllist_node {
	CPND_CKPT_CLIENT_NODE *cnode;
	uns32 cl_ref_cnt;
	struct cpnd_ckpt_cllist_node *next;
} CPND_CKPT_CLLIST_NODE;

/******************************************************************************
 CPND deadlock prevention structure
 ******************************************************************************/
typedef struct cpnd_sync_send_node {
	NCS_QELEM qelem;
	MDS_DEST dest;
	CPSV_EVT *evt;
} CPND_SYNC_SEND_NODE;

/******************************************************************************
 CPND structure for storing timed out CPD requests
 ******************************************************************************/
typedef struct cpnd_cpd_deferred_req_node {
	NCS_QELEM qelem;
	CPSV_EVT evt;
} CPND_CPD_DEFERRED_REQ_NODE;

/*****************************************************************************
* Data Structure used to hold CPND control block
*****************************************************************************/
typedef struct cpnd_cb_tag {
	/* Identification Information about the CPND */
	MDS_DEST cpnd_mdest_id;
	uns32 cpnd_cb_hdl_id;
	uns32 cpnd_mds_hdl;
	uns32 pool_id;
	SYSF_MBX cpnd_mbx;	/* mailbox */
	SaNameT comp_name;

	uns32 cli_id_gen;	/* for generating client_id */
	GBL_SHM_PTR shm_addr;
	/* Information about the CPD */
	MDS_DEST cpd_mdest_id;
	NCS_BOOL is_cpd_up;
	NCS_BOOL is_joined_cl;
	uns32 num_rep;		/* Number of shared memory segments */

	uns32 gl_cpnd_shm_id;	/* the global Checkpoint Shared
				   Memory need to store the local
				   Checkpoint Control data. This 
				   Shall be opened using a standard
				   key_t */
	/* CPND data */
	NCS_PATRICIA_TREE client_info_db;	/* CPND_CKPT_CLIENT_NODE - node */
	NCS_PATRICIA_TREE ckpt_info_db;	/* CPND_CKPT_NODE - node */
	NCS_PATRICIA_TREE writeevt_db;	/*All replica write evt node */
	SaClmHandleT clm_hdl;
	SaSelectionObjectT clm_sel_obj;
	SaClmNodeIdT nodeid;
	SaAmfHandleT amf_hdl;	/* AMF handle, obtained thru AMF init        */
	uint8_t *cpnd_res_shm_name;
	NCS_BOOL cpnd_first_time;
	NCS_BOOL read_lck_flag;

	uns32 cpnd_active_id;
	uns32 cpnd_standby_id;

	NCS_NODE_ID node_id;
	uns32 cpnd_self_id;
	uns32 cpnd_remote_id;

	SaAmfHAStateT ha_state;	/* present AMF HA state of the component     */
	EDU_HDL cpnd_edu_hdl;	/* edu handle used for encode/decode         */
	NCSCONTEXT task_hdl;

	NCS_LOCK cpnd_sync_send_lock;	/* Lock to protect access to the CPND deadlock prevention structures */
	NCS_QUEUE cpnd_sync_send_list;	/* List of sync sends coming in from other CONDs */
	NCS_BOOL cpnd_sync_send_in_progress;	/* Flag indicates whether sync send to another COND is in progress */
	MDS_DEST target_cpnd_dest;	/* MDS_DEST of the target CPND to which the sync send is in progress */

	NCS_LOCK cpnd_cpd_up_lock;

	NCS_QUEUE cpnd_cpd_deferred_reqs_list;	/* Queue for storing CPD timeout requests  */

} CPND_CB;

/* CB prototypes */
CPND_CB *cpnd_cb_create(uns32 pool_id);
NCS_BOOL cpnd_cleanup_mbx(NCSCONTEXT arg, NCSCONTEXT msg);
uns32 cpnd_cb_destroy(CPND_CB *cpnd_cb);
void cpnd_dump_cb(CPND_CB *cpnd_cb);

/* Amf prototypes */
uns32 cpnd_amf_init(CPND_CB *cpnd_cb);
void cpnd_amf_de_init(CPND_CB *cpnd_cb);
uns32 cpnd_amf_register(CPND_CB *cpnd_cb);
uns32 cpnd_amf_deregister(CPND_CB *cpnd_cb);
uns32 cpnd_client_extract_bits(uns32 bitmap_value, uns32 *bit_position);
uns32 cpnd_res_ckpt_sec_del(CPND_CKPT_NODE *cp_node);
uns32 cpnd_ckpt_replica_create_res(NCS_OS_POSIX_SHM_REQ_INFO *open_req, char *buf, CPND_CKPT_NODE **cp_node,
					    uns32 ref_cnt, CKPT_INFO *cp_info);
int32 cpnd_find_free_loc(CPND_CB *cb, CPND_TYPE_INFO type);
uns32 cpnd_ckpt_write_header(CPND_CB *cb, uns32 nckpts);
uns32 cpnd_cli_info_write_header(CPND_CB *cb, int32 n_clients);
uns32 cpnd_write_client_info(CPND_CB *cb, CPND_CKPT_CLIENT_NODE *cl_node, int32 offset);
uns32 cpnd_client_bitmap_set(SaCkptHandleT client_hdl);
uns32 cpnd_update_ckpt_with_clienthdl(CPND_CB *cb, CPND_CKPT_NODE *cp_node, SaCkptHandleT client_hdl);
uns32 cpnd_write_ckpt_info(CPND_CB *cb, CPND_CKPT_NODE *cp_node, int32 offset, SaCkptHandleT client_hdl);
int32 cpnd_restart_shm_client_update(CPND_CB *cb, CPND_CKPT_CLIENT_NODE *cl_node);
uns32 client_bitmap_reset(uns32 *bitmap_value, uns32 client_hdl);
uns32 client_bitmap_isset(uns32 bitmap_value);
int32 cpnd_find_ckpt_exists(CPND_CB *cb, CPND_CKPT_NODE *cp_node);

NCS_BOOL cpnd_match_evt(void *key, void *qelem);
NCS_BOOL cpnd_match_dest(void *key, void *qelem);
void cpnd_restart_reset_close_flag(CPND_CB *cb, CPND_CKPT_NODE *cp_node);
void cpnd_clm_cluster_track_cb(const SaClmClusterNotificationBufferT *notificationBuffer,
					SaUint32T numberOfMembers, SaAisErrorT error);
#define m_CPSV_CONVERT_EXPTIME_TEN_MILLI_SEC(t) \
     SaTimeT now; \
     t = (( (t) - (m_GET_TIME_STAMP(now)*(1000000000)))/(10000000));

#endif
