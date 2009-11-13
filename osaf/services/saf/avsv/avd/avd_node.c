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
 *            Ericsson AB
 *
 */

/*****************************************************************************

  DESCRIPTION: This module deals with the creation, accessing and deletion of
  the AVND database on the AVD.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"
#include <saImmOm.h>
#include <immutil.h>
#include "avd_cluster.h"
#include "avd_imm.h"

static SaAisErrorT avd_nodeswbdl_config_get(AVD_AVND *node);

static NCS_PATRICIA_TREE avd_node_name_db;	/* SaNameT index */
static NCS_PATRICIA_TREE avd_node_id_db;	/* SaClmNodeIdT index */
static NCS_PATRICIA_TREE avd_nodegroup_db;
static NCS_PATRICIA_TREE avd_nodeswbundle_db;

/*****************************************************************************
 * Function: avd_node_add_nodeid
 *
 * Purpose:  This function will add a AVD_AVND structure to the
 * tree with node_id value as key.
 *
 * Input: avnd - The avnd structure that needs to be added.
 *
 * Returns: The pointer to AVD_AVND structure found in the tree with node id.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 avd_node_add_nodeid(AVD_AVND *avnd)
{
	if (avnd && avnd->node_info.nodeId) {
		avnd->tree_node_id_node.key_info = (uns8 *)&(avnd->node_info.nodeId);
		avnd->tree_node_id_node.bit = 0;
		avnd->tree_node_id_node.left = NCS_PATRICIA_NODE_NULL;
		avnd->tree_node_id_node.right = NCS_PATRICIA_NODE_NULL;

		m_AVD_CB_AVND_TBL_LOCK(avd_cb, NCS_LOCK_WRITE);

		if (ncs_patricia_tree_add(&avd_node_id_db, &avnd->tree_node_id_node)
		    != NCSCC_RC_SUCCESS) {
			/* log an error */
			return NCSCC_RC_FAILURE;
		}

		m_AVD_CB_AVND_TBL_UNLOCK(avd_cb, NCS_LOCK_WRITE);
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_node_delete_nodeid
 *
 * Purpose:  This function will delete AVD_AVND structure from 
 * the tree with node id as key.
 *
 * Input: avnd - The avnd structure that needs to be deleted.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 * 
 **************************************************************************/
void avd_node_delete_nodeid(AVD_AVND *node)
{
	(void) ncs_patricia_tree_del(&avd_node_id_db, &node->tree_node_id_node);
}

/*****************************************************************************
 * Function: avd_node_find
 *
 * Purpose:  This function will find a AVD_AVND structure in the
 * tree with node_name value as key.
 *
 * Input: dn - The node name on which the AVND is running.
 *
 * Returns: The pointer to AVD_AVND structure found in the tree.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

AVD_AVND *avd_node_find(const SaNameT *dn)
{
	AVD_AVND *avnd;
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	avnd = (AVD_AVND *)ncs_patricia_tree_get(&avd_node_name_db, (uns8 *)&tmp);

	if (avnd != AVD_AVND_NULL) {
		/* Adjust the pointer
		 */
		avnd = (AVD_AVND *)(((char *)avnd)
				    - (((char *)&(AVD_AVND_NULL->tree_node_name_node))
				       - ((char *)AVD_AVND_NULL)));
	}

	return avnd;
}

/*****************************************************************************
 * Function: avd_node_find_nodeid
 *
 * Purpose:  This function will find a AVD_AVND structure in the
 * tree with node_id value as key.
 *
 * Input: node_id - The node id on which the AVND is running.
 *
 * Returns: The pointer to AVD_AVND structure found in the tree.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

AVD_AVND *avd_node_find_nodeid(SaClmNodeIdT node_id)
{
	AVD_AVND *avnd;

	avnd = (AVD_AVND *)ncs_patricia_tree_get(&avd_node_id_db, (uns8 *)&node_id);

	return avnd;
}

/*****************************************************************************
 * Function: avd_node_getnext
 *
 * Purpose:  This function will find the next AVD_AVND structure in the
 * tree whose node_name value is next of the given node_name value. The function 
 * internaly converts the length to network order.
 *
 * Input: dn - The node name on which the AVND is running.
 *
 * Returns: The pointer to AVD_AVND structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_AVND *avd_node_getnext(const SaNameT *dn)
{
	AVD_AVND *avnd;
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	avnd = (AVD_AVND *)ncs_patricia_tree_getnext(&avd_node_name_db, (uns8 *)&tmp);

	if (avnd != AVD_AVND_NULL) {
		/* Adjust the pointer
		 */
		avnd = (AVD_AVND *)(((char *)avnd)
				    - (((char *)&(AVD_AVND_NULL->tree_node_name_node))
				       - ((char *)AVD_AVND_NULL)));
	}

	return avnd;
}

/*****************************************************************************
 * Function: avd_node_getnext_nodeid
 *
 * Purpose:  This function will find the next AVD_AVND structure in the
 * tree whose node_id value is next of the given node_id value.
 *
 * Input: node_id - The node id on which the AVND is running.
 *
 * Returns: The pointer to AVD_AVND structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_AVND *avd_node_getnext_nodeid(SaClmNodeIdT node_id)
{
	AVD_AVND *avnd;

	avnd = (AVD_AVND *)ncs_patricia_tree_getnext(&avd_node_id_db, (uns8 *)&node_id);

	return avnd;
}

/*****************************************************************************
 * Function: avd_node_delete
 *
 * Purpose:  This function will delete and free AVD_AVND structure from 
 * the tree with node name as key.
 *
 * Input: avnd - The avnd structure that needs to be deleted.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES: Make sure that the node is removed from the node_id tree of the CB.
 * avd_node_delete_nodeid() should be called prior to calling this func.
 *
 * 
 **************************************************************************/

void avd_node_delete(AVD_AVND *node)
{
	assert(node->pg_csi_list.n_nodes == 0);
	avd_node_delete_nodeid(node);
	ncs_patricia_tree_del(&avd_node_name_db, &node->tree_node_name_node);
	free(node);
}

/*****************************************************************************
 * Function: avd_avnd_del_cluster_list
 *
 * Purpose:  This function will del the given Node from the Cluster list.
 *
 * Input: node - The node pointer
 *
 * Returns: None.
 *
 * NOTES: None
 *
 *
 **************************************************************************/
void avd_avnd_del_cluster_list(AVD_AVND *node)
{
	AVD_AVND *i_node = NULL;
	AVD_AVND *prev_node = NULL;

	if (node->node_on_cluster != NULL) {
		/* remove Node from Cluster */
		i_node = node->node_on_cluster->list_of_avnd_node;

		while ((i_node != NULL) && (i_node != node)) {
			prev_node = i_node;
			i_node = i_node->cluster_list_node_next;
		}

		if (i_node != node) {
			/* Log a fatal error */
		} else {
			if (prev_node == NULL) {
				node->node_on_cluster->list_of_avnd_node = node->cluster_list_node_next;
			} else {
				prev_node->cluster_list_node_next = node->cluster_list_node_next;
			}
		}

		node->cluster_list_node_next = NULL;
		node->node_on_cluster = NULL;
	}

	return;
}

/**
 * Validate configuration attributes for an AMF node object
 * @param node
 * @param opdata
 * 
 * @return int
 */
static int avd_node_config_validate(const AVD_AVND *node, const CcbUtilOperationData_t *opdata)
{
	const char *name = (char *)node->node_info.nodeName.value;

	if (node->saAmfNodeAutoRepair > SA_TRUE) {
		avd_log(NCSFL_SEV_ERROR, "Invalid saAmfNodeAutoRepair %u for '%s'", node->saAmfNodeAutoRepair, name);
		return -1;
	}

	if (node->saAmfNodeFailfastOnInstantiationFailure > SA_TRUE) {
		avd_log(NCSFL_SEV_ERROR, "Invalid saAmfNodeFailfastOnInstantiationFailure %u for '%s'",
			node->saAmfNodeFailfastOnInstantiationFailure, name);
		return -1;
	}

	if (node->saAmfNodeFailfastOnTerminationFailure > SA_TRUE) {
		avd_log(NCSFL_SEV_ERROR, "Invalid saAmfNodeFailfastOnTerminationFailure %u for '%s'",
			node->saAmfNodeFailfastOnTerminationFailure, name);
		return -1;
	}

	if (!avd_admin_state_is_valid(node->saAmfNodeAdminState)) {
		avd_log(NCSFL_SEV_ERROR, "Invalid admin state %u for '%s'", node->saAmfNodeAdminState, name);
		return -1;
	}

	return 0;
}

/**
 * Create a new Node object
 * @param dn
 * @param attributes
 * 
 * @return AVD_AVND*
 */
AVD_AVND *avd_node_create(SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	int rc = -1;
	AVD_AVND *node;

	/*
	** If called from cold sync, attributes==NULL.
	** If called at new active at failover, the object is found in the DB
	** but needs to get configuration attributes initialized.
	*/
	if (NULL == (node = avd_node_find(dn))) {
		if ((node = calloc(1, sizeof(AVD_AVND))) == NULL) {
			avd_log(NCSFL_SEV_ERROR, "calloc FAILED");
			return NULL;
		}

		memcpy(node->node_info.nodeName.value, dn->value, dn->length);
		node->node_info.nodeName.length = dn->length;
		node->tree_node_name_node.key_info = (uns8 *)&(node->node_info.nodeName);
		node->pg_csi_list.order = NCS_DBLIST_ANY_ORDER;
		node->pg_csi_list.cmp_cookie = avsv_dblist_uns32_cmp;
		node->saAmfNodeAdminState = SA_AMF_ADMIN_UNLOCKED;
		node->saAmfNodeOperState = SA_AMF_OPERATIONAL_DISABLED;
		node->node_state = AVD_AVND_STATE_ABSENT;
		node->node_info.member = SA_FALSE;
		node->type = AVSV_AVND_CARD_PAYLOAD;
		node->avm_oper_state = SA_AMF_OPERATIONAL_ENABLED;
	}

	/* If no attributes supplied, go direct and add to DB */
	if (NULL == attributes)
		goto add_to_db;

	/*  TODO this is a workaround! */
	if (immutil_getAttr("saAmfNodeClmNode", attributes, 0, &node->saAmfNodeClmNode) == SA_AIS_OK) {
		SaImmAccessorHandleT accessorHandle;
		SaImmAttrValuesT_2 **clm_node_attrs;

		(void)immutil_saImmOmAccessorInitialize(avd_cb->immOmHandle, &accessorHandle);

		if (immutil_saImmOmAccessorGet_2(accessorHandle, &node->saAmfNodeClmNode,
						 NULL, &clm_node_attrs) != SA_AIS_OK) {
			avd_log(NCSFL_SEV_ERROR, "Get saImmOmAccessorGet_2 FAILED for '%s'",
				node->saAmfNodeClmNode.value);
			goto done;
		}

		if (immutil_getAttr("saClmNodeID", (const SaImmAttrValuesT_2 **)clm_node_attrs,
				    0, &node->node_info.nodeId) != SA_AIS_OK) {
			avd_log(NCSFL_SEV_ERROR, "Get saClmNodeID FAILED for '%s'", node->saAmfNodeClmNode.value);
			goto done;
		}

		(void)immutil_saImmOmAccessorFinalize(accessorHandle);
	} else {
		avd_log(NCSFL_SEV_ERROR, "saAmfNodeClmNode not configured for '%s'", node->saAmfNodeClmNode.value);
		goto done;
	}

	if (immutil_getAttr("saAmfNodeSuFailOverProb", attributes, 0, &node->saAmfNodeSuFailOverProb) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfNodeSuFailOverProb FAILED for '%s'", dn->value);
		goto done;
	}

	if (immutil_getAttr("saAmfNodeSuFailoverMax", attributes, 0, &node->saAmfNodeSuFailoverMax) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfNodeSuFailoverMax FAILED for '%s'", dn->value);
		goto done;
	}

	if (immutil_getAttr("saAmfNodeAutoRepair", attributes, 0, &node->saAmfNodeAutoRepair) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfNodeAutoRepair FAILED for '%s'", dn->value);
		goto done;
	}

	if (immutil_getAttr("saAmfNodeFailfastOnTerminationFailure", attributes, 0,
			    &node->saAmfNodeFailfastOnTerminationFailure) != SA_AIS_OK) {
		node->saAmfNodeFailfastOnTerminationFailure = SA_FALSE;
	}

	if (immutil_getAttr("saAmfNodeFailfastOnInstantiationFailure", attributes, 0,
			    &node->saAmfNodeFailfastOnInstantiationFailure) != SA_AIS_OK) {
		node->saAmfNodeFailfastOnInstantiationFailure = SA_FALSE;
	}

	if (immutil_getAttr("saAmfNodeAdminState", attributes, 0, &node->saAmfNodeAdminState) != SA_AIS_OK) {
		/* Empty, assign default value */
		node->saAmfNodeAdminState = SA_AMF_ADMIN_UNLOCKED;
	}

add_to_db:
	(void) ncs_patricia_tree_add(&avd_node_name_db, &node->tree_node_name_node);
	if (NULL == avd_node_find_nodeid(node->node_info.nodeId))
		avd_node_add_nodeid(node);
	rc = 0;

done:
	if (rc != 0) {
		free(node);
		node = NULL;
	}

	return node;
}

/**
 * Get configuration for all AMF node objects from IMM and
 * create AVD internal objects.
 * 
 * @return int
 */
SaAisErrorT avd_node_config_get(void)
{
	SaAisErrorT error, rc = SA_AIS_ERR_FAILED_OPERATION;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfNode";
	AVD_AVND *node;

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, NULL, SA_IMM_SUBTREE,
		SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
		NULL, &searchHandle);

	if (SA_AIS_OK != error) {
		avd_log(NCSFL_SEV_ERROR, "No objects found");
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, &attributes) == SA_AIS_OK) {
		avd_log(NCSFL_SEV_NOTICE, "'%s'", dn.value);

		if ((node = avd_node_create(&dn, (const SaImmAttrValuesT_2 **)attributes)) == NULL)
			goto done2;

		if (avd_node_config_validate(node, NULL) != 0) {
			avd_node_delete(node);
			avd_log(NCSFL_SEV_ERROR, "AMF node '%s' validation error", dn.value);
			goto done2;
		}

		avd_nodeswbdl_config_get(node);
	}

	rc = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:

	return rc;
}

static const char *node_state_name[] = {
	"ABSENT",
	"NO_CONFIG",
	"PRESENT",
	"GO_DOWN",
	"SHUTTING_DOWN",
	"NCS_INIT"
};

void avd_node_state_set(AVD_AVND *node, AVD_AVND_STATE node_state)
{
	assert(node != NULL);
	assert(node_state <= AVD_AVND_STATE_NCS_INIT);
	avd_log(NCSFL_SEV_NOTICE, "'%s' %s => %s",
		node->node_info.nodeName.value, node_state_name[node->node_state], node_state_name[node_state]);
	node->node_state = node_state;
	/*  TODO why isn't this followed by: */
/*     m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVND_NODE_STATE); */
}

static const char *oper_state_name[] = {
	"INVALID",
	"ENABLED",
	"DISABLED"
};

/**
 * Set AMF operational state of a node. Log & update IMM, checkpoint with standby
 * @param node
 * @param oper_state
 */
void avd_node_oper_state_set(AVD_AVND *node, SaAmfOperationalStateT oper_state)
{
	assert(node != NULL);
	assert(oper_state <= SA_AMF_OPERATIONAL_DISABLED);
	avd_log(NCSFL_SEV_NOTICE, "'%s' %s => %s",
		node->node_info.nodeName.value, oper_state_name[node->saAmfNodeOperState], oper_state_name[oper_state]);
	node->saAmfNodeOperState = oper_state;
	avd_saImmOiRtObjectUpdate(&node->node_info.nodeName, "saAmfNodeOperState",
		 SA_IMM_ATTR_SAUINT32T, &node->saAmfNodeOperState);
	/*  TODO is this always correct or do we need a new param? */
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, node, AVSV_CKPT_AVND_OPER_STATE);
}

/*****************************************************************************
 * Function: avd_node_ccb_completed_create_hdlr
 *
 * Purpose: This routine handles create operations on SaAmfNode objects.
 *
 *
 * Input  : Ccb Util Oper Data.
 *
 * Returns: None.
 *
 * NOTES  : None.
 *
 *
 **************************************************************************/
static SaAisErrorT avd_node_ccb_completed_create_hdlr(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_OK;
	AVD_AVND *node;

	avd_log(NCSFL_SEV_NOTICE, "'%s'", opdata->objectName.value);

	if ((node = avd_node_create(&opdata->objectName, opdata->param.create.attrValues)) == NULL) {
		rc = SA_AIS_ERR_BAD_OPERATION;
		goto done;
	}

	if (avd_node_config_validate(node, opdata) != 0) {
		avd_log(NCSFL_SEV_ERROR, "AMF node '%s' validation error", node->node_info.nodeName.value);
		rc = SA_AIS_ERR_BAD_OPERATION;
		avd_node_delete(node);
		free(node);
		goto done;
	}

	/* Save for later use in apply */
	opdata->userData = node;

 done:
	return rc;
}

/*****************************************************************************
 * Function: avd_node_ccb_completed_delete_hdlr
 *
 * Purpose: This routine handles delete operations on SaAmfCluster objects.
 *
 * Input  : Ccb Util Oper Data.
 *
 * Returns: None.
 *
 ****************************************************************************/
static SaAisErrorT avd_node_ccb_completed_delete_hdlr(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_OK;
	AVD_AVND *node;

	avd_log(NCSFL_SEV_NOTICE, "'%s'", opdata->objectName.value);
	node = avd_node_find(&opdata->objectName);
	assert(node != NULL);

	if (node->node_info.member) {
		avd_log(NCSFL_SEV_ERROR, "Node '%s' is still cluster member", opdata->objectName.value);
		return SA_AIS_ERR_BAD_OPERATION;
	}

	if (node->type == AVSV_AVND_CARD_SYS_CON) {
		avd_log(NCSFL_SEV_ERROR, "Cannot remove controller node");
		return SA_AIS_ERR_BAD_OPERATION;
	}

	/* Check to see that the node is in admin locked state before delete */
	if (node->saAmfNodeAdminState != SA_AMF_ADMIN_LOCKED) {
		avd_log(NCSFL_SEV_ERROR, "Node '%s' not locked", opdata->objectName.value);
		return SA_AIS_ERR_BAD_OPERATION;
	}

	/* Check to see that no SUs exists on this node */
	if (node->list_of_su != NULL) {
		avd_log(NCSFL_SEV_ERROR, "Node '%s' still has SUs", opdata->objectName.value);
		return SA_AIS_ERR_BAD_OPERATION;
	}

	if (node->list_of_avd_sw_bdl != NULL) {
		avd_log(NCSFL_SEV_ERROR, "Node '%s' still has SW Bundles", opdata->objectName.value);
		return SA_AIS_ERR_BAD_OPERATION;
	}

	return rc;
}

/*****************************************************************************
 * Function: avd_node_ccb_completed_modify_hdlr
 *
 * Purpose: This routine handles CCB modify completed operations on SaAmfNode objects.
 *
 *
 * Input  : Ccb Util Oper Data.
 *
 * Returns: None.
 *
 * NOTES  : None.
 *
 *
 **************************************************************************/
static SaAisErrorT avd_node_ccb_completed_modify_hdlr(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_OK;
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;

	avd_log(NCSFL_SEV_NOTICE, "'%s'", opdata->objectName.value);

	while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
		const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;
		void *value = attribute->attrValues[0];

		if (!strcmp(attribute->attrName, "saAmfNodeClmNode")) {
			avd_log(NCSFL_SEV_ERROR, "Modification of saAmfNodeClmNode not allowed");
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		} else if (!strcmp(attribute->attrName, "saAmfNodeSuFailOverProb")) {
			SaTimeT su_failover_prob = *((SaTimeT *)value);
			if (su_failover_prob == 0) {
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
		} else if (!strcmp(attribute->attrName, "saAmfNodeAutoRepair")) {
			SaBoolT val = *((SaBoolT *)value);
			if (val > SA_TRUE) {
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
		} else if (!strcmp(attribute->attrName, "saAmfNodeFailfastOnTerminationFailure")) {
			SaBoolT val = *((SaBoolT *)value);
			if (val > SA_TRUE) {
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
		} else if (!strcmp(attribute->attrName, "saAmfNodeFailfastOnInstantiationFailure")) {
			SaBoolT val = *((SaBoolT *)value);
			if (val > SA_TRUE) {
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
		} else {
			avd_log(NCSFL_SEV_ERROR, "Unknown attribute %s", attribute->attrName);
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}
	}

 done:
	return rc;
}

/*****************************************************************************
 * Function: avd_node_ccb_completed_cb
 * 
 * Purpose: This routine handles all CCB operations on SaAmfNode objects.
 * 
 *
 * Input  : Ccb Util Oper Data
 *
 * Returns: None.
 *
 * NOTES  : None.
 *
 *
 **************************************************************************/
static SaAisErrorT avd_node_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc;

	avd_trace("'%s'", opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		rc = avd_node_ccb_completed_create_hdlr(opdata);
		break;
	case CCBUTIL_MODIFY:
		rc = avd_node_ccb_completed_modify_hdlr(opdata);
		break;
	case CCBUTIL_DELETE:
		rc = avd_node_ccb_completed_delete_hdlr(opdata);
		break;
	default:
		assert(0);
		break;
	}

	return rc;
}

/*****************************************************************************
 * Function: avd_node_ccb_apply_delete_hdlr
 *
 * Purpose: This routine handles delete operations on SaAmfCluster objects.
 *
 *
 * Input  : Ccb Util Oper Data.
 *
 * Returns: None.
 *
 ****************************************************************************/
static void avd_node_ccb_apply_delete_hdlr(CcbUtilOperationData_t *opdata)
{
	AVD_AVND *node;

	avd_log(NCSFL_SEV_NOTICE, "'%s'", opdata->objectName.value);

	node = avd_node_find(&opdata->objectName);
	assert(node != NULL);

	/* Remove the node from the node_id tree. */
	m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);
	avd_avnd_del_cluster_list(node);
	avd_node_delete_nodeid(node);
	avd_node_delete(node);
	m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

	/* Check point to the standby AVD to delete this node */
	m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(avd_cb, node, AVSV_CKPT_AVD_NODE_CONFIG);
}

/*****************************************************************************
 * Function: avd_node_ccb_apply_modify_hdlr
 *
 * Purpose: This routine handles modify operations on SaAmfCluster objects.
 *
 *
 * Input  : Ccb Util Oper Data.
 *
 * Returns: None.
 *
 * NOTES  : None.
 *
 *
 **************************************************************************/
static void avd_node_ccb_apply_modify_hdlr(CcbUtilOperationData_t *opdata)
{
	uns32 rc;
	AVD_AVND *avd_avnd;
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;

	avd_log(NCSFL_SEV_NOTICE, "'%s'", opdata->objectName.value);

	/* Find the node name. */
	avd_avnd = avd_node_find(&opdata->objectName);
	assert(avd_avnd != NULL);

	i = 0;
	/* Modifications can be done for the following parameters. */
	while (((attr_mod = opdata->param.modify.attrMods[i++])) != NULL) {
		void *value;
		const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;
		AVSV_PARAM_INFO param;

		value = attribute->attrValues[0];

		if (!strcmp(attribute->attrName, "saAmfNodeSuFailOverProb")) {
			SaTimeT su_failover_prob;
			SaTimeT backup_time;
			su_failover_prob = *((SaTimeT *)value);
			SaTimeT temp_su_prob;

			m_NCS_OS_HTONLL_P(&temp_su_prob, su_failover_prob);
			su_failover_prob = temp_su_prob;

			memset(((uns8 *)&param), '\0', sizeof(AVSV_PARAM_INFO));
			param.class_id = AVSV_SA_AMF_NODE;
			param.attr_id = saAmfNodeSuFailoverProb_ID;
			param.act = AVSV_OBJ_OPR_MOD;
			param.name = avd_avnd->node_info.nodeName;

			if (avd_avnd->node_state != AVD_AVND_STATE_ABSENT) {
				backup_time = avd_avnd->saAmfNodeSuFailOverProb;
				param.value_len = sizeof(SaTimeT);
				memcpy((char *)&param.value[0], (char *)&su_failover_prob, param.value_len);

				avd_avnd->saAmfNodeSuFailOverProb = m_NCS_OS_NTOHLL_P(&su_failover_prob);

				rc = avd_snd_op_req_msg(avd_cb, avd_avnd, &param);
				assert(rc == NCSCC_RC_SUCCESS);
			} else {
				avd_avnd->saAmfNodeSuFailOverProb = m_NCS_OS_NTOHLL_P(&su_failover_prob);
			}

			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, avd_avnd, AVSV_CKPT_AVD_NODE_CONFIG);

		} else if (!strcmp(attribute->attrName, "saAmfNodeSuFailoverMax")) {
			uns32 back_val;
			uns32 failover_val;

			failover_val = *((SaUint32T *)value);

			memset(((uns8 *)&param), '\0', sizeof(AVSV_PARAM_INFO));
			param.class_id = AVSV_SA_AMF_NODE;
			param.attr_id = saAmfNodeSuFailoverMax_ID;
			param.act = AVSV_OBJ_OPR_MOD;
			param.name = avd_avnd->node_info.nodeName;

			if (avd_avnd->node_state != AVD_AVND_STATE_ABSENT) {
				back_val = avd_avnd->saAmfNodeSuFailoverMax;
				param.value_len = sizeof(uns32);
				m_NCS_OS_HTONL_P(&param.value[0], failover_val);
				avd_avnd->saAmfNodeSuFailoverMax = failover_val;

				rc = avd_snd_op_req_msg(avd_cb, avd_avnd, &param);
				assert(rc == NCSCC_RC_SUCCESS);
			} else {
				avd_avnd->saAmfNodeSuFailoverMax = failover_val;
			}

			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, avd_avnd, AVSV_CKPT_AVD_NODE_CONFIG);
		} else {
			assert(0);
		}
	}			/* while (attr_mod != NULL) */
}

/*****************************************************************************
 * Function: avd_node_ccb_apply_cb
 * 
 * Purpose: This routine handles all CCB operations on SaAmfNode objects.
 * 
 *
 * Input  : Ccb Util Oper Data
 *
 * Returns: None.
 *
 * NOTES  : None.
 *
 *
 **************************************************************************/
static void avd_node_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_trace("'%s'", opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		break;
	case CCBUTIL_MODIFY:
		avd_node_ccb_apply_modify_hdlr(opdata);
		break;
	case CCBUTIL_DELETE:
		avd_node_ccb_apply_delete_hdlr(opdata);
		break;
	default:
		assert(0);
		break;
	}
}

/*****************************************************************************
 * Function: avd_node_admin_op_cb
 *
 * Purpose: This routine handles admin Oper on SaAmfNode objects.
 *
 *
 * Input  : ...
 *
 * Returns: None.
 *
 * NOTES  : None.
 *
 *
 **************************************************************************/
static void avd_node_admin_op_cb(SaImmOiHandleT immOiHandle,
				 SaInvocationT invocation,
				 const SaNameT *objectName,
				 SaImmAdminOperationIdT operationId, const SaImmAdminOperationParamsT_2 **params)
{
	SaAisErrorT rc = SA_AIS_OK;
	AVD_AVND *avd_avnd;

	avd_log(NCSFL_SEV_NOTICE, "%s", objectName->value);

	/* Find the node name. */
	avd_avnd = avd_node_find(objectName);
	assert(avd_avnd != NULL);

	switch (operationId) {
		/* Valid B.04 AMF node admin operations */
	case SA_AMF_ADMIN_SHUTDOWN:
		if ((avd_avnd->saAmfNodeAdminState == SA_AMF_ADMIN_LOCKED) && (operationId == SA_AMF_ADMIN_SHUTDOWN)) {
			rc = NCSCC_RC_INV_VAL;
			goto done;
		}
		/* Don't put break, let it call avd_sg_app_node_admin_func */
	case SA_AMF_ADMIN_UNLOCK:
	case SA_AMF_ADMIN_LOCK:
		if (operationId == avd_avnd->saAmfNodeAdminState)
			goto done;

		if (avd_sg_app_node_admin_func(avd_cb, avd_avnd, operationId) != NCSCC_RC_SUCCESS)
			rc = SA_AIS_ERR_BAD_OPERATION;
		break;
	case SA_AMF_ADMIN_LOCK_INSTANTIATION:
	case SA_AMF_ADMIN_UNLOCK_INSTANTIATION:
	case SA_AMF_ADMIN_RESTART:
	default:
		rc = SA_AIS_ERR_NOT_SUPPORTED;
		break;
	}

 done:
	(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, rc);
}

/*****************************************************************************
 * Function: avd_ng_del_cluster_list
 *
 * Purpose:  This function will del the given Ng from the Cluster list.
 *
 * Input: node group - The node group pointer
 *
 * Returns: None.
 *
 * NOTES: None
 *
 *
 **************************************************************************/
static void avd_ng_del_cluster_list(AVD_AMF_NG *node_gr)
{
	AVD_AMF_NG *i_node_gr = NULL;
	AVD_AMF_NG *prev_node_gr = NULL;

	if (node_gr->ng_on_cluster != NULL) {
		/* remove Node from Cluster */
		i_node_gr = node_gr->ng_on_cluster->list_of_avd_ng;

		while ((i_node_gr != NULL) && (i_node_gr != node_gr)) {
			prev_node_gr = i_node_gr;
			i_node_gr = i_node_gr->cluster_list_ng_next;
		}

		if (i_node_gr != node_gr) {
			/* Log a fatal error */
		} else {
			if (prev_node_gr == NULL) {
				node_gr->ng_on_cluster->list_of_avd_ng = node_gr->cluster_list_ng_next;
			} else {
				prev_node_gr->cluster_list_ng_next = node_gr->cluster_list_ng_next;
			}
		}

		node_gr->cluster_list_ng_next = NULL;
		node_gr->ng_on_cluster = NULL;
	}
}

/*****************************************************************************
 * Function: avd_ng_find
 *
 * Purpose:  This function will find a AVD_AMF_NG structure in the
 *           tree with ng_name value as key.
 *
 * Input: dn - The name of the Cluster.
 *        
 * Returns: The pointer to AVD_AMF_NG structure found in the tree.
 *
 * NOTES:
 *
 *
 **************************************************************************/
static AVD_AMF_NG *avd_ng_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_AMF_NG *)ncs_patricia_tree_get(&avd_nodegroup_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_ng_delete
 *
 * Purpose:  This function will delete a AVD_AMF_NG structure from the
 *           tree. 
 *
 * Input: ng - The Node Gr structure that needs to be deleted.
 *
 * Returns: -
 *
 * NOTES:
 *
 * 
 **************************************************************************/
static void avd_ng_delete(AVD_AMF_NG *ng)
{
	uns32 rc;

	avd_ng_del_cluster_list(ng);
	rc = ncs_patricia_tree_del(&avd_nodegroup_db, &ng->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	free(ng->saAmfNGNodeList);
	free(ng);
}

/**
 * Validate configuration attributes for an AMF node group object
 * @param ng
 * @param opdata
 * 
 * @return int
 */
static int avd_ng_config_validate(const AVD_AMF_NG *ng, const CcbUtilOperationData_t *opdata)
{
	int i;
	char *p;

	p = strchr((char *)ng->ng_name.value, ',');
	if (p == NULL) {
		avd_log(NCSFL_SEV_ERROR, "No parent to '%s' ", ng->ng_name.value);
		return -1;
	}
	p++;

	if (strncmp(p, "safAmfCluster=", 14) != 0) {
		avd_log(NCSFL_SEV_ERROR, "Wrong parent '%s' to '%s' ", p, ng->ng_name.value);
		return -1;
	}

	/* Make sure all nodes exist */
	for (i = 0; i < ng->number_nodes; i++) {
		AVD_AVND *node = avd_node_find(&ng->saAmfNGNodeList[i]);
		if (node == NULL) {
			/* Node does not exist in current model, check CCB */
			if ((opdata != NULL) &&
			    (ccbutil_getCcbOpDataByDN(opdata->ccbId, &ng->saAmfNGNodeList[i]) == NULL)) {
				avd_log(NCSFL_SEV_ERROR, "Node '%s' does not exist either in model or CCB",
					ng->saAmfNGNodeList[i].value);
				return SA_AIS_ERR_BAD_OPERATION;
			}
			avd_log(NCSFL_SEV_ERROR, "Node '%s' does not exist", ng->saAmfNGNodeList[i].value);
			return -1;
		}
	}

	return 0;
}

/**
 * Create a new Node Group object
 * @param dn
 * @param attributes
 * 
 * @return AVD_AVND*
 */
static AVD_AMF_NG *avd_ng_create(SaNameT *ng_name, const SaImmAttrValuesT_2 **attributes)
{
	int rc = -1, i = 0, j;
	AVD_AMF_NG *ng;
	const SaImmAttrValuesT_2 *attr;

	if ((ng = calloc(1, sizeof(AVD_AMF_NG))) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "calloc failed");
		return NULL;
	}

	memcpy(ng->ng_name.value, ng_name->value, ng_name->length);
	ng->ng_name.length = ng_name->length;
	ng->tree_node.key_info = (uns8 *)&(ng->ng_name);

	while ((attr = attributes[i++]) != NULL) {
		if (!strcmp(attr->attrName, "saAmfNHNodeList")) {
			ng->number_nodes = attr->attrValuesNumber;
			ng->saAmfNGNodeList = malloc(attr->attrValuesNumber * sizeof(SaNameT));
			for (j = 0; j < attr->attrValuesNumber; j++) {
				ng->saAmfNGNodeList[j] = *((SaNameT *)attr->attrValues[j]);
			}
		}
	}

	if (ncs_patricia_tree_add(&avd_nodegroup_db, &ng->tree_node) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_ERROR, "ncs_patricia_tree_add failed");
		goto done;
	}
	rc = 0;

done:
	if (rc != 0) {
		free(ng);
		ng = NULL;
	}

	return ng;
}

/*****************************************************************************
 * Function: avd_ng_ccb_apply_cb
 * 
 * Purpose: This routine handles all CCB operations on SaAmfNodeGroup objects.
 * 
 *
 * Input  : Ccb Util Oper Data.
 *  
 * Returns: None.
 *  
 * NOTES  : None.
 *
 *
 **************************************************************************/
static void avd_ng_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_trace("'%s'", opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		break;
	case CCBUTIL_DELETE: {
		AVD_AMF_NG *ng = avd_ng_find(&opdata->objectName);
		avd_ng_delete(ng);
		break;
	}
	default:
		assert(0);
	}
}

/*****************************************************************************
 * Function: avd_ng_ccb_completed_cb
 * 
 * Purpose: This routine handles all CCB operations on SaAmfNodeGroup objects.
 * 
 *
 * Input  : Ccb Util Oper Data
 *  
 * Returns: None.
 *  
 * NOTES  : None.
 *
 *
 **************************************************************************/
static SaAisErrorT avd_ng_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_AMF_NG *ng;

	avd_log(NCSFL_SEV_NOTICE, "'%s', %llu", opdata->objectName.value, opdata->ccbId);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if ((ng = avd_ng_create(&opdata->objectName, opdata->param.create.attrValues)) == NULL) {
			goto done;
		}

		if (avd_ng_config_validate(ng, opdata) != 0) {
			avd_log(NCSFL_SEV_ERROR, "AMF node group '%s' validation error", ng->ng_name.value);
			avd_ng_delete(ng);
			free(ng->saAmfNGNodeList);
			free(ng);
			goto done;
		}

		break;
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfNodeGroup not supported");
		goto done;
		break;
	case CCBUTIL_DELETE:
		avd_log(NCSFL_SEV_ERROR, "Deletion of SaAmfNodeGroup not supported");
		goto done;
		break;
	default:
		assert(0);
		break;
	}

	rc = SA_AIS_OK;
 done:
	return rc;
}

/**
 * Get configuration for all AMF node group objects from IMM and
 * create AVD internal objects.
 * 
 * @return int
 */
SaAisErrorT avd_node_group_config_get(void)
{
	SaAisErrorT error, rc = SA_AIS_ERR_FAILED_OPERATION;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfNodeGroup";
	AVD_AMF_NG *ng;

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, NULL, SA_IMM_SUBTREE,
						  SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
						  NULL, &searchHandle);

	if (SA_AIS_OK != error) {
		avd_log(NCSFL_SEV_ERROR, "No objects found");
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, &attributes) == SA_AIS_OK) {
		avd_log(NCSFL_SEV_NOTICE, "'%s'", dn.value);

		if ((ng = avd_ng_create(&dn, (const SaImmAttrValuesT_2 **)attributes)) == NULL)
			goto done2;

		if (avd_ng_config_validate(ng, NULL) != 0) {
			avd_ng_delete(ng);
			avd_log(NCSFL_SEV_ERROR, "AMF node group '%s' validation error", dn.value);
			goto done2;
		}
	}

	rc = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:

	return rc;
}

/*****************************************************************************
 * Function: avd_nodeswbdl_find
 *
 * Purpose:  This function will find a AVD_AMF_SW_BUNDLE structure in the
 *           tree with sw_bdl_name value as key.
 *
 * Input: dn - The name of the Cluster.
 *        
 * Returns: The pointer to AVD_AMF_SW_BUNDLE structure found in the tree.
 *
 * NOTES:
 *
 *
 **************************************************************************/

static AVD_AMF_SW_BUNDLE *avd_nodeswbdl_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_AMF_SW_BUNDLE *)ncs_patricia_tree_get(&avd_nodeswbundle_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_sw_bdl_delete
 *
 * Purpose:  This function will delete a AVD_AMF_SW_BUNDLE structure from the
 *           tree. 
 *
 * Input: sw_bdl - The Sw Bundle structure that needs to be deleted.
 *
 * Returns: -
 *
 * NOTES:
 *
 * 
 **************************************************************************/
static void avd_nodeswbdl_delete(AVD_AMF_SW_BUNDLE *sw_bdl)
{
	uns32 rc;

	rc = ncs_patricia_tree_del(&avd_nodeswbundle_db, &sw_bdl->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	free(sw_bdl->saAmfNodeSwBundlePathPrefix);
	free(sw_bdl);
}

/**
 * Validate configuration attributes for an AMF node Sw bundle object.
 * Validate the naming tree.
 * @param ng
 * @param opdata
 * 
 * @return int
 */
static int avd_nodeswbdl_config_validate(const AVD_AMF_SW_BUNDLE *sw_bdl)
{
	char *parent;
	char *dn = (char *)sw_bdl->sw_bdl_name.value;

	if ((parent = strchr(dn, ',')) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "No parent to '%s' ", dn);
		return -1;
	}

	parent++;

	/* Should be children to nodes */
	if (strncmp(parent, "safAmfNode=", 11) != 0) {
		avd_log(NCSFL_SEV_ERROR, "Wrong parent '%s' to '%s' ", parent, dn);
		return -1;
	}

	return 0;
}

/**
 * Create a new Node Sw Bundle object
 * @param dn
 * @param attributes
 * 
 * @return AVD_AVND*
 */
static AVD_AMF_SW_BUNDLE *avd_nodeswbdl_create(SaNameT *sw_bdl_name, const SaImmAttrValuesT_2 **attributes, AVD_AVND *node)
{
	int rc = -1;
	AVD_AMF_SW_BUNDLE *sw_bdl;
	const char *prefix;

	if ((sw_bdl = calloc(1, sizeof(AVD_AMF_SW_BUNDLE))) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "calloc failed");
		return NULL;
	}

	memcpy(sw_bdl->sw_bdl_name.value, sw_bdl_name->value, sw_bdl_name->length);
	sw_bdl->sw_bdl_name.length = sw_bdl_name->length;
	sw_bdl->tree_node.key_info = (uns8 *)&(sw_bdl->sw_bdl_name);

	prefix = immutil_getStringAttr(attributes, "saAmfNodeSwBundlePathPrefix", 0);
	if (prefix == NULL) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfNodeSwBundlePathPrefix FAILED for '%s'", sw_bdl_name->value);
		goto done;
	}

	sw_bdl->saAmfNodeSwBundlePathPrefix = strdup(prefix);
	sw_bdl->node_sw_bdl_on_node = node;

	if (ncs_patricia_tree_add(&avd_nodeswbundle_db, &sw_bdl->tree_node) != NCSCC_RC_SUCCESS) {
		avd_log(NCSFL_SEV_ERROR, "ncs_patricia_tree_add failed");
		goto done;
	}
	rc = 0;

done:
	if (rc != 0) {
		free(sw_bdl);
		sw_bdl = NULL;
	}

	return sw_bdl;
}

static void avd_nodeswbdl_add_to_model(AVD_AMF_SW_BUNDLE *sw_bdl)
{
	sw_bdl->node_list_sw_bdl_next = sw_bdl->node_sw_bdl_on_node->list_of_avd_sw_bdl;
	sw_bdl->node_sw_bdl_on_node->list_of_avd_sw_bdl = sw_bdl;
}

/**
 * Get configuration for all AMF node Sw Bundle objects from IMM and
 * create AVD internal objects.
 * 
 * @return int
 */
static SaAisErrorT avd_nodeswbdl_config_get(AVD_AVND *node)
{
	SaAisErrorT error, rc = SA_AIS_ERR_FAILED_OPERATION;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfNodeSwBundle";
	AVD_AMF_SW_BUNDLE *sw_bdl;

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, &node->node_info.nodeName, SA_IMM_SUBTREE,
		SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
		NULL, &searchHandle);

	if (SA_AIS_OK != error) {
		avd_log(NCSFL_SEV_ERROR, "No objects found");
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, &attributes) == SA_AIS_OK) {
		avd_log(NCSFL_SEV_NOTICE, "'%s'", dn.value);

		if ((sw_bdl = avd_nodeswbdl_create(&dn, (const SaImmAttrValuesT_2 **)attributes, node)) == NULL)
			goto done2;

		if (avd_nodeswbdl_config_validate(sw_bdl) != 0) {
			avd_log(NCSFL_SEV_ERROR, "AMF Node Sw Bundle '%s' validation error", dn.value);
			goto done2;
		}

		avd_nodeswbdl_add_to_model(sw_bdl);
	}

	rc = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:

	return rc;
}

/*****************************************************************************
 * Function: avd_nodeswbdl_ccb_apply_create_hdlr
 * 
 * Purpose: This routine handles create operations on SaAmfNodeSwBundle objects.
 * 
 *
 * Input  : Ccb Util Oper Data.
 *  
 * Returns: None.
 *  
 * NOTES  : None.
 *
 *
 **************************************************************************/

static void avd_nodeswbdl_ccb_apply_create_hdlr(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "'%s', %llu", opdata->objectName.value, opdata->ccbId);
	avd_nodeswbdl_add_to_model(opdata->userData);
}

/*****************************************************************************
 * Function: avd_node_del_swbdl
 *
 * Purpose:  This function will del the given sw_bdl from the Node list.
 *
 * Input: cb - the AVD control block
 *        sw_bundle - The Sw Bundle pointer
 *
 * Returns: None.
 *
 * NOTES: None
 *
 *
 **************************************************************************/
static void avd_node_del_swbdl(AVD_AMF_SW_BUNDLE *sw_bdl)
{
	AVD_AMF_SW_BUNDLE *i_sw_bdl = NULL;
	AVD_AMF_SW_BUNDLE *prev_sw_bdl = NULL;

	if (sw_bdl->node_sw_bdl_on_node != NULL) {
		/* remove Sw Bdl from Node */
		i_sw_bdl = sw_bdl->node_sw_bdl_on_node->list_of_avd_sw_bdl;

		while ((i_sw_bdl != NULL) && (i_sw_bdl != sw_bdl)) {
			prev_sw_bdl = i_sw_bdl;
			i_sw_bdl = i_sw_bdl->node_list_sw_bdl_next;
		}

		if (i_sw_bdl != sw_bdl) {
			/* Log a fatal error */
		} else {
			if (prev_sw_bdl == NULL) {
				sw_bdl->node_sw_bdl_on_node->list_of_avd_sw_bdl = sw_bdl->node_list_sw_bdl_next;
			} else {
				prev_sw_bdl->node_list_sw_bdl_next = sw_bdl->node_list_sw_bdl_next;
			}
		}

		sw_bdl->node_list_sw_bdl_next = NULL;
		sw_bdl->node_sw_bdl_on_node = NULL;
	}
}

/*****************************************************************************
 * Function: avd_nodeswbdl_ccb_apply_delete_hdlr
 * 
 * Purpose: This routine handles delete operations on SaAmfNodeSwBundle objects.
 * 
 *
 * Input  : Ccb Util Oper Data.
 *  
 * Returns: None.
 *  
 ****************************************************************************/
static void avd_nodeswbdl_ccb_apply_delete_hdlr(CcbUtilOperationData_t *opdata)
{
	AVD_AMF_SW_BUNDLE *sw_bdl;

	avd_log(NCSFL_SEV_NOTICE, "'%s'", opdata->objectName.value);
	sw_bdl = avd_nodeswbdl_find(&opdata->objectName);
	assert(sw_bdl != NULL);
	avd_node_del_swbdl(sw_bdl);
	avd_nodeswbdl_delete(sw_bdl);
}

/*****************************************************************************
 * Function: avd_nodeswbdl_ccb_apply_cb
 * 
 * Purpose: This routine handles all CCB operations on SaAmfNodeSwBundle objects.
 * 
 *
 * Input  : Ccb Util Oper Data.
 *  
 * Returns: None.
 *  
 * NOTES  : None.
 *
 *
 **************************************************************************/
static void avd_nodeswbdl_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_trace("'%s'", opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		avd_nodeswbdl_ccb_apply_create_hdlr(opdata);
		break;
	case CCBUTIL_DELETE:
		avd_nodeswbdl_ccb_apply_delete_hdlr(opdata);
		break;
	default:
		assert(0);
	}
}

static SaAisErrorT avd_nodeswbdl_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_AVND *node;
	AVD_AMF_SW_BUNDLE *sw_bdl;

	avd_log(NCSFL_SEV_NOTICE, "'%s', %llu", opdata->objectName.value, opdata->ccbId);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		node = avd_node_find(opdata->param.create.parentName);
		if (node == NULL) {
			avd_log(NCSFL_SEV_ERROR, "AMF Node Sw Bundle must be created under a node");
			goto done;
		}
		if ((sw_bdl = avd_nodeswbdl_create(&opdata->objectName, opdata->param.create.attrValues, node)) == NULL) {
			goto done;
		}

		if (avd_nodeswbdl_config_validate(sw_bdl) != 0) {
			avd_log(NCSFL_SEV_ERROR, "AMF Node Sw Bundle '%s' validation error", opdata->objectName.value);
			goto done;
		}

		opdata->userData = sw_bdl;	/* Save for later use in apply */

		break;
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfNodeSwBundle not supported");
		goto done;
		break;
	case CCBUTIL_DELETE:
		avd_log(NCSFL_SEV_ERROR, "Deletion of SaAmfNodeSwBundle not supported");
		goto done;
		break;
	default:
		assert(0);
		break;
	}

	rc = SA_AIS_OK;
 done:
	return rc;
}

void avd_node_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	assert(ncs_patricia_tree_init(&avd_node_name_db, &patricia_params) == NCSCC_RC_SUCCESS);
	assert(ncs_patricia_tree_init(&avd_nodegroup_db, &patricia_params) == NCSCC_RC_SUCCESS);
	assert(ncs_patricia_tree_init(&avd_nodeswbundle_db, &patricia_params) == NCSCC_RC_SUCCESS);

	patricia_params.key_size = sizeof(SaClmNodeIdT);
	assert(ncs_patricia_tree_init(&avd_node_id_db, &patricia_params) == NCSCC_RC_SUCCESS);

	avd_class_impl_set("SaAmfNode", NULL,
		avd_node_admin_op_cb, avd_node_ccb_completed_cb, avd_node_ccb_apply_cb);

	avd_class_impl_set("SaAmfNodeGroup", NULL, NULL, avd_ng_ccb_completed_cb, avd_ng_ccb_apply_cb);

	avd_class_impl_set("SaAmfNodeSwBundle", NULL,
		NULL, avd_nodeswbdl_ccb_completed_cb, avd_nodeswbdl_ccb_apply_cb);
}
