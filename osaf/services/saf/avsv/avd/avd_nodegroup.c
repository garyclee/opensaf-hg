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

#include <immutil.h>
#include <logtrace.h>

#include <avd.h>
#include <avd_cluster.h>
#include <avd_imm.h>

static NCS_PATRICIA_TREE nodegroup_db;

static void ng_db_add(AVD_AMF_NG *ng)
{
	unsigned int rc = ncs_patricia_tree_add(&nodegroup_db, &ng->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
}

AVD_AMF_NG *avd_ng_get(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_AMF_NG *)ncs_patricia_tree_get(&nodegroup_db, (uns8 *)&tmp);
}

/**
 * Validate configuration attributes for an SaAmfNodeGroup object
 * @param ng
 * @param opdata
 * 
 * @return int
 */
static int is_config_valid(const SaNameT *dn, const SaImmAttrValuesT_2 **attributes, CcbUtilOperationData_t *opdata)
{
	int i = 0, j;
	char *p;
	const SaImmAttrValuesT_2 *attr;

	p = strchr((char *)dn->value, ',');
	if (p == NULL) {
		LOG_ER("No parent to '%s' ", dn->value);
		return 0;
	}

	if (strncmp(++p, "safAmfCluster=", 14) != 0) {
		LOG_ER("Wrong parent '%s' to '%s' ", p, dn->value);
		return 0;
	}

	while ((attr = attributes[i++]) != NULL)
		if (!strcmp(attr->attrName, "saAmfNGNodeList"))
			break;

	assert(attr);
	assert(attr->attrValuesNumber > 0);

	for (j = 0; j < attr->attrValuesNumber; j++) {
		AVD_AVND *node = avd_node_get((SaNameT *)attr->attrValues[j]);
		if (node == NULL) {
			if (opdata == NULL) {
				LOG_ER("Node '%s' does not exist in model", dn->value);
				return 0;
			}

			/* Node does not exist in current model, check CCB */
			if (ccbutil_getCcbOpDataByDN(opdata->ccbId, (SaNameT *)attr->attrValues[j]) == NULL) {
				LOG_ER("Node '%s' does not exist either in model or CCB",
					((SaNameT *)attr->attrValues[j])->value);
				return SA_AIS_ERR_BAD_OPERATION;
			}
		}
	}

	return 1;
}

/**
 * Create a new SaAmfNodeGroup object
 * @param dn
 * @param attributes
 * 
 * @return AVD_AVND*
 */
static AVD_AMF_NG *ng_create(SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	int rc = -1;
	unsigned int i, values_number;
	AVD_AMF_NG *ng;
	const SaNameT *node_name;

	TRACE_ENTER2("'%s'", dn->value);

	if ((ng = calloc(1, sizeof(AVD_AMF_NG))) == NULL) {
		LOG_ER("calloc failed");
		return NULL;
	}

	memcpy(ng->ng_name.value, dn->value, dn->length);
	ng->ng_name.length = dn->length;
	ng->tree_node.key_info = (uns8 *)&(ng->ng_name);

	if ((immutil_getAttrValuesNumber("saAmfNGNodeList", attributes,
		&values_number) == SA_AIS_OK) && (values_number > 0)) {

		ng->number_nodes = values_number;
		ng->saAmfNGNodeList = malloc(values_number * sizeof(SaNameT));
		for (i = 0; i < values_number; i++) {
			if ((node_name = immutil_getNameAttr(attributes, "saAmfNGNodeList", i)) != NULL)
				ng->saAmfNGNodeList[i] = *node_name;
		}
	}
	else {
		LOG_ER("Node groups must contian at least one node");
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

/**
 * Get configuration for all AMF node group objects from IMM and
 * create AVD internal objects.
 * 
 * @return int
 */
SaAisErrorT avd_ng_config_get(void)
{
	SaAisErrorT error, rc = SA_AIS_ERR_FAILED_OPERATION;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfNodeGroup";
	AVD_AMF_NG *ng;

	TRACE_ENTER();

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, NULL, SA_IMM_SUBTREE,
		SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
		NULL, &searchHandle);

	if (SA_AIS_OK != error) {
		LOG_ER("No objects found");
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {
		if (!is_config_valid(&dn, attributes, NULL)) {
			error = SA_AIS_ERR_FAILED_OPERATION;
			goto done2;
		}

		if ((ng = ng_create(&dn, (const SaImmAttrValuesT_2 **)attributes)) == NULL)
			goto done2;


		ng_db_add(ng);
	}

	rc = SA_AIS_OK;

done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
done1:
	TRACE_LEAVE2("%u", rc);
	return rc;
}

static SaAisErrorT ng_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if (is_config_valid(&opdata->objectName, opdata->param.create.attrValues, opdata))
		    rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		LOG_ER("Modification of SaAmfNodeGroup not supported");
		goto done;
		break;
	case CCBUTIL_DELETE:
		LOG_ER("Deletion of SaAmfNodeGroup not supported");
		goto done;
		break;
	default:
		assert(0);
		break;
	}

done:
	TRACE_LEAVE2("%u", rc);
	return rc;
}

static void ng_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	AVD_AMF_NG *ng;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		ng = ng_create(&opdata->objectName, opdata->param.create.attrValues);
		assert(ng);
		ng_db_add(ng);
		break;
	default:
		assert(0);
	}

	TRACE_LEAVE();
}

void avd_ng_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	assert(ncs_patricia_tree_init(&nodegroup_db, &patricia_params) == NCSCC_RC_SUCCESS);

	avd_class_impl_set("SaAmfNodeGroup", NULL, NULL, ng_ccb_completed_cb, ng_ccb_apply_cb);
}

