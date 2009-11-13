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

  DESCRIPTION:This module deals with the creation, accessing and deletion of
  the SI database and SU SI relationship list on the AVD.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <avd_si.h>
#include <avd_dblog.h>
#include <saImmOm.h>
#include <immutil.h>
#include <avd_app.h>
#include <avd_cluster.h>
#include <avd_imm.h>
#include <avd_susi.h>
#include <avd_csi.h>
#include <avd_proc.h>

static NCS_PATRICIA_TREE avd_si_db;
static NCS_PATRICIA_TREE avd_svctype_db;
static NCS_PATRICIA_TREE avd_svctypecstypes_db;

static SaAisErrorT avd_svctypecstypes_config_get(SaNameT *svctype_name);

/*****************************************************************************
 * Function: avd_svctype_find
 *
 * Purpose:  This function will find a AVD_SVC_TYPE structure in the
 *           tree with svc_type_name value as key.
 *
 * Input: dn - The name of the svc_type_name.
 *        
 * Returns: The pointer to AVD_SVC_TYPE structure found in the tree.
 *
 * NOTES:
 *
 *
 **************************************************************************/
static AVD_SVC_TYPE *avd_svctype_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SVC_TYPE *)ncs_patricia_tree_get(&avd_svctype_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_svctype_delete
 *
 * Purpose:  This function will delete a AVD_SVC_TYPE structure from the
 *           tree. 
 *
 * Input: svc_type - The svc_type structure that needs to be deleted.
 *
 * Returns: Ok/Error.
 *
 * NOTES:
 *
 * 
 **************************************************************************/
static void avd_svctype_delete(AVD_SVC_TYPE *svc_type)
{
	unsigned int rc = ncs_patricia_tree_del(&avd_svctype_db, &svc_type->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	free(svc_type);
}

/*****************************************************************************
 * Function: avd_si_find
 *
 * Purpose:  This function will find a AVD_SI structure in the
 * tree with si_name value as key.
 *
 * Input: dn - The name of the service instance.
 *        
 * Returns: The pointer to AVD_SI structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SI *avd_si_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SI *)ncs_patricia_tree_get(&avd_si_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_si_getnext
 *
 * Purpose:  This function will find the next AVD_SI structure in the
 * tree whose si_name value is next of the given si_name value.
 *
 * Input: dn - The name of the service instance.
 *        
 * Returns: The pointer to AVD_SI structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SI *avd_si_getnext(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SI *)ncs_patricia_tree_getnext(&avd_si_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_si_delete
 *
 * Purpose:  This function will delete and free AVD_SI structure from 
 * the tree.
 *
 * Input: si - The SI structure that needs to be deleted.
 *
 * Returns: None
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_si_delete(AVD_SI *si)
{
	unsigned int rc;

	avd_app_del_si(si->si_on_app, si);
	avd_sg_del_si(si->sg_of_si, si);
	rc = ncs_patricia_tree_del(&avd_si_db, &si->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);

	free(si);
}

/*****************************************************************************
 * Function: avd_si_add_to_model
 * 
 * Purpose: This routine adds SaAmfSI objects to Patricia tree.
 * 
 *
 * Input  : Ccb Util Oper Data.
 *  
 * Returns: None.
 *  
 * NOTES  : None.
 *
 *
 **************************************************************************/
static void avd_si_add_to_model(AVD_SI *si)
{
	assert(si != NULL);

	/* Add SI to SvcType */
	si->si_list_svc_type_next = si->si_on_svc_type->list_of_si;
	si->si_on_svc_type->list_of_si = si;

	/* Add SI to App */
	avd_app_add_si(si->si_on_app, si);

	/* Add SI to SG */
	avd_sg_add_si(si->sg_of_si, si);
}

/**
 * Validate configuration attributes for an AMF SI object
 * @param si
 * 
 * @return int
 */
static int avd_si_config_validate(const AVD_SI *si)
{
	if (si->saAmfSIProtectedbySG.length > 0) {
		if (avd_sg_find(&si->saAmfSIProtectedbySG) == NULL) {
			avd_log(NCSFL_SEV_ERROR, "SG '%s' does not exist", si->saAmfSIProtectedbySG.value);
			return -1;
		}
	}

	if (si->si_on_svc_type == NULL) {
		avd_log(NCSFL_SEV_ERROR, "SvcType '%s' does not exist", si->saAmfSvcType.value);
		return -1;
	}

	return 0;
}

AVD_SI *avd_si_create(SaNameT *si_name, const SaImmAttrValuesT_2 **attributes)
{
	int i, rc = -1;
	AVD_SI *si;
	SaUint32T attrValuesNumber;
	AVD_APP *app;
	SaNameT app_name;
	char *p;

	/*
	** If called from cold sync, attributes==NULL.
	** If called at new active at failover, the object is found in the DB
	** but needs to get configuration attributes initialized.
	*/
	if (NULL == (si = avd_si_find(si_name))) {
		if ((si = calloc(1, sizeof(AVD_SI))) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "calloc FAILED");
			return NULL;
		}

		memcpy(si->name.value, si_name->value, si_name->length);
		si->name.length = si_name->length;
		si->tree_node.key_info = (uns8 *)&si->name;
		si->si_switch = AVSV_SI_TOGGLE_STABLE;
		si->saAmfSIAdminState = SA_AMF_ADMIN_UNLOCKED;
		si->si_dep_state = AVD_SI_NO_DEPENDENCY;
		si->saAmfSIAssignmentState = SA_AMF_ASSIGNMENT_UNASSIGNED;
	}

	/* If no attributes supplied, go direct and add to DB */
	if (NULL == attributes)
		goto add_to_db;

	memset(&app_name, 0, sizeof(app_name));
	p = strstr((char*)si->name.value, "safApp");
	app_name.length = strlen(p);
	memcpy(app_name.value, p, app_name.length);
	app = avd_app_find(&app_name);
	si->si_on_app = app;

	if (immutil_getAttr("saAmfSvcType", attributes, 0, &si->saAmfSvcType) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfSvcType FAILED for '%s'", si_name->value);
		goto done;
	}

	/* Optional, strange... */
	(void)immutil_getAttr("saAmfSIProtectedbySG", attributes, 0, &si->saAmfSIProtectedbySG);

	if (immutil_getAttr("saAmfSIRank", attributes, 0, &si->saAmfSIRank) != SA_AIS_OK) {
		/* Empty, assign default value */
		si->saAmfSIRank = 0;
	}

	/* Optional, [0..*] */
	if (immutil_getAttrValuesNumber("saAmfSIActiveWeight", attributes, &attrValuesNumber) == SA_AIS_OK) {
		si->saAmfSIActiveWeight = malloc(attrValuesNumber * sizeof(char *));
		for (i = 0; i < attrValuesNumber; i++) {
			si->saAmfSIActiveWeight[i] =
			    strdup(immutil_getStringAttr(attributes, "saAmfSIActiveWeight", i));
		}
	} else {
		/*  TODO */
	}

	/* Optional, [0..*] */
	if (immutil_getAttrValuesNumber("saAmfSIStandbyWeight", attributes, &attrValuesNumber) == SA_AIS_OK) {
		si->saAmfSIStandbyWeight = malloc(attrValuesNumber * sizeof(char *));
		for (i = 0; i < attrValuesNumber; i++) {
			si->saAmfSIStandbyWeight[i] =
			    strdup(immutil_getStringAttr(attributes, "saAmfSIStandbyWeight", i));
		}
	} else {
		/*  TODO */
	}

	if (immutil_getAttr("saAmfSIPrefActiveAssignments", attributes, 0,
			    &si->saAmfSIPrefActiveAssignments) != SA_AIS_OK) {

		/* Empty, assign default value */
		si->saAmfSIPrefActiveAssignments = 1;
	}

	if (immutil_getAttr("saAmfSIPrefStandbyAssignments", attributes, 0,
			    &si->saAmfSIPrefStandbyAssignments) != SA_AIS_OK) {

		/* Empty, assign default value */
		si->saAmfSIPrefStandbyAssignments = 1;
	}

	if (immutil_getAttr("saAmfSIAdminState", attributes, 0, &si->saAmfSIAdminState) != SA_AIS_OK) {
		/* Empty, assign default value */
		si->saAmfSIAdminState = SA_AMF_ADMIN_UNLOCKED;
	}

	si->si_on_svc_type = avd_svctype_find(&si->saAmfSvcType);
	si->si_on_app = app;
	if (si->saAmfSIProtectedbySG.length > 0)
		si->sg_of_si = avd_sg_find(&si->saAmfSIProtectedbySG);

add_to_db:
	(void)ncs_patricia_tree_add(&avd_si_db, &si->tree_node);
	rc = 0;

done:
	if (rc != 0) {
		avd_si_delete(si);
		si = NULL;
	}

	return si;
}

/**
 * Get configuration for all AMF SI objects from IMM and create
 * AVD internal objects.
 * 
 * @param cb
 * @param app
 * 
 * @return int
 */
SaAisErrorT avd_si_config_get(AVD_APP *app)
{
	SaAisErrorT error = SA_AIS_ERR_FAILED_OPERATION;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT si_name;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfSI";
	AVD_SI *si;

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	if (immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, &app->name, SA_IMM_SUBTREE,
	      SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
	      NULL, &searchHandle) != SA_AIS_OK) {

		avd_log(NCSFL_SEV_ERROR, "No objects found (1)");
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &si_name, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {

		avd_log(NCSFL_SEV_NOTICE, "'%s'", si_name.value);

		if ((si = avd_si_create(&si_name, attributes)) == NULL)
			goto done2;

		if (avd_si_config_validate(si) != 0) {
			avd_si_delete(si);
			goto done2;
		}

		avd_si_add_to_model(si);

		if (avd_sirankedsu_config_get(&si_name, si) != SA_AIS_OK)
			goto done2;

		if (avd_csi_config_get(&si_name, si) != SA_AIS_OK)
			goto done2;
	}

	error = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:

	return error;
}

/*****************************************************************************
 * Function: avd_si_del_svc_type_list
 *
 * Purpose:  This function will del the given si from svc_type list.
 *
 * Input: cb - the AVD control block
 *        si - The si pointer
 *
 * Returns: None.
 *
 * NOTES: None
 *
 *
 **************************************************************************/
void avd_si_del_svc_type_list(AVD_CL_CB *cb, AVD_SI *si)
{
	AVD_SI *i_si = NULL;
	AVD_SI *prev_si = NULL;

	if (si->si_on_svc_type != NULL) {
		i_si = si->si_on_svc_type->list_of_si;

		while ((i_si != NULL) && (i_si != si)) {
			prev_si = i_si;
			i_si = i_si->si_list_svc_type_next;
		}

		if (i_si != si) {
			/* Log a fatal error */
		} else {
			if (prev_si == NULL) {
				si->si_on_svc_type->list_of_si = si->si_list_svc_type_next;
			} else {
				prev_si->si_list_svc_type_next = si->si_list_svc_type_next;
			}
		}

		si->si_list_svc_type_next = NULL;
		si->si_on_svc_type = NULL;
	}
}

/*****************************************************************************
 * Function: avd_si_del_app_list
 *
 * Purpose:  This function will del the given si from app list.
 *
 * Input: cb - the AVD control block
 *        si - The si pointer
 *
 * Returns: None.
 *
 * NOTES: None
 *
 *
 **************************************************************************/
void avd_si_del_app_list(AVD_CL_CB *cb, AVD_SI *si)
{
	AVD_SI *i_si = NULL;
	AVD_SI *prev_si = NULL;

	if (si->si_on_app != NULL) {
		i_si = si->si_on_app->list_of_si;

		while ((i_si != NULL) && (i_si != si)) {
			prev_si = i_si;
			i_si = i_si->si_list_app_next;
		}

		if (i_si != si) {
			/* Log a fatal error */
		} else {
			if (prev_si == NULL) {
				si->si_on_app->list_of_si = si->si_list_app_next;
			} else {
				prev_si->si_list_app_next = si->si_list_app_next;
			}
		}

		si->si_list_app_next = NULL;
		si->si_on_app = NULL;
	}
}

/*****************************************************************************
 * Function: avd_si_ccb_completed_modify_hdlr
 *
 * Purpose: This routine handles modify operations on SaAmfSI objects.
 *
 *
 * Input  : Control Block, Ccb Util Oper Data.
 *
 * Returns: None.
 *
 * NOTES  : None.
 *
 *
 **************************************************************************/
static SaAisErrorT avd_si_ccb_completed_modify_hdlr(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_OK;
	AVD_SI *si;
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;

	si = avd_si_find(&opdata->objectName);
	assert(si != NULL);

	/* Modifications can only be done for these attributes. */
	i = 0;
	while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
		const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;

		if (strcmp(attribute->attrName, "saAmfSIPrefActiveAssignments") &&
		    strcmp(attribute->attrName, "saAmfSIPrefStandbyAssignments")) {
			avd_log(NCSFL_SEV_ERROR, "Modification of attribute %s is not supported", attribute->attrName);
			rc = SA_AIS_ERR_BAD_OPERATION;
			break;
		}
	}

	return rc;
}

/*****************************************************************************
 * Function: avd_si_admin_op_cb
 *
 * Purpose: This routine handles admin Oper on SaAmfSI objects.
 *
 *
 * Input  : ...
 *
 * Returns: None.
 *
 * NOTES  : None.
 *
 *
 **************************************************************************/
static void avd_si_admin_op_cb(SaImmOiHandleT immOiHandle,
			       SaInvocationT invocation,
			       const SaNameT *objectName,
			       SaImmAdminOperationIdT operationId, const SaImmAdminOperationParamsT_2 **params)
{
	SaAisErrorT rc = SA_AIS_OK;
	AVD_SI *avd_si;
	SaAmfAdminStateT adm_state = 0;
	SaAmfAdminStateT back_val;

	avd_log(NCSFL_SEV_NOTICE, "'%s', op %llu", objectName->value, operationId);

	avd_si = avd_si_find(objectName);

	if (avd_si->sg_of_si->sg_ncs_spec == SA_TRUE) {
		avd_log(NCSFL_SEV_ERROR, "Admin operations of OpenSAF MW is not supported");
		rc = SA_AIS_ERR_BAD_OPERATION;
		goto done;
	}

	switch (operationId) {
	case SA_AMF_ADMIN_UNLOCK:
		if (SA_AMF_ADMIN_UNLOCKED == avd_si->saAmfSIAdminState) {
			avd_log(NCSFL_SEV_ERROR, "already unlocked");
			rc = SA_AIS_ERR_NO_OP;
			goto done;
		}

		if (SA_AMF_ADMIN_LOCKED_INSTANTIATION == avd_si->saAmfSIAdminState) {
			avd_log(NCSFL_SEV_ERROR, "locked instantiation");
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		if (avd_si->sg_of_si->sg_fsm_state != AVD_SG_FSM_STABLE) {
			avd_log(NCSFL_SEV_ERROR, "SG of SI is not in stable state");
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		if (avd_si->max_num_csi == avd_si->num_csi) {
			switch (avd_si->sg_of_si->sg_redundancy_model) {
			case SA_AMF_2N_REDUNDANCY_MODEL:
				if (avd_sg_2n_si_func(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
					avd_log(NCSFL_SEV_ERROR, "avd_sg_2n_si_func failed");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				break;

			case SA_AMF_N_WAY_ACTIVE_REDUNDANCY_MODEL:
				if (avd_sg_nacvred_si_func(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
					m_AVD_SET_SI_ADMIN(avd_cb, avd_si, SA_AMF_ADMIN_LOCKED);
					avd_log(NCSFL_SEV_ERROR, "avd_sg_nacvred_si_func failed");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				break;

			case SA_AMF_N_WAY_REDUNDANCY_MODEL:
				if (avd_sg_nway_si_func(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
					m_AVD_SET_SI_ADMIN(avd_cb, avd_si, SA_AMF_ADMIN_LOCKED);
					avd_log(NCSFL_SEV_ERROR, "avd_sg_nway_si_func failed");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				break;

			case SA_AMF_NPM_REDUNDANCY_MODEL:
				if (avd_sg_npm_si_func(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
					m_AVD_SET_SI_ADMIN(avd_cb, avd_si, SA_AMF_ADMIN_LOCKED);
					avd_log(NCSFL_SEV_ERROR, "avd_sg_npm_si_func failed");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				break;

			case SA_AMF_NO_REDUNDANCY_MODEL:
			default:
				if (avd_sg_nored_si_func(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
					m_AVD_SET_SI_ADMIN(avd_cb, avd_si, SA_AMF_ADMIN_LOCKED);
					avd_log(NCSFL_SEV_ERROR, "avd_sg_nored_si_func failed");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				break;
			}
		}

		break;

	case SA_AMF_ADMIN_SHUTDOWN:
		if (SA_AMF_ADMIN_SHUTTING_DOWN == avd_si->saAmfSIAdminState) {
			avd_log(NCSFL_SEV_ERROR, "already shutting down");
			rc = SA_AIS_ERR_NO_OP;
			goto done;
		}

		if ((SA_AMF_ADMIN_LOCKED == avd_si->saAmfSIAdminState) ||
		    (SA_AMF_ADMIN_LOCKED_INSTANTIATION == avd_si->saAmfSIAdminState)) {
			avd_log(NCSFL_SEV_ERROR, "is locked (instantiation)");
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}
		adm_state = SA_AMF_ADMIN_SHUTTING_DOWN;

		/* Don't break */

	case SA_AMF_ADMIN_LOCK:
		if (0 == adm_state)
			adm_state = SA_AMF_ADMIN_LOCKED;

		if (SA_AMF_ADMIN_LOCKED == avd_si->saAmfSIAdminState) {
			avd_log(NCSFL_SEV_ERROR, "already locked");
			rc = SA_AIS_ERR_NO_OP;
			goto done;
		}

		if (SA_AMF_ADMIN_LOCKED_INSTANTIATION == avd_si->saAmfSIAdminState) {
			avd_log(NCSFL_SEV_ERROR, "locked instantiation");
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		if (avd_si->list_of_sisu == AVD_SU_SI_REL_NULL) {
			m_AVD_SET_SI_ADMIN(avd_cb, avd_si, SA_AMF_ADMIN_LOCKED);
			avd_log(NCSFL_SEV_WARNING, "SI has no assignments");
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		/* SI lock should not be done, this SI is been DISABLED because
		   of SI-SI dependency */
		if ((avd_si->si_dep_state != AVD_SI_ASSIGNED) && (avd_si->si_dep_state != AVD_SI_TOL_TIMER_RUNNING)) {
			avd_log(NCSFL_SEV_WARNING, "DISABLED because of SI-SI dependency");
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		/* Check if other semantics are happening for other SUs. If yes
		 * return an error.
		 */
		if (avd_si->sg_of_si->sg_fsm_state != AVD_SG_FSM_STABLE) {
			avd_log(NCSFL_SEV_WARNING, "'%s' SG is not STABLE", objectName->value);
			if ((avd_si->sg_of_si->sg_fsm_state != AVD_SG_FSM_SI_OPER) ||
			    (avd_si->saAmfSIAdminState != SA_AMF_ADMIN_SHUTTING_DOWN) ||
			    (adm_state != SA_AMF_ADMIN_LOCKED)) {
				avd_log(NCSFL_SEV_WARNING, "'%s' other semantics...", objectName->value);
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
		}

		back_val = avd_si->saAmfSIAdminState;
		m_AVD_SET_SI_ADMIN(avd_cb, avd_si, (adm_state));

		switch (avd_si->sg_of_si->sg_redundancy_model) {
		case SA_AMF_2N_REDUNDANCY_MODEL:
			if (avd_sg_2n_si_admin_down(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
				m_AVD_SET_SI_ADMIN(avd_cb, avd_si, back_val);
				avd_log(NCSFL_SEV_ERROR, "avd_sg_2n_si_admin_down failed");
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
			break;

		case SA_AMF_N_WAY_REDUNDANCY_MODEL:
			if (avd_sg_nway_si_admin_down(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
				m_AVD_SET_SI_ADMIN(avd_cb, avd_si, back_val);
				avd_log(NCSFL_SEV_ERROR, "avd_sg_nway_si_admin_down failed");
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
			break;

		case SA_AMF_N_WAY_ACTIVE_REDUNDANCY_MODEL:
			if (avd_sg_nacvred_si_admin_down(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
				m_AVD_SET_SI_ADMIN(avd_cb, avd_si, back_val);
				avd_log(NCSFL_SEV_ERROR, "avd_sg_nacvred_si_admin_down failed");
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
			break;

		case SA_AMF_NPM_REDUNDANCY_MODEL:
			if (avd_sg_npm_si_admin_down(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
				m_AVD_SET_SI_ADMIN(avd_cb, avd_si, back_val);
				avd_log(NCSFL_SEV_ERROR, "avd_sg_npm_si_admin_down failed");
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
			break;

		case SA_AMF_NO_REDUNDANCY_MODEL:
		default:
			if (avd_sg_nored_si_admin_down(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
				m_AVD_SET_SI_ADMIN(avd_cb, avd_si, back_val);
				avd_log(NCSFL_SEV_ERROR, "avd_sg_nored_si_admin_down failed");
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
			break;
		}
		break;
	case SA_AMF_ADMIN_LOCK_INSTANTIATION:
	case SA_AMF_ADMIN_UNLOCK_INSTANTIATION:
	case SA_AMF_ADMIN_RESTART:
	default:
		rc = SA_AIS_ERR_NOT_SUPPORTED;
		break;
	}

 done:
	avd_log(NCSFL_SEV_NOTICE, "'%s', op %llu, result %d", objectName->value, operationId, rc);
	(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, rc);
}

static SaAisErrorT avd_si_rt_attr_cb(SaImmOiHandleT immOiHandle,
				     const SaNameT *objectName, const SaImmAttrNameT *attributeNames)
{
	AVD_SI *si = avd_si_find(objectName);
	SaImmAttrNameT attributeName;
	int i = 0;

	avd_trace("%s", objectName->value);
	assert(si != NULL);

	while ((attributeName = attributeNames[i++]) != NULL) {
		if (!strcmp("saAmfSINumCurrActiveAssignments", attributeName)) {
			(void)avd_saImmOiRtObjectUpdate(objectName, attributeName,
						       SA_IMM_ATTR_SAUINT32T, &si->saAmfSINumCurrActiveAssignments);
		} else if (!strcmp("saAmfSINumCurrStandbyAssignments", attributeName)) {
			(void)avd_saImmOiRtObjectUpdate(objectName, attributeName,
						       SA_IMM_ATTR_SAUINT32T, &si->saAmfSINumCurrStandbyAssignments);
		} else
			assert(0);
	}

	return SA_AIS_OK;
}

/*****************************************************************************
 * Function: avd_si_ccb_completed_cb
 * 
 * Purpose: This routine handles all CCB operations on SaAmfSI objects.
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
static SaAisErrorT avd_si_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_SI *si;
	AVD_APP *app;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		app = avd_app_find(opdata->param.create.parentName);

		if ((si = avd_si_create(&opdata->objectName, opdata->param.create.attrValues)) == NULL) {
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}

		if (avd_si_config_validate(si) != 0) {
			avd_log(NCSFL_SEV_ERROR, "AMF SI '%s' validation error", opdata->objectName.value);
			avd_si_delete(si);
			goto done;
		}

		opdata->userData = si;	/* Save for later use in apply */
		rc = SA_AIS_OK;

		break;
	case CCBUTIL_MODIFY:
		rc = avd_si_ccb_completed_modify_hdlr(opdata);
		break;
	case CCBUTIL_DELETE:
		si = avd_si_find(&opdata->objectName);
		if ((NULL != si->list_of_csi) || (NULL != si->list_of_csi)) {
			avd_log(NCSFL_SEV_ERROR, "SaAmfSI is in use");
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
 * Function: avd_si_ccb_apply_delete_hdlr
 * 
 * Purpose: This routine handles delete operations on SaAmfSvcType objects.
 * 
 *
 * Input  : Ccb Util Oper Data
 *  
 * Returns: None.
 *  
 ****************************************************************************/
static void avd_si_ccb_apply_delete_hdlr(CcbUtilOperationData_t *opdata)
{
	AVD_SI *si;

	si = avd_si_find(&opdata->objectName);
	assert(si != NULL);

	avd_si_delete(si);
}

/*****************************************************************************
 * Function: avd_si_ccb_apply_modify_hdlr
 * 
 * Purpose: This routine handles delete operations on SaAmfSI objects.
 * 
 *
 * Input  : Ccb Util Oper Data
 *  
 * Returns: None.
 *  
 ****************************************************************************/
static void avd_si_ccb_apply_modify_hdlr(CcbUtilOperationData_t *opdata)
{
	AVD_SI *si;
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;

	si = avd_si_find(&opdata->objectName);
	assert(si != NULL);

	/* Modifications can be done for any parameters. */
	i = 0;
	while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
		void *value;
		const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;

		value = attribute->attrValues[0];

		if (!strcmp(attribute->attrName, "saAmfSIPrefActiveAssignments")) {
			si->saAmfSIPrefActiveAssignments = *((SaUint32T *)value);
		} else if (!strcmp(attribute->attrName, "saAmfSIPrefStandbyAssignments")) {
			si->saAmfSIPrefStandbyAssignments = *((SaUint32T *)value);
		} else {
			assert(0);
		}
	}
}

/*****************************************************************************
 * Function: avd_si_ccb_apply_cb
 * 
 * Purpose: This routine handles all CCB apply operations on SaAmfSI objects.
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
static void avd_si_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		avd_si_add_to_model(opdata->userData);
		break;
	case CCBUTIL_DELETE:
		avd_si_ccb_apply_delete_hdlr(opdata);
		break;
	case CCBUTIL_MODIFY:
		avd_si_ccb_apply_modify_hdlr(opdata);
		break;
	default:
		assert(0);
		break;
	}
}

#if 0
/*****************************************************************************
 * Function: avd_svc_type_cs_type_del_si_list
 *
 * Purpose:  This function will del the given svc_type_cs_type from si list.
 *
 * Input: cb - the AVD control block
 *        svc_type_cs_type - The svc_type_cs_type pointer
 *
 * Returns: None.
 *
 * NOTES: None
 *
 *
 **************************************************************************/
static void avd_svctype_cstype_del_si_list(AVD_CL_CB *cb, AVD_SVC_TYPE_CS_TYPE *svc_type_cs_type)
{
	AVD_SVC_TYPE_CS_TYPE *i_svc_type_cs_type = NULL;
	AVD_SVC_TYPE_CS_TYPE *prev_svc_type_cs_type = NULL;

	if (svc_type_cs_type->cs_type_on_svc_type != NULL) {
		i_svc_type_cs_type = svc_type_cs_type->cs_type_on_svc_type->list_of_cs_type;

		while ((i_svc_type_cs_type != NULL) && (i_svc_type_cs_type != svc_type_cs_type)) {
			prev_svc_type_cs_type = i_svc_type_cs_type;
			i_svc_type_cs_type = i_svc_type_cs_type->cs_type_list_svc_type_next;
		}

		if (i_svc_type_cs_type == svc_type_cs_type) {
			if (prev_svc_type_cs_type == NULL) {
				svc_type_cs_type->cs_type_on_svc_type->list_of_cs_type =
				    svc_type_cs_type->cs_type_list_svc_type_next;
			} else {
				prev_svc_type_cs_type->cs_type_list_svc_type_next =
				    svc_type_cs_type->cs_type_list_svc_type_next;
			}

			svc_type_cs_type->cs_type_list_svc_type_next = NULL;
			svc_type_cs_type->cs_type_on_svc_type = NULL;
		}
	}
}
#endif
/*****************************************************************************
 * Function: avd_svctype_create
 * 
 * Purpose: This routine creates new SaAmfSvcType objects.
 * 
 *
 * Input  : Ccb Util Oper Data
 *  
 * Returns: Pointer to Svc Type structure.
 *  
 * NOTES  : None.
 *
 *
 **************************************************************************/
static AVD_SVC_TYPE *avd_svctype_create(const SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	int i;
	AVD_SVC_TYPE *svct;
	SaUint32T attrValuesNumber;

	if ((svct = calloc(1, sizeof(AVD_SVC_TYPE))) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "calloc FAILED");
		return NULL;
	}

	memcpy(svct->name.value, dn->value, dn->length);
	svct->name.length = dn->length;
	svct->tree_node.key_info = (uns8 *)&svct->name;

	/* Optional, [0..*] */
	if (immutil_getAttrValuesNumber("saAmfSvcDefActiveWeight", attributes, &attrValuesNumber) == SA_AIS_OK) {
		svct->saAmfSvcDefActiveWeight = malloc((attrValuesNumber + 1) * sizeof(char *));
		for (i = 0; i < attrValuesNumber; i++) {
			svct->saAmfSvcDefActiveWeight[i] =
			    strdup(immutil_getStringAttr(attributes, "saAmfSvcDefActiveWeight", i));
		}
		svct->saAmfSvcDefActiveWeight[i] = NULL;
	}

	/* Optional, [0..*] */
	if (immutil_getAttrValuesNumber("saAmfSvcDefStandbyWeight", attributes, &attrValuesNumber) == SA_AIS_OK) {
		svct->saAmfSvcDefStandbyWeight = malloc((attrValuesNumber + 1) * sizeof(char *));
		for (i = 0; i < attrValuesNumber; i++) {
			svct->saAmfSvcDefStandbyWeight[i] =
			    strdup(immutil_getStringAttr(attributes, "saAmfSvcDefStandbyWeight", i));
		}
		svct->saAmfSvcDefStandbyWeight[i] = NULL;
	}

	i = ncs_patricia_tree_add(&avd_svctype_db, &svct->tree_node);
	assert (i == NCSCC_RC_SUCCESS);

	return svct;
}

/*****************************************************************************
 * Function: avd_svctype_ccb_completed_cb
 * 
 * Purpose: This routine handles all CCB operations on SaAmfSvcType objects.
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
static SaAisErrorT avd_svctype_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_SVC_TYPE *svc_type;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if ((svc_type = avd_svctype_create(&opdata->objectName, opdata->param.create.attrValues)) == NULL) {
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}

		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfSvcType not supported");
		break;
	case CCBUTIL_DELETE:
		svc_type = avd_svctype_find(&opdata->objectName);
		if ((NULL != svc_type->list_of_si) || (NULL != svc_type->list_of_cs_type)) {
			avd_log(NCSFL_SEV_ERROR, "SaAmfSvcType is in use");
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
 * Function: avd_svctype_ccb_apply_cb
 * 
 * Purpose: This routine handles all CCB apply operations on SaAmfSvcType objects.
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
static void avd_svctype_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		break;
	case CCBUTIL_DELETE: {
		AVD_SVC_TYPE *svct = avd_svctype_find(&opdata->objectName);
		avd_svctype_delete(svct);
		break;
	}
	default:
		assert(0);
		break;
	}
}

static int avd_svctype_config_validate(AVD_SVC_TYPE *svc_type)
{
	char *parent;
	char *dn = (char *)svc_type->name.value;

	if ((parent = strchr(dn, ',')) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "No parent to '%s' ", dn);
		return -1;
	}

	parent++;

	/* Should be children to the SvcBasetype */
	if (strncmp(parent, "safSvcType=", 11) != 0) {
		avd_log(NCSFL_SEV_ERROR, "Wrong parent '%s' to '%s' ", parent, dn);
		return -1;
	}

	return 0;
}

/**
 * Get configuration for all SaAmfSvcType objects from IMM and 
 * create AVD internal objects. 
 * 
 * @param cb
 * @param app
 * 
 * @return int
 */
SaAisErrorT avd_svctype_config_get(void)
{
	SaAisErrorT error = SA_AIS_ERR_FAILED_OPERATION;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfSvcType";
	AVD_SVC_TYPE *svc_type;

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	if (immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, NULL, SA_IMM_SUBTREE,
		SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
		NULL, &searchHandle) != SA_AIS_OK) {

		avd_log(NCSFL_SEV_ERROR, "No objects found (1)");
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {
		avd_log(NCSFL_SEV_NOTICE, "'%s'", dn.value);

		if ((svc_type = avd_svctype_create(&dn, attributes)) == NULL)
			goto done2;

		if (avd_svctype_config_validate(svc_type) != 0)
			goto done2;

		if (avd_svctypecstypes_config_get(&dn) != SA_AIS_OK)
			goto done2;
	}

	error = SA_AIS_OK;
 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:

	return error;
}

static AVD_SVC_TYPE_CS_TYPE *avd_svctypecstypes_create(SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	uns32 rc;
	AVD_SVC_TYPE_CS_TYPE *svctypecstype;

	if ((svctypecstype = calloc(1, sizeof(AVD_SVC_TYPE_CS_TYPE))) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "calloc failed");
		return NULL;
	}

	memcpy(svctypecstype->name.value, dn->value, dn->length);
	svctypecstype->name.length = dn->length;
	svctypecstype->tree_node.key_info = (uns8 *)&(svctypecstype->name);

	if (immutil_getAttr("saAmfSvcMaxNumCSIs", attributes, 0, &svctypecstype->saAmfSvcMaxNumCSIs) != SA_AIS_OK)
		svctypecstype->saAmfSvcMaxNumCSIs = -1; /* no limit */

	rc = ncs_patricia_tree_add(&avd_svctypecstypes_db, &svctypecstype->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);

	return svctypecstype;
}

static void avd_svctypecstypes_delete(AVD_SVC_TYPE_CS_TYPE *svctypecstype)
{
	uns32 rc;

	rc = ncs_patricia_tree_del(&avd_svctypecstypes_db, &svctypecstype->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	free(svctypecstype);
}

AVD_SVC_TYPE_CS_TYPE *avd_svctypecstypes_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SVC_TYPE_CS_TYPE*)ncs_patricia_tree_get(&avd_svctypecstypes_db, (uns8 *)&tmp);
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
static SaAisErrorT avd_svctypecstypes_config_get(SaNameT *svctype_name)
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
		avd_log(NCSFL_SEV_ERROR, "saImmOmSearchInitialize_2 failed: %u", error);
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {

		avd_log(NCSFL_SEV_NOTICE, "'%s' (%u)", dn.value, dn.length);

		if ((svctypecstype = avd_svctypecstypes_create(&dn, attributes)) == NULL) {
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

/**
 * Handles the CCB Completed operation for the 
 * SaAmfSvcTypeCSTypes class.
 * 
 * @param opdata 
 */
static SaAisErrorT avd_svctypecstypes_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_SVC_TYPE_CS_TYPE *svctypecstype;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if ((svctypecstype = avd_svctypecstypes_create(&opdata->objectName,
			opdata->param.create.attrValues)) == NULL) {
			goto done;
		}

		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfSvcTypeCSTypes not supported");
		break;
	case CCBUTIL_DELETE:
		svctypecstype = avd_svctypecstypes_find(&opdata->objectName);
		if (svctypecstype->curr_num_csis == 0) {
			rc = SA_AIS_OK;
		}
		break;
	default:
		assert(0);
		break;
	}

 done:
	return rc;
}

/**
 * Handles the CCB Apply operation for the SaAmfSvcTypeCSTypes 
 * class. 
 * 
 * @param opdata 
 */
static void avd_svctypecstypes_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	AVD_SVC_TYPE_CS_TYPE *svctypecstype;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		break;
	case CCBUTIL_DELETE:
		svctypecstype = avd_svctypecstypes_find(&opdata->objectName);
		avd_svctypecstypes_delete(svctypecstype);
		break;
	default:
		assert(0);
		break;
	}
}

void avd_si_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);

	assert(ncs_patricia_tree_init(&avd_si_db, &patricia_params) == NCSCC_RC_SUCCESS);
	assert(ncs_patricia_tree_init(&avd_svctype_db, &patricia_params) == NCSCC_RC_SUCCESS);
	assert(ncs_patricia_tree_init(&avd_svctypecstypes_db, &patricia_params) == NCSCC_RC_SUCCESS);

	avd_class_impl_set("SaAmfSI", avd_si_rt_attr_cb,
	       avd_si_admin_op_cb, avd_si_ccb_completed_cb, avd_si_ccb_apply_cb);
	avd_class_impl_set("SaAmfSvcType", NULL, NULL, avd_svctype_ccb_completed_cb, avd_svctype_ccb_apply_cb);
	avd_class_impl_set("SaAmfSvcBaseType", NULL, NULL, avd_imm_default_OK_completed_cb, NULL);
	avd_class_impl_set("SaAmfSvcTypeCSTypes", NULL, NULL,
	       avd_svctypecstypes_ccb_completed_cb, avd_svctypecstypes_ccb_apply_cb);
}

