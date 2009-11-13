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

/*****************************************************************************

  DESCRIPTION:This module deals with the creation, accessing and deletion of
  the CSI database and component CSI relationship list on the AVD.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <avd_dblog.h>
#include <avd_util.h>
#include <avsv_util.h>
#include <avd_csi.h>
#include <avd_pg.h>
#include <saImmOm.h>
#include <immutil.h>
#include <avd_cluster.h>
#include <avd_imm.h>

static NCS_PATRICIA_TREE avd_csi_db;
static NCS_PATRICIA_TREE avd_cstype_db;

/***************************************************************************************/
/**************************** Start class SaAmfCSType **********************************/
/***************************************************************************************/

/**
 * Allocate memory for a new SaAmfCSType object, initialize from attributes, add to DB.
 * @param dn
 * @param attributes
 * 
 * @return avd_cstype_t*
 */
static avd_cstype_t *avd_cstype_create(const SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	unsigned int rc;
	avd_cstype_t *cst;
	SaUint32T values_number;

	if ((cst = calloc(1, sizeof(*cst))) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "calloc failed");
		return NULL;
	}

	memcpy(cst->name.value, dn->value, dn->length);
	cst->name.length = dn->length;
	cst->tree_node.key_info = (uns8 *)&cst->name;

	if ((immutil_getAttrValuesNumber("saAmfCSAttrName", attributes, &values_number) == SA_AIS_OK) &&
	    (values_number > 0)) {
		int i;

		cst->saAmfCSAttrName = calloc((values_number + 1), sizeof(char *));
		for (i = 0; i < values_number; i++) {
			cst->saAmfCSAttrName[i] = strdup(immutil_getStringAttr(attributes, "saAmfCSAttrName", i));
		}
	}

	rc = ncs_patricia_tree_add(&avd_cstype_db, &cst->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	return cst;
}

/**
 * Delete from DB and return memory
 * @param cst
 */
static void avd_cstype_delete(avd_cstype_t *cst)
{
	unsigned int rc;
	char *p;
	int i = 0;

	rc = ncs_patricia_tree_del(&avd_cstype_db, &cst->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);

	while ((p = cst->saAmfCSAttrName[i++]) != NULL) {
		free(p);
	}

	free(cst->saAmfCSAttrName);
	free(cst);
}

/**
 * Lookup object using name in DB
 * @param dn
 * 
 * @return avd_cstype_t*
 */
static avd_cstype_t *avd_cstype_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (avd_cstype_t *)ncs_patricia_tree_get(&avd_cstype_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_cstype_csi_del_list
 *
 * Purpose:  This function delets the given csi from cs_type list
 * in the SaAmfCSType object.
 *
 * Input: csi - The csi pointer
 *
 * Returns: None.
 *
 * NOTES: None
 *
 *
 **************************************************************************/
static void avd_cstype_csi_del_list(AVD_CSI *csi)
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
			assert(0);
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

static int avd_cstype_config_validate(const avd_cstype_t *cst)
{
	char *parent;
	char *dn = (char *)cst->name.value;

	if ((parent = strchr(dn, ',')) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "No parent to '%s' ", dn);
		return -1;
	}

	parent++;

	/* Should be children to the CS Base type */
	if (strncmp(parent, "safCSType=", 10) != 0) {
		avd_log(NCSFL_SEV_ERROR, "Wrong parent '%s' to '%s' ", parent, dn);
		return -1;
	}

	return 0;
}

/**
 * Get configuration for all SaAmfCSType objects from IMM and create internal objects.
 * 
 * 
 * @return int
 */
SaAisErrorT avd_cstype_config_get(void)
{
	SaAisErrorT error;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfCSType";
	avd_cstype_t *cst;

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, NULL, SA_IMM_SUBTREE,
						  SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
						  NULL, &searchHandle);

	if (SA_AIS_OK != error) {
		avd_log(NCSFL_SEV_ERROR, "saImmOmSearchInitialize_2 failed: %u", error);
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {
		avd_log(NCSFL_SEV_NOTICE, "'%s'", dn.value);

		if ((cst = avd_cstype_create(&dn, attributes)) == NULL) {
			error = SA_AIS_ERR_FAILED_OPERATION;
			goto done2;
		}

		if (avd_cstype_config_validate(cst) != 0) {
			error = SA_AIS_ERR_FAILED_OPERATION;
			goto done2;
		}

	}

	error = SA_AIS_OK;

	if (avd_cstype_db.n_nodes == 0)
		avd_log(NCSFL_SEV_WARNING, "No CSType found");

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:

	return error;
}

/**
 * Handle a CCB completed event for SaAmfCSType
 * @param opdata
 * 
 * @return SaAisErrorT
 */
static SaAisErrorT avd_cstype_ccb_completed_hdlr(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	avd_cstype_t *cst;

	avd_log(NCSFL_SEV_NOTICE, "'%s', %llu", opdata->objectName.value, opdata->ccbId);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if ((cst = avd_cstype_create(&opdata->objectName, opdata->param.create.attrValues)) == NULL) {
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}

		if (avd_cstype_config_validate(cst) != 0) {
			rc = SA_AIS_ERR_BAD_OPERATION;
			avd_cstype_delete(cst);
			goto done;
		}

		opdata->userData = cst;	/* Save for later use in apply */
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfCSType not supported");
		break;
	case CCBUTIL_DELETE:
		cst = avd_cstype_find(&opdata->objectName);
		if (cst->list_of_csi != NULL) {
			avd_log(NCSFL_SEV_ERROR, "SaAmfCSType is in use");
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}
		rc = SA_AIS_OK;
		break;
	default:
		assert(0);
		break;
	}

 done:
	return rc;
}

/*****************************************************************************
 * Function: avd_cstype_ccb_apply_cb
 *
 * Purpose: This routine handles all CCB operations on SaAmfCSType objects.
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
static void avd_cstype_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "'%s', %llu", opdata->objectName.value, opdata->ccbId);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		break;
	case CCBUTIL_DELETE:
		avd_cstype_delete(opdata->userData);
		break;
	default:
		assert(0);
		break;
	}
}

/***************************************************************************************/
/**************************** End class SaAmfCSType ************************************/
/***************************************************************************************/

/***************************************************************************************/
/**************************** Start class SaAmfCSIAttribute ****************************/
/***************************************************************************************/

/*****************************************************************************
 * Function: avd_csiattr_del 
 *
 * Purpose:  This function will find and delete the CSI_ATTR in the list.
 *
 * Input: csi - The CSI in which the attr needs to be deleted.
          attr - The attr to be deleted.
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static void avd_csiattr_del(AVD_CSI *csi, AVD_CSI_ATTR *attr)
{
	AVD_CSI_ATTR *i_attr = NULL;
	AVD_CSI_ATTR *p_attr = NULL;

	/* remove ATTR from CSI list */
	i_attr = csi->list_attributes;

	while ((i_attr != NULL) && (i_attr != attr)) {
		p_attr = i_attr;
		i_attr = i_attr->attr_next;
	}

	if (i_attr != attr) {
		/* Log a fatal error */
		assert(0);
	} else {
		if (p_attr == NULL) {
			csi->list_attributes = i_attr->attr_next;
		} else {
			p_attr->attr_next = i_attr->attr_next;
		}
	}

	free(attr);
	csi->num_attributes--;
}

static void avd_csi_add_csiattr(AVD_CSI *csi, AVD_CSI_ATTR *csiattr)
{
	int cnt = 0;
	AVD_CSI_ATTR *ptr;

	/* Count number of attributes (multivalue) */
	ptr = csiattr;
	while (ptr != NULL) {
		cnt++;
		if (ptr->attr_next != NULL)
			ptr = ptr->attr_next;
		else
			break;
	}

	ptr->attr_next = csi->list_attributes;
	csi->list_attributes = csiattr;
#if 0
	if (csi->list_attributes == NULL) {
		csi->list_attributes = csiattr;
		goto done;
	}

	/* keep the list in CSI in ascending order */
	ptr = NULL;
	i_attr = csi->list_attributes;
	while ((i_attr != NULL) && (m_CMP_HORDER_SANAMET(i_attr->name_value.name, csiattr->name_value.name) < 0)) {
		ptr = i_attr;
		i_attr = i_attr->attr_next;
	}

	if (ptr == NULL) {
		csi->list_attributes = csiattr;
		csiattr->attr_next = i_attr;
	} else {
		ptr->attr_next = csiattr;
		csiattr->attr_next = i_attr;
	}
 done:
#endif
	csi->num_attributes += cnt;
}

static AVD_CSI_ATTR *avd_csiattr_create(const SaNameT *csiattr_name, const SaImmAttrValuesT_2 **attributes)
{
	AVD_CSI_ATTR *csiattr = NULL, *tmp;
	unsigned int values_number;

	/* Handle multi value attributes */
	if ((immutil_getAttrValuesNumber("saAmfCSIAttriValue", attributes, &values_number) == SA_AIS_OK) &&
	    (values_number > 0)) {
		int i;
		const char *value;

		for (i = 0; i < values_number; i++) {
			if ((tmp = calloc(1, sizeof(AVD_CSI_ATTR))) == NULL) {
				m_AVD_LOG_MEM_FAIL(AVD_CSI_ALLOC_FAILED);
				return NULL;
			}

			memcpy(tmp->name_value.name.value, csiattr_name->value, csiattr_name->length);
			tmp->name_value.name.length = csiattr_name->length;
			tmp->attr_next = csiattr;
			csiattr = tmp;

			if ((value = immutil_getStringAttr(attributes, "saAmfCSIAttriValue", i)) != NULL) {
				csiattr->name_value.name.length =
				    sprintf((char *)csiattr->name_value.name.value, "%s", csiattr_name->value);
				csiattr->name_value.value.length =
				    snprintf((char *)csiattr->name_value.value.value, SA_MAX_NAME_LENGTH, "%s", value);
			} else {
				/* Param exist but has no value */
				csiattr->name_value.value.length = 0;
			}
		}
	}

	if (csiattr == NULL) {
		/* No values found, create value empty attribute */
		if ((csiattr = calloc(1, sizeof(AVD_CSI_ATTR))) == NULL) {
			m_AVD_LOG_MEM_FAIL(AVD_CSI_ALLOC_FAILED);
			return NULL;
		}

		memcpy(csiattr->name_value.name.value, csiattr_name->value, csiattr_name->length);
		csiattr->name_value.name.length = csiattr_name->length;
	}

	return csiattr;
}

static AVD_CSI_ATTR *avd_csiattr_find(const AVD_CSI *csi, const SaNameT *csiattr_name)
{
	AVD_CSI_ATTR *csiattr = NULL;

	csiattr = csi->list_attributes;
	while (csiattr != NULL) {
		if (memcmp(csiattr_name, &csiattr->name_value.name, sizeof(SaNameT)) == 0)
			break;
		csiattr = csiattr->attr_next;
	}

	return csiattr;
}

static SaAisErrorT avd_csiattr_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_CSI_ATTR *csiattr;
	AVD_CSI *csi;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:{
			csi = avd_csi_find(opdata->param.create.parentName);
			if ((csiattr =
			     avd_csiattr_create(&opdata->objectName, opdata->param.create.attrValues)) == NULL) {
				rc = SA_AIS_ERR_NO_MEMORY;
				goto done;
			}

			opdata->userData = csiattr;	/* Save for later use in apply */
			rc = SA_AIS_OK;
			break;
		}
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfCSIAttribute not supported");
		break;
	case CCBUTIL_DELETE:
		rc = SA_AIS_OK;
		break;
	default:
		assert(0);
		break;
	}

 done:
	return rc;
}

static void avd_csiattr_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	AVD_CSI_ATTR *csiattr;
	AVD_CSI *csi = avd_csi_find(opdata->param.create.parentName);

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:{
			AVD_CSI *csi = avd_csi_find(opdata->param.create.parentName);
			avd_csi_add_csiattr(csi, opdata->userData);
			break;
		}
	case CCBUTIL_DELETE:
		while ((csiattr = avd_csiattr_find(csi, &opdata->objectName)) != NULL) {
			avd_csiattr_del(csi, csiattr);
		}
		break;
	default:
		assert(0);
		break;
	}
}

/**
 * Get configuration for the AMF CSI Attribute objects related
 * to this CSI from IMM and create AVD internal objects.
 * @param cb
 * 
 * @return int
 */
static SaAisErrorT avd_csiattr_config_get(const SaNameT *csi_name, AVD_CSI *csi)
{
	SaAisErrorT error;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT csiattr_name;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfCSIAttribute";
	AVD_CSI_ATTR *csiattr;

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	if ((error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, csi_name,
						       SA_IMM_SUBTREE,
						       SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR,
						       &searchParam, NULL, &searchHandle)) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "saImmOmSearchInitialize failed: %u", error);
		goto done1;
	}

	while ((error =
		immutil_saImmOmSearchNext_2(searchHandle, &csiattr_name,
					    (SaImmAttrValuesT_2 ***)&attributes)) == SA_AIS_OK) {
		avd_log(NCSFL_SEV_NOTICE, "'%s'", csiattr_name.value);

		if ((csiattr = avd_csiattr_create(&csiattr_name, attributes)) != NULL)
			avd_csi_add_csiattr(csi, csiattr);
	}

	error = SA_AIS_OK;

	(void)immutil_saImmOmSearchFinalize(searchHandle);

 done1:
	return error;
}

/***************************************************************************************/
/**************************** End class SaAmfCSIAttribute ******************************/
/***************************************************************************************/

/***************************************************************************************/
/**************************** Start class SaAmfCSI *************************************/
/***************************************************************************************/

/**
 * Add the CSI to the DB
 * @param csi
 */
static void avd_csi_add_to_model(AVD_CSI *csi)
{
	unsigned int rc;

	avd_trace("'%s'", csi->name.value);
	rc = ncs_patricia_tree_add(&avd_csi_db, &csi->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);

	avd_si_add_csi_list(avd_cb, csi);
}

/*****************************************************************************
 * Function: avd_csi_delete
 *
 * Purpose:  This function will delete and free AVD_CSI structure from 
 * the tree.
 *
 * Input: csi - The CSI structure that needs to be deleted.
 *
 * Returns: None
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static void avd_csi_delete(AVD_CSI *csi)
{
	AVD_CSI_ATTR *i_csi_attr;
	unsigned int rc;

	/* Delete CSI attributes */
	i_csi_attr = csi->list_attributes;
	while (i_csi_attr != NULL) {
		avd_csiattr_del(csi, i_csi_attr);
		i_csi_attr = csi->list_attributes;
	}

	/* Delete CSI from the CSType list */
	avd_cstype_csi_del_list(csi);

	rc = ncs_patricia_tree_del(&avd_csi_db, &csi->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	free(csi);
}

/*****************************************************************************
 * Function: avd_csi_find
 *
 * Purpose:  This function will find a AVD_CSI structure in the
 * tree with csi_name value as key.
 *
 * Input: dn - The name of the CSI.
 *        
 * Returns: The pointer to AVD_CSI structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_CSI *avd_csi_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);
	
	return (AVD_CSI *)ncs_patricia_tree_get(&avd_csi_db, (uns8 *)&tmp);
}

/**
 * Validate configuration attributes for an AMF CSI object
 * @param csi
 * 
 * @return int
 */
static int avd_csi_config_validate(const AVD_CSI *csi)
{
	if (csi->cstype == NULL) {
		avd_log(NCSFL_SEV_ERROR, "CSType '%s' not found", csi->name.value);
		return -1;
	}

	/* Verify that the SI can contain this component */
	{
		AVD_SVC_TYPE_CS_TYPE *svctypecstype;
		SaNameT svctypecstype_name;

		avd_create_association_class_dn(&csi->saAmfCSType, &csi->si->saAmfSvcType,
			"safMemberCSType", &svctypecstype_name);
		svctypecstype = avd_svctypecstypes_find(&svctypecstype_name);
		if (svctypecstype == NULL) {
			avd_log(NCSFL_SEV_ERROR, "Not found '%s'", svctypecstype_name.value);
			return -1;
		}

		if (svctypecstype->curr_num_csis == svctypecstype->saAmfSvcMaxNumCSIs) {
			avd_log(NCSFL_SEV_ERROR, "SI '%s' cannot contain more CSIs of this type '%s*",
				csi->si->name.value, csi->saAmfCSType.value);
			return -1;
		}
	}

	return 0;
}

static AVD_CSI *avd_csi_create(const SaNameT *csi_name, const SaImmAttrValuesT_2 **attributes, const SaNameT *si_name)
{
	int rc = -1;
	AVD_CSI *csi;
	unsigned int values_number;

	if ((csi = calloc(1, sizeof(*csi))) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "calloc FAILED");
		return NULL;
	}

	memcpy(csi->name.value, csi_name->value, csi_name->length);
	csi->name.length = csi_name->length;
	csi->tree_node.key_info = (uns8 *)&(csi->name);

	/* initialize the pg node-list */
	csi->pg_node_list.order = NCS_DBLIST_ANY_ORDER;
	csi->pg_node_list.cmp_cookie = avsv_dblist_uns32_cmp;
	csi->pg_node_list.free_cookie = 0;

	if (immutil_getAttr("saAmfCSType", attributes, 0, &csi->saAmfCSType) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfCSType FAILED for '%s'", csi_name->value);
		goto done;
	}

	if ((immutil_getAttrValuesNumber("saAmfCSIDependencies", attributes, &values_number) == SA_AIS_OK) &&
	    (values_number > 0)) {
		int i;

		csi->saAmfCSIDependencies = calloc((values_number + 1), sizeof(SaNameT *));
		for (i = 0; i < values_number; i++) {
			csi->saAmfCSIDependencies[i] = malloc(sizeof(SaNameT));
			if (immutil_getAttr("saAmfCSIDependencies", attributes, 0,
					    &csi->saAmfCSIDependencies[i]) != SA_AIS_OK) {
				avd_log(NCSFL_SEV_ERROR, "Get saAmfCSIDependencies FAILED for '%s'", csi_name->value);
				goto done;
			}
		}
	}

	csi->cstype = avd_cstype_find(&csi->saAmfCSType);
	csi->si = avd_si_find(si_name);

	rc = 0;

 done:
	if (rc != 0) {
		free(csi);
		csi = NULL;
	}
	return csi;
}

/**
 * Get configuration for all AMF CSI objects from IMM and create
 * AVD internal objects.
 * 
 * @param si_name
 * @param si
 * 
 * @return int
 */
SaAisErrorT avd_csi_config_get(const SaNameT *si_name, AVD_SI *si)
{
	SaAisErrorT error;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT csi_name;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfCSI";
	AVD_CSI *csi;

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, si_name, SA_IMM_SUBTREE,
						  SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
						  NULL, &searchHandle);

	if (SA_AIS_OK != error) {
		avd_log(NCSFL_SEV_ERROR, "saImmOmSearchInitialize_2 failed: %u", error);
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &csi_name, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {

		avd_log(NCSFL_SEV_NOTICE, "'%s'", csi_name.value);

		if ((csi = avd_csi_create(&csi_name, attributes, si_name)) == NULL) {
			error = SA_AIS_ERR_FAILED_OPERATION;
			goto done2;
		}

		if (avd_csi_config_validate(csi) != 0) {
			/*  TODO avd_csi_delete() */
			error = SA_AIS_ERR_FAILED_OPERATION;
			goto done2;
		}

		avd_csi_add_to_model(csi);

		if (avd_csiattr_config_get(&csi_name, csi) != SA_AIS_OK) {
			error = SA_AIS_ERR_FAILED_OPERATION;
			goto done2;
		}
	}

	error = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:

	return error;
}

/*****************************************************************************
 * Function: avd_csi_ccb_completed_cb
 * 
 * Purpose: This routine handles all CCB operations on SaAmfCSI objects.
 *
 * Input  : Ccb Util Oper Data
 *  
 * Returns: None.
 *  
 * NOTES  : None.
 *
 *
 **************************************************************************/
static SaAisErrorT avd_csi_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_CSI *csi;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if ((csi = avd_csi_create(&opdata->objectName, opdata->param.create.attrValues,
					  opdata->param.create.parentName)) == NULL) {
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}

		if (avd_csi_config_validate(csi) != 0) {
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		opdata->userData = csi;	/* Save for later use in apply */
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfCSI not supported");
		break;
	case CCBUTIL_DELETE:
		csi = avd_csi_find(&opdata->objectName);
		/* Check to see that the SI of which the CSI is a
		 * part is in admin locked state before
		 * making the row status as not in service or delete 
		 */

		if ((csi->si->saAmfSIAdminState != SA_AMF_ADMIN_UNLOCKED) ||
		    (csi->si->list_of_sisu != NULL) || (csi->list_compcsi != NULL)) {
			avd_log(NCSFL_SEV_ERROR, "SaAmfCSI is in use");
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}
		rc = SA_AIS_OK;
		break;
	default:
		assert(0);
		break;
	}

 done:
	return rc;
}

/*****************************************************************************
 * Function: avd_csi_ccb_apply_delete_hdlr
 *
 * Purpose: This routine handles delete operations on SaAmfCSI objects.
 *
 *
 * Input  : Ccb Util Oper Data.
 *
 * Returns: None.
 *
 ****************************************************************************/
static void avd_csi_ccb_apply_delete_hdlr(CcbUtilOperationData_t *opdata)
{
	AVD_CSI *avd_csi;
	AVD_PG_CSI_NODE *curr;

	avd_csi = avd_csi_find(&opdata->objectName);

	/* decrement the active csi number of this SI */
	avd_csi->si->num_csi--;

	/* inform the avnds that track this csi */
	for (curr = (AVD_PG_CSI_NODE *)m_NCS_DBLIST_FIND_FIRST(&avd_csi->pg_node_list);
	     curr != NULL; curr = (AVD_PG_CSI_NODE *)m_NCS_DBLIST_FIND_NEXT(&curr->csi_dll_node)) {

		avd_snd_pg_upd_msg(avd_cb, curr->node, 0, 0, &avd_csi->name);
	}

	m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

	/* remove the csi from the SI list.  */
	avd_si_del_csi_list(avd_cb, avd_csi);

	/* delete the pg-node list */
	avd_pg_csi_node_del_all(avd_cb, avd_csi);

	/* free memory and remove from DB */
	avd_csi_delete(avd_csi);

	m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);
}

/*****************************************************************************
 * Function: avd_csi_ccb_apply_cb
 *
 * Purpose: This routine handles all CCB operations on SaAmfCSI objects.
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
static void avd_csi_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		avd_csi_add_to_model(opdata->userData);
		break;
	case CCBUTIL_DELETE:
		avd_csi_ccb_apply_delete_hdlr(opdata);
		break;
	default:
		assert(0);
		break;
	}
}

/***************************************************************************************/
/**************************** End class SaAmfCSI ***************************************/
/***************************************************************************************/

/**
 * Create an SaAmfCSIAssignment runtime object in IMM.
 * @param ha_state
 * @param csi_dn
 * @param comp_dn
 */
static void avd_create_csiassignment_in_imm(SaAmfHAStateT ha_state,
       const SaNameT *csi_dn, const SaNameT *comp_dn)
{
       SaAisErrorT rc; 
       SaNameT dn;
       SaAmfReadinessStateT saAmfCSICompHAReadinessState = SA_AMF_READINESS_IN_SERVICE;
       void *arr1[] = { &dn };
       void *arr2[] = { &ha_state };
       void *arr3[] = { &saAmfCSICompHAReadinessState };
       const SaImmAttrValuesT_2 attr_safCSIComp = {
               .attrName = "safCSIComp",
               .attrValueType = SA_IMM_ATTR_SANAMET,
               .attrValuesNumber = 1,
               .attrValues = arr1
       };
       const SaImmAttrValuesT_2 attr_saAmfCSICompHAState = {
               .attrName = "saAmfCSICompHAState",
               .attrValueType = SA_IMM_ATTR_SAUINT32T,
               .attrValuesNumber = 1,
               .attrValues = arr2
       };
       const SaImmAttrValuesT_2 attr_saAmfCSICompHAReadinessState = {
               .attrName = "saAmfCSICompHAReadinessState",
               .attrValueType = SA_IMM_ATTR_SAUINT32T,
               .attrValuesNumber = 1,
               .attrValues = arr3
       };
       const SaImmAttrValuesT_2 *attrValues[] = {
               &attr_safCSIComp,
               &attr_saAmfCSICompHAState,
               &attr_saAmfCSICompHAReadinessState,
               NULL
       };

       if (avd_cb->avail_state_avd != SA_AMF_HA_ACTIVE)
               return;

       avd_create_association_class_dn(comp_dn, NULL, "safCSIComp", &dn);

       if ((rc = avd_saImmOiRtObjectCreate("SaAmfCSIAssignment", csi_dn, attrValues)) != SA_AIS_OK)
           avd_log(NCSFL_SEV_ERROR, "rc=%u, '%s'", rc, dn.value);
}

/*****************************************************************************
 * Function: avd_compcsi_create
 *
 * Purpose:  This function will create and add a AVD_COMP_CSI_REL structure
 * to the list of component csi relationships of a SU SI relationship.
 * It will add the element to the head of the list.
 *
 * Input: cb - the AVD control block
 *        susi - The SU SI relationship structure that encompasses this
 *               component CSI relationship.
 *        avd_csi - Pointer to CSI for SaAmfCSIAssignment object creation.
 *        avd_comp - Pointer to Comp for SaAmfCSIAssignment object creation.
 *
 * Returns: Pointer to AVD_COMP_CSI_REL structure or AVD_COMP_CSI_REL_NULL
 *          if failure.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_COMP_CSI_REL *avd_compcsi_create(AVD_CL_CB *cb, AVD_SU_SI_REL *susi, AVD_CSI *avd_csi, AVD_COMP *avd_comp)
{
	AVD_COMP_CSI_REL *comp_csi;

	/* Allocate a new block structure now
	 */
	if ((comp_csi = calloc(1, sizeof(AVD_COMP_CSI_REL))) == NULL) {
		/* log an error */
		m_AVD_LOG_MEM_FAIL(AVD_COMPCSI_ALLOC_FAILED);
		return NULL;
	}

	comp_csi->susi_csicomp_next = NULL;
	if (susi->list_of_csicomp == NULL) {
		susi->list_of_csicomp = comp_csi;
                goto done;
	}

	comp_csi->susi_csicomp_next = susi->list_of_csicomp;
	susi->list_of_csicomp = comp_csi;

done:
        if ((avd_csi == NULL) && (avd_comp == NULL)) {
		avd_log(NCSFL_SEV_ERROR, "Either csi or comp is NULL");
                return comp_csi;
	}

        avd_create_csiassignment_in_imm(susi->state, &avd_csi->name, &avd_comp->comp_info.name);

	return comp_csi;
}

/** Delete an SaAmfCSIAssignment from IMM
 * 
 * @param comp_dn
 * @param csi_dn
 */
static void avd_delete_csiassignment_from_imm(const SaNameT *comp_dn, const SaNameT *csi_dn)
{
       SaAisErrorT rc;
       SaNameT dn; 

       if (avd_cb->avail_state_avd != SA_AMF_HA_ACTIVE)
               return;

       avd_create_association_class_dn(comp_dn, csi_dn, "safCSIComp", &dn);

       if ((rc = avd_saImmOiRtObjectDelete(&dn)) != SA_AIS_OK)
               avd_log(NCSFL_SEV_ERROR, "rc=%u, '%s'", rc, dn.value);
}

/*****************************************************************************
 * Function: avd_compcsi_delete
 *
 * Purpose:  This function will delete and free all the AVD_COMP_CSI_REL
 * structure from the list_of_csicomp in the SUSI relationship
 * 
 * Input: cb - the AVD control block
 *        susi - The SU SI relationship structure that encompasses this
 *               component CSI relationship.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE .
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_compcsi_delete(AVD_CL_CB *cb, AVD_SU_SI_REL *susi, NCS_BOOL ckpt)
{
	AVD_COMP_CSI_REL *lcomp_csi;
	AVD_COMP_CSI_REL *i_compcsi, *prev_compcsi = NULL;

	while (susi->list_of_csicomp != NULL) {
		lcomp_csi = susi->list_of_csicomp;

		i_compcsi = lcomp_csi->csi->list_compcsi;
		while ((i_compcsi != NULL) && (i_compcsi != lcomp_csi)) {
			prev_compcsi = i_compcsi;
			i_compcsi = i_compcsi->csi_csicomp_next;
		}
		if (i_compcsi != lcomp_csi) {
			/* not found */
		} else {
			if (prev_compcsi == NULL) {
				lcomp_csi->csi->list_compcsi = i_compcsi->csi_csicomp_next;
			} else {
				prev_compcsi->csi_csicomp_next = i_compcsi->csi_csicomp_next;
			}
			lcomp_csi->csi->compcsi_cnt--;

			/* trigger pg upd */
			if (!ckpt) {
				avd_pg_compcsi_chg_prc(cb, lcomp_csi, TRUE);
			}

			i_compcsi->csi_csicomp_next = NULL;
		}

		susi->list_of_csicomp = lcomp_csi->susi_csicomp_next;
		lcomp_csi->susi_csicomp_next = NULL;
		avd_delete_csiassignment_from_imm(&lcomp_csi->comp->comp_info.name, &lcomp_csi->csi->name);
		free(lcomp_csi);

	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_si_add_csi_list
 *
 * Purpose:  This function will add the given CSI to the SI list, and fill
 * the CSIs pointers. 
 *
 * Input: cb - the AVD control block
 *        csi - The CSI pointer
 *
 * Returns: None. 
 *
 * NOTES: None
 *
 * 
 **************************************************************************/

void avd_si_add_csi_list(AVD_CL_CB *cb, AVD_CSI *csi)
{
	AVD_CSI *i_csi = NULL;
	AVD_CSI *prev_csi = NULL;

	assert((csi != NULL) && (csi->si != NULL));

	i_csi = csi->si->list_of_csi;

	while ((i_csi != NULL) && (csi->rank <= i_csi->rank)) {
		prev_csi = i_csi;
		i_csi = i_csi->si_list_of_csi_next;
	}

	if (prev_csi == NULL) {
		csi->si_list_of_csi_next = csi->si->list_of_csi;
		csi->si->list_of_csi = csi;
	} else {
		prev_csi->si_list_of_csi_next = csi;
		csi->si_list_of_csi_next = i_csi;
	}
}

/*****************************************************************************
 * Function: avd_si_del_csi_list
 *
 * Purpose:  This function will del the given CSI from the SI list, and fill
 * the CSIs pointer with NULL
 *
 * Input: cb - the AVD control block
 *        si - The SI pointer
 *
 * Returns: None. 
 *
 * NOTES: None
 *
 * 
 **************************************************************************/

void avd_si_del_csi_list(AVD_CL_CB *cb, AVD_CSI *csi)
{
	AVD_CSI *i_csi = NULL;
	AVD_CSI *prev_csi = NULL;

	if (csi->si != AVD_SI_NULL) {
		/* remove CSI from the SI */
		prev_csi = NULL;
		i_csi = csi->si->list_of_csi;

		while ((i_csi != NULL) && (i_csi != csi)) {
			prev_csi = i_csi;
			i_csi = i_csi->si_list_of_csi_next;
		}

		if (i_csi != csi) {
			/* Log a fatal error */
			assert(0);
		} else {
			if (prev_csi == NULL) {
				csi->si->list_of_csi = csi->si_list_of_csi_next;
			} else {
				prev_csi->si_list_of_csi_next = csi->si_list_of_csi_next;
			}
		}

		csi->si_list_of_csi_next = NULL;
		csi->si = AVD_SI_NULL;
	}			/* if (csi->si != AVD_SI_NULL) */
}

void avd_csi_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	assert(ncs_patricia_tree_init(&avd_cstype_db, &patricia_params) == NCSCC_RC_SUCCESS);
	assert(ncs_patricia_tree_init(&avd_csi_db, &patricia_params) == NCSCC_RC_SUCCESS);

	avd_class_impl_set("SaAmfCSI", NULL, NULL, avd_csi_ccb_completed_cb, avd_csi_ccb_apply_cb);
	avd_class_impl_set("SaAmfCSIAttribute", NULL, NULL, avd_csiattr_ccb_completed_cb, avd_csiattr_ccb_apply_cb);
	avd_class_impl_set("SaAmfCSType", NULL, NULL, avd_cstype_ccb_completed_hdlr, avd_cstype_ccb_apply_cb);
	avd_class_impl_set("SaAmfCSBaseType", NULL, NULL, avd_imm_default_OK_completed_cb, NULL);
}
