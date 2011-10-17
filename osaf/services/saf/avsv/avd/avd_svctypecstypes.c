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

#include <logtrace.h>
#include <avd_si.h>
#include <avd_imm.h>

static NCS_PATRICIA_TREE svctypecstypes_db;

static void svctypecstype_db_add(AVD_SVC_TYPE_CS_TYPE *svctypecstype)
{
	unsigned int rc = ncs_patricia_tree_add(&svctypecstypes_db, &svctypecstype->tree_node);
	osafassert(rc == NCSCC_RC_SUCCESS);
}


static AVD_SVC_TYPE_CS_TYPE *svctypecstypes_create(SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	AVD_SVC_TYPE_CS_TYPE *svctypecstype;

	TRACE_ENTER2("'%s'", dn->value);

	if ((svctypecstype = calloc(1, sizeof(AVD_SVC_TYPE_CS_TYPE))) == NULL) {
		LOG_ER("calloc failed");
		return NULL;
	}

	memcpy(svctypecstype->name.value, dn->value, dn->length);
	svctypecstype->name.length = dn->length;
	svctypecstype->tree_node.key_info = (uint8_t *)&(svctypecstype->name);

	if (immutil_getAttr("saAmfSvctMaxNumCSIs", attributes, 0, &svctypecstype->saAmfSvctMaxNumCSIs) != SA_AIS_OK)
		svctypecstype->saAmfSvctMaxNumCSIs = -1; /* no limit */

	return svctypecstype;
}

static void svctypecstypes_delete(AVD_SVC_TYPE_CS_TYPE *svctypecstype)
{
	uint32_t rc;

	rc = ncs_patricia_tree_del(&svctypecstypes_db, &svctypecstype->tree_node);
	osafassert(rc == NCSCC_RC_SUCCESS);
	free(svctypecstype);
}

AVD_SVC_TYPE_CS_TYPE *avd_svctypecstypes_get(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SVC_TYPE_CS_TYPE*)ncs_patricia_tree_get(&svctypecstypes_db, (uint8_t *)&tmp);
}

/**
 * Get configuration for all SaAmfSvcTypeCSTypes objects from 
 * IMM and create AVD internal objects. 
 * 
 * @param sutype_name 
 * @param sut 
 * 
 * @return SaAisErrorT 
 */
SaAisErrorT avd_svctypecstypes_config_get(SaNameT *svctype_name)
{
	AVD_SVC_TYPE_CS_TYPE *svctypecstype;
	SaAisErrorT error;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfSvcTypeCSTypes";

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, svctype_name, SA_IMM_SUBTREE,
		SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
		NULL, &searchHandle);
	
	if (SA_AIS_OK != error) {
		LOG_ER("saImmOmSearchInitialize_2 failed: %u", error);
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {

		if ((svctypecstype = avd_svctypecstypes_get(&dn))== NULL) {
			if ((svctypecstype = svctypecstypes_create(&dn, attributes)) == NULL) {
				error = SA_AIS_ERR_FAILED_OPERATION;
				goto done2;
			}
			svctypecstype_db_add(svctypecstype);
		}
	}

	error = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:
	return error;
}

/**
 * Handles the CCB Completed operation for the 
 * SaAmfSvcTypeCSTypes class.
 * 
 * @param opdata 
 */
static SaAisErrorT svctypecstypes_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_SVC_TYPE_CS_TYPE *svctypecstype;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		LOG_ER("Modification of SaAmfSvcTypeCSTypes not supported");
		break;
	case CCBUTIL_DELETE:
		svctypecstype = avd_svctypecstypes_get(&opdata->objectName);
		if (svctypecstype->curr_num_csis == 0) {
			rc = SA_AIS_OK;
			opdata->userData = svctypecstype;
		}
		break;
	default:
		osafassert(0);
		break;
	}

	return rc;
}

/**
 * Handles the CCB Apply operation for the SaAmfSvcTypeCSTypes 
 * class. 
 * 
 * @param opdata 
 */
static void svctypecstypes_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	AVD_SVC_TYPE_CS_TYPE *svctypecstype;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		svctypecstype = svctypecstypes_create(&opdata->objectName, opdata->param.create.attrValues);
		osafassert(svctypecstype);
		svctypecstype_db_add(svctypecstype);
		break;
	case CCBUTIL_DELETE:
		svctypecstypes_delete(opdata->userData);
		break;
	default:
		osafassert(0);
		break;
	}
}

void avd_svctypecstypes_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	osafassert(ncs_patricia_tree_init(&svctypecstypes_db, &patricia_params) == NCSCC_RC_SUCCESS);
	avd_class_impl_set("SaAmfSvcTypeCSTypes", NULL, NULL, svctypecstypes_ccb_completed_cb, svctypecstypes_ccb_apply_cb);
}

