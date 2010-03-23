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

#include <avd_dblog.h>
#include <avd_util.h>
#include <avd_cluster.h>
#include <avd_app.h>
#include <avd_imm.h>
#include <avd_sutype.h>
#include <avd_ckpt_msg.h>
#include <avd_ntf.h>
#include <avd_sg.h>
#include <avd_proc.h>

static NCS_PATRICIA_TREE sg_db;

void avd_sg_db_add(AVD_SG *sg)
{
	unsigned int rc;

	if (avd_sg_get(&sg->name) == NULL) {
		rc = ncs_patricia_tree_add(&sg_db, &sg->tree_node);
		assert(rc == NCSCC_RC_SUCCESS);
	}
}

void avd_sg_db_remove(AVD_SG *sg)
{
	unsigned int rc = ncs_patricia_tree_del(&sg_db, &sg->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
}

/**
 * Add the SG to the model
 * @param sg
 */
static void sg_add_to_model(AVD_SG *sg)
{
	SaNameT app_name;
	AVD_APP *app;
	char *p;

	memset(&app_name, 0, sizeof(app_name));
	p = strstr((char*)sg->name.value, "safApp");
	app_name.length = strlen(p);
	memcpy(app_name.value, p, app_name.length);
	app = avd_app_get(&app_name);
	sg->sg_on_app = app;


        if ((avd_sg_get(&sg->name) != NULL) && (TRUE == sg->add_to_model)) {
                /* Means the it has been added into db and links with other objects alraedy created. */
                return;
        }
	avd_sg_db_add(sg);
	avd_sgtype_add_sg(sg);
	avd_app_add_sg(sg->sg_on_app, sg);

	/* SGs belonging to a magic app will be NCS, TODO Better name! */
	if (!strcmp((char *)app_name.value, "safApp=OpenSAF"))
		sg->sg_ncs_spec = TRUE;

	m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(avd_cb, sg, AVSV_CKPT_AVD_SG_CONFIG);
        sg->add_to_model = TRUE;
}

/**
 * Remove the SG to the model
 * @param sg
 */
static void sg_remove_from_model(AVD_SG *sg)
{
	avd_sgtype_remove_sg(sg);
	avd_app_remove_sg(sg->sg_on_app, sg);
	avd_sg_db_remove(sg);

	m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(avd_cb, sg, AVSV_CKPT_AVD_SG_CONFIG);
}

AVD_SG *avd_sg_new(const SaNameT *dn)
{
	AVD_SG *sg;

	if ((sg = calloc(1, sizeof(AVD_SG))) == NULL) {
		LOG_ER("calloc FAILED");
		return NULL;
	}

	memcpy(sg->name.value, dn->value, dn->length);
	sg->name.length = dn->length;
	sg->tree_node.key_info = (uns8 *)&(sg->name);
	sg->sg_ncs_spec = SA_FALSE;
	sg->sg_fsm_state = AVD_SG_FSM_STABLE;
	sg->adjust_state = AVSV_SG_STABLE;

	return sg;
}

void avd_sg_delete(AVD_SG **sg)
{
	sg_remove_from_model(*sg);
	avd_sg_db_remove(*sg);
	free(*sg);
	*sg = NULL;
}

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

void avd_sg_remove_si(AVD_SG *sg, AVD_SI* si)
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

AVD_SG *avd_sg_get(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SG *)ncs_patricia_tree_get(&sg_db, (uns8 *)&tmp);
}

AVD_SG *avd_sg_getnext(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SG *)ncs_patricia_tree_getnext(&sg_db, (uns8 *)&tmp);
}

/**
 * Validate configuration attributes for an SaAmfSG object
 * 
 * @param dn
 * @param sg
 * 
 * @return int
 */
static int is_config_valid(const SaNameT *dn, const SaImmAttrValuesT_2 **attributes, CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc;
	SaNameT aname;
	SaBoolT abool;
	SaAmfAdminStateT admstate;
	char *parent;

	if ((parent = strchr((char*)dn->value, ',')) == NULL) {
		LOG_ER("No parent to '%s' ", dn->value);
		return 0;
	}

	if (strncmp(++parent, "safApp=", 7) != 0) {
		LOG_ER("Wrong parent '%s' to '%s' ", parent, dn->value);
		return 0;
	}

	rc = immutil_getAttr("saAmfSGType", attributes, 0, &aname);
	assert(rc == SA_AIS_OK);

	if (avd_sgtype_get(&aname) == NULL) {
		if (opdata == NULL) {
			LOG_ER("SG type '%s' does not exist in model", aname.value);
			return 0;
		}

		/* SG type does not exist in current model, check CCB */
		if (ccbutil_getCcbOpDataByDN(opdata->ccbId, &aname) == NULL) {
			LOG_ER("SG type '%s' does not exist in existing model or in CCB", aname.value);
			return 0;
		}
	}

	if ((immutil_getAttr("saAmfSGAutoRepair", attributes, 0, &abool) == SA_AIS_OK) &&
	    (abool > SA_TRUE)) {
		LOG_ER("Invalid saAmfSGAutoRepair %u for '%s'", abool, dn->value);
		return 0;
	}

	if ((immutil_getAttr("saAmfSGAutoAdjust", attributes, 0, &abool) == SA_AIS_OK) &&
	    (abool > SA_TRUE)) {
		LOG_ER("Invalid saAmfSGAutoAdjust %u for '%s'", abool, dn->value);
		return 0;
	}

	if ((immutil_getAttr("saAmfSGAdminState", attributes, 0, &admstate) == SA_AIS_OK) &&
	    !avd_admin_state_is_valid(admstate)) {
		LOG_ER("Invalid saAmfSGAdminState %u for '%s'", admstate, dn->value);
		return 0;
	}

	if ((immutil_getAttr("saAmfSGSuHostNodeGroup", attributes, 0, &aname) == SA_AIS_OK) &&
	    (avd_ng_get(&aname) == NULL)) {
		LOG_ER("Invalid saAmfSGSuHostNodeGroup '%s' for '%s'", aname.value, dn->value);
		return 0;
	}

	return 1;
}

static AVD_SG *sg_create(const SaNameT *sg_name, const SaImmAttrValuesT_2 **attributes)
{
	int rc = -1;
	AVD_SG *sg;
	AVD_AMF_SG_TYPE *sgt;
	SaAisErrorT error;

	TRACE_ENTER2("'%s'", sg_name->value);

	/*
	** If called at new active at failover, the object is found in the DB
	** but needs to get configuration attributes initialized.
	*/
	if (NULL == (sg = avd_sg_get(sg_name))) {
		if ((sg = avd_sg_new(sg_name)) == NULL)
			goto done;
	}

	error = immutil_getAttr("saAmfSGType", attributes, 0, &sg->saAmfSGType);
	assert(error == SA_AIS_OK);

	sgt = avd_sgtype_get(&sg->saAmfSGType);
	assert(sgt);
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

	rc = 0;

done:
	if (rc != 0)
		avd_sg_delete(&sg);

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

	TRACE_ENTER();

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, app_dn, SA_IMM_SUBTREE,
		SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
		NULL, &searchHandle);

	if (SA_AIS_OK != error) {
		LOG_ER("saImmOmSearchInitialize_2 failed: %u", error);
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn,
		(SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {

		if (!is_config_valid(&dn, attributes, NULL)) {
			error = SA_AIS_ERR_FAILED_OPERATION;
			goto done2;
		}

		if ((sg = sg_create(&dn, attributes)) == NULL) {
			error = SA_AIS_ERR_FAILED_OPERATION;
			goto done2;
		}

		sg_add_to_model(sg);

		if (avd_su_config_get(&dn, sg) != SA_AIS_OK) {
			error = SA_AIS_ERR_FAILED_OPERATION;
			goto done2;
		}
	}

	error = SA_AIS_OK;

done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
done1:
	TRACE_LEAVE2("%u", error);
	return error;
}

static void sg_ccb_apply_delete_hdlr(AVD_SG *sg)
{
	TRACE_ENTER2("'%s'", sg->name.value);

	/* check point to the standby AVD that this record need to be deleted */
	m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(avd_cb, sg, AVSV_CKPT_AVD_SG_CONFIG);
	avd_sg_delete(&sg);
}

static SaAisErrorT ccb_completed_modify_hdlr(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_OK;
	AVD_SG *avd_sg = NULL;
	AVD_AMF_SG_TYPE *avd_sg_type = NULL;
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;

	avd_sg = avd_sg_get(&opdata->objectName);
	assert(avd_sg != NULL);

	/* Validate whether we can modify it. */

	/*  TODO... */
	if (avd_sg->saAmfSGAdminState != SA_AMF_ADMIN_UNLOCKED) {

		while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
			const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;
			void *value = attribute->attrValues[0];

			if (!strcmp(attribute->attrName, "saAmfSGType")) {
				SaNameT sg_type_name = *((SaNameT *)value);

				avd_sg_type = avd_sgtype_get(&sg_type_name);
				if (NULL == avd_sg_type) {
					LOG_ER("SG Type '%s' not found", sg_type_name.value);
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

static void ccb_apply_modify_hdlr(CcbUtilOperationData_t *opdata)
{
	AVD_SG *avd_sg;
	AVD_AMF_SG_TYPE *avd_sg_type;
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;
	AVD_SU *ncs_su = NULL;
	AVD_AVND *su_node_ptr = NULL;

	TRACE_ENTER2("'%s'", opdata->objectName.value);

	avd_sg = avd_sg_get(&opdata->objectName);
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
				avd_sg_type = avd_sgtype_get(&sg_type_name);
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

static void sg_admin_op_cb(SaImmOiHandleT immOiHandle, SaInvocationT invocation,
	const SaNameT *object_name, SaImmAdminOperationIdT op_id,
	const SaImmAdminOperationParamsT_2 **params)
{
	SaAisErrorT rc = SA_AIS_OK;
	AVD_SG *sg;
	SaAmfAdminStateT adm_state;

	avd_log(NCSFL_SEV_NOTICE, "'%s', %llu", object_name->value, op_id);
	sg = avd_sg_get(object_name);

	if (sg->sg_ncs_spec == SA_TRUE) {
		LOG_ER("Admin Op on OpenSAF MW SG is not allowed");
		rc = SA_AIS_ERR_BAD_OPERATION;
		goto done;
	}

	switch (op_id) {
	case SA_AMF_ADMIN_UNLOCK:
		if (sg->saAmfSGAdminState == SA_AMF_ADMIN_UNLOCKED) {
			LOG_ER("%s is already unlocked", object_name->value);
			rc = SA_AIS_ERR_NO_OP;
			goto done;
		}

		if (sg->saAmfSGAdminState == SA_AMF_ADMIN_LOCKED_INSTANTIATION) {
			LOG_ER("%s is locked instantiation", object_name->value);
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
			LOG_ER("%s is already locked", object_name->value);
			rc = SA_AIS_ERR_NO_OP;
			goto done;
		}

		if (sg->saAmfSGAdminState == SA_AMF_ADMIN_LOCKED_INSTANTIATION) {
			LOG_ER("%s is locked instantiation", object_name->value);
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
			LOG_ER("%s is shutting down", object_name->value);
			rc = SA_AIS_ERR_NO_OP;
			goto done;
		}

		if ((sg->saAmfSGAdminState == SA_AMF_ADMIN_LOCKED) ||
		    (sg->saAmfSGAdminState == SA_AMF_ADMIN_LOCKED_INSTANTIATION)) {
			LOG_ER("%s is locked (instantiation)", object_name->value);
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

static SaAisErrorT sg_rt_attr_cb(SaImmOiHandleT immOiHandle,
	const SaNameT *objectName, const SaImmAttrNameT *attributeNames)
{
	AVD_SG *sg = avd_sg_get(objectName);
	SaImmAttrNameT attributeName;
	int i = 0;

	TRACE_ENTER2("'%s'", objectName->value);
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
static SaAisErrorT sg_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_SG *sg = NULL;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if (is_config_valid(&opdata->objectName, opdata->param.create.attrValues, opdata))
			rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		rc = ccb_completed_modify_hdlr(opdata);
		break;
	case CCBUTIL_DELETE:
		sg = avd_sg_get(&opdata->objectName);
		if (sg->saAmfSGAdminState != SA_AMF_ADMIN_LOCKED_INSTANTIATION) {
			LOG_ER("SG admin state is not locked instantiation");
			goto done;
		}
		if (sg->list_of_su != NULL) {
			LOG_ER("SUs still exist in this SG");
			goto done;
		}
		if (sg->list_of_si != NULL) {
			LOG_ER("SIs still exist in this SG");
			goto done;
		}
		rc = SA_AIS_OK;
		opdata->userData = sg;	/* Save for later use in apply */
		break;
	default:
		assert(0);
		break;
	}
done:
	TRACE_LEAVE2("%u", rc);
	return rc;
}

/**
 * Apply SaAmfSG CCB changes
 * @param opdata
 * 
 * @return SaAisErrorT
 */
static void sg_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	AVD_SG *sg;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		sg = sg_create(&opdata->objectName, opdata->param.create.attrValues);
		assert(sg);
		sg_add_to_model(sg);
		break;
	case CCBUTIL_DELETE:
		sg_ccb_apply_delete_hdlr(opdata->userData);
		break;
	case CCBUTIL_MODIFY:
		ccb_apply_modify_hdlr(opdata);
		break;
	default:
		assert(0);
	}
}

void avd_sg_remove_su(AVD_SU* su)
{
	AVD_SU *i_su = NULL;
	AVD_SU *prev_su = NULL;

	if (su->sg_of_su != NULL) {
		/* remove SU from SG */
		i_su = su->sg_of_su->list_of_su;

		while ((i_su != NULL) && (i_su != su)) {
			prev_su = i_su;
			i_su = i_su->sg_list_su_next;
		}

		if (i_su != su) {
			/* Log a fatal error */
		} else {
			if (prev_su == NULL) {
				su->sg_of_su->list_of_su = su->sg_list_su_next;
			} else {
				prev_su->sg_list_su_next = su->sg_list_su_next;
			}
		}

		su->sg_list_su_next = NULL;
		su->sg_of_su = NULL;
	}
	/* if (su->sg_of_su != AVD_SG_NULL) */
	return;
}

void avd_sg_add_su(AVD_SU* su)
{
	AVD_SU *i_su = NULL;
	AVD_SU *prev_su = NULL;

	if ((su == NULL) || (su->sg_of_su == NULL))
		return;

	i_su = su->sg_of_su->list_of_su;

	while ((i_su != NULL) && (i_su->saAmfSURank < su->saAmfSURank)) {
		prev_su = i_su;
		i_su = i_su->sg_list_su_next;
	}

	if (prev_su == NULL) {
		su->sg_list_su_next = su->sg_of_su->list_of_su;
		su->sg_of_su->list_of_su = su;
	} else {
		prev_su->sg_list_su_next = su;
		su->sg_list_su_next = i_su;
	}
}

void avd_sg_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	assert(ncs_patricia_tree_init(&sg_db, &patricia_params) == NCSCC_RC_SUCCESS);

	avd_class_impl_set("SaAmfSG", sg_rt_attr_cb, sg_admin_op_cb, sg_ccb_completed_cb, sg_ccb_apply_cb);
}
