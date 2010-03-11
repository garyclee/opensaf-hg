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
 
  DESCRIPTION:This is the include file for AvM->AVD interface.
..............................................................................

  Function Included in this Module:
 
******************************************************************************
*/

#ifndef __AVM_AVD_H__
#define __AVM_AVD_H__

#define NCS_SAF 1
#include "ncsgl_defs.h"
#include "ncs_svd.h"
#include "usrbuf.h"
#include "ncsft.h"
#include "ncs_ubaid.h"
#include "ncsencdec_pub.h"
#include "ncs_stack.h"
#include "ncssysf_def.h"
#include "ncsdlib.h"
#include "ncs_lib.h"
#include "ncssysfpool.h"
#include "mds_papi.h"
#include "ncspatricia.h"
#include "saAis.h"
#include "saEvt.h"
#include "saClm.h"

/* Message format versions */
#define AVSV_AVD_AVM_MSG_FMT_VER_1   1

typedef struct avm_avd_msg_t AVM_AVD_MSG_T;
typedef struct avd_avm_msg_t AVD_AVM_MSG_T;
typedef struct list_node_ptr_type AVM_LIST_NODE_T;

typedef enum {
	NCS_SERVICE_AVM_AVD_SUB_ID_LIST_NODE = 0,
	NCS_SERVICE_AVM_AVD_SUB_ID_MSG,
	NCS_SERVICE_AVD_AVM_SUB_ID_MSG
} NCS_SERVICE_AVM_AVD_SUB_ID;

#define m_MMGR_ALLOC_AVM_AVD_MSG           (AVM_AVD_MSG_T*)m_NCS_MEM_ALLOC( sizeof(AVM_AVD_MSG_T), \
                                                 NCS_MEM_REGION_PERSISTENT, \
                                                 NCS_SERVICE_ID_AVM_AVD, \
                                                 NCS_SERVICE_AVM_AVD_SUB_ID_MSG)

#define m_MMGR_FREE_AVM_AVD_MSG(p)         m_NCS_MEM_FREE(p, \
                                                 NCS_MEM_REGION_PERSISTENT, \
                                                 NCS_SERVICE_ID_AVM_AVD, \
                                                 NCS_SERVICE_AVM_AVD_SUB_ID_MSG)

#define m_MMGR_ALLOC_AVD_AVM_MSG           (AVD_AVM_MSG_T*)m_NCS_MEM_ALLOC( sizeof(AVD_AVM_MSG_T), \
                                                 NCS_MEM_REGION_PERSISTENT, \
                                                 NCS_SERVICE_ID_AVM_AVD, \
                                                 NCS_SERVICE_AVD_AVM_SUB_ID_MSG)

#define m_MMGR_FREE_AVD_AVM_MSG(p)         m_NCS_MEM_FREE(p, \
                                                 NCS_MEM_REGION_PERSISTENT, \
                                                 NCS_SERVICE_ID_AVM_AVD, \
                                                 NCS_SERVICE_AVD_AVM_SUB_ID_MSG)

#define m_MMGR_ALLOC_AVM_AVD_LIST_NODE     (AVM_LIST_NODE_T*)m_NCS_MEM_ALLOC( sizeof(AVM_LIST_NODE_T), \
                                                 NCS_MEM_REGION_PERSISTENT, \
                                                 NCS_SERVICE_ID_AVM_AVD, \
                                                 NCS_SERVICE_AVM_AVD_SUB_ID_LIST_NODE)

#define m_MMGR_FREE_AVM_AVD_LIST_NODE(p)   m_NCS_MEM_FREE(p, \
                                                 NCS_MEM_REGION_PERSISTENT, \
                                                 NCS_SERVICE_ID_AVM_AVD, \
                                                 NCS_SERVICE_AVM_AVD_SUB_ID_LIST_NODE)

/* AVM->AVD messages */
typedef enum avm_avd_msg_type {

	AVM_AVD_NODE_SHUTDOWN_REQ_MSG,
	AVM_AVD_NODE_FAILOVER_REQ_MSG,
	AVM_AVD_FAULT_DOMAIN_RESP_MSG,
	AVM_AVD_NODE_RESET_RESP_MSG,
	AVM_AVD_NODE_OPERSTATE_MSG,
	AVM_AVD_SYS_CON_ROLE_MSG,

} AVM_AVD_MSG_TYPE_T;

/* AVD->AVM messages */
typedef enum avd_avm_msg_type {

	AVD_AVM_NODE_SHUTDOWN_RESP_MSG,
	AVD_AVM_NODE_FAILOVER_RESP_MSG,
	AVD_AVM_FAULT_DOMAIN_REQ_MSG,
	AVD_AVM_NODE_RESET_REQ_MSG,
	AVD_AVM_SYS_CON_ROLE_ACK_MSG,
	AVD_AVM_D_HRT_BEAT_LOST_MSG,
	AVD_AVM_ND_HRT_BEAT_LOST_MSG,
	AVD_AVM_D_HRT_BEAT_RESTORE_MSG,
	AVD_AVM_ND_HRT_BEAT_RESTORE_MSG
} AVD_AVM_MSG_TYPE_T;

/* Operstate of the nodes */

typedef enum node_operstate_type {

	AVM_NODES_DISABLED,
	AVM_NODES_ENABLED,
	AVM_NODES_ABSENT
} AVM_NODE_OPERSTATE_T;

/* Failfast response from AVD*/
typedef enum failover_resp_type {

	AVD_NODES_SWITCHOVER_SUCCESS,
	AVD_NODES_SWITCHOVER_FAILURE
} AVD_NODE_SWITCHOVER_STATUS_T;

/* Failfast response from AVD*/
typedef enum reset_resp_type {

	AVM_NODE_RESET_SUCCESS,
	AVM_NODE_RESET_FAILURE,
	AVM_NODE_INVALID_NAME,
	AVM_NODE_INVALID_REQ
} AVM_NODE_RESET_RESP_T;

typedef enum avm_rol_chg_cause_type {
	AVM_INIT_ROLE,
	AVM_ADMIN_SWITCH_OVER,
	AVM_FAIL_OVER
} AVM_ROLE_CHG_CAUSE_T;

/* Linked list of nodes to reprsent fault domain */
struct list_node_ptr_type {
	SaNameT node_name;
	AVM_LIST_NODE_T *next;
};

#define AVM_LIST_NODE_NULL ((AVM_LIST_NODE_T *)0x0)

typedef struct list_head_type {
	uns32 count;
	AVM_LIST_NODE_T *node;
} AVM_LIST_HEAD_T;

/*Shutdown-Request, Failover-Req, Operstate msgs from AvM to AvD */

typedef struct avm_avd_node_switchover_req_type {

	AVM_LIST_HEAD_T head;
	AVM_NODE_OPERSTATE_T oper_state;

} AVM_AVD_NODE_SWITCHOVER_REQ_T, AVM_AVD_NODE_OPERSTATE_T;

typedef struct avm_avd_node_reset_resp_type {

	SaNameT node_name;
	uns32 resp;

} AVM_AVD_NODE_RESET_RESP_T;

typedef struct avm_avd_faultdomain_resp_type {
	SaNameT node_name;
	AVM_LIST_HEAD_T head;
} AVM_AVD_FAULT_DOMAIN_RESP_T;

typedef struct avm_avd_sys_con_role_type {
	SaAmfHAStateT role;
	AVM_ROLE_CHG_CAUSE_T cause;

} AVM_AVD_SYS_CON_ROLE_T;

/* Reset-Req from AvD to AvM */
typedef struct avd_avm_node_reset_req_type {
	SaNameT node_name;

} AVD_AVM_NODE_RESET_REQ_T;

/* Shutdown and Failover response from AVD */

typedef struct avd_avm_node_shutdown_response_type {
	uns32 recovery_status;
	SaNameT node_name;

} AVD_AVM_NODE_SHUTDOWN_RESP_T, AVD_AVM_NODE_FAILOVER_RESP_T;

/* Fault Domain Req for a faulty node from AVD */
typedef struct avd_avm_fault_domain_req_type {
	SaNameT node_name;
} AVD_AVM_FAULT_DOMAIN_REQ_T;

typedef struct avd_avm_sys_con_role_ack_type {
	SaAmfHAStateT role;
	uns32 rc;
} AVD_AVM_SYS_CON_ROLE_ACK_T;

/* This Structure is used for both HB  restore and
   lost message type */
typedef struct avd_avm_hrt_beat_type {
	SaNameT node_name;
} AVD_AVM_D_HRT_BEAT_T;

typedef struct avd_avm_nd_hrt_beat_type {
	uns32 node_id;
	SaNameT node_name;

} AVD_AVM_ND_HRT_BEAT_T;

/* Structure reprsenting messages from AVM to AVD */
struct avm_avd_msg_t {
	AVM_AVD_MSG_TYPE_T msg_type;
	union {
		AVM_AVD_NODE_SWITCHOVER_REQ_T shutdown_req;
		AVM_AVD_NODE_SWITCHOVER_REQ_T failover_req;
		AVM_AVD_NODE_OPERSTATE_T operstate;
		AVM_AVD_FAULT_DOMAIN_RESP_T faultdomain_resp;
		AVM_AVD_NODE_RESET_RESP_T reset_resp;
		AVM_AVD_SYS_CON_ROLE_T role;
	} avm_avd_msg;

};

#define AVM_AVD_MSG_NULL ((AVM_AVD_MSG_T *)0x0)

/* Structure reprsenting messages from AVD to AVM */

struct avd_avm_msg_t {
	AVD_AVM_MSG_TYPE_T msg_type;
	union {
		AVD_AVM_NODE_SHUTDOWN_RESP_T shutdown_resp;
		AVD_AVM_NODE_FAILOVER_RESP_T failover_resp;
		AVD_AVM_FAULT_DOMAIN_REQ_T fault_domain_req;
		AVD_AVM_NODE_RESET_REQ_T reset_req;
		AVD_AVM_SYS_CON_ROLE_ACK_T role_ack;
		AVD_AVM_D_HRT_BEAT_T avd_hb_info;	/* Structure of HB lost and restore message type */
		AVD_AVM_ND_HRT_BEAT_T avnd_hb_info;	/* Structure of HB lost and restore message type */
	} avd_avm_msg;

};

#define AVD_AVM_MSG_NULL ((AVD_AVM_MSG_T *)0x0)

/* AVM_AVD_MSG free routines*/
extern uns32 avm_avd_free_msg(AVM_AVD_MSG_T **);

#endif   /* __AVM_AVD_H__ */
