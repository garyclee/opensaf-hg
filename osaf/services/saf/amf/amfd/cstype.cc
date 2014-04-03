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

#include <logtrace.h>

#include <util.h>
#include <csi.h>
#include <imm.h>

static NCS_PATRICIA_TREE cstype_db;

//
// TODO(HANO) Temporary use this function instead of strdup which uses malloc.
// Later on remove this function and use std::string instead
#include <cstring>
static char *StrDup(const char *s)
{
	char *c = new char[strlen(s) + 1];
	std::strcpy(c,s);
	return c;
}

static void cstype_add_to_model(avd_cstype_t *cst)
{
	unsigned int rc = ncs_patricia_tree_add(&cstype_db, &cst->tree_node);
	osafassert(rc == NCSCC_RC_SUCCESS);
}

static avd_cstype_t *cstype_create(const SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	avd_cstype_t *cst;
	SaUint32T values_number;

	TRACE_ENTER2("'%s'", dn->value);

	cst = new avd_cstype_t();

	memcpy(cst->name.value, dn->value, dn->length);
	cst->name.length = dn->length;
	cst->tree_node.key_info = (uint8_t *)&cst->name;

	if ((immutil_getAttrValuesNumber(const_cast<SaImmAttrNameT>("saAmfCSAttrName"), attributes, &values_number) == SA_AIS_OK) &&
	    (values_number > 0)) {
		unsigned int i;

		cst->saAmfCSAttrName = new SaStringT[values_number + 1]();
		for (i = 0; i < values_number; i++)
			cst->saAmfCSAttrName[i] = StrDup(immutil_getStringAttr(attributes, "saAmfCSAttrName", i));
	}

	return cst;
}

/**
 * Delete from DB and return memory
 * @param cst
 */
static void cstype_delete(avd_cstype_t *cst)
{
	unsigned int rc;
	char *p;
	int i = 0;

	rc = ncs_patricia_tree_del(&cstype_db, &cst->tree_node);
	osafassert(rc == NCSCC_RC_SUCCESS);

	if (cst->saAmfCSAttrName != NULL) {
		while ((p = cst->saAmfCSAttrName[i++]) != NULL) {
			delete [] p;
		}
	}

	delete [] cst->saAmfCSAttrName;
	delete cst;
}

/**
 * Lookup object using name in DB
 * @param dn
 * 
 * @return avd_cstype_t*
 */
avd_cstype_t *avd_cstype_get(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (avd_cstype_t *)ncs_patricia_tree_get(&cstype_db, (uint8_t *)&tmp);
}

void avd_cstype_add_csi(AVD_CSI *csi)
{
	csi->csi_list_cs_type_next = csi->cstype->list_of_csi;
	csi->cstype->list_of_csi = csi;
}

void avd_cstype_remove_csi(AVD_CSI *csi)
{
	AVD_CSI *i_csi;
	AVD_CSI *prev_csi = NULL;

	if (csi->cstype != NULL) {
		i_csi = csi->cstype->list_of_csi;

		while ((i_csi != NULL) && (i_csi != csi)) {
			prev_csi = i_csi;
			i_csi = i_csi->csi_list_cs_type_next;
		}

		if (i_csi != csi) {
			/* Log a fatal error */
			osafassert(0);
		} else {
			if (prev_csi == NULL) {
				csi->cstype->list_of_csi = csi->csi_list_cs_type_next;
			} else {
				prev_csi->csi_list_cs_type_next = csi->csi_list_cs_type_next;
			}
		}

		csi->csi_list_cs_type_next = NULL;
		csi->cstype = NULL;
	}
}

static int is_config_valid(const SaNameT *dn, CcbUtilOperationData_t *opdata)
{
	char *parent;

	if ((parent = strchr((char*)dn->value, ',')) == NULL) {
		report_ccb_validation_error(opdata, "No parent to '%s' ", dn->value);
		return 0;
	}

	/* Should be children to the Comp Base type */
	if (strncmp(++parent, "safCSType=", 10) != 0) {
		report_ccb_validation_error(opdata, "Wrong parent '%s' to '%s' ", parent, dn->value);
		return 0;
	}

	return 1;
}

/**
 * Get configuration for all SaAmfCSType objects from IMM and create internal objects.
 * 
 * 
 * @return int
 */
SaAisErrorT avd_cstype_config_get(void)
{
	SaAisErrorT error = SA_AIS_ERR_FAILED_OPERATION;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfCSType";
	avd_cstype_t *cst;

	TRACE_ENTER();

	searchParam.searchOneAttr.attrName = const_cast<SaImmAttrNameT>("SaImmAttrClassName");
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	if (immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, NULL, SA_IMM_SUBTREE,
		SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam, NULL, &searchHandle) != SA_AIS_OK) {
		LOG_ER("saImmOmSearchInitialize_2 failed: %u", error);
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {
		if (!is_config_valid(&dn, NULL))
			goto done2;

		if ((cst = avd_cstype_get(&dn)) == NULL){
			if ((cst = cstype_create(&dn, attributes)) == NULL)
				goto done2;

			cstype_add_to_model(cst);
		}
	}

	error = SA_AIS_OK;

done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
done1:
	TRACE_LEAVE();
	return error;
}

/**
 * Handle a CCB completed event for SaAmfCSType
 * @param opdata
 * 
 * @return SaAisErrorT
 */
static SaAisErrorT cstype_ccb_completed_hdlr(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	avd_cstype_t *cst;
	AVD_CSI *csi; 
	bool csi_exist = false;
	CcbUtilOperationData_t *t_opData;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if (is_config_valid(&opdata->objectName, opdata))
			rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		report_ccb_validation_error(opdata, "Modification of SaAmfCSType not supported");
		break;
	case CCBUTIL_DELETE:
		cst = avd_cstype_get(&opdata->objectName);
		if (cst->list_of_csi != NULL) {
			/* check whether there exists a delete operation for 
			 * each of the CSI in the cs_type list in the current CCB 
			 */                      
			csi = cst->list_of_csi;
			while (csi != NULL) {  
				t_opData = ccbutil_getCcbOpDataByDN(opdata->ccbId, &csi->name);
				if ((t_opData == NULL) || (t_opData->operationType != CCBUTIL_DELETE)) {
					csi_exist = true;   
					break;                  
				}                       
				csi = csi->csi_list_cs_type_next;
			}                       
			if (csi_exist == true) {
				report_ccb_validation_error(opdata, "SaAmfCSType '%s' is in use", cst->name.value);
				goto done;
			}
		}
		opdata->userData = cst;	/* Save for later use in apply */
		rc = SA_AIS_OK;
		break;
	default:
		osafassert(0);
		break;
	}

done:
	return rc;
}

static void cstype_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_cstype_t *cst;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		cst = cstype_create(&opdata->objectName, opdata->param.create.attrValues);
		osafassert(cst);
		cstype_add_to_model(cst);
		break;
	case CCBUTIL_DELETE:
		cstype_delete(static_cast<avd_cstype_t*>(opdata->userData));
		break;
	default:
		osafassert(0);
		break;
	}
}

void avd_cstype_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	osafassert(ncs_patricia_tree_init(&cstype_db, &patricia_params) == NCSCC_RC_SUCCESS);

	avd_class_impl_set("SaAmfCSType", NULL, NULL, cstype_ccb_completed_hdlr,
		cstype_ccb_apply_cb);
	avd_class_impl_set("SaAmfCSBaseType", NULL, NULL,
		avd_imm_default_OK_completed_cb, NULL);
}
