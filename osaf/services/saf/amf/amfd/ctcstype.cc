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
 *            Ericsson
 *
 */

#include <saImmOm.h>
#include <immutil.h>
#include <logtrace.h>

#include <comp.h>
#include <imm.h>

static NCS_PATRICIA_TREE ctcstype_db;

static void ctcstype_db_add(AVD_CTCS_TYPE *ctcstype)
{
	unsigned int rc = ncs_patricia_tree_add(&ctcstype_db, &ctcstype->tree_node);
	osafassert(rc == NCSCC_RC_SUCCESS);
}

static int is_config_valid(const SaNameT *dn, const SaImmAttrValuesT_2 **attributes, CcbUtilOperationData_t *opdata)
{
	SaUint32T uint32;
	char *parent;

	/* Second comma should be parent */
	if ((parent = strchr((char*)dn->value, ',')) == NULL) {
		report_ccb_validation_error(opdata, "No parent to '%s' ", dn->value);
		return 0;
	}

	if ((parent = strchr(++parent, ',')) == NULL) {
		report_ccb_validation_error(opdata, "No parent to '%s' ", dn->value);
		return 0;
	}

	/* Should be children to SaAmfCompType */
	if (strncmp(++parent, "safVersion=", 11) != 0) {
		report_ccb_validation_error(opdata, "Wrong parent '%s'", parent);
		return 0;
	}

	if ((immutil_getAttr(const_cast<SaImmAttrNameT>("saAmfCtCompCapability"), attributes, 0, &uint32) == SA_AIS_OK) &&
	    (uint32 > SA_AMF_COMP_NON_PRE_INSTANTIABLE)) {
		report_ccb_validation_error(opdata, "Invalid saAmfCtCompCapability %u for '%s'", uint32, dn->value);
		return 0;
	}

	return 1;
}

static AVD_CTCS_TYPE *ctcstype_create(const SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	AVD_CTCS_TYPE *ctcstype;
	int rc = -1;
	SaAisErrorT error;

	TRACE_ENTER2("'%s'", dn->value);

	if ((ctcstype = static_cast<AVD_CTCS_TYPE*>(calloc(1, sizeof(AVD_CTCS_TYPE)))) == NULL) {
		LOG_ER("calloc FAILED");
		return NULL;
	}

	memcpy(ctcstype->name.value, dn->value, dn->length);
	ctcstype->name.length = dn->length;
	ctcstype->tree_node.key_info = (uint8_t *)&(ctcstype->name);

	error = immutil_getAttr(const_cast<SaImmAttrNameT>("saAmfCtCompCapability"), attributes, 0, &ctcstype->saAmfCtCompCapability);
	osafassert(error == SA_AIS_OK);

	if (immutil_getAttr(const_cast<SaImmAttrNameT>("saAmfCtDefNumMaxActiveCSIs"), attributes, 0, &ctcstype->saAmfCtDefNumMaxActiveCSIs) !=
	    SA_AIS_OK)
		ctcstype->saAmfCtDefNumMaxActiveCSIs = -1;	/* no limit */

	    if (immutil_getAttr(const_cast<SaImmAttrNameT>("saAmfCtDefNumMaxStandbyCSIs"), attributes, 0, &ctcstype->saAmfCtDefNumMaxStandbyCSIs) !=
	    SA_AIS_OK)
		ctcstype->saAmfCtDefNumMaxStandbyCSIs = -1;	/* no limit */

	rc = 0;

	if (rc != 0) {
		free(ctcstype);
		ctcstype = NULL;
	}
	return ctcstype;
}

static void ctcstype_delete(AVD_CTCS_TYPE *ctcstype)
{
	unsigned int rc;

	rc = ncs_patricia_tree_del(&ctcstype_db, &ctcstype->tree_node);
	osafassert(rc == NCSCC_RC_SUCCESS);
	free(ctcstype);
}

AVD_CTCS_TYPE *avd_ctcstype_get(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_CTCS_TYPE *)ncs_patricia_tree_get(&ctcstype_db, (uint8_t *)&tmp);
}

SaAisErrorT avd_ctcstype_config_get(const SaNameT *comp_type_dn, AVD_COMP_TYPE *comp_type)
{
	SaAisErrorT error = SA_AIS_ERR_FAILED_OPERATION;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfCtCsType";
	AVD_CTCS_TYPE *ctcstype;

	TRACE_ENTER();

	searchParam.searchOneAttr.attrName = const_cast<SaImmAttrNameT>("SaImmAttrClassName");
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	if (immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, comp_type_dn,
		SA_IMM_SUBTREE, SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR,
		&searchParam, NULL, &searchHandle) != SA_AIS_OK) {

		LOG_ER("saImmOmSearchInitialize_2 failed: %u", error);
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {
		if (!is_config_valid(&dn, attributes, NULL))
			goto done2;

		if ((ctcstype = avd_ctcstype_get(&dn)) == NULL ) {
			if ((ctcstype = ctcstype_create(&dn, attributes)) == NULL)
				goto done2;

			ctcstype_db_add(ctcstype);
		}
	}

	error = SA_AIS_OK;

done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
done1:
	TRACE_LEAVE2("%u", error);
	return error;
}

static SaAisErrorT ctcstype_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if (is_config_valid(&opdata->objectName, opdata->param.create.attrValues, opdata))
			rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		report_ccb_validation_error(opdata, "Modification of SaAmfCtCsType not supported");
		break;
	case CCBUTIL_DELETE:
		rc = SA_AIS_OK;
		break;
	default:
		osafassert(0);
		break;
	}

	TRACE_LEAVE2("%u", rc);
	return rc;
}

static void ctcstype_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	AVD_CTCS_TYPE *ctcstype;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		ctcstype = ctcstype_create(&opdata->objectName, opdata->param.create.attrValues);
		osafassert(ctcstype);
		ctcstype_db_add(ctcstype);
		break;
	case CCBUTIL_DELETE:
		ctcstype = avd_ctcstype_get(&opdata->objectName);
		ctcstype_delete(ctcstype);
		break;
	default:
		osafassert(0);
		break;
	}

	TRACE_LEAVE();
}

void avd_ctcstype_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	osafassert(ncs_patricia_tree_init(&ctcstype_db, &patricia_params) == NCSCC_RC_SUCCESS);
	avd_class_impl_set(const_cast<SaImmClassNameT>("SaAmfCtCsType"), NULL, NULL, ctcstype_ccb_completed_cb, ctcstype_ccb_apply_cb);
}

