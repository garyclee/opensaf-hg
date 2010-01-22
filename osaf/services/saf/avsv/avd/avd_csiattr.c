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

#include <stdbool.h>
#include <logtrace.h>

#include <avd_util.h>
#include <avsv_util.h>
#include <avd_csi.h>
#include <avd_imm.h>

static AVD_CSI_ATTR *csiattr_create(const SaNameT *csiattr_name, const SaImmAttrValuesT_2 **attributes)
{
	int rc = 0;
	AVD_CSI_ATTR *csiattr = NULL, *tmp;
	unsigned int values_number;
	char *p1, *p2;
	SaNameT dn = *csiattr_name;
	int i;
	const char *value;
	
	p2 = strchr((char*)dn.value, ',');
	*p2 = '\0';
	
	p1 = strchr((char*)dn.value, '=');
	p1++;
	
	if ((p2 - p1) > SA_MAX_NAME_LENGTH) {
		rc = -1;
		LOG_ER("CSI attr name too long (impl limit)");
		goto done;
	}

	/* Handle multi value attributes */
	if ((immutil_getAttrValuesNumber("saAmfCSIAttriValue", attributes,
		&values_number) == SA_AIS_OK) && (values_number > 0)) {
		
		for (i = 0; i < values_number; i++) {
			if ((tmp = calloc(1, sizeof(AVD_CSI_ATTR))) == NULL) {
				rc = -1;
				LOG_ER("calloc FAILED");
				goto done;
			}

			tmp->attr_next = csiattr;
			csiattr = tmp;

			csiattr->name_value.name.length = p2 - p1;
			memcpy(csiattr->name_value.name.value, p1, csiattr->name_value.name.length);

			if ((value = immutil_getStringAttr(attributes, "saAmfCSIAttriValue", i)) != NULL) {
				if (strlen(value) > SA_MAX_NAME_LENGTH) {
					rc = -1;
					LOG_ER("CSI attr value too long (impl limit)");
					goto done;
				}

				csiattr->name_value.value.length =
				    snprintf((char *)csiattr->name_value.value.value, SA_MAX_NAME_LENGTH, "%s", value);
			} else {
				/* Param exist but has no value */
				csiattr->name_value.value.length = 0;
			}
		}
	} else {
		/* No values found, create value empty attribute */
		if ((csiattr = calloc(1, sizeof(AVD_CSI_ATTR))) == NULL) {
			rc = -1;
			LOG_ER("calloc FAILED");
			goto done;
		}

		csiattr->name_value.name.length = p2 - p1;
		memcpy(csiattr->name_value.name.value, p1, csiattr->name_value.name.length);
	}

done:
	if (rc != 0) {
		tmp = csiattr;
		while (tmp != NULL) {
			free(tmp);
			tmp = tmp->attr_next;
		}
		csiattr = NULL;
	}

	return csiattr;
}

static int is_config_valid(const SaNameT *dn)
{
	char *parent;

	if ((parent = strchr((char*)dn->value, ',')) == NULL) {
		LOG_ER("No parent to '%s' ", dn->value);
		return 0;
	}

	if (strncmp(++parent, "safCsi=", 7) != 0) {
		LOG_ER("Wrong parent '%s' to '%s' ", parent, dn->value);
		return 0;
	}

	return 1;
}

/**
 * Get configuration for the AMF CSI Attribute objects related
 * to this CSI from IMM and create AVD internal objects.
 * @param cb
 * 
 * @return int
 */
SaAisErrorT avd_csiattr_config_get(const SaNameT *csi_name, AVD_CSI *csi)
{
	SaAisErrorT error;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT csiattr_name;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfCSIAttribute";
	AVD_CSI_ATTR *csiattr;

	TRACE_ENTER();
	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	if ((error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, csi_name,
		SA_IMM_SUBTREE, SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR,
		&searchParam, NULL, &searchHandle)) != SA_AIS_OK) {

		LOG_ER("saImmOmSearchInitialize failed: %u", error);
		goto done1;
	}

	while ((error = immutil_saImmOmSearchNext_2(searchHandle, &csiattr_name,
		(SaImmAttrValuesT_2 ***)&attributes)) == SA_AIS_OK) {

		if ((csiattr = csiattr_create(&csiattr_name, attributes)) != NULL)
			avd_csi_add_csiattr(csi, csiattr);
	}

	error = SA_AIS_OK;

	(void)immutil_saImmOmSearchFinalize(searchHandle);

done1:
	TRACE_LEAVE2("%u", error);
	return error;
}

static SaAisErrorT csiattr_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if (is_config_valid(&opdata->objectName))
			rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		LOG_ER("Modification of SaAmfCSIAttribute not supported");
		break;
	case CCBUTIL_DELETE:
//		rc = SA_AIS_OK;
		break;
	default:
		assert(0);
		break;
	}

	return rc;
}

static void csiattr_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	AVD_CSI_ATTR *csiattr;
	AVD_CSI *csi;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		csiattr = csiattr_create(&opdata->objectName, opdata->param.create.attrValues);
		assert(csiattr);
		csi = avd_csi_get(opdata->param.create.parentName);
		avd_csi_add_csiattr(csi, csiattr);
		break;
	case CCBUTIL_DELETE:
#if 0
		csi = avd_csi_get(opdata->param.create.parentName);
		csiattr = avd_csiattr_find(csi, &opdata->objectName);
		avd_csi_remove_csiattr(csi, csiattr);
		free(csiattr);
#endif
		assert(0);
		break;
	default:
		assert(0);
		break;
	}
}

void avd_csiattr_constructor(void)
{
	avd_class_impl_set("SaAmfCSIAttribute", NULL, NULL, csiattr_ccb_completed_cb, csiattr_ccb_apply_cb);
}

