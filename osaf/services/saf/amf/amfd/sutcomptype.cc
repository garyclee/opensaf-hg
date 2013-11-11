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

#include <util.h>
#include <su.h>
#include <sutype.h>
#include <imm.h>

static NCS_PATRICIA_TREE sutcomptype_db;

static void sutcomptype_db_add(AVD_SUTCOMP_TYPE *sutcomptype)
{
	unsigned int rc = ncs_patricia_tree_add(&sutcomptype_db, &sutcomptype->tree_node);
	osafassert(rc == NCSCC_RC_SUCCESS);
}

static AVD_SUTCOMP_TYPE *sutcomptype_create(SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	AVD_SUTCOMP_TYPE *sutcomptype;

	TRACE_ENTER2("'%s'", dn->value);

	sutcomptype = new AVD_SUTCOMP_TYPE();

	memcpy(sutcomptype->name.value, dn->value, dn->length);
	sutcomptype->name.length = dn->length;
	sutcomptype->tree_node.key_info = (uint8_t *)&(sutcomptype->name);

	if (immutil_getAttr(const_cast<SaImmAttrNameT>("saAmfSutMaxNumComponents"), attributes, 0, &sutcomptype->saAmfSutMaxNumComponents) != SA_AIS_OK)
		sutcomptype->saAmfSutMaxNumComponents = -1; /* no limit */

	    if (immutil_getAttr(const_cast<SaImmAttrNameT>("saAmfSutMinNumComponents"), attributes, 0, &sutcomptype->saAmfSutMinNumComponents) != SA_AIS_OK)
		sutcomptype->saAmfSutMinNumComponents = 1;

	return sutcomptype;
}

static void sutcomptype_delete(AVD_SUTCOMP_TYPE *sutcomptype)
{
	uint32_t rc;

	rc = ncs_patricia_tree_del(&sutcomptype_db, &sutcomptype->tree_node);
	osafassert(rc == NCSCC_RC_SUCCESS);
	delete sutcomptype;
}

AVD_SUTCOMP_TYPE *avd_sutcomptype_get(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SUTCOMP_TYPE *)ncs_patricia_tree_get(&sutcomptype_db, (uint8_t *)&tmp);
}

static int is_config_valid(const SaNameT *dn, const SaImmAttrValuesT_2 **attributes, CcbUtilOperationData_t *opdata)
{
	// TODO
	return 1;
}

SaAisErrorT avd_sutcomptype_config_get(SaNameT *sutype_name, struct avd_sutype *sut)
{
	AVD_SUTCOMP_TYPE *sutcomptype;
	SaAisErrorT error;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfSutCompType";

	TRACE_ENTER();

	searchParam.searchOneAttr.attrName = const_cast<SaImmAttrNameT>("SaImmAttrClassName");
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, sutype_name, SA_IMM_SUBTREE,
		SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
		NULL, &searchHandle);
	
	if (SA_AIS_OK != error) {
		LOG_ER("saImmOmSearchInitialize_2 failed: %u", error);
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {
		if (!is_config_valid(&dn, attributes, NULL))
			goto done2;
		if ((sutcomptype = avd_sutcomptype_get(&dn)) == NULL) {
			if ((sutcomptype = sutcomptype_create(&dn, attributes)) == NULL) {
				error = SA_AIS_ERR_FAILED_OPERATION;
				goto done2;
			}

			sutcomptype_db_add(sutcomptype);
		}
	}

	error = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:
	TRACE_LEAVE2("%u", error);
	return error;
}

static SaAisErrorT sutcomptype_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_SUTCOMP_TYPE *sutcomptype = NULL;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if (is_config_valid(&opdata->objectName, opdata->param.create.attrValues, opdata))
		    rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		report_ccb_validation_error(opdata, "Modification of SaAmfSUType not supported");
		break;
	case CCBUTIL_DELETE:
		sutcomptype = avd_sutcomptype_get(&opdata->objectName);
		if (sutcomptype->curr_num_components == 0) {
			rc = SA_AIS_OK;
			opdata->userData = sutcomptype;	/* Save for later use in apply */
		}
		break;
	default:
		osafassert(0);
		break;
	}

	return rc;
}

static void sutcomptype_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	AVD_SUTCOMP_TYPE *sutcomptype = NULL;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		sutcomptype = sutcomptype_create(&opdata->objectName, opdata->param.create.attrValues);
		osafassert(sutcomptype);
		sutcomptype_db_add(sutcomptype);
		break;
	case CCBUTIL_DELETE:
		sutcomptype_delete(static_cast<AVD_SUTCOMP_TYPE*>(opdata->userData));
		break;
	default:
		osafassert(0);
		break;
	}
}

void avd_sutcomptype_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	osafassert(ncs_patricia_tree_init(&sutcomptype_db, &patricia_params) == NCSCC_RC_SUCCESS);

	avd_class_impl_set(const_cast<SaImmClassNameT>("SaAmfSutCompType"), NULL, NULL, sutcomptype_ccb_completed_cb, sutcomptype_ccb_apply_cb);
}

