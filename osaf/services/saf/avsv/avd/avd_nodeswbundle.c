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

#include <saImmOm.h>
#include <immutil.h>
#include <logtrace.h>

#include <avd.h>
#include <avd_cluster.h>
#include <avd_imm.h>

static NCS_PATRICIA_TREE nodeswbundle_db;

AVD_NODE_SW_BUNDLE *avd_nodeswbdl_get(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_NODE_SW_BUNDLE *)ncs_patricia_tree_get(&nodeswbundle_db, (uns8 *)&tmp);
}

static void nodeswbdl_delete(AVD_NODE_SW_BUNDLE *sw_bdl)
{
	uns32 rc = ncs_patricia_tree_del(&nodeswbundle_db, &sw_bdl->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	avd_node_remove_swbdl(sw_bdl);
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
static int is_config_valid(const SaNameT *dn, const SaImmAttrValuesT_2 **attributes,
	const CcbUtilOperationData_t *opdata)
{
	char *parent;
	const char *path_prefix;

	if ((parent = strchr((char*)dn->value, ',')) == NULL) {
		LOG_ER("No parent to '%s' ", dn->value);
		return 0;
	}

	/* Should be children to nodes */
	if (strncmp(++parent, "safAmfNode=", 11) != 0) {
		LOG_ER("Wrong parent '%s' to '%s' ", parent, dn->value);
		return 0;
	}

	path_prefix = immutil_getStringAttr(attributes, "saAmfNodeSwBundlePathPrefix", 0);
	assert(path_prefix);

	if (path_prefix[0] != '/') {
		LOG_ER("Invalid absolute path '%s' for '%s' ", path_prefix, dn->value);
		return 0;
	}

	return 1;
}

/**
 * Create a new Node Sw Bundle object
 * @param dn
 * @param attributes
 * 
 * @return AVD_AVND*
 */
static AVD_NODE_SW_BUNDLE *nodeswbdl_create(SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	SaNameT node_name;
	AVD_AVND *node;
	AVD_NODE_SW_BUNDLE *sw_bdl;
	const char *prefix;

	TRACE_ENTER2("'%s'", dn->value);

	avsv_sanamet_init(dn, &node_name, "safAmfNode");
	node = avd_node_get(&node_name);

	if ((sw_bdl = calloc(1, sizeof(AVD_NODE_SW_BUNDLE))) == NULL) {
		LOG_ER("calloc failed");
		return NULL;
	}

	memcpy(sw_bdl->sw_bdl_name.value, dn->value, dn->length);
	sw_bdl->sw_bdl_name.length = dn->length;
	sw_bdl->tree_node.key_info = (uns8 *)&(sw_bdl->sw_bdl_name);

	prefix = immutil_getStringAttr(attributes, "saAmfNodeSwBundlePathPrefix", 0);
	assert(prefix != NULL);

	sw_bdl->saAmfNodeSwBundlePathPrefix = strdup(prefix);
	sw_bdl->node_sw_bdl_on_node = node;

	return sw_bdl;
}

static void nodeswbdl_add_to_model(AVD_NODE_SW_BUNDLE *sw_bdl)
{
	unsigned int rc = ncs_patricia_tree_add(&nodeswbundle_db, &sw_bdl->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	avd_node_add_swbdl(sw_bdl);
}

/**
 * Get configuration for all SaAmfNodeSwBundle objects from IMM and
 * create AVD internal objects.
 * 
 * @return int
 */
SaAisErrorT avd_nodeswbdl_config_get(AVD_AVND *node)
{
	SaAisErrorT error, rc = SA_AIS_ERR_FAILED_OPERATION;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfNodeSwBundle";
	AVD_NODE_SW_BUNDLE *sw_bdl;

	TRACE_ENTER();

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, &node->node_info.nodeName, SA_IMM_SUBTREE,
		SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
		NULL, &searchHandle);

	if (SA_AIS_OK != error) {
		LOG_ER("No objects found");
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {
		if (!is_config_valid(&dn, attributes, NULL))
			goto done2;

		if ((sw_bdl = nodeswbdl_create(&dn, attributes)) == NULL)
			goto done2;

		nodeswbdl_add_to_model(sw_bdl);
	}

	rc = SA_AIS_OK;

done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
done1:
	TRACE_LEAVE2("%u", rc);
	return rc;
}

static SaAisErrorT nodeswbdl_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if (is_config_valid(&opdata->objectName, opdata->param.create.attrValues, opdata))
			rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		LOG_ER("Modification of SaAmfNodeSwBundle not supported");
		break;
	case CCBUTIL_DELETE:
		LOG_ER("Deletion of SaAmfNodeSwBundle not supported");
		break;
	default:
		assert(0);
		break;
	}

	TRACE_LEAVE2("%u", rc);
	return rc;
}

static void nodeswbdl_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	AVD_NODE_SW_BUNDLE *sw_bdl;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		sw_bdl = nodeswbdl_create(&opdata->objectName, opdata->param.create.attrValues);
		assert(sw_bdl);
		nodeswbdl_add_to_model(sw_bdl);
		break;
	case CCBUTIL_DELETE:
		nodeswbdl_delete(opdata->userData);
		break;
	default:
		assert(0);
	}

	TRACE_LEAVE();
}

void avd_nodeswbundle_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	assert(ncs_patricia_tree_init(&nodeswbundle_db, &patricia_params) == NCSCC_RC_SUCCESS);
	avd_class_impl_set("SaAmfNodeSwBundle", NULL, NULL, nodeswbdl_ccb_completed_cb, nodeswbdl_ccb_apply_cb);
}

