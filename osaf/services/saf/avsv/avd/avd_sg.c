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
  the SG database on the AVD.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <ncspatricia.h>
#include <avd_dblog.h>
#include <avd_cluster.h>
#include <avd_app.h>
#include <avd_imm.h>
#include <avd_su.h>
#include <avd_ckpt_msg.h>
#include <avd_ntf.h>
#include <avd_sg.h>
#include <avd_proc.h>

static NCS_PATRICIA_TREE avd_sg_db;
static NCS_PATRICIA_TREE avd_sg_type_db;

/*****************************************************************************/
/************************* Start Class SaAmfSGType ***************************/
/*****************************************************************************/

/*****************************************************************************
 * Function: avd_sgtype_find
 *
 * Purpose:  This function will find a AVD_AMF_SG_TYPE structure in the
 *           tree with name value as key.
 *
 * Input: dn - The DN of the SGType.
 *        
 * Returns: The pointer to AVD_AMF_SG_TYPE structure found in the tree.
 *
 * NOTES:
 *
 *
 **************************************************************************/

AVD_AMF_SG_TYPE *avd_sgtype_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_AMF_SG_TYPE *)ncs_patricia_tree_get(&avd_sg_type_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_sgtype_delete
 *
 * Purpose:  This function will delete a AVD_AMF_SG_TYPE structure from the
 *           tree. 
 *
 * Input: sg_type - The sg_type structure that needs to be deleted.
 *        
 * Returns: Ok/Error.
 *
 * NOTES:
 *
 * 
 **************************************************************************/
static void avd_sgtype_delete(AVD_AMF_SG_TYPE *sg_type)
{
	(void)ncs_patricia_tree_del(&avd_sg_type_db, &sg_type->tree_node);
	free(sg_type->saAmfStgValidSuTypes);
	free(sg_type);
}

/**
 * Add the SG type to the DB
 * @param sgt
 */
static void avd_sgtype_add_to_model(AVD_AMF_SG_TYPE *sgt)
{
	unsigned int rc;

	avd_trace("'%s'", sgt->name.value);
	rc = ncs_patricia_tree_add(&avd_sg_type_db, &sgt->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
}

/**
 * Validate configuration attributes for an SaAmfSGType object
 * 
 * @param dn
 * @param sgt
 * @param opdata
 * 
 * @return int
 */
static int avd_sgtype_config_validate(const AVD_AMF_SG_TYPE *sgt, const CcbUtilOperationData_t *opdata_)
{
	int i;
	const char *dn = (char *)sgt->name.value;
	char *parent;

	if ((parent = strchr(dn, ',')) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "No parent to '%s' ", dn);
		return -1;
	}

	parent++;

	/* SGType should be children to SGBaseType */
	if (strncmp(parent, "safSgType=", 10) != 0) {
		avd_log(NCSFL_SEV_ERROR, "Wrong parent '%s' to '%s' ", parent, dn);
		return -1;
	}

	if ((sgt->saAmfSgtRedundancyModel < SA_AMF_2N_REDUNDANCY_MODEL) ||
	    (sgt->saAmfSgtRedundancyModel > SA_AMF_NO_REDUNDANCY_MODEL)) {
		avd_log(NCSFL_SEV_ERROR, "Invalid saAmfSgtRedundancyModel %u for '%s'",
			sgt->saAmfSgtRedundancyModel, dn);
		return -1;
	}

	/* Make sure all SU types exist */
	for (i = 0; i < sgt->number_su_type; i++) {
		AVD_SU_TYPE *su_type = avd_sutype_find(&sgt->saAmfSGtValidSuTypes[i]);
		if (su_type == NULL) {
			avd_log(NCSFL_SEV_ERROR, "AMF SU type '%s' does not exist", sgt->saAmfSGtValidSuTypes[i].value);
			return -1;
		}
	}

	if (sgt->saAmfSgtDefAutoRepair > SA_TRUE) {
		avd_log(NCSFL_SEV_ERROR, "Invalid saAmfSgtDefAutoRepair %u for '%s'", sgt->saAmfSgtDefAutoRepair, dn);
		return -1;
	}

	if (sgt->saAmfSgtDefAutoAdjust > SA_TRUE) {
		avd_log(NCSFL_SEV_ERROR, "Invalid saAmfSgtDefAutoAdjust %u for '%s'", sgt->saAmfSgtDefAutoAdjust, dn);
		return -1;
	}

	return 0;
}

/**
 * Create a new SG type object and initialize its attributes from the attribute list.
 * @param dn
 * @param attributes
 * 
 * @return AVD_AMF_SG_TYPE*
 */
static AVD_AMF_SG_TYPE *avd_sgtype_create(SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	int i = 0, rc = -1, j;
	AVD_AMF_SG_TYPE *sgt;
	const SaImmAttrValuesT_2 *attr;

	if ((sgt = calloc(1, sizeof(AVD_AMF_SG_TYPE))) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "calloc failed");
		return NULL;
	}

	memcpy(sgt->name.value, dn->value, dn->length);
	sgt->name.length = dn->length;
	sgt->tree_node.key_info = (uns8 *)&(sgt->name);

	if (immutil_getAttr("saAmfSgtRedundancyModel", attributes, 0, &sgt->saAmfSgtRedundancyModel) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfSgtRedundancyModel FAILED for '%s'", dn->value);
		goto done;
	}

	while ((attr = attributes[i++]) != NULL) {
		if (!strcmp(attr->attrName, "saAmfSgtValidSuTypes")) {
			sgt->number_su_type = attr->attrValuesNumber;
			sgt->saAmfSGtValidSuTypes = malloc(attr->attrValuesNumber * sizeof(SaNameT));
			for (j = 0; j < attr->attrValuesNumber; j++) {
				sgt->saAmfSGtValidSuTypes[j] = *((SaNameT *)attr->attrValues[j]);
			}
		}
	}

	if (sgt->number_su_type == 0) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfSgtValidSuTypes FAILED for '%s'", dn->value);
		goto done;
	}

	if (immutil_getAttr("saAmfSgtDefAutoRepair", attributes, 0, &sgt->saAmfSgtDefAutoRepair) != SA_AIS_OK) {
		sgt->saAmfSgtDefAutoRepair = SA_TRUE;
	}

	if (immutil_getAttr("saAmfSgtDefAutoAdjust", attributes, 0, &sgt->saAmfSgtDefAutoAdjust) != SA_AIS_OK) {
		sgt->saAmfSgtDefAutoAdjust = SA_FALSE;
	}

	if (immutil_getAttr("saAmfSgtDefAutoAdjustProb", attributes, 0, &sgt->saAmfSgtDefAutoAdjustProb) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfSgtDefAutoAdjustProb FAILED for '%s'", dn->value);
		goto done;
	}

	if (immutil_getAttr("saAmfSgtDefCompRestartProb", attributes, 0, &sgt->saAmfSgtDefCompRestartProb) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfSgtDefCompRestartProb FAILED for '%s'", dn->value);
		goto done;
	}

	if (immutil_getAttr("saAmfSgtDefCompRestartMax", attributes, 0, &sgt->saAmfSgtDefCompRestartMax) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfSgtDefCompRestartMax FAILED for '%s'", dn->value);
		goto done;
	}

	if (immutil_getAttr("saAmfSgtDefSuRestartProb", attributes, 0, &sgt->saAmfSgtDefSuRestartProb) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfSgtDefSuRestartProb FAILED for '%s'", dn->value);
		goto done;
	}

	if (immutil_getAttr("saAmfSgtDefSuRestartMax", attributes, 0, &sgt->saAmfSgtDefSuRestartProb) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfSgtDefSuRestartMax FAILED for '%s'", dn->value);
		goto done;
	}

	rc = 0;

 done:
	if (rc != 0) {
		free(sgt);
		sgt = NULL;
	}

	return sgt;
}

/**
 * Get configuration for all SaAmfSGType objects from IMM and create AVD internal objects.
 * @param cb
 * 
 * @return int
 */
SaAisErrorT avd_sgtype_config_get(void)
{
	SaAisErrorT error = SA_AIS_ERR_FAILED_OPERATION;
	AVD_AMF_SG_TYPE *sgt;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfSGType";

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

		if ((sgt = avd_sgtype_create(&dn, attributes)) == NULL)
			goto done2;

		if (avd_sgtype_config_validate(sgt, NULL) != 0)
			goto done2;

		avd_sgtype_add_to_model(sgt);
	}

	error = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:

	return error;
}

/**
 * Validate SaAmfSGType CCB completed
 * @param opdata
 * 
 * @return SaAisErrorT
 */
static SaAisErrorT avd_sgtype_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_AMF_SG_TYPE *sgt;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if ((sgt = avd_sgtype_create(&opdata->objectName, opdata->param.create.attrValues)) == NULL)
			goto done;

		if (avd_sgtype_config_validate(sgt, opdata) != 0) {
			avd_sgtype_delete(sgt);
			goto done;
		}

		opdata->userData = sgt;	/* Save for later use in apply */
		break;
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfSGType not supported");
		goto done;
		break;
	case CCBUTIL_DELETE:
		sgt = avd_sgtype_find(&opdata->objectName);
		if (sgt->list_of_sg != NULL) {
			avd_log(NCSFL_SEV_ERROR, "SGs exist of this SG type");
			goto done;
		}
		break;
	default:
		assert(0);
		break;
	}

	rc = SA_AIS_OK;
 done:
	return rc;
}

/**
 * Apply SaAmfSGType CCB changes
 * @param opdata
 * 
 * @return SaAisErrorT
 */
static void avd_sgtype_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		avd_sgtype_add_to_model(opdata->userData);
		break;
	case CCBUTIL_DELETE:{
			AVD_AMF_SG_TYPE *sgt = avd_sgtype_find(&opdata->objectName);
			avd_sgtype_delete(sgt);
			break;
		}
	default:
		assert(0);
	}
}

/*****************************************************************************
 * Function: avd_sgtype_del_sg
 *
 * Purpose:  This function will del the given sg from sg_type list.
 *
 * Input: sg - The sg pointer
 *
 * Returns: None.
 *
 * NOTES: None
 *
 *
 **************************************************************************/
static void avd_sgtype_del_sg(AVD_SG *sg)
{
	AVD_SG *i_sg = NULL;
	AVD_SG *prev_sg = NULL;

	if (sg->sg_on_sg_type != NULL) {
		i_sg = sg->sg_on_sg_type->list_of_sg;

		while ((i_sg != NULL) && (i_sg != sg)) {
			prev_sg = i_sg;
			i_sg = i_sg->sg_list_sg_type_next;
		}

		if (i_sg != sg) {
			/* Log a fatal error */
			assert(0);
		} else {
			if (prev_sg == NULL) {
				sg->sg_on_sg_type->list_of_sg = sg->sg_list_sg_type_next;
			} else {
				prev_sg->sg_list_sg_type_next = sg->sg_list_sg_type_next;
			}
		}

		sg->sg_list_sg_type_next = NULL;
		sg->sg_on_sg_type = NULL;
	}
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************
 * Function: avd_sg_add_si
 *
 * Purpose:  This function will add the given SI to the SG list, and fill
 * the SIs pointers. 
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

void avd_sg_add_si(AVD_SG *sg, AVD_SI* si)
{
	AVD_SI *i_si = AVD_SI_NULL;
	AVD_SI *prev_si = AVD_SI_NULL;

	i_si = sg->list_of_si;

	while ((i_si != AVD_SI_NULL) && (i_si->saAmfSIRank < si->saAmfSIRank)) {
		prev_si = i_si;
		i_si = i_si->sg_list_of_si_next;
	}

	if (prev_si == AVD_SI_NULL) {
		si->sg_list_of_si_next = si->sg_of_si->list_of_si;
		sg->list_of_si = si;
	} else {
		prev_si->sg_list_of_si_next = si;
		si->sg_list_of_si_next = i_si;
	}
}

void avd_sg_del_si(AVD_SG *sg, AVD_SI* si)
{
	AVD_SI *i_si = NULL;
	AVD_SI *prev_si = NULL;

	if (sg->list_of_si != NULL) {
		i_si = sg->list_of_si;

		while ((i_si != NULL) && (i_si != si)) {
			prev_si = i_si;
			i_si = i_si->sg_list_of_si_next;
		}

		if (i_si == si) {
			if (prev_si == NULL) {
				sg->list_of_si = si->sg_list_of_si_next;
			} else {
				prev_si->sg_list_of_si_next = si->sg_list_of_si_next;
			}

			si->sg_list_of_si_next = NULL;
			si->sg_of_si = NULL;
		}
	}
}

/*****************************************************************************
 * Function: avd_sg_find
 *
 * Purpose:  This function will find a AVD_SG structure in the
 * tree with sg_name value as key.
 *
 * Input: dn - The name of the service group that needs to be
 *                    found.
 *
 * Returns: The pointer to AVD_SG structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SG *avd_sg_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SG *)ncs_patricia_tree_get(&avd_sg_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_sg_getnext
 *
 * Purpose:  This function will get the next AVD_SG structure in the
 * tree whose sg_name value is next of the given sg_name value.
 *
 * Input: dn - The name of the service group that needs to be
 *                    found.
 *        
 * Returns: The pointer to AVD_SG structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SG *avd_sg_getnext(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SG *)ncs_patricia_tree_getnext(&avd_sg_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_sg_delete
 *
 * Purpose:  This function will delete and free AVD_SG structure from 
 * the tree.
 *
 * Input: sg - The SG structure that needs to be deleted.
 *
 * Returns: -
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_sg_delete(AVD_SG *sg)
{
	avd_trace("'%s'", sg->name.value);
	(void)ncs_patricia_tree_del(&avd_sg_db, &sg->tree_node);
	avd_sgtype_del_sg(sg);
	avd_app_del_sg(sg->sg_on_app, sg);
	free(sg);
}

/**
 * Add the SG to the DB
 * @param sgt
 */
static void avd_sg_add_to_model(AVD_SG *sg)
{
	assert(sg != NULL);
	avd_trace("'%s'", sg->name.value);

	/* Add SG to list in SG Type */
	sg->sg_list_sg_type_next = sg->sg_on_sg_type->list_of_sg;
	sg->sg_on_sg_type->list_of_sg = sg;

	/* Add SG to list in App */
	avd_app_add_sg(sg->sg_on_app, sg);

	m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(avd_cb, sg, AVSV_CKPT_AVD_SG_CONFIG);
}

/**
 * Validate configuration attributes for an SaAmfSG object
 * 
 * @param dn
 * @param sg
 * 
 * @return int
 */
static int avd_sg_config_validate(const AVD_SG *sg)
{
	const char *dn = (char *)sg->name.value;

	if (sg->saAmfSGAutoRepair > SA_TRUE) {
		avd_log(NCSFL_SEV_ERROR, "Invalid saAmfSGAutoRepair %u for '%s'", sg->saAmfSGAutoRepair, dn);
		return -1;
	}

	if (sg->saAmfSGAutoAdjust > SA_TRUE) {
		avd_log(NCSFL_SEV_ERROR, "Invalid saAmfSGAutoAdjust %u for '%s'", sg->saAmfSGAutoAdjust, dn);
		return -1;
	}

	/*  Validate saAmfSGSuHostNodeGroup */

	return 0;
}

AVD_SG *avd_sg_create(const SaNameT *sg_name, const SaImmAttrValuesT_2 **attributes)
{
	int rc = -1;
	AVD_SG *sg;
	AVD_AMF_SG_TYPE *sgt;
	AVD_APP *app;
	SaNameT app_name;
	char *p;

	/*
	** If called from cold sync, attributes==NULL.
	** If called at new active at failover, the object is found in the DB
	** but needs to get configuration attributes initialized.
	*/
	if (NULL == (sg = avd_sg_find(sg_name))) {
		if ((sg = calloc(1, sizeof(AVD_SG))) == NULL) {
			avd_log(NCSFL_SEV_ERROR, "calloc FAILED");
			return NULL;
		}

		memcpy(sg->name.value, sg_name->value, sg_name->length);
		sg->name.length = sg_name->length;
		sg->tree_node.key_info = (uns8 *)&(sg->name);
		sg->sg_ncs_spec = SA_FALSE;
		sg->sg_fsm_state = AVD_SG_FSM_STABLE;
		sg->adjust_state = AVSV_SG_STABLE;
	}


	/* If no attributes supplied, go direct and add to DB */
	if (NULL == attributes)
		goto add_to_db;

	memset(&app_name, 0, sizeof(app_name));
	p = strstr((char*)sg->name.value, "safApp");
	app_name.length = strlen(p);
	memcpy(app_name.value, p, app_name.length);
	app = avd_app_find(&app_name);
	sg->sg_on_app = app;

	/* SGs belonging to a magic app will be NCS, TODO Better name! */
	if (!strcmp((char *)app_name.value, "safApp=OpenSAF")) {
		sg->sg_ncs_spec = TRUE;
	}

	if (immutil_getAttr("saAmfSGType", attributes, 0, &sg->saAmfSGType) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfSGType FAILED for '%s'", sg_name->value);
		goto done;
	}

	if ((sgt = avd_sgtype_find(&sg->saAmfSGType)) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "saAmfSGType '%s' does not exist", sg->saAmfSGType.value);
		goto done;
	}

	sg->sg_on_sg_type = sgt;

	(void)immutil_getAttr("saAmfSGSuHostNodeGroup", attributes, 0, &sg->saAmfSGSuHostNodeGroup);

	if (immutil_getAttr("saAmfSGAutoRepair", attributes, 0, &sg->saAmfSGAutoRepair) != SA_AIS_OK) {
		sg->saAmfSGAutoRepair = sgt->saAmfSgtDefAutoRepair;
	}

	if (immutil_getAttr("saAmfSGAutoAdjust", attributes, 0, &sg->saAmfSGAutoAdjust) != SA_AIS_OK) {
		sg->saAmfSGAutoAdjust = sgt->saAmfSgtDefAutoAdjust;
	}

	if (immutil_getAttr("saAmfSGNumPrefActiveSUs", attributes, 0, &sg->saAmfSGNumPrefActiveSUs) != SA_AIS_OK) {
		sg->saAmfSGNumPrefActiveSUs = 1;
	}

	if (immutil_getAttr("saAmfSGNumPrefStandbySUs", attributes, 0, &sg->saAmfSGNumPrefStandbySUs) != SA_AIS_OK) {
		sg->saAmfSGNumPrefStandbySUs = 1;
	}

	if (immutil_getAttr("saAmfSGNumPrefInserviceSUs", attributes, 0, &sg->saAmfSGNumPrefInserviceSUs) != SA_AIS_OK) {
		/* empty => later assign number of SUs */
	}

	if (immutil_getAttr("saAmfSGNumPrefAssignedSUs", attributes, 0, &sg->saAmfSGNumPrefAssignedSUs) != SA_AIS_OK) {
		/* empty => later assign to saAmfSGNumPrefInserviceSUs */
	}

	if (immutil_getAttr("saAmfSGMaxActiveSIsperSUs", attributes, 0, &sg->saAmfSGMaxActiveSIsperSUs) != SA_AIS_OK) {
		/* empty => assign magic number for no limit */
		sg->saAmfSGMaxActiveSIsperSUs = -1;
	}

	if (immutil_getAttr("saAmfSGMaxStandbySIsperSUs", attributes, 0, &sg->saAmfSGMaxStandbySIsperSUs) != SA_AIS_OK) {
		/* empty => assign magic number for no limit */
		sg->saAmfSGMaxActiveSIsperSUs = -1;
	}

	if (immutil_getAttr("saAmfSGAutoAdjustProb", attributes, 0, &sg->saAmfSGAutoAdjustProb) != SA_AIS_OK) {
		sg->saAmfSGAutoAdjustProb = sgt->saAmfSgtDefAutoAdjustProb;
	}

	if (immutil_getAttr("saAmfSGCompRestartProb", attributes, 0, &sg->saAmfSGCompRestartProb) != SA_AIS_OK) {
		sg->saAmfSGCompRestartProb = sgt->saAmfSgtDefCompRestartProb;
	}

	if (immutil_getAttr("saAmfSGCompRestartMax", attributes, 0, &sg->saAmfSGCompRestartMax) != SA_AIS_OK) {
		sg->saAmfSGCompRestartMax = sgt->saAmfSgtDefCompRestartMax;
	}

	if (immutil_getAttr("saAmfSGSuRestartProb", attributes, 0, &sg->saAmfSGSuRestartProb) != SA_AIS_OK) {
		sg->saAmfSGSuRestartProb = sgt->saAmfSgtDefSuRestartProb;
	}

	if (immutil_getAttr("saAmfSGSuRestartMax", attributes, 0, &sg->saAmfSGSuRestartMax) != SA_AIS_OK) {
		sg->saAmfSGSuRestartMax = sgt->saAmfSgtDefSuRestartMax;
	}

	if (immutil_getAttr("saAmfSGAdminState", attributes, 0, &sg->saAmfSGAdminState) != SA_AIS_OK) {
		sg->saAmfSGAdminState = SA_AMF_ADMIN_UNLOCKED;
	}

	/*  TODO use value in type instead? */
	sg->sg_redundancy_model = sgt->saAmfSgtRedundancyModel;

add_to_db:
	(void) ncs_patricia_tree_add(&avd_sg_db, &sg->tree_node);
	rc = 0;

done:
	if (rc != 0) {
		free(sg);
		sg = NULL;
	}

	return sg;
}

/**
 * Get configuration for all AMF SG objects from IMM and
 * create AVD internal objects.
 * @param cb
 * 
 * @return int
 */
SaAisErrorT avd_sg_config_get(const SaNameT *app_dn, AVD_APP *app)
{
	AVD_SG *sg;
	SaAisErrorT error;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfSG";

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, app_dn, SA_IMM_SUBTREE,
						  SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
						  NULL, &searchHandle);

	if (SA_AIS_OK != error) {
		avd_log(NCSFL_SEV_ERROR, "saImmOmSearchInitialize_2 failed: %u", error);
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {
		avd_log(NCSFL_SEV_NOTICE, "'%s'", dn.value);

		if ((sg = avd_sg_create(&dn, attributes)) == NULL) {
			error = SA_AIS_ERR_FAILED_OPERATION;
			goto done2;
		}

		if (avd_sg_config_validate(sg) != 0) {
			avd_log(NCSFL_SEV_ERROR, "AMF SG '%s' validation error", dn.value);
			avd_sg_delete(sg);
			goto done2;
		}

		avd_sg_add_to_model(sg);

		if (avd_su_config_get(&dn, sg) != SA_AIS_OK) {
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
 * Function: avd_sg_ccb_apply_delete_hdlr
 *
 * Purpose: This routine handles delete operations on SaAmfSG objects.
 *
 *
 * Input  : Ccb Util Oper Data.
 *
 * Returns: None.
 *
 ****************************************************************************/
static void avd_sg_ccb_apply_delete_hdlr(CcbUtilOperationData_t *opdata)
{
	AVD_SG *sg;

	avd_log(NCSFL_SEV_NOTICE, "'%s'", opdata->objectName.value);

	sg = avd_sg_find(&opdata->objectName);
	assert(sg != NULL);

	/* check point to the standby AVD that this record need to be deleted */
	m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(avd_cb, sg, AVSV_CKPT_AVD_SG_CONFIG);

	avd_sg_delete(sg);
}

/*****************************************************************************
 * Function: avd_sg_ccb_completed_modify_hdlr
 *
 * Purpose: This routine validates modify operations on SaAmfSG objects.
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
static SaAisErrorT avd_sg_ccb_completed_modify_hdlr(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_OK;
	AVD_SG *avd_sg = NULL;
	AVD_AMF_SG_TYPE *avd_sg_type = NULL;
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;

	avd_sg = avd_sg_find(&opdata->objectName);
	assert(avd_sg != NULL);

	/* Validate whether we can modify it. */

	/*  TODO... */
	if (avd_sg->saAmfSGAdminState != SA_AMF_ADMIN_UNLOCKED) {

		while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
			const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;
			void *value = attribute->attrValues[0];

			if (!strcmp(attribute->attrName, "saAmfSGType")) {
				SaNameT sg_type_name = *((SaNameT *)value);

				avd_sg_type = avd_sgtype_find(&sg_type_name);
				if (NULL == avd_sg_type) {
					avd_log(NCSFL_SEV_ERROR, "SG Type '%s' not found", sg_type_name.value);
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
			}
		}		/* while (attr_mod != NULL) */

		/* Modifications can be done for any parameters. */
		while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
			const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;

			if (!strcmp(attribute->attrName, "saAmfSGType")) {
				/*  Just for the sake of avoiding else. */
			} else if (!strcmp(attribute->attrName, "saAmfSGType")) {
				avd_log(NCSFL_SEV_ERROR,
					"Attribute saAmfSGType cannot be modified when SG is unlocked");
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			} else if (!strcmp(attribute->attrName, "saAmfSGHostNodeGroup")) {
				avd_log(NCSFL_SEV_ERROR,
					"Attribute saAmfSGHostNodeGroup cannot be modified when SG is unlocked");
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			} else if (!strcmp(attribute->attrName, "saAmfSGAutoRepair")) {
			} else if (!strcmp(attribute->attrName, "saAmfSGAutoAdjust")) {
			} else if (!strcmp(attribute->attrName, "saAmfSGNumPrefActiveSUs")) {
			} else if (!strcmp(attribute->attrName, "saAmfSGNumPrefStandbySUs")) {
			} else if (!strcmp(attribute->attrName, "saAmfSGNumPrefInserviceSUs")) {
			} else if (!strcmp(attribute->attrName, "saAmfSGNumPrefAssignedSUs")) {
			} else if (!strcmp(attribute->attrName, "saAmfSGMaxActiveSIsperSUs")) {
			} else if (!strcmp(attribute->attrName, "saAmfSGMaxStandbySIsperSUs")) {
			} else if (!strcmp(attribute->attrName, "saAmfSGAutoAdjustProb")) {
			} else if (!strcmp(attribute->attrName, "SaAmfSGCompRestartProb")) {
			} else if (!strcmp(attribute->attrName, "SaAmfSGCompRestartMax")) {
			} else if (!strcmp(attribute->attrName, "saAmfSGSuRestartProb")) {
			} else if (!strcmp(attribute->attrName, "saAmfSGSuRestartMax")) {
			} else {
				assert(0);
			}
		}		/* while (attr_mod != NULL) */

	} /* Admin state is not UNLOCKED */
	else {			/* Admin state is UNLOCKED */
		i = 0;
		/* Modifications can be done for the following parameters. */
		while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
			const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;
			void *value = attribute->attrValues[0];

			if (!strcmp(attribute->attrName, "saAmfSGSuRestartProb")) {
			} else if (!strcmp(attribute->attrName, "saAmfSGSuRestartMax")) {
			} else if (!strcmp(attribute->attrName, "SaAmfSGCompRestartProb")) {
			} else if (!strcmp(attribute->attrName, "SaAmfSGCompRestartMax")) {
			} else if (!strcmp(attribute->attrName, "saAmfSGNumPrefInserviceSUs")) {
				uns32 pref_inservice_su;
				pref_inservice_su = *((SaUint32T *)value);

				if ((pref_inservice_su == 0) ||
				    ((avd_sg->sg_redundancy_model < SA_AMF_N_WAY_ACTIVE_REDUNDANCY_MODEL) &&
				     (pref_inservice_su < SG_2N_PREF_INSVC_SU_MIN))) {
					avd_log(NCSFL_SEV_ERROR,
						"Minimum preferred num of su should be 2 in 2N, N+M and NWay red models");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
			} else {
				avd_log(NCSFL_SEV_ERROR,
					"Attribute '%s' cannot be modified when SG is unlocked", attribute->attrName);
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
		}		/* while (attr_mod != NULL) */
	}			/* Admin state is UNLOCKED */

 done:
	return rc;
}

/*****************************************************************************
 * Function: avd_sg_ccb_apply_modify_hdlr
 *
 * Purpose: This routine applies modify operations on SaAmfSG objects.
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
static void avd_sg_ccb_apply_modify_hdlr(CcbUtilOperationData_t *opdata)
{
	AVD_SG *avd_sg;
	AVD_AMF_SG_TYPE *avd_sg_type;
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;
	AVD_SU *ncs_su = NULL;
	AVD_AVND *su_node_ptr = NULL;

	avd_log(NCSFL_SEV_NOTICE, "'%s'", opdata->objectName.value);

	avd_sg = avd_sg_find(&opdata->objectName);
	assert(avd_sg != NULL);

	/* Validate whether we can modify it. */

	if (avd_sg->saAmfSGAdminState != SA_AMF_ADMIN_UNLOCKED) {
		attr_mod = opdata->param.modify.attrMods[i++];
		while (attr_mod != NULL) {
			void *value;
			const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;

			value = attribute->attrValues[0];

			if (!strcmp(attribute->attrName, "saAmfSGType")) {
				SaNameT sg_type_name = *((SaNameT *)value);

				avd_sg->saAmfSGType = sg_type_name;
				avd_sg_type = avd_sgtype_find(&sg_type_name);
				assert(NULL != avd_sg_type);

				/* Fill the relevant values from sg_type. */
				avd_sg->saAmfSGAutoAdjustProb = avd_sg_type->saAmfSgtDefAutoAdjustProb;
				avd_sg->saAmfSGCompRestartProb = avd_sg_type->saAmfSgtDefCompRestartProb;
				avd_sg->saAmfSGCompRestartMax = avd_sg_type->saAmfSgtDefCompRestartMax;
				avd_sg->saAmfSGSuRestartProb = avd_sg_type->saAmfSgtDefSuRestartProb;
				avd_sg->saAmfSGSuRestartMax = avd_sg_type->saAmfSgtDefSuRestartMax;
				avd_sg->sg_redundancy_model = avd_sg_type->saAmfSgtRedundancyModel;

				/* Add sg to sg_type   */
				/*  TODO remove from old type list */
				avd_sg->sg_on_sg_type = avd_sg_type;
				avd_sg->sg_list_sg_type_next = avd_sg->sg_on_sg_type->list_of_sg;
				avd_sg->sg_on_sg_type->list_of_sg = avd_sg;
			}
			attr_mod = opdata->param.modify.attrMods[i++];
		}		/* while (attr_mod != NULL) */

		/* Modifications can be done for any parameters. */
		attr_mod = opdata->param.modify.attrMods[i++];
		while (attr_mod != NULL) {
			void *value;
			const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;

			value = attribute->attrValues[0];

			if (!strcmp(attribute->attrName, "saAmfSGType")) {
				/*  Just for the sake of avoiding else. */
			} else if (!strcmp(attribute->attrName, "saAmfSGAutoRepair")) {
				avd_sg->saAmfSGAutoRepair = *((SaUint32T *)value);
			} else if (!strcmp(attribute->attrName, "saAmfSGAutoAdjust")) {
				avd_sg->saAmfSGAutoAdjust = *((SaUint32T *)value);
			} else if (!strcmp(attribute->attrName, "saAmfSGNumPrefActiveSUs")) {
				avd_sg->saAmfSGNumPrefActiveSUs = *((SaUint32T *)value);
			} else if (!strcmp(attribute->attrName, "saAmfSGNumPrefStandbySUs")) {
				avd_sg->saAmfSGNumPrefStandbySUs = *((SaUint32T *)value);
			} else if (!strcmp(attribute->attrName, "saAmfSGNumPrefInserviceSUs")) {
				avd_sg->saAmfSGNumPrefInserviceSUs = *((SaUint32T *)value);
			} else if (!strcmp(attribute->attrName, "saAmfSGNumPrefAssignedSUs")) {
				avd_sg->saAmfSGNumPrefAssignedSUs = *((SaUint32T *)value);
			} else if (!strcmp(attribute->attrName, "saAmfSGMaxActiveSIsperSUs")) {
				avd_sg->saAmfSGMaxActiveSIsperSUs = *((SaUint32T *)value);
			} else if (!strcmp(attribute->attrName, "saAmfSGMaxStandbySIsperSUs")) {
				avd_sg->saAmfSGMaxStandbySIsperSUs = *((SaUint32T *)value);
			} else if (!strcmp(attribute->attrName, "saAmfSGAutoAdjustProb")) {
				avd_sg->saAmfSGAutoAdjustProb = *((SaTimeT *)value);
			} else if (!strcmp(attribute->attrName, "SaAmfSGCompRestartProb")) {
				avd_sg->saAmfSGCompRestartProb = *((SaTimeT *)value);
			} else if (!strcmp(attribute->attrName, "SaAmfSGCompRestartMax")) {
				avd_sg->saAmfSGCompRestartMax = *((SaUint32T *)value);
			} else if (!strcmp(attribute->attrName, "saAmfSGSuRestartProb")) {
				avd_sg->saAmfSGSuRestartProb = *((SaTimeT *)value);
			} else if (!strcmp(attribute->attrName, "saAmfSGSuRestartMax")) {
				avd_sg->saAmfSGSuRestartMax = *((SaUint32T *)value);
			} else {
				assert(0);
			}

			attr_mod = opdata->param.modify.attrMods[i++];
		}		/* while (attr_mod != NULL) */

		m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, avd_sg, AVSV_CKPT_AVD_SG_CONFIG);

	} /* Admin state is not UNLOCKED */
	else {			/* Admin state is UNLOCKED */
		i = 0;
		/* Modifications can be done for the following parameters. */
		attr_mod = opdata->param.modify.attrMods[i++];
		while (attr_mod != NULL) {
			void *value;
			const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;
			AVSV_PARAM_INFO param;

			value = attribute->attrValues[0];

			if (!strcmp(attribute->attrName, "saAmfSGSuRestartProb")) {
				SaTimeT su_restart_prob;
				SaTimeT temp_su_restart_prob;
				su_restart_prob = *((SaTimeT *)value);
				m_NCS_OS_HTONLL_P(&temp_su_restart_prob, su_restart_prob);

				memset(((uns8 *)&param), '\0', sizeof(AVSV_PARAM_INFO));
				param.class_id = AVSV_SA_AMF_SG;
				param.attr_id = saAmfSGSuRestartProb_ID;
				param.act = AVSV_OBJ_OPR_MOD;
				param.value_len = sizeof(SaTimeT);
				memcpy((char *)&param.value[0], (char *)&temp_su_restart_prob, param.value_len);

				avd_sg->saAmfSGSuRestartProb = su_restart_prob;

				/* This value has to be updated on each SU on this SG */
				ncs_su = avd_sg->list_of_su;
				while (ncs_su) {
					m_AVD_GET_SU_NODE_PTR(avd_cb, ncs_su, su_node_ptr);

					if ((su_node_ptr) && (su_node_ptr->node_state == AVD_AVND_STATE_PRESENT)) {
						param.name = ncs_su->name;

						if (avd_snd_op_req_msg(avd_cb, su_node_ptr, &param) != NCSCC_RC_SUCCESS) {
							assert(0);
						}
					}
					ncs_su = ncs_su->sg_list_su_next;
				}
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, avd_sg, AVSV_CKPT_AVD_SG_CONFIG);
			} else if (!strcmp(attribute->attrName, "saAmfSGSuRestartMax")) {
				uns32 su_restart_max;
				su_restart_max = *((SaUint32T *)value);

				memset(((uns8 *)&param), '\0', sizeof(AVSV_PARAM_INFO));
				param.class_id = AVSV_SA_AMF_SG;
				param.attr_id = saAmfSGSuRestartMax_ID;
				param.act = AVSV_OBJ_OPR_MOD;

				param.value_len = sizeof(uns32);
				m_NCS_OS_HTONL_P(&param.value[0], su_restart_max);

				avd_sg->saAmfSGSuRestartMax = su_restart_max;
				/* This value has to be updated on each SU on this SG */
				ncs_su = avd_sg->list_of_su;
				while (ncs_su) {
					m_AVD_GET_SU_NODE_PTR(avd_cb, ncs_su, su_node_ptr);

					if ((su_node_ptr) && (su_node_ptr->node_state == AVD_AVND_STATE_PRESENT)) {
						param.name = ncs_su->name;

						if (avd_snd_op_req_msg(avd_cb, su_node_ptr, &param)
						    != NCSCC_RC_SUCCESS) {
							assert(0);
						}
					}
					ncs_su = ncs_su->sg_list_su_next;
				}
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, avd_sg, AVSV_CKPT_AVD_SG_CONFIG);

			} else if (!strcmp(attribute->attrName, "SaAmfSGCompRestartProb")) {
				SaTimeT comp_restart_prob;
				SaTimeT temp_comp_restart_prob;
				comp_restart_prob = *((SaTimeT *)value);
				m_NCS_OS_HTONLL_P(&temp_comp_restart_prob, comp_restart_prob);

				memset(((uns8 *)&param), '\0', sizeof(AVSV_PARAM_INFO));
				param.class_id = AVSV_SA_AMF_SG;
				param.attr_id = saAmfSGCompRestartProb_ID;
				param.act = AVSV_OBJ_OPR_MOD;

				param.value_len = sizeof(SaTimeT);
				memcpy((char *)&param.value[0], (char *)&temp_comp_restart_prob, param.value_len);

				avd_sg->saAmfSGCompRestartProb = comp_restart_prob;

				/* This value has to be updated on each SU on this SG */
				ncs_su = avd_sg->list_of_su;
				while (ncs_su) {
					m_AVD_GET_SU_NODE_PTR(avd_cb, ncs_su, su_node_ptr);

					if ((su_node_ptr) && (su_node_ptr->node_state == AVD_AVND_STATE_PRESENT)) {
						param.name = ncs_su->name;

						if (avd_snd_op_req_msg(avd_cb, su_node_ptr, &param)
						    != NCSCC_RC_SUCCESS) {
							assert(0);
						}
					}
					ncs_su = ncs_su->sg_list_su_next;
				}
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, avd_sg, AVSV_CKPT_AVD_SG_CONFIG);

			} else if (!strcmp(attribute->attrName, "SaAmfSGCompRestartMax")) {
				uns32 comp_restart_max;
				comp_restart_max = *((SaUint32T *)value);

				memset(((uns8 *)&param), '\0', sizeof(AVSV_PARAM_INFO));
				param.class_id = AVSV_SA_AMF_SG;
				param.attr_id = saAmfSGCompRestartMax_ID;
				param.act = AVSV_OBJ_OPR_MOD;

				param.value_len = sizeof(uns32);
				m_NCS_OS_HTONL_P(&param.value[0], comp_restart_max);
				avd_sg->saAmfSGCompRestartMax = comp_restart_max;
				/* This value has to be updated on each SU on this SG */
				ncs_su = avd_sg->list_of_su;
				while (ncs_su) {
					m_AVD_GET_SU_NODE_PTR(avd_cb, ncs_su, su_node_ptr);

					if ((su_node_ptr) && (su_node_ptr->node_state == AVD_AVND_STATE_PRESENT)) {
						param.name = ncs_su->name;

						if (avd_snd_op_req_msg(avd_cb, su_node_ptr, &param)
						    != NCSCC_RC_SUCCESS) {
							assert(0);
						}
					}
					ncs_su = ncs_su->sg_list_su_next;
				}
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, avd_sg, AVSV_CKPT_AVD_SG_CONFIG);
			} else if (!strcmp(attribute->attrName, "saAmfSGNumPrefInserviceSUs")) {
				uns32 pref_inservice_su, back_val;
				pref_inservice_su = *((SaUint32T *)value);

				if ((pref_inservice_su == 0) ||
				    ((avd_sg->sg_redundancy_model <
				      SA_AMF_N_WAY_ACTIVE_REDUNDANCY_MODEL) &&
				     (pref_inservice_su < SG_2N_PREF_INSVC_SU_MIN))) {
					/* minimum preferred num of su should be 2 in 2N, N+M and NWay 
					   red models */
					/* log information error */
					assert(0);
				}

				back_val = avd_sg->saAmfSGNumPrefInserviceSUs;
				avd_sg->saAmfSGNumPrefInserviceSUs = pref_inservice_su;
				if (avd_sg_app_su_inst_func(avd_cb, avd_sg) != NCSCC_RC_SUCCESS) {
					assert(0);
				}
				m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, avd_sg, AVSV_CKPT_AVD_SG_CONFIG);
			} else {
				assert(0);
			}

			attr_mod = opdata->param.modify.attrMods[i++];
		}		/* while (attr_mod != NULL) */

	}			/* Admin state is UNLOCKED */
}

/*****************************************************************************
 * Function: avd_sg_admin_op_cb
 *
 * Purpose: This routine handles admin Oper on SaAmfSG objects.
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
static void avd_sg_admin_op_cb(SaImmOiHandleT immOiHandle,
			       SaInvocationT invocation,
			       const SaNameT *object_name,
			       SaImmAdminOperationIdT op_id, const SaImmAdminOperationParamsT_2 **params)
{
	SaAisErrorT rc = SA_AIS_OK;
	AVD_SG *sg;
	SaAmfAdminStateT adm_state;

	avd_log(NCSFL_SEV_NOTICE, "'%s', %llu", object_name->value, op_id);
	sg = avd_sg_find(object_name);

	if (sg->sg_ncs_spec == SA_TRUE) {
		avd_log(NCSFL_SEV_ERROR, "Admin Op on OpenSAF MW SG is not allowed");
		rc = SA_AIS_ERR_BAD_OPERATION;
		goto done;
	}

	switch (op_id) {
	case SA_AMF_ADMIN_UNLOCK:
		if (sg->saAmfSGAdminState == SA_AMF_ADMIN_UNLOCKED) {
			avd_log(NCSFL_SEV_ERROR, "%s is already unlocked", object_name->value);
			rc = SA_AIS_ERR_NO_OP;
			goto done;
		}

		if (sg->saAmfSGAdminState == SA_AMF_ADMIN_LOCKED_INSTANTIATION) {
			avd_log(NCSFL_SEV_ERROR, "%s is locked instantiation", object_name->value);
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		adm_state = sg->saAmfSGAdminState;
		m_AVD_SET_SG_ADMIN(avd_cb, sg, SA_AMF_ADMIN_UNLOCKED);
		if (avd_sg_app_sg_admin_func(avd_cb, sg) != NCSCC_RC_SUCCESS) {
			m_AVD_SET_SG_ADMIN(avd_cb, sg, adm_state);
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}
		break;

	case SA_AMF_ADMIN_LOCK:
		if (sg->saAmfSGAdminState == SA_AMF_ADMIN_LOCKED) {
			avd_log(NCSFL_SEV_ERROR, "%s is already locked", object_name->value);
			rc = SA_AIS_ERR_NO_OP;
			goto done;
		}

		if (sg->saAmfSGAdminState == SA_AMF_ADMIN_LOCKED_INSTANTIATION) {
			avd_log(NCSFL_SEV_ERROR, "%s is locked instantiation", object_name->value);
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		adm_state = sg->saAmfSGAdminState;
		m_AVD_SET_SG_ADMIN(avd_cb, sg, SA_AMF_ADMIN_LOCKED);
		if (avd_sg_app_sg_admin_func(avd_cb, sg) != NCSCC_RC_SUCCESS) {
			m_AVD_SET_SG_ADMIN(avd_cb, sg, adm_state);
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		break;
	case SA_AMF_ADMIN_SHUTDOWN:
		if (sg->saAmfSGAdminState == SA_AMF_ADMIN_SHUTTING_DOWN) {
			avd_log(NCSFL_SEV_ERROR, "%s is shutting down", object_name->value);
			rc = SA_AIS_ERR_NO_OP;
			goto done;
		}

		if ((sg->saAmfSGAdminState == SA_AMF_ADMIN_LOCKED) ||
		    (sg->saAmfSGAdminState == SA_AMF_ADMIN_LOCKED_INSTANTIATION)) {
			avd_log(NCSFL_SEV_ERROR, "%s is locked (instantiation)", object_name->value);
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		adm_state = sg->saAmfSGAdminState;
		m_AVD_SET_SG_ADMIN(avd_cb, sg, SA_AMF_ADMIN_SHUTTING_DOWN);
		if (avd_sg_app_sg_admin_func(avd_cb, sg) != NCSCC_RC_SUCCESS) {
			m_AVD_SET_SG_ADMIN(avd_cb, sg, adm_state);
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
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
	(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, rc);
}

static SaAisErrorT avd_sg_rt_attr_cb(SaImmOiHandleT immOiHandle,
				     const SaNameT *objectName, const SaImmAttrNameT *attributeNames)
{
	AVD_SG *sg = avd_sg_find(objectName);
	SaImmAttrNameT attributeName;
	int i = 0;

	avd_trace("%s", objectName->value);
	assert(sg != NULL);

	while ((attributeName = attributeNames[i++]) != NULL) {
		if (!strcmp("saAmfSGNumCurrAssignedSUs", attributeName)) {
			(void)avd_saImmOiRtObjectUpdate(objectName, attributeName,
						       SA_IMM_ATTR_SAUINT32T, &sg->saAmfSGNumCurrAssignedSUs);
		} else if (!strcmp("saAmfSGNumCurrNonInstantiatedSpareSUs", attributeName)) {
			(void)avd_saImmOiRtObjectUpdate(objectName, attributeName,
						       SA_IMM_ATTR_SAUINT32T, &sg->saAmfSGNumCurrNonInstantiatedSpareSUs);
		} else if (!strcmp("saAmfSGNumCurrInstantiatedSpareSUs", attributeName)) {
			(void)avd_saImmOiRtObjectUpdate(objectName, attributeName,
						       SA_IMM_ATTR_SAUINT32T, &sg->saAmfSGNumCurrInstantiatedSpareSUs);
		} else
			assert(0);
	}

	return SA_AIS_OK;
}

/**
 * Validate SaAmfSG CCB completed
 * @param opdata
 * 
 * @return SaAisErrorT
 */
static SaAisErrorT avd_sg_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_SG *sg;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE: {
		if ((sg = avd_sg_create(&opdata->objectName, opdata->param.create.attrValues)) == NULL)
			goto done;

		if (avd_sg_config_validate(sg) != 0) {
			avd_log(NCSFL_SEV_ERROR, "AMF SG '%s' validation error", sg->name.value);
			avd_sg_delete(sg);
			goto done;
		}

		opdata->userData = sg;	/* Save for later use in apply */
		rc = SA_AIS_OK;

		break;
	}
	case CCBUTIL_MODIFY:
		rc = avd_sg_ccb_completed_modify_hdlr(opdata);
		goto done;
		break;
	case CCBUTIL_DELETE:
		sg = avd_sg_find(&opdata->objectName);
		if (sg->saAmfSGAdminState != SA_AMF_ADMIN_LOCKED_INSTANTIATION) {
			avd_log(NCSFL_SEV_ERROR, "SG admin state is not locked instantiation");
		}
		goto done;
		if (sg->list_of_su != NULL) {
			avd_log(NCSFL_SEV_ERROR, "SUs still exist in this SG");
			goto done;
		}
		if (sg->list_of_si != NULL) {
			avd_log(NCSFL_SEV_ERROR, "SIs still exist in this SG");
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

/**
 * Apply SaAmfSG CCB changes
 * @param opdata
 * 
 * @return SaAisErrorT
 */
static void avd_sg_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		avd_sg_add_to_model(opdata->userData);
		break;
	case CCBUTIL_DELETE:{
			avd_sg_ccb_apply_delete_hdlr(opdata);
			break;
		}
	case CCBUTIL_MODIFY:
		avd_sg_ccb_apply_modify_hdlr(opdata);
		break;
	default:
		assert(0);
	}
}

/*****************************************************************************/
/************************* End Class SaAmfSG *********************************/
/*****************************************************************************/

void avd_sg_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	assert(ncs_patricia_tree_init(&avd_sg_db, &patricia_params) == NCSCC_RC_SUCCESS);
	assert(ncs_patricia_tree_init(&avd_sg_type_db, &patricia_params) == NCSCC_RC_SUCCESS);

	avd_class_impl_set("SaAmfSG", avd_sg_rt_attr_cb, avd_sg_admin_op_cb,
		avd_sg_ccb_completed_cb, avd_sg_ccb_apply_cb);
	avd_class_impl_set("SaAmfSGType", NULL, NULL, avd_sgtype_ccb_completed_cb, avd_sgtype_ccb_apply_cb);
	avd_class_impl_set("SaAmfSGBaseType", NULL, NULL, avd_imm_default_OK_completed_cb, NULL);
}
