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
  the component type database on the AVD.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"

#include <saImmOm.h>
#include <immutil.h>
#include "avd_imm.h"

static SaAisErrorT avd_ctcstype_config_get(const SaNameT *comp_type_dn, AVD_COMP_TYPE *comp_type);

/* Global variable for the singleton object used by comp class */
AVD_COMP_GLOBALATTR avd_comp_global_attrs;

static NCS_PATRICIA_TREE avd_comptype_db;
static NCS_PATRICIA_TREE avd_ctcstype_db;

/*****************************************************************************
 * Function: avd_comptype_find
 *
 * Purpose:  This function will find a AVD_AMF_COMP_TYPE structure in the
 *           tree with comp_type_name value as key.
 *
 * Input: dn - The name of the comp_type_name.
 *        
 * Returns: The pointer to AVD_AMF_COMP_TYPE structure found in the tree.
 *
 * NOTES:
 *
 *
 **************************************************************************/

AVD_COMP_TYPE *avd_comptype_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_COMP_TYPE *)ncs_patricia_tree_get(&avd_comptype_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_comptype_delete
 *
 * Purpose:  This function will delete a AVD_AMF_COMP_TYPE structure from the
 *           tree. 
 *
 * Input: comp_type - The comp_type structure that needs to be deleted.
 *
 * Returns: -
 *
 * NOTES:
 *
 * 
 **************************************************************************/
static void avd_comptype_delete(AVD_COMP_TYPE *avd_comp_type)
{
	unsigned int rc;

	assert(NULL == avd_comp_type->list_of_comp);

	rc = ncs_patricia_tree_del(&avd_comptype_db, &avd_comp_type->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);

	free(avd_comp_type);
}

void avd_comptype_add_comp_list(AVD_COMP *comp)
{
	comp->comp_type_list_comp_next = comp->comp_type->list_of_comp;
	comp->comp_type->list_of_comp = comp;
}

/*****************************************************************************
 * Function: avd_comp_del_comp_type_list
 *
 * Purpose:  This function will del the given comp from comp_type list.
 *
 * Input: comp - The comp pointer
 *
 * Returns: None.
 *
 * NOTES: None
 *
 *
 **************************************************************************/
void avd_comptype_delete_comp_list(AVD_COMP *comp)
{
	AVD_COMP *i_comp = NULL;
	AVD_COMP *prev_comp = NULL;

	if (comp->comp_type != NULL) {
		i_comp = comp->comp_type->list_of_comp;

		while ((i_comp != NULL) && (i_comp != comp)) {
			prev_comp = i_comp;
			i_comp = i_comp->comp_type_list_comp_next;
		}

		if (i_comp != comp) {
			/* Log a fatal error */
			assert(0);
		} else {
			if (prev_comp == NULL) {
				comp->comp_type->list_of_comp = comp->comp_type_list_comp_next;
			} else {
				prev_comp->comp_type_list_comp_next = comp->comp_type_list_comp_next;
			}
		}

		comp->comp_type_list_comp_next = NULL;
		comp->comp_type = NULL;
	}
}

/*****************************************************************************
 * Function: avd_comptype_create
 * 
 * Purpose: This routine creates new SaAmfCompType objects.
 * 
 *
 * Input  : Ccb Util Oper Data
 *  
 * Returns: Pointer to Comp Type structure.
 *  
 * NOTES  : None.
 *
 *
 **************************************************************************/
static AVD_COMP_TYPE *avd_comptype_create(const SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	AVD_COMP_TYPE *compt;
	int rc = -1;
	const char *str;

	if ((compt = calloc(1, sizeof(AVD_COMP_TYPE))) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "calloc FAILED");
		return NULL;
	}

	memcpy(compt->name.value, dn->value, dn->length);
	compt->name.length = dn->length;
	compt->tree_node.key_info = (uns8 *)&(compt->name);

	if (immutil_getAttr("saAmfCtCompCategory", attributes, 0, &compt->saAmfCtCompCategory) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfCtCompCategory  FAILED for '%s'", dn->value);
		goto done;
	}

	(void)immutil_getAttr("saAmfCtSwBundle", attributes, 0, &compt->saAmfCtSwBundle);
	if ((str = immutil_getStringAttr(attributes, "saAmfCtDefCmdEnv", 0)) != NULL)
		strcpy(compt->saAmfCtDefCmdEnv, str);
	(void)immutil_getAttr("saAmfCtDefClcCliTimeout", attributes, 0, &compt->saAmfCtDefClcCliTimeout);
	(void)immutil_getAttr("saAmfCtDefCallbackTimeout", attributes, 0, &compt->saAmfCtDefCallbackTimeout);

	if ((str = immutil_getStringAttr(attributes, "saAmfCtRelPathInstantiateCmd", 0)) != NULL)
		strcpy(compt->saAmfCtRelPathInstantiateCmd, str);
	if ((str = immutil_getStringAttr(attributes, "saAmfCtDefInstantiateCmdArgv", 0)) != NULL)
		strcpy(compt->saAmfCtDefInstantiateCmdArgv, str);

	(void)immutil_getAttr("saAmfCtDefInstantiationLevel", attributes, 0, &compt->saAmfCtDefInstantiationLevel);

	if ((str = immutil_getStringAttr(attributes, "saAmfCtRelPathTerminateCmd", 0)) != NULL)
		strcpy(compt->saAmfCtRelPathTerminateCmd, str);
	if ((str = immutil_getStringAttr(attributes, "saAmfCtDefTerminateCmdArgv", 0)) != NULL)
		strcpy(compt->saAmfCtDefTerminateCmdArgv, str);
	if ((str = immutil_getStringAttr(attributes, "saAmfCtRelPathCleanupCmd", 0)) != NULL)
		strcpy(compt->saAmfCtRelPathCleanupCmd, str);
	if ((str = immutil_getStringAttr(attributes, "saAmfCtDefCleanupCmdArgv", 0)) != NULL)
		strcpy(compt->saAmfCtDefCleanupCmdArgv, str);
	if ((str = immutil_getStringAttr(attributes, "saAmfCtRelPathAmStartCmd", 0)) != NULL)
		strcpy(compt->saAmfCtRelPathAmStartCmd, str);
	if ((str = immutil_getStringAttr(attributes, "saAmfCtDefAmStartCmdArgv", 0)) != NULL)
		strcpy(compt->saAmfCtDefAmStartCmdArgv, str);
	if ((str = immutil_getStringAttr(attributes, "saAmfCtRelPathAmStopCmd", 0)) != NULL)
		strcpy(compt->saAmfCtRelPathAmStopCmd, str);
	if ((str = immutil_getStringAttr(attributes, "saAmfCtDefAmStopCmdArgv", 0)) != NULL)
		strcpy(compt->saAmfCtDefAmStopCmdArgv, str);

	(void)immutil_getAttr("saAmfCtDefQuiesingCompleteTimeout", attributes, 0,
			      &compt->saAmfCompQuiescingCompleteTimeout);

	if (immutil_getAttr("saAmfCtDefRecoveryOnError", attributes, 0, &compt->saAmfCtDefRecoveryOnError) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfCtDefRecoveryOnError FAILED for '%s'", dn->value);
		goto done;
	}

	(void)immutil_getAttr("saAmfCtDefDisableRestart", attributes, 0, &compt->saAmfCtDefDisableRestart);

	rc = ncs_patricia_tree_add(&avd_comptype_db, &compt->tree_node);
	assert (rc == NCSCC_RC_SUCCESS);
	rc = 0;

 done:
	if (rc != 0) {
		free(compt);
		compt = NULL;
	}

	return compt;
}

/*****************************************************************************
 * Function: avd_comptype_config_validate
 * 
 * Purpose: This routine handles all CCB operations on SaAmfCompType objects.
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
static int avd_comptype_config_validate(AVD_COMP_TYPE *comp_type)
{
	char *parent;
	char *dn = (char *)comp_type->name.value;

	if ((parent = strchr(dn, ',')) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "No parent to '%s' ", dn);
		return -1;
	}

	parent++;

	/* Should be children to the Comp Base type */
	if (strncmp(parent, "safCompType=", 12) != 0) {
		avd_log(NCSFL_SEV_ERROR, "Wrong parent '%s' to '%s' ", parent, dn);
		return -1;
	}

	/* We do not support Proxy, Container and Contained as of now. */
	if (((comp_type->saAmfCtCompCategory & SA_AMF_COMP_PROXY) ||
	     (comp_type->saAmfCtCompCategory & SA_AMF_COMP_CONTAINER) ||
	     (comp_type->saAmfCtCompCategory & SA_AMF_COMP_CONTAINED))) {

		avd_log(NCSFL_SEV_ERROR, "Wrong saAmfCtCompCategory value '%u'", comp_type->saAmfCtCompCategory);
		return -1;
	}

	if (comp_type->saAmfCtDefRecoveryOnError > SA_AMF_CONTAINER_RESTART) {
		avd_log(NCSFL_SEV_ERROR, "Wrong saAmfCtDefRecoveryOnError value '%u'",
			comp_type->saAmfCtDefRecoveryOnError);
		return -1;
	}

	return 0;
}

/**
 * Get configuration for all SaAmfCompType objects from IMM and
 * create AVD internal objects.
 * @param cb
 * 
 * @return int
 */
SaAisErrorT avd_comptype_config_get(void)
{
	SaAisErrorT error = SA_AIS_ERR_FAILED_OPERATION;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfCompType";
	AVD_COMP_TYPE *comp_type;

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, NULL,
						  SA_IMM_SUBTREE, SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR,
						  &searchParam, NULL, &searchHandle);

	if (SA_AIS_OK != error) {
		avd_log(NCSFL_SEV_ERROR, "saImmOmSearchInitialize_2 failed: %u", error);
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {

		avd_log(NCSFL_SEV_NOTICE, "'%s'", dn.value);

		if ((comp_type = avd_comptype_create(&dn, attributes)) == NULL)
			goto done2;

		if (avd_comptype_config_validate(comp_type) != 0)
			goto done2;

		if (avd_ctcstype_config_get(&dn, comp_type) != SA_AIS_OK)
			goto done2;
	}

	error = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:
	return error;
}

/*****************************************************************************
 * Function: avd_comptype_ccb_apply_delete_hdlr
 * 
 * Purpose: This routine handles delete operations on SaAmfCompType objects.
 * 
 *
 * Input  : Ccb Util Oper Data
 *  
 * Returns: None.
 *  
 ****************************************************************************/
static void avd_comptype_ccb_apply_delete_hdlr(CcbUtilOperationData_t *opdata)
{
	AVD_COMP_TYPE *avd_comp_type;

	avd_comp_type = avd_comptype_find(&opdata->objectName);
	assert(avd_comp_type != NULL);

	avd_comptype_delete(avd_comp_type);
}

/*****************************************************************************
 * Function: avd_comptype_ccb_apply_cb
 * 
 * Purpose: This routine handles all CCB apply operations on SaAmfCompType objects.
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
static void avd_comptype_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		break;
	case CCBUTIL_DELETE:
		avd_comptype_ccb_apply_delete_hdlr(opdata);
		break;
	default:
		assert(0);
		break;
	}
}

/*****************************************************************************
 * Function: avd_comptype_ccb_completed_cb
 * 
 * Purpose: This routine handles all CCB operations on SaAmfCompType objects.
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
static SaAisErrorT avd_comptype_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_COMP_TYPE *comp_type;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if ((comp_type = avd_comptype_create(&opdata->objectName,
		     opdata->param.create.attrValues)) == NULL) {
			goto done;
		}

		if (avd_comptype_config_validate(comp_type) != 0) {
			goto done;
		}

		opdata->userData = comp_type;	/* Save for later use in apply */
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfCompType not supported");
		break;
	case CCBUTIL_DELETE:
		comp_type = avd_comptype_find(&opdata->objectName);
		if (NULL != comp_type->list_of_comp) {
			avd_log(NCSFL_SEV_ERROR, "SaAmfCompType is in use");
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

void avd_comptype_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	assert(ncs_patricia_tree_init(&avd_comptype_db, &patricia_params) == NCSCC_RC_SUCCESS);

	avd_class_impl_set("SaAmfCompBaseType", NULL, NULL, avd_imm_default_OK_completed_cb, NULL);
	avd_class_impl_set("SaAmfCompType", NULL, NULL, avd_comptype_ccb_completed_cb, avd_comptype_ccb_apply_cb);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

SaAisErrorT avd_compglobalattrs_config_get(void)
{
	SaAisErrorT rc;
	const SaImmAttrValuesT_2 **attributes;
	SaImmAccessorHandleT accessorHandle;
	SaNameT dn = {.value = "safRdn=compGlobalAttributes,safApp=safAmfService" };

	dn.length = strlen((char *)dn.value);

	immutil_saImmOmAccessorInitialize(avd_cb->immOmHandle, &accessorHandle);
	rc = immutil_saImmOmAccessorGet_2(accessorHandle, &dn, NULL, (SaImmAttrValuesT_2 ***)&attributes);
	if (rc != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "saImmOmAccessorGet_2 FAILED %u", rc);
		rc = SA_AIS_ERR_FAILED_OPERATION;
		goto done;
	}

	avd_log(NCSFL_SEV_NOTICE, "'%s'", dn.value);

	if (immutil_getAttr("saAmfNumMaxInstantiateWithoutDelay", attributes, 0,
			    &avd_comp_global_attrs.saAmfNumMaxInstantiateWithoutDelay) != SA_AIS_OK) {
		avd_comp_global_attrs.saAmfNumMaxInstantiateWithoutDelay = 2;
	}

	if (immutil_getAttr("saAmfNumMaxInstantiateWithDelay", attributes, 0,
			    &avd_comp_global_attrs.saAmfNumMaxInstantiateWithDelay) != SA_AIS_OK) {
		avd_comp_global_attrs.saAmfNumMaxInstantiateWithDelay = 0;
	}

	if (immutil_getAttr("saAmfNumMaxAmStartAttempts", attributes, 0,
			    &avd_comp_global_attrs.saAmfNumMaxAmStartAttempts) != SA_AIS_OK) {
		avd_comp_global_attrs.saAmfNumMaxAmStartAttempts = 2;
	}

	if (immutil_getAttr("saAmfNumMaxAmStopAttempts", attributes, 0,
			    &avd_comp_global_attrs.saAmfNumMaxAmStopAttempts) != SA_AIS_OK) {
		avd_comp_global_attrs.saAmfNumMaxAmStopAttempts = 2;
	}

	if (immutil_getAttr("saAmfDelayBetweenInstantiateAttempts", attributes, 0,
			    &avd_comp_global_attrs.saAmfDelayBetweenInstantiateAttempts) != SA_AIS_OK) {
		avd_comp_global_attrs.saAmfDelayBetweenInstantiateAttempts = 0;
	}

	immutil_saImmOmAccessorFinalize(accessorHandle);

	rc = SA_AIS_OK;

 done:
	return rc;
}

static void avd_compglobalattrs_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	int i = 0;
	const SaImmAttrModificationT_2 *attrMod;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_MODIFY:
		while ((attrMod = opdata->param.modify.attrMods[i++]) != NULL) {
			if (!strcmp("saAmfNumMaxInstantiateWithoutDelay", attrMod->modAttr.attrName)) {
				avd_comp_global_attrs.saAmfNumMaxInstantiateWithoutDelay =
				    *((SaUint32T *)attrMod->modAttr.attrValues[0]);
			}
			if (!strcmp("saAmfNumMaxInstantiateWithDelay", attrMod->modAttr.attrName)) {
				avd_comp_global_attrs.saAmfNumMaxInstantiateWithDelay =
				    *((SaUint32T *)attrMod->modAttr.attrValues[0]);
			}
			if (!strcmp("saAmfNumMaxAmStartAttempts", attrMod->modAttr.attrName)) {
				avd_comp_global_attrs.saAmfNumMaxAmStartAttempts =
				    *((SaUint32T *)attrMod->modAttr.attrValues[0]);
			}
			if (!strcmp("saAmfNumMaxAmStopAttempts", attrMod->modAttr.attrName)) {
				avd_comp_global_attrs.saAmfNumMaxAmStopAttempts =
				    *((SaUint32T *)attrMod->modAttr.attrValues[0]);
			}
			if (!strcmp("saAmfDelayBetweenInstantiateAttempts", attrMod->modAttr.attrName)) {
				avd_comp_global_attrs.saAmfDelayBetweenInstantiateAttempts =
				    *((SaTimeT *)attrMod->modAttr.attrValues[0]);
			}
		}
		break;
	default:
		assert(0);
		break;
	}
}

static SaAisErrorT avd_compglobalattrs_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		avd_log(NCSFL_SEV_ERROR, "SaAmfCompGlobalAttributes already created");
		break;
	case CCBUTIL_MODIFY:
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_DELETE:
		avd_log(NCSFL_SEV_ERROR, "SaAmfCompGlobalAttributes cannot be deleted");
		break;
	default:
		assert(0);
		break;
	}

	return rc;
}

__attribute__ ((constructor))
static void avd_compglobalattrs_constructor(void)
{
	avd_class_impl_set("SaAmfCompGlobalAttributes", NULL,
			       NULL, avd_compglobalattrs_ccb_completed_cb, avd_compglobalattrs_ccb_apply_cb);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

static int avd_ctcstype_config_validate(const AVD_CTCS_TYPE *ctcstype)
{
	char *parent;
	char *dn = (char *)ctcstype->name.value;

	/* Second comma should be parent */
	if ((parent = strchr(dn, ',')) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "No parent to '%s' ", dn);
		return -1;
	}

	parent++;

	if ((parent = strchr(parent, ',')) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "No parent to '%s' ", dn);
		return -1;
	}

	parent++;

	/* Should be children to SaAmfCompType */
	if (strncmp(parent, "safVersion=", 11) != 0) {
		avd_log(NCSFL_SEV_ERROR, "Wrong parent '%s'", parent);
		return -1;
	}

	if (ctcstype->saAmfCtCompCapability > SA_AMF_COMP_NON_PRE_INSTANTIABLE) {
		avd_log(NCSFL_SEV_ERROR, "Wrong saAmfCtCompCapability value '%u'", ctcstype->saAmfCtCompCapability);
		return -1;
	}

	return 0;
}

static AVD_CTCS_TYPE *avd_ctcstype_create(const SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	AVD_CTCS_TYPE *ctcstype;
	int rc = -1;

	if ((ctcstype = calloc(1, sizeof(AVD_CTCS_TYPE))) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "calloc FAILED");
		return NULL;
	}

	memcpy(ctcstype->name.value, dn->value, dn->length);
	ctcstype->name.length = dn->length;
	ctcstype->tree_node.key_info = (uns8 *)&(ctcstype->name);

	if (immutil_getAttr("saAmfCtCompCapability", attributes, 0, &ctcstype->saAmfCtCompCapability) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfCtCompCapability  FAILED for '%s'", dn->value);
		goto done;
	}

	if (immutil_getAttr("saAmfCtDefNumMaxActiveCSIs", attributes, 0, &ctcstype->saAmfCtDefNumMaxActiveCSIs) !=
	    SA_AIS_OK)
		ctcstype->saAmfCtDefNumMaxActiveCSIs = -1;	/* no limit */

	if (immutil_getAttr("saAmfCtDefNumMaxStandbyCSIs", attributes, 0, &ctcstype->saAmfCtDefNumMaxStandbyCSIs) !=
	    SA_AIS_OK)
		ctcstype->saAmfCtDefNumMaxStandbyCSIs = -1;	/* no limit */

	rc = ncs_patricia_tree_add(&avd_ctcstype_db, &ctcstype->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	rc = 0;

 done:
	if (rc != 0) {
		free(ctcstype);
		ctcstype = NULL;
	}

	return ctcstype;
}

static void avd_ctcstype_delete(AVD_CTCS_TYPE *ctcstype)
{
	unsigned int rc;

	avd_trace("'%s'", ctcstype->name.value);
	rc = ncs_patricia_tree_del(&avd_ctcstype_db, &ctcstype->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	free(ctcstype);
}

AVD_CTCS_TYPE *avd_ctcstype_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_CTCS_TYPE *)ncs_patricia_tree_get(&avd_ctcstype_db, (uns8 *)&tmp);
}

static SaAisErrorT avd_ctcstype_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	AVD_CTCS_TYPE *ctcstype;
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		ctcstype = avd_ctcstype_create(&opdata->objectName, opdata->param.create.attrValues);

		if (ctcstype == NULL)
			goto done;

		if (avd_ctcstype_config_validate(ctcstype) != 0) {
			avd_ctcstype_delete(ctcstype);
			goto done;
		}

		opdata->userData = ctcstype;
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfCtCsType not supported");
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

static void avd_ctcstype_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		break;
	case CCBUTIL_DELETE:{
		AVD_CTCS_TYPE *ctcstype = avd_ctcstype_find(&opdata->objectName);
		avd_ctcstype_delete(ctcstype);
		break;
	}
	default:
		assert(0);
		break;
	}
}

static SaAisErrorT avd_ctcstype_config_get(const SaNameT *comp_type_dn, AVD_COMP_TYPE *comp_type)
{
	SaAisErrorT error = SA_AIS_ERR_FAILED_OPERATION;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfCtCsType";
	AVD_CTCS_TYPE *ctcstype;

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	if (immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, comp_type_dn,
					      SA_IMM_SUBTREE, SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR,
					      &searchParam, NULL, &searchHandle) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "saImmOmSearchInitialize_2 failed: %u", error);
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {

		avd_log(NCSFL_SEV_NOTICE, "'%s'", dn.value);

		if ((ctcstype = avd_ctcstype_create(&dn, attributes)) == NULL)
			goto done2;

		if (avd_ctcstype_config_validate(ctcstype) != 0) {
			avd_ctcstype_delete(ctcstype);
			goto done2;
		}
	}

	error = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:

	return error;
}

void avd_ctcstype_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	assert(ncs_patricia_tree_init(&avd_ctcstype_db, &patricia_params) == NCSCC_RC_SUCCESS);

	avd_class_impl_set("SaAmfCtCsType", NULL, NULL, avd_ctcstype_ccb_completed_cb, avd_ctcstype_ccb_apply_cb);
}
