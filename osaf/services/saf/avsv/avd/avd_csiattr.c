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

static int is_csiattr_valid(AVD_CSI *csi, char *csiattr)
{
	AVD_CSI_ATTR *i_attr = csi->list_attributes;
	char *p1, *p2;
	
	p2 = strchr(csiattr, ',');
	*p2 = '\0';

	p1 = strchr(csiattr, '=');
	p1++;

	if ((p2 - p1) > SA_MAX_NAME_LENGTH)
		goto done;

	while (i_attr != NULL) {
		if (strncmp((char *)&i_attr->name_value.name.value, p1, p2 - p1) == 0)
			return 0;
		i_attr = i_attr->attr_next;
	}

done:
	return 1;
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
		{
		SaNameT csi_dn;
		AVD_CSI *csi;

		if (!is_config_valid(&opdata->objectName)) 
			break;

		/* extract the parent csi dn */
		strncpy((char *)&csi_dn.value, 
				strstr((char *)&opdata->objectName.value, "safCsi="),
				SA_MAX_NAME_LENGTH); 
		csi_dn.length = strlen((char *)&csi_dn.value);

		if (NULL == (csi = avd_csi_get(&csi_dn)))
			break;

		/* check whether the SI of parent csi is in locked state */
		if ((csi->si->saAmfSIAdminState != SA_AMF_ADMIN_LOCKED) ||
		    (csi->si->list_of_sisu != NULL) || (csi->list_compcsi != NULL))
			break;

		/* check whether an attribute with this name already exists in csi
		   if exists, only then the modification of attr values allowed */
		if (!is_csiattr_valid(csi, (char *)&opdata->objectName.value))
			break;

		rc = SA_AIS_OK;
		break;
		}
	case CCBUTIL_DELETE:
		rc = SA_AIS_OK;
		break;
	default:
		assert(0);
		break;
	}

	TRACE_LEAVE();
	return rc;
}

static void csiattr_create_apply(CcbUtilOperationData_t *opdata)
{
	AVD_CSI_ATTR *csiattr;
	AVD_CSI *csi;

	csiattr = csiattr_create(&opdata->objectName, opdata->param.create.attrValues);
	csi = avd_csi_get(opdata->param.create.parentName);
	avd_csi_add_csiattr(csi, csiattr);
}

static void csiattr_modify_apply(CcbUtilOperationData_t *opdata)
{
	const SaImmAttrModificationT_2 *attr_mod;
	const SaImmAttrValuesT_2 *attribute;
	AVD_CSI_ATTR *csiattr = NULL, *i_attr;
	char *p1, *p2;
	int i;
	AVD_CSI *csi;

	csi = avd_csi_get(opdata->param.create.parentName);

	attr_mod = opdata->param.modify.attrMods[0];
	attribute = &attr_mod->modAttr;

	p2 = strchr((char *)&opdata->objectName.value, ',');
	*p2 = '\0';
	p1 = strchr((char *)&opdata->objectName.value, '=');
	p1++;

	/* create new name-value pairs for the modified csi attribute */
	for (i = 0; i < attribute->attrValuesNumber; i++) {
		char *value = attribute->attrValues[i++];
		if ((i_attr = calloc(1, sizeof(AVD_CSI_ATTR))) == NULL) {
			LOG_ER("%s:calloc FAILED", __FUNCTION__);
			return;
		}

		i_attr->attr_next = csiattr;
		csiattr = i_attr;

		csiattr->name_value.name.length = p2 - p1;
		memcpy(csiattr->name_value.name.value, p1, csiattr->name_value.name.length);
		csiattr->name_value.value.length = strlen (value);
		memcpy(csiattr->name_value.value.value, value, csiattr->name_value.value.length );
	}

	/* add the modified csiattr values to parent csi */
	avd_csi_add_csiattr(csi, csiattr);
}

static void csiattr_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		csiattr_create_apply(opdata);
		break;
	case CCBUTIL_DELETE:
		break;
	case CCBUTIL_MODIFY:
		csiattr_modify_apply(opdata);
		break;
	default:
		assert(0);
		break;
	}

	TRACE_LEAVE();
}

void avd_csiattr_constructor(void)
{
	avd_class_impl_set("SaAmfCSIAttribute", NULL, NULL, csiattr_ccb_completed_cb, csiattr_ccb_apply_cb);
}

