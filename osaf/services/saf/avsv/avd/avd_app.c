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

/*****************************************************************************
  DESCRIPTION: This module deals with the creation, accessing and deletion of
  the Application database on the AVD.
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <avd_app.h>
#include <avd_cluster.h>
#include <avd_dblog.h>
#include <avd_imm.h>
#include <avd_si.h>
#include <avd_util.h>

// TODO remove
extern int avd_admin_state_is_valid(SaAmfAdminStateT state);

static NCS_PATRICIA_TREE avd_app_db;
static NCS_PATRICIA_TREE avd_apptype_db;

/*****************************************************************************
 * Function: avd_apptype_find
 *
 * Purpose:  This function will find a AVD_AMF_APP_TYPE structure in the
 *           tree with app_type_name value as key.
 *
 * Input: dn - The name of the app_type_name.
 *
 * Returns: The pointer to AVD_AMF_APP_TYPE structure found in the tree.
 *
 * NOTES:
 *
 *
 **************************************************************************/

static AVD_APP_TYPE *avd_apptype_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_APP_TYPE *)ncs_patricia_tree_get(&avd_apptype_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_apptype_delete
 *
 * Purpose:  This function will delete a AVD_AMF_APP_TYPE structure from the
 *           tree. 
 *
 * Input: apptype - The app_type structure that needs to be deleted.
 *
 * Returns: -
 *
 * NOTES:
 *
 * 
 **************************************************************************/
static void avd_apptype_delete(AVD_APP_TYPE *apptype)
{
	unsigned int rc = ncs_patricia_tree_del(&avd_apptype_db, &apptype->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	free(apptype->sgAmfApptSGTypes);
	free(apptype);
}

static void avd_apptype_add_to_model(AVD_APP_TYPE *app_type)
{
	unsigned int rc;

	assert(app_type != NULL);
	avd_trace("'%s'", app_type->name.value);
	rc = ncs_patricia_tree_add(&avd_apptype_db, &app_type->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
}

static int avd_apptype_config_validate(const AVD_APP_TYPE *app_type, const CcbUtilOperationData_t *opdata)
{
	int i;
	char *parent;
	char *dn = (char *)app_type->name.value;

	if ((parent = strchr(dn, ',')) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "No parent to '%s' ", dn);
		return -1;
	}

	parent++;

	/* AppType should be children to AppBaseType */
	if (strncmp(parent, "safAppType=", 11) != 0) {
		avd_log(NCSFL_SEV_ERROR, "Wrong parent '%s' to '%s' ", parent, dn);
		return -1;
	}

	if ((opdata != NULL) && (strncmp((char *)opdata->param.create.parentName->value, "safAppType=", 11) != 0)) {
		avd_log(NCSFL_SEV_ERROR, "Wrong parent '%s' to '%s' ",
			opdata->param.create.parentName->value, opdata->objectName.value);
		return -1;
	}

	/* Make sure all SG types exist */
	for (i = 0; i < app_type->no_sg_types; i++) {
		AVD_AMF_SG_TYPE *sg_type = avd_sgtype_find(&app_type->sgAmfApptSGTypes[i]);
		if (sg_type == NULL) {
			/* SG type does not exist in current model, check CCB */
			if ((opdata != NULL) &&
			    (ccbutil_getCcbOpDataByDN(opdata->ccbId, &app_type->sgAmfApptSGTypes[i]) == NULL)) {
				avd_log(NCSFL_SEV_ERROR, "SG type '%s' does not exist either in model or CCB",
					app_type->sgAmfApptSGTypes[i].value);
				return SA_AIS_ERR_BAD_OPERATION;
			}
			avd_log(NCSFL_SEV_ERROR, "AMF SG type '%s' does not exist",
				app_type->sgAmfApptSGTypes[i].value);
			return -1;
		}
	}

	return 0;
}

static AVD_APP_TYPE *avd_apptype_create(SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	int i = 0, j;
	int rc = -1;
	AVD_APP_TYPE *app_type;
	const SaImmAttrValuesT_2 *attr;

	if ((app_type = calloc(1, sizeof(AVD_APP_TYPE))) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "calloc FAILED");
		goto done;
	}

	/* Prepare everything for adding the object later to the DB */
	memcpy(app_type->name.value, dn->value, dn->length);
	app_type->name.length = dn->length;
	app_type->tree_node.key_info = (uns8 *)&(app_type->name);

	while ((attr = attributes[i++]) != NULL) {
		if (!strcmp(attr->attrName, "saAmfApptSGTypes")) {
			app_type->no_sg_types = attr->attrValuesNumber;
			app_type->sgAmfApptSGTypes = malloc(attr->attrValuesNumber * sizeof(SaNameT));
			for (j = 0; j < attr->attrValuesNumber; j++) {
				app_type->sgAmfApptSGTypes[j] = *((SaNameT *)attr->attrValues[j]);
			}
		}
	}

	rc = 0;

 done:
	if (rc != 0) {
		free(app_type);
		app_type = NULL;
	}

	return app_type;
}

/*****************************************************************************
 * Function: avd_app_type_ccb_completed_cb
 * 
 * Purpose: This routine handles all CCB operations on SaAmfApplication objects.
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
static SaAisErrorT avd_apptype_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	AVD_APP_TYPE *app_type;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if ((app_type = avd_apptype_create(&opdata->objectName, opdata->param.create.attrValues)) == NULL) {
			goto done;
		}

		if (avd_apptype_config_validate(app_type, opdata) != 0) {
			avd_log(NCSFL_SEV_ERROR, "AMF app type '%s' validation error", app_type->name.value);
			free(app_type->sgAmfApptSGTypes);
			free(app_type);
			goto done;
		}

		opdata->userData = app_type;	/* Save for later use in apply */

		break;
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfAppType not supported");
		return SA_AIS_ERR_BAD_OPERATION;
		break;
	case CCBUTIL_DELETE:
		app_type = avd_apptype_find(&opdata->objectName);
		if (NULL != app_type->list_of_app) {
			avd_log(NCSFL_SEV_ERROR, "SaAmfAppType is in use");
			return SA_AIS_ERR_BAD_OPERATION;
		}
		break;
	default:
		assert(0);
		break;
	}
 done:
	return SA_AIS_OK;
}

/*****************************************************************************
 * Function: avd_app_type_ccb_apply_cb
 * 
 * Purpose: This routine handles all CCB apply operations on SaAmfAppType objects.
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
static void avd_apptype_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		avd_apptype_add_to_model(opdata->userData);
		break;
	case CCBUTIL_DELETE:{
			AVD_APP_TYPE *apptype = avd_apptype_find(&opdata->objectName);
			avd_apptype_delete(apptype);
			break;
		}
	default:
		assert(0);
		break;
	}
}

SaAisErrorT avd_apptype_config_get(void)
{
	SaAisErrorT rc = SA_AIS_ERR_INVALID_PARAM, error;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfAppType";
	AVD_APP_TYPE *app_type;

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, NULL, SA_IMM_SUBTREE,
		SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
		NULL, &searchHandle);

	if (SA_AIS_OK != error) {
		avd_log(NCSFL_SEV_ERROR, "No AMF app types found");
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {
		avd_log(NCSFL_SEV_NOTICE, "'%s'", dn.value);

		if ((app_type = avd_apptype_create(&dn, attributes)) == NULL)
			goto done2;

		if (avd_apptype_config_validate(app_type, NULL) != 0)
			goto done2;

		avd_apptype_add_to_model(app_type);
	}

	rc = SA_AIS_OK;
 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:

	return rc;
}

/*****************************************************************************
 * Function: avd_apptype_del_app_list
 *
 * Purpose:  This function will del the given app from the app_type list.
 *
 * Input: app - The app pointer
 *
 * Returns: None.
 *
 * NOTES: None
 *
 *
 **************************************************************************/
static void avd_apptype_del_app_list(AVD_APP *app)
{
	AVD_APP *i_app = NULL;
	AVD_APP *prev_app = NULL;

	if (app->app_on_app_type != NULL) {
		i_app = app->app_on_app_type->list_of_app;

		while ((i_app != NULL) && (i_app != app)) {
			prev_app = i_app;
			i_app = i_app->app_type_list_app_next;
		}

		if (i_app != app) {
			/* Log a fatal error */
		} else {
			if (prev_app == NULL) {
				app->app_on_app_type->list_of_app = app->app_type_list_app_next;
			} else {
				prev_app->app_type_list_app_next = app->app_type_list_app_next;
			}
		}

		app->app_type_list_app_next = NULL;
		app->app_on_app_type = NULL;
	}
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************
 * Function: avd_app_find
 *
 * Purpose:  This function will find a AVD_AMF_APP structure in the
 *           tree with app_name value as key.
 *
 * Input: dn - The name of the app.
 *
 * Returns: The pointer to AVD_AMF_APP structure found in the tree.
 *
 * NOTES:
 *
 *
 **************************************************************************/

AVD_APP *avd_app_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_APP *)ncs_patricia_tree_get(&avd_app_db, (uns8 *)&tmp);
}

AVD_APP *avd_app_getnext(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_APP *)ncs_patricia_tree_getnext(&avd_app_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_app_delete
 *
 * Purpose:  This function will delete a AVD_APP structure from the tree. 
 *
 * Input: app - The app structure that needs to be deleted.
 *
 * Returns: Ok/Error.
 *
 * NOTES:
 *
 * 
 **************************************************************************/
void avd_app_delete(AVD_APP *app)
{
	unsigned int rc = ncs_patricia_tree_del(&avd_app_db, &app->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	avd_apptype_del_app_list(app);
	free(app);
}

static void avd_app_add_to_model(AVD_APP *app)
{
	avd_trace("'%s'", app->name.value);

	/* Find application type and make a link with app type */
	app->app_on_app_type = avd_apptype_find(&app->saAmfAppType);
	assert(app->app_on_app_type != NULL);

	app->app_type_list_app_next = app->app_on_app_type->list_of_app;
	app->app_on_app_type->list_of_app = app;
}

/**
 * Add an SI to the specified APP
 * 
 * @param app 
 * @param si
 */
void avd_app_add_si(AVD_APP *app, AVD_SI *si)
{
	si->si_list_app_next = si->si_on_app->list_of_si;
	si->si_on_app->list_of_si = si;
}

void avd_app_del_si(AVD_APP *app, AVD_SI *si)
{
	AVD_SI *i_si;
	AVD_SI *prev_si = NULL;

	i_si = app->list_of_si;

	while ((i_si != NULL) && (i_si != si)) {
		prev_si = i_si;
		i_si = i_si->si_list_app_next;
	}
	
	if (i_si != si) {
		/* Log a fatal error */
		assert(0);
	} else {
		if (prev_si == NULL) {
			app->list_of_si = si->si_list_app_next;
		} else {
			prev_si->si_list_app_next = si->si_list_app_next;
		}
	}
	
	si->si_list_app_next = NULL;
	si->si_on_app = NULL;
}

/**
 * Add an SG to the specified APP, checkpoint runtime attr
 * 
 * @param app 
 * @param sg 
 */
void avd_app_add_sg(AVD_APP *app, AVD_SG *sg)
{
	sg->sg_list_app_next = app->list_of_sg;
	app->list_of_sg = sg;
	app->saAmfApplicationCurrNumSGs++;
	// TODO checkpoint
}

/**
 * Delete an SG from the specified APP, checkpoint runtime attr
 * 
 * @param app 
 * @param sg 
 */
void avd_app_del_sg(AVD_APP *app, AVD_SG *sg)
{
	AVD_SG *i_sg;
	AVD_SG *prev_sg = NULL;

	i_sg = app->list_of_sg;

	while ((i_sg != NULL) && (i_sg != sg)) {
		prev_sg = i_sg;
		i_sg = i_sg->sg_list_app_next;
	}
	
	if (i_sg != sg) {
		/* Log a fatal error */
		assert(0);
	} else {
		if (prev_sg == NULL) {
			app->list_of_sg = sg->sg_list_app_next;
		} else {
			prev_sg->sg_list_app_next = sg->sg_list_app_next;
		}
	}
	
	app->saAmfApplicationCurrNumSGs--;
	// TODO checkpoint	
	sg->sg_list_app_next = NULL;
	sg->sg_on_app = NULL;
}

static int avd_app_config_validate(const AVD_APP *app, const CcbUtilOperationData_t *opdata)
{
	/* Applications should be root objects */
	if (strchr((char *)app->name.value, ',') != NULL) {
		avd_log(NCSFL_SEV_ERROR, "Parent to '%s' is not root", app->name.value);
		return -1;
	}

	if (avd_apptype_find(&app->saAmfAppType) == NULL) {
		/* App type does not exist in current model, check CCB if passed as param */
		if ((opdata != NULL) && ccbutil_getCcbOpDataByDN(opdata->ccbId, &app->saAmfAppType) == NULL) {
			avd_log(NCSFL_SEV_ERROR, "App type '%s' does not exist in existing model or in CCB",
				app->saAmfAppType.value);
			return -1;
		}
		avd_log(NCSFL_SEV_ERROR, "Invalid saAmfAppType for '%s'", app->name.value);
		return -1;
	}

	if (!avd_admin_state_is_valid(app->saAmfApplicationAdminState)) {
		avd_log(NCSFL_SEV_ERROR, "Invalid admin state %u for '%s'",
			app->saAmfApplicationAdminState, app->name.value);
		return -1;
	}

	return 0;
}

AVD_APP *avd_app_create(const SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	int rc = -1;
	AVD_APP *app;

	/*
	** If called from cold sync, attributes==NULL.
	** If called at new active at failover, the object is found in the DB
	** but needs to get configuration attributes initialized.
	*/
	if (NULL == (app = avd_app_find(dn))) {
		if ((app = calloc(1, sizeof(AVD_APP))) == NULL) {
			avd_log(NCSFL_SEV_ERROR, "calloc FAILED");
			goto done;
		}

		memcpy(app->name.value, dn->value, dn->length);
		app->name.length = dn->length;
		app->tree_node.key_info = (uns8 *)&(app->name);
	}

	/* If no attributes supplied, go direct and add to DB */
	if (NULL == attributes)
		goto add_to_db;

	if (immutil_getAttr("saAmfAppType", attributes, 0, &app->saAmfAppType) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfAppType FAILED for '%s'", dn->value);
		goto done;
	}
	
	if (immutil_getAttr("saAmfApplicationAdminState", attributes, 0, &app->saAmfApplicationAdminState) != SA_AIS_OK) {
		/* Empty, assign default value */
		app->saAmfApplicationAdminState = SA_AMF_ADMIN_UNLOCKED;
	}

add_to_db:
	(void)	ncs_patricia_tree_add(&avd_app_db, &app->tree_node);
	rc = 0;

done:
	if (rc != 0) {
		free(app);
		app = NULL;
	}

	return app;
}

/*****************************************************************************
 * Function: avd_app_ccb_completed_cb
 * 
 * Purpose: This routine handles all CCB operations on SaAmfApplication objects.
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
static SaAisErrorT avd_app_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_APP *app;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if ((app = avd_app_create(&opdata->objectName,
					  (const SaImmAttrValuesT_2 **)opdata->param.create.attrValues)) == NULL) {
			goto done;
		}

		if (avd_app_config_validate(app, opdata) != 0) {
			avd_log(NCSFL_SEV_ERROR, "AMF app '%s' validation error", app->name.value);
			avd_app_delete(app);
			goto done;
		}

		opdata->userData = app;	/* Save for later use in apply */
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfApplication not supported");
		goto done;
		break;
	case CCBUTIL_DELETE:
		app = avd_app_find(&opdata->objectName);
		if (app->saAmfApplicationAdminState != SA_AMF_ADMIN_LOCKED_INSTANTIATION) {
			avd_log(NCSFL_SEV_ERROR, "Wrong admin state (%u) for delete of '%s'",
				app->saAmfApplicationAdminState, app->saAmfAppType.value);
			goto done;
		}
		if ((app->list_of_sg != NULL) || (app->list_of_si != NULL)) {
			avd_log(NCSFL_SEV_ERROR, "Application still has SGs or SIs");
			goto done;
		}
		rc = SA_AIS_OK;
		break;
	default:
		assert(0);
		break;
	}

	rc = SA_AIS_OK;
 done:
	return rc;
}

static void avd_app_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		avd_app_add_to_model(opdata->userData);
		break;
	case CCBUTIL_DELETE:{
			AVD_APP *app = avd_app_find(&opdata->objectName);
			avd_app_delete(app);
			break;
		}
	default:
		assert(0);
	}
}

/*****************************************************************************
 * Function: avd_app_admin_op_cb
 *
 * Purpose: This routine handles admin Oper on SaAmfApplication objects.
 *
 *
 * Input  : Ccb Util Oper Data, Test Flag.
 *
 * Returns: None.
 *
 * NOTES  : None.
 *
 *
 **************************************************************************/
static void avd_app_admin_op_cb(SaImmOiHandleT immOiHandle,
				SaInvocationT invocation,
				const SaNameT *object_name,
				SaImmAdminOperationIdT op_id, const SaImmAdminOperationParamsT_2 **params)
{
	SaAisErrorT rc;
	AVD_APP *app;
	AVD_SG *sg;

	avd_log(NCSFL_SEV_NOTICE, "%s", object_name->value);

	/* Find the app name. */
	app = avd_app_find(object_name);
	assert(app != NULL);

	if (op_id == SA_AMF_ADMIN_UNLOCK) {
		if (app->saAmfApplicationAdminState == SA_AMF_ADMIN_UNLOCKED) {
			avd_log(NCSFL_SEV_ERROR, "%s is already unlocked", object_name->value);
			rc = SA_AIS_ERR_NO_OP;
			goto done;
		}

		if (app->saAmfApplicationAdminState == SA_AMF_ADMIN_LOCKED_INSTANTIATION) {
			avd_log(NCSFL_SEV_ERROR, "%s is locked instantiation", object_name->value);
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}
	}

	if (op_id == SA_AMF_ADMIN_LOCK) {
		if (app->saAmfApplicationAdminState == SA_AMF_ADMIN_LOCKED) {
			avd_log(NCSFL_SEV_ERROR, "%s is already locked", object_name->value);
			rc = SA_AIS_ERR_NO_OP;
			goto done;
		}

		if (app->saAmfApplicationAdminState == SA_AMF_ADMIN_LOCKED_INSTANTIATION) {
			avd_log(NCSFL_SEV_ERROR, "%s is locked instantiation", object_name->value);
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}
	}

	sg = app->list_of_sg;
	/* recursively perform admin op */
#if 0
	while (sg != NULL) {
		avd_sa_avd_sg_object_admin_hdlr(avd_cb, object_name, op_id, params);
		sg = sg->sg_list_app_next;
	}
#endif
	switch (op_id) {
	case SA_AMF_ADMIN_SHUTDOWN:
	case SA_AMF_ADMIN_UNLOCK:
	case SA_AMF_ADMIN_LOCK:
	case SA_AMF_ADMIN_LOCK_INSTANTIATION:
	case SA_AMF_ADMIN_UNLOCK_INSTANTIATION:
	case SA_AMF_ADMIN_RESTART:
	default:
		rc = SA_AIS_ERR_NOT_SUPPORTED;
		break;
	}
 done:
	(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, rc);
}

static SaAisErrorT avd_app_rt_attr_cb(SaImmOiHandleT immOiHandle,
				      const SaNameT *objectName, const SaImmAttrNameT *attributeNames)
{
	AVD_APP *app = avd_app_find(objectName);
	SaImmAttrNameT attributeName;
	int i = 0;

	avd_trace("%s", objectName->value);
	assert(app != NULL);

	while ((attributeName = attributeNames[i++]) != NULL) {
		avd_trace("Attribute %s", attributeName);
		if (!strcmp(attributeName, "saAmfApplicationCurrNumSGs")) {
			(void)avd_saImmOiRtObjectUpdate(objectName, attributeName,
						       SA_IMM_ATTR_SAUINT32T, &app->saAmfApplicationCurrNumSGs);
		} else
			assert(0);
	}

	return SA_AIS_OK;
}

SaAisErrorT avd_app_config_get(void)
{
	SaAisErrorT error = SA_AIS_ERR_FAILED_OPERATION;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfApplication";
	AVD_APP *app;

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	if (immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, NULL, SA_IMM_SUBTREE,
					      SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
					      NULL, &searchHandle) != SA_AIS_OK) {

		avd_log(NCSFL_SEV_ERROR, "No AMF applications found");
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, &attributes) == SA_AIS_OK) {
		avd_log(NCSFL_SEV_NOTICE, "'%s'", dn.value);

		if ((app = avd_app_create(&dn, (const SaImmAttrValuesT_2 **)attributes)) == NULL)
			goto done2;

		if (avd_app_config_validate(app, NULL) != 0) {
			avd_app_delete(app);
			goto done2;
		}

		avd_app_add_to_model(app);

		if (avd_sg_config_get(&dn, app) != SA_AIS_OK)
			goto done2;

		if (avd_si_config_get(app) != SA_AIS_OK)
			goto done2;
	}

	error = SA_AIS_OK;
 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:

	return error;
}

void avd_app_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	assert(ncs_patricia_tree_init(&avd_app_db, &patricia_params) == NCSCC_RC_SUCCESS);
	assert(ncs_patricia_tree_init(&avd_apptype_db, &patricia_params) == NCSCC_RC_SUCCESS);

	avd_class_impl_set("SaAmfAppBaseType", NULL, NULL, avd_imm_default_OK_completed_cb, NULL);
	avd_class_impl_set("SaAmfApplication", avd_app_rt_attr_cb,
		avd_app_admin_op_cb, avd_app_ccb_completed_cb, avd_app_ccb_apply_cb);
	avd_class_impl_set("SaAmfAppType", NULL, NULL, avd_apptype_ccb_completed_cb, avd_apptype_ccb_apply_cb);
}
