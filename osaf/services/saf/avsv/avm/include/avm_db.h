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
  
  ..............................................................................
 
  DESCRIPTION:This file defines structures for AVM_ENT_INFO_T.
  ..............................................................................

 
******************************************************************************
*/

#ifndef __AVM_DB_H__
#define __AVM_DB_H__

/* Linked List representation of Fault Domain */
typedef struct fault_domain_type {
	SaHpiEntityPathT ent_path;
	struct fault_domain_type *next;

} AVM_FAULT_DOMAIN_T;

#define AVM_FAULT_DOMAIN_NULL ((AVM_FAULT_DOMAIN_T *)0x0)

/* Linked list of Entity Info nodes*/
typedef struct avm_ent_info_list_node {
	AVM_ENT_PATH_STR_T *ep_str;
	AVM_ENT_INFO_T *ent_info;
	struct avm_ent_info_list_node *next;

} AVM_ENT_INFO_LIST_T;

#define AVM_ENT_INFO_LIST_NULL ((AVM_ENT_INFO_LIST_T *)0x0)

typedef struct avm_validation_data_type AVM_VALID_INFO_T;

/*Linked List of Node Info nodes */

typedef struct avm_node_info_list {
	AVM_NODE_INFO_T *node_info;
	struct avm_ent_info_list_node *next;
} AVM_NODE_INFO_LIST_T;

#define AVM_NODE_INFO_LIST_NULL ((AVM_NODE_INFO_LIST_T *)0x0)

typedef struct avm_ent_dec_name_type {
	uns8 name[NCS_MAX_INDEX_LEN];
	uns32 length;
} AVM_ENT_DESC_NAME_T;

typedef struct avm_valid_range_type {
	uns32 min;
	uns32 max;
} AVM_VALID_LOC_RANGE_T;

typedef struct avm_location_type {

	AVM_ENT_DESC_NAME_T parent;
	AVM_VALID_LOC_RANGE_T range[AVM_MAX_LOCATION_RANGE];
	uns32 count;
} AVM_ENT_VALID_LOCATION_T;

typedef struct avm_inv_data_type {
	SaHpiTextBufferT product_name;
	SaHpiTextBufferT product_version;
} AVM_INV_DATA_T;

struct avm_validation_data_type {
	NCS_PATRICIA_NODE tree_node;

	AVM_ENT_DESC_NAME_T desc_name;

	uns32 entity_type;
/* Validation Data used by AvM to validate the inserted FRU */
	AVM_INV_DATA_T inv_data;

/*Flag to indicate whether the entity is node or not */
	uns8 is_node;
	uns8 is_fru;
	AVM_ENT_VALID_LOCATION_T location[MAX_POSSIBLE_PARENTS];
	uns32 parent_cnt;

	AVM_VALID_INFO_T *parents[MAX_POSSIBLE_PARENTS];
};

#define AVM_VALID_INFO_NULL ((AVM_VALID_INFO_T*)0x0)

typedef struct avm_sensor_event_type {
	SaHpiSensorNumT SensorNum;
	SaHpiSensorTypeT SensorType;
	SaHpiEventCategoryT EventCategory;
	uns32 sensor_assert;
} AVM_SENSOR_EVENT_T;

/* Struct representing Entities in System */

struct avm_ent {
	NCS_PATRICIA_NODE tree_node;
	NCS_PATRICIA_NODE tree_node_str;

	SaHpiEntityPathT entity_path;	/* Entity Path */
	SaHpiEntityTypeT entity_type;	/* Entity Type */

	AVM_ENT_PATH_STR_T ep_str;

/* Fault Domain of the Node */
	AVM_FAULT_DOMAIN_T *fault_domain;

	NCS_BOOL is_node;
/* Node Information */
	SaNameT node_name;

	uns32 act_policy;

	NCS_BOOL depends_on_valid;
	AVM_ENT_PATH_STR_T dep_ep_str;
	SaHpiEntityPathT depends_on;	/* Entity Path on which an entity is dependent
					   to become active. Typically for pay 
					   load blades, SCXB will be the depends_on entity */

	AVM_ENT_INFO_LIST_T *dependents;

	AVM_FSM_STATES_T current_state;
	AVM_FSM_STATES_T previous_state;
	AVM_FSM_STATES_T previous_diff_state;

/* parent and Children node pointers to store the containment hierarchy */
	struct avm_ent *parent;
	AVM_ENT_INFO_LIST_T *child;

	uns32 sensor_assert;
	uns32 sensor_count;
	AVM_SENSOR_EVENT_T sensors[AVM_MAX_SENSOR_COUNT];

	AVM_ENT_DHCP_CONF dhcp_serv_conf;
	uns32 boot_prg_status;	/* Boot progress status information */
	struct avm_tmr upgd_succ_tmr;
	struct avm_tmr boot_succ_tmr;

	uns32 ent_hdl;

	AVM_ENT_DESC_NAME_T desc_name;
	AVM_VALID_INFO_T *valid_info;
	AVM_ENT_DESC_NAME_T parent_desc_name;
	AVM_VALID_INFO_T *parent_valid_info;

	uns32 adm_req;
	uns32 adm_shutdown;
	uns32 adm_lock;
	NCS_BOOL is_fru;
	NCS_BOOL power_cycle;
	uns32 row_status;
	/* vivek_avm */
	uns32 cb_hdl;
	struct avm_tmr bios_upgrade_tmr;
	struct avm_tmr ipmc_tmr;
	struct avm_tmr ipmc_mod_tmr;
	struct avm_tmr role_chg_wait_tmr;
	struct avm_tmr bios_failover_tmr;
};

#define AVM_ENT_INFO_NULL ((AVM_ENT_INFO_T*)0x0)

struct avm_node {
	NCS_PATRICIA_NODE tree_node_name;
	NCS_PATRICIA_NODE tree_node_id;

	SaClmNodeIdT node_id;
	SaNameT node_name;

	AVM_ENT_INFO_T *ent_info;

};

#define AVM_NODE_INFO_NULL ((AVM_NODE_INFO_T*)0x0)

/* AvM data received from BOM, It contains anchors for the three patricia trees we are using.*/

typedef struct avm_db {
	NCS_PATRICIA_TREE valid_info_anchor;
	NCS_PATRICIA_TREE ent_info_anchor;
	NCS_PATRICIA_TREE ent_info_str_anchor;
	NCS_PATRICIA_TREE node_name_anchor;
} AVM_DB_T;

#define m_AVM_ENT_IS_VALID(ent_info, o_rc)\
{\
 SaHpiEntityPathT entity_path; \
 o_rc = 0; \
 memset(&entity_path.Entry, 0, sizeof(SaHpiEntityPathT));\
 rc = memcmp((ent_info)->entity_path.Entry, entity_path.Entry, sizeof(SaHpiEntityPathT));\
}

#define AVM_CHASSIS_TYPE_1405 "CHS1405"
#define AVM_CHASSIS_TYPE_1406 "CHS1406"

extern AVM_ENT_INFO_T *avm_add_ent_info(AVM_CB_T *avm_cb, SaHpiEntityPathT *entity_path);

extern AVM_ENT_INFO_T *avm_find_ent_info(AVM_CB_T *avm_cb, SaHpiEntityPathT *entity_path);

extern AVM_ENT_INFO_T *avm_find_first_ent_info(AVM_CB_T *avm_cb);

extern AVM_ENT_INFO_T *avm_find_next_ent_info(AVM_CB_T *avm_cb, SaHpiEntityPathT *entity_path);

extern uns32 avm_delete_ent_info(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info);

extern AVM_NODE_INFO_T *avm_add_node_name_info(AVM_CB_T *avm_cb, SaNameT node_name);

extern AVM_NODE_INFO_T *avm_find_node_name_info(AVM_CB_T *avm_cb, SaNameT node_name);

extern AVM_NODE_INFO_T *avm_find_next_node_name_info(AVM_CB_T *avm_cb, SaNameT node_name);

extern uns32 avm_delete_node_name_info(AVM_CB_T *avm_cb, AVM_NODE_INFO_T *node_info);

extern uns32 avm_populate_ent_info(AVM_CB_T *cb, AVM_ENT_INFO_T *ent_info);

extern uns32 avm_populate_node_info(AVM_CB_T *cb, SaHpiEntityPathT *entity_path, AVM_NODE_INFO_T *node_info);

extern AVM_VALID_INFO_T *avm_add_valid_info(AVM_CB_T *avm_cb, AVM_ENT_DESC_NAME_T *desc_name);

extern AVM_VALID_INFO_T *avm_find_valid_info(AVM_CB_T *avm_cb, AVM_ENT_DESC_NAME_T *desc_name);

extern AVM_VALID_INFO_T *avm_find_next_valid_info(AVM_CB_T *avm_cb, AVM_ENT_DESC_NAME_T *desc_name);
extern uns32 avm_delete_valid_info(AVM_CB_T *avm_cb, AVM_VALID_INFO_T *valid_info);

extern uns32 avm_add_ent_str_info(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info, AVM_ENT_PATH_STR_T *ep);
extern AVM_ENT_INFO_T *avm_find_ent_str_info(AVM_CB_T *avm_cb, AVM_ENT_PATH_STR_T *ep, NCS_BOOL is_host_order);

extern AVM_ENT_INFO_T *avm_find_next_ent_str_info(AVM_CB_T *avm_cb, AVM_ENT_PATH_STR_T *ep);

extern uns32 avm_delete_ent_str_info(AVM_CB_T *avm_cb, AVM_ENT_INFO_T *ent_info);
extern uns32 find_parent_entity_path(SaHpiEntityPathT *entity_path, SaHpiEntityPathT *parent_entity_path);

extern uns32 avm_insert_fault_domain(AVM_ENT_INFO_T *ent_info, SaHpiEntityPathT *entity_path);

EXTERN_C uns32 ncsavmentdeploytableentry_tbl_reg(void);

EXTERN_C uns32 ncsavmentdeploytableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);

EXTERN_C uns32
ncsavmentdeploytableentry_extract(NCSMIB_PARAM_VAL *param,
				  NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
EXTERN_C uns32
ncsavmentdeploytableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
EXTERN_C uns32
ncsavmentdeploytableentry_next(NCSCONTEXT cb,
			       NCSMIB_ARG *arg, NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);
EXTERN_C uns32
ncsavmentdeploytableentry_setrow(NCSCONTEXT cb,
				 NCSMIB_ARG *args,
				 NCSMIB_SETROW_PARAM_VAL *params,
				 struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);
EXTERN_C uns32 ncsavmentfaultdomaintableentry_tbl_reg(void);

EXTERN_C uns32 ncsavmentfaultdomaintableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);

EXTERN_C uns32
ncsavmentfaultdomaintableentry_extract(NCSMIB_PARAM_VAL *param,
				       NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
EXTERN_C uns32
ncsavmentfaultdomaintableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
EXTERN_C uns32
ncsavmentfaultdomaintableentry_next(NCSCONTEXT cb,
				    NCSMIB_ARG *arg, NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);
EXTERN_C uns32
ncsavmentfaultdomaintableentry_setrow(NCSCONTEXT cb,
				      NCSMIB_ARG *args,
				      NCSMIB_SETROW_PARAM_VAL *params,
				      struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);

/* To register the 'ncsavmscalars' table information with MibLib */
EXTERN_C uns32 ncsavmscalars_tbl_reg(void);

EXTERN_C uns32 ncsavmscalars_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);

EXTERN_C uns32 ncsavmscalars_extract(NCSMIB_PARAM_VAL *param,
				     NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
EXTERN_C uns32 ncsavmscalars_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
EXTERN_C uns32 ncsavmscalars_next(NCSCONTEXT cb,
				  NCSMIB_ARG *arg, NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);

EXTERN_C uns32 ncsavmscalars_setrow(NCSCONTEXT cb,
				    NCSMIB_ARG *args,
				    NCSMIB_SETROW_PARAM_VAL *params,
				    struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);

/*EXTERN_C uns32  ncsavmsupporttableentry_tbl_reg(void);*/

EXTERN_C uns32 ncsavmsupporttableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);

EXTERN_C uns32
ncsavmsupporttableentry_extract(NCSMIB_PARAM_VAL *param, NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
EXTERN_C uns32
ncsavmsupporttableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
EXTERN_C uns32
ncsavmsupporttableentry_next(NCSCONTEXT cb,
			     NCSMIB_ARG *arg, NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);
EXTERN_C uns32
ncsavmsupporttableentry_setrow(NCSCONTEXT cb,
			       NCSMIB_ARG *args,
			       NCSMIB_SETROW_PARAM_VAL *params,
			       struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);

EXTERN_C uns32 ncsavmentupgradetableentry_tbl_reg(void);

EXTERN_C uns32 ncsavmentupgradetableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data);

EXTERN_C uns32
ncsavmentupgradetableentry_extract(NCSMIB_PARAM_VAL *param,
				   NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer);
EXTERN_C uns32
ncsavmentupgradetableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag);
EXTERN_C uns32
ncsavmentupgradetableentry_next(NCSCONTEXT cb,
				NCSMIB_ARG *arg, NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len);
EXTERN_C uns32
ncsavmentupgradetableentry_setrow(NCSCONTEXT cb,
				  NCSMIB_ARG *args,
				  NCSMIB_SETROW_PARAM_VAL *params,
				  struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag);

EXTERN_C uns32 ncsavmentupgradetableentry_verify_instance(NCSMIB_ARG *args);

#endif   /*__AVM_DB_H__*/
