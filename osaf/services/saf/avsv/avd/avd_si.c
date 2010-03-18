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

#include <avd_util.h>
#include <avd_si.h>
#include <avd_app.h>
#include <avd_imm.h>
#include <avd_csi.h>
#include <avd_proc.h>

static NCS_PATRICIA_TREE si_db;
static void avd_si_add_csi_db(struct avd_csi_tag* csi);

static void avd_si_arrange_dep_csi(struct avd_csi_tag* csi)
{
	AVD_CSI *temp_csi = NULL;
	AVD_SI *temp_si = NULL;
revisit:
        temp_csi = csi->si->list_of_csi;
        while (temp_csi) {
                if ((FALSE == temp_csi->dep_csi_added) && (0 == memcmp(&temp_csi->saAmfCSIDependencies, &csi->name, sizeof(SaNameT)))) {
                        temp_csi->rank = csi->rank + 1;
                        /* We need to rearrange Dep CSI rank as per modified. */
                        /* Store the SI pointer as avd_si_remove_csi makes it NULL in the end */
                        temp_si = temp_csi->si;
                        avd_si_remove_csi(temp_csi);
                        temp_csi->si = temp_si;
                        avd_si_add_csi_db(temp_csi);
                        temp_csi->dep_csi_added = TRUE;
                        /* We need to check whether any other CSI is dependent on temp_csi.*/
                        avd_si_arrange_dep_csi(temp_csi);
                        /* We need to revisit again as the list has been modified. */
                        goto revisit;
                }
                temp_csi = temp_csi->si_list_of_csi_next;
        }
	return;
}

void avd_si_add_csi(struct avd_csi_tag* avd_csi)
{
	AVD_CSI *temp_csi = NULL;
        bool found = FALSE;

        /* Find whether csi (avd_csi->saAmfCSIDependencies) is already in the DB. */
        if (avd_csi->rank != 1) /* (rank != 1) ==> (rank is 0). i.e. avd_csi is dependent on another CSI. */ {
                temp_csi = avd_csi->si->list_of_csi;
                while (temp_csi) {
                        if (0 == memcmp(&temp_csi->name, &avd_csi->saAmfCSIDependencies, sizeof(SaNameT))) {
				/* The CSI is dependent on other CSI, so its rank will be one more than Dep CSI */
				avd_csi->rank = temp_csi->rank + 1;
				found = TRUE;
                        }
			temp_csi = temp_csi->si_list_of_csi_next;
                }
                if (found == FALSE) {
			/* Not found, means saAmfCSIDependencies is yet to come. So put it in the end with rank 0. 
                           There may be existing CSIs which will be dependent on avd_csi, but don't run 
                           avd_si_arrange_dep_csi() as of now. All dependent CSI's rank will be changed when 
			   CSI on which avd_csi(avd_csi->saAmfCSIDependencies) will come.*/
                        goto add_csi;

                }
        }

        /* We need to check whether any other previously added CSI(with rank = 0) was dependent on this CSI. */
        avd_si_arrange_dep_csi(avd_csi);
add_csi:
        avd_si_add_csi_db(avd_csi);

	return;
}

static void avd_si_add_csi_db(struct avd_csi_tag* csi)
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

//	csi->si->num_csi++;
}

void avd_si_remove_csi(AVD_CSI* csi)
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
#if 0
		assert(csi->si->num_csi);
		csi->si->num_csi--;
#endif
		csi->si_list_of_csi_next = NULL;
		csi->si = AVD_SI_NULL;
	}			/* if (csi->si != AVD_SI_NULL) */
}

AVD_SI *avd_si_new(const SaNameT *dn)
{
	AVD_SI *si;

	if ((si = calloc(1, sizeof(AVD_SI))) == NULL) {
		LOG_ER("calloc FAILED");
		return NULL;
	}

	memcpy(si->name.value, dn->value, dn->length);
	si->name.length = dn->length;
	si->tree_node.key_info = (uns8 *)&si->name;
	si->si_switch = AVSV_SI_TOGGLE_STABLE;
	si->saAmfSIAdminState = SA_AMF_ADMIN_UNLOCKED;
	si->si_dep_state = AVD_SI_NO_DEPENDENCY;
	si->saAmfSIAssignmentState = SA_AMF_ASSIGNMENT_UNASSIGNED;

	return si;
}

void avd_si_delete(AVD_SI **si)
{
	unsigned int rc;

	avd_app_remove_si((*si)->si_on_app, *si);
	avd_sg_remove_si((*si)->sg_of_si, *si);
	rc = ncs_patricia_tree_del(&si_db, &(*si)->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(avd_cb, si, AVSV_CKPT_AVD_SI_CONFIG);
	free(*si);
	*si = NULL;
}

void avd_si_db_add(AVD_SI *si)
{
	unsigned int rc;

	if (avd_si_get(&si->name) == NULL) {
		rc = ncs_patricia_tree_add(&si_db, &si->tree_node);
		assert(rc == NCSCC_RC_SUCCESS);
	}
}

AVD_SI *avd_si_get(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SI *)ncs_patricia_tree_get(&si_db, (uns8 *)&tmp);
}

AVD_SI *avd_si_getnext(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SI *)ncs_patricia_tree_getnext(&si_db, (uns8 *)&tmp);
}

static void si_add_to_model(AVD_SI *si)
{
	AVD_APP *app;
	SaNameT app_name = {0};
	char *p;

	avd_si_db_add(si);

	p = strstr((char*)si->name.value, "safApp");
	app_name.length = strlen(p);
	memcpy(app_name.value, p, app_name.length);
	app = avd_app_get(&app_name);
	si->si_on_app = app;

	si->si_on_svc_type = avd_svctype_get(&si->saAmfSvcType);

	if (si->saAmfSIProtectedbySG.length > 0)
		si->sg_of_si = avd_sg_get(&si->saAmfSIProtectedbySG);

	/* Add SI to SvcType */
	si->si_list_svc_type_next = si->si_on_svc_type->list_of_si;
	si->si_on_svc_type->list_of_si = si;

	avd_app_add_si(si->si_on_app, si);
	avd_sg_add_si(si->sg_of_si, si);
	m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(avd_cb, si, AVSV_CKPT_AVD_SI_CONFIG);
	avd_saImmOiRtObjectUpdate(&si->name, "saAmfSIAssignmentState",
		SA_IMM_ATTR_SAUINT32T, &si->saAmfSIAssignmentState);
}

/**
 * Validate configuration attributes for an AMF SI object
 * @param si
 * 
 * @return int
 */
static int is_config_valid(const SaNameT *dn, const SaImmAttrValuesT_2 **attributes, CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc;
	SaNameT aname;
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

	rc = immutil_getAttr("saAmfSvcType", attributes, 0, &aname);
	assert(rc == SA_AIS_OK);

	if (avd_svctype_get(&aname) == NULL) {
		/* SVC type does not exist in current model, check CCB if passed as param */
		if (opdata == NULL) {
			LOG_ER("SVC type '%s' does not exist in model", aname.value);
			return 0;
		}

		if (ccbutil_getCcbOpDataByDN(opdata->ccbId, &aname) == NULL) {
			LOG_ER("SVC type '%s' does not exist in existing model or in CCB", aname.value);
			return 0;
		}
	}

	if (immutil_getAttr("saAmfSIProtectedbySG", attributes, 0, &aname) == SA_AIS_OK) {
		if (avd_sg_get(&aname) == NULL) {
			if (opdata == NULL) {
				LOG_ER("SG '%s' does not exist", aname.value);
				return 0;
			}

			/* SG does not exist in current model, check CCB */
			if (ccbutil_getCcbOpDataByDN(opdata->ccbId, &aname) == NULL) {
				LOG_ER("SG '%s' does not exist in existing model or in CCB", aname.value);
				return 0;
			}
		}
	}

	if ((immutil_getAttr("saAmfSIAdminState", attributes, 0, &admstate) == SA_AIS_OK) &&
	    !avd_admin_state_is_valid(admstate)) {
		LOG_ER("Invalid saAmfSIAdminState %u for '%s'", admstate, dn->value);
		return 0;
	}

	return 1;
}

static AVD_SI *si_create(SaNameT *si_name, const SaImmAttrValuesT_2 **attributes)
{
	int i, rc = -1;
	AVD_SI *si;
	SaUint32T attrValuesNumber;
	SaAisErrorT error;

	TRACE_ENTER2("'%s'", si_name->value);

	/*
	** If called at new active at failover, the object is found in the DB
	** but needs to get configuration attributes initialized.
	*/
	if (NULL == (si = avd_si_get(si_name))) {
		if ((si = avd_si_new(si_name)) == NULL)
			goto done;
	}

	error = immutil_getAttr("saAmfSvcType", attributes, 0, &si->saAmfSvcType);
	assert(error == SA_AIS_OK);

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

	rc = 0;

done:
	if (rc != 0)
		avd_si_delete(&si);

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

	TRACE_ENTER();

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	if (immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, &app->name, SA_IMM_SUBTREE,
	      SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
	      NULL, &searchHandle) != SA_AIS_OK) {

		LOG_ER("No objects found (1)");
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &si_name, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {
		if (!is_config_valid(&si_name, attributes, NULL))
			goto done2;

		if ((si = si_create(&si_name, attributes)) == NULL)
			goto done2;

		si_add_to_model(si);

		if (avd_sirankedsu_config_get(&si_name, si) != SA_AIS_OK)
			goto done2;

		if (avd_csi_config_get(&si_name, si) != SA_AIS_OK)
			goto done2;
	}

	error = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:
	TRACE_LEAVE2("%u", error);
	return error;
}

static SaAisErrorT si_ccb_completed_modify_hdlr(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_OK;
	AVD_SI *si;
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;

	si = avd_si_get(&opdata->objectName);
	assert(si != NULL);

	/* Modifications can only be done for these attributes. */
	i = 0;
	while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
		const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;

		if (strcmp(attribute->attrName, "saAmfSIPrefActiveAssignments") &&
		    strcmp(attribute->attrName, "saAmfSIPrefStandbyAssignments")) {
			LOG_ER("Modification of attribute %s is not supported", attribute->attrName);
			rc = SA_AIS_ERR_BAD_OPERATION;
			break;
		}
	}

	return rc;
}

static void si_admin_op_cb(SaImmOiHandleT immOiHandle, SaInvocationT invocation,
	const SaNameT *objectName, SaImmAdminOperationIdT operationId,
	const SaImmAdminOperationParamsT_2 **params)
{
	SaAisErrorT rc = SA_AIS_OK;
	AVD_SI *avd_si;
	SaAmfAdminStateT adm_state = 0;
	SaAmfAdminStateT back_val;

	avd_si = avd_si_get(objectName);

	if (avd_si->sg_of_si->sg_ncs_spec == SA_TRUE) {
		LOG_ER("Admin operations of OpenSAF MW is not supported");
		rc = SA_AIS_ERR_BAD_OPERATION;
		goto done;
	}

	switch (operationId) {
	case SA_AMF_ADMIN_UNLOCK:
		if (SA_AMF_ADMIN_UNLOCKED == avd_si->saAmfSIAdminState) {
			LOG_ER("already unlocked");
			rc = SA_AIS_ERR_NO_OP;
			goto done;
		}

		if (SA_AMF_ADMIN_LOCKED_INSTANTIATION == avd_si->saAmfSIAdminState) {
			LOG_ER("locked instantiation");
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		if (avd_si->sg_of_si->sg_fsm_state != AVD_SG_FSM_STABLE) {
			LOG_ER("SG of SI is not in stable state");
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		if (avd_si->max_num_csi == avd_si->num_csi) {
			switch (avd_si->sg_of_si->sg_redundancy_model) {
			case SA_AMF_2N_REDUNDANCY_MODEL:
				if (avd_sg_2n_si_func(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
					LOG_ER("avd_sg_2n_si_func failed");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				break;

			case SA_AMF_N_WAY_ACTIVE_REDUNDANCY_MODEL:
				if (avd_sg_nacvred_si_func(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
					m_AVD_SET_SI_ADMIN(avd_cb, avd_si, SA_AMF_ADMIN_LOCKED);
					LOG_ER("avd_sg_nacvred_si_func failed");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				break;

			case SA_AMF_N_WAY_REDUNDANCY_MODEL:
				if (avd_sg_nway_si_func(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
					m_AVD_SET_SI_ADMIN(avd_cb, avd_si, SA_AMF_ADMIN_LOCKED);
					LOG_ER("avd_sg_nway_si_func failed");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				break;

			case SA_AMF_NPM_REDUNDANCY_MODEL:
				if (avd_sg_npm_si_func(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
					m_AVD_SET_SI_ADMIN(avd_cb, avd_si, SA_AMF_ADMIN_LOCKED);
					LOG_ER("avd_sg_npm_si_func failed");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				break;

			case SA_AMF_NO_REDUNDANCY_MODEL:
			default:
				if (avd_sg_nored_si_func(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
					m_AVD_SET_SI_ADMIN(avd_cb, avd_si, SA_AMF_ADMIN_LOCKED);
					LOG_ER("avd_sg_nored_si_func failed");
					rc = SA_AIS_ERR_BAD_OPERATION;
					goto done;
				}
				break;
			}
		}

		break;

	case SA_AMF_ADMIN_SHUTDOWN:
		if (SA_AMF_ADMIN_SHUTTING_DOWN == avd_si->saAmfSIAdminState) {
			LOG_ER("already shutting down");
			rc = SA_AIS_ERR_NO_OP;
			goto done;
		}

		if ((SA_AMF_ADMIN_LOCKED == avd_si->saAmfSIAdminState) ||
		    (SA_AMF_ADMIN_LOCKED_INSTANTIATION == avd_si->saAmfSIAdminState)) {
			LOG_ER("is locked (instantiation)");
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}
		adm_state = SA_AMF_ADMIN_SHUTTING_DOWN;

		/* Don't break */

	case SA_AMF_ADMIN_LOCK:
		if (0 == adm_state)
			adm_state = SA_AMF_ADMIN_LOCKED;

		if (SA_AMF_ADMIN_LOCKED == avd_si->saAmfSIAdminState) {
			LOG_ER("already locked");
			rc = SA_AIS_ERR_NO_OP;
			goto done;
		}

		if (SA_AMF_ADMIN_LOCKED_INSTANTIATION == avd_si->saAmfSIAdminState) {
			LOG_ER("locked instantiation");
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		if (avd_si->list_of_sisu == AVD_SU_SI_REL_NULL) {
			m_AVD_SET_SI_ADMIN(avd_cb, avd_si, SA_AMF_ADMIN_LOCKED);
			LOG_WA("SI has no assignments");
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		/* SI lock should not be done, this SI is been DISABLED because
		   of SI-SI dependency */
		if ((avd_si->si_dep_state != AVD_SI_ASSIGNED) && (avd_si->si_dep_state != AVD_SI_TOL_TIMER_RUNNING)) {
			LOG_WA("DISABLED because of SI-SI dependency");
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		/* Check if other semantics are happening for other SUs. If yes
		 * return an error.
		 */
		if (avd_si->sg_of_si->sg_fsm_state != AVD_SG_FSM_STABLE) {
			LOG_WA("'%s' SG is not STABLE", objectName->value);
			if ((avd_si->sg_of_si->sg_fsm_state != AVD_SG_FSM_SI_OPER) ||
			    (avd_si->saAmfSIAdminState != SA_AMF_ADMIN_SHUTTING_DOWN) ||
			    (adm_state != SA_AMF_ADMIN_LOCKED)) {
				LOG_WA("'%s' other semantics...", objectName->value);
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
				LOG_ER("avd_sg_2n_si_admin_down failed");
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
			break;

		case SA_AMF_N_WAY_REDUNDANCY_MODEL:
			if (avd_sg_nway_si_admin_down(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
				m_AVD_SET_SI_ADMIN(avd_cb, avd_si, back_val);
				LOG_ER("avd_sg_nway_si_admin_down failed");
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
			break;

		case SA_AMF_N_WAY_ACTIVE_REDUNDANCY_MODEL:
			if (avd_sg_nacvred_si_admin_down(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
				m_AVD_SET_SI_ADMIN(avd_cb, avd_si, back_val);
				LOG_ER("avd_sg_nacvred_si_admin_down failed");
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
			break;

		case SA_AMF_NPM_REDUNDANCY_MODEL:
			if (avd_sg_npm_si_admin_down(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
				m_AVD_SET_SI_ADMIN(avd_cb, avd_si, back_val);
				LOG_ER("avd_sg_npm_si_admin_down failed");
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
			break;

		case SA_AMF_NO_REDUNDANCY_MODEL:
		default:
			if (avd_sg_nored_si_admin_down(avd_cb, avd_si) != NCSCC_RC_SUCCESS) {
				m_AVD_SET_SI_ADMIN(avd_cb, avd_si, back_val);
				LOG_ER("avd_sg_nored_si_admin_down failed");
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
	(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, rc);
}

static SaAisErrorT si_rt_attr_cb(SaImmOiHandleT immOiHandle,
	const SaNameT *objectName, const SaImmAttrNameT *attributeNames)
{
	AVD_SI *si = avd_si_get(objectName);
	SaImmAttrNameT attributeName;
	int i = 0;

	TRACE_ENTER2("%s", objectName->value);
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

static SaAisErrorT si_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_SI *si;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if (is_config_valid(&opdata->objectName, opdata->param.create.attrValues, opdata))
			rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		rc = si_ccb_completed_modify_hdlr(opdata);
		break;
	case CCBUTIL_DELETE:
		si = avd_si_get(&opdata->objectName);
		if ((NULL != si->list_of_csi) || (NULL != si->list_of_csi)) {
			LOG_ER("SaAmfSI is in use");
			goto done;
		}
		rc = SA_AIS_OK;
		opdata->userData = si;	/* Save for later use in apply */
		break;
	default:
		assert(0);
		break;
	}
done:
	TRACE_LEAVE2("%u", rc);
	return rc;
}

static void si_ccb_apply_modify_hdlr(CcbUtilOperationData_t *opdata)
{
	AVD_SI *si;
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	si = avd_si_get(&opdata->objectName);
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

static void si_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	AVD_SI *si;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		si = si_create(&opdata->objectName, opdata->param.create.attrValues);
		assert(si);
		si_add_to_model(si);
		break;
	case CCBUTIL_DELETE:
		si = opdata->userData;
		avd_si_delete(&si);
		break;
	case CCBUTIL_MODIFY:
		si_ccb_apply_modify_hdlr(opdata);
		break;
	default:
		assert(0);
		break;
	}

	TRACE_LEAVE();
}

/**
 * Update the SI assignment state. See table 11 p.88 in AMF B.04
 * IMM is updated aswell as the standby peer.
 * @param si
 */
static void si_update_ass_state(AVD_SI *si)
{
	SaAmfAssignmentStateT newState;

	if (si->saAmfSINumCurrActiveAssignments == 0)
		newState = SA_AMF_ASSIGNMENT_UNASSIGNED;
	else {
		switch (si->sg_of_si->sg_on_sg_type->saAmfSgtRedundancyModel) {
		case SA_AMF_2N_REDUNDANCY_MODEL:
			/* fall through */
		case SA_AMF_NPM_REDUNDANCY_MODEL:
			if (si->saAmfSINumCurrActiveAssignments == 1 && si->saAmfSINumCurrStandbyAssignments == 1)
				newState = SA_AMF_ASSIGNMENT_FULLY_ASSIGNED;
			else
				newState = SA_AMF_ASSIGNMENT_PARTIALLY_ASSIGNED;
			break;
		case SA_AMF_N_WAY_REDUNDANCY_MODEL:
			if (si->saAmfSINumCurrActiveAssignments == 1 &&
				si->saAmfSINumCurrStandbyAssignments == si->saAmfSIPrefStandbyAssignments)
				newState = SA_AMF_ASSIGNMENT_FULLY_ASSIGNED;
			else
				newState = SA_AMF_ASSIGNMENT_PARTIALLY_ASSIGNED;
			break;
		case SA_AMF_N_WAY_ACTIVE_REDUNDANCY_MODEL:
			if (si->saAmfSINumCurrActiveAssignments == si->saAmfSIPrefActiveAssignments &&
				si->saAmfSINumCurrStandbyAssignments == 0)
				newState = SA_AMF_ASSIGNMENT_FULLY_ASSIGNED;
			else
				newState = SA_AMF_ASSIGNMENT_PARTIALLY_ASSIGNED;
			break;
		case SA_AMF_NO_REDUNDANCY_MODEL:
			if (si->saAmfSINumCurrActiveAssignments == 1 && si->saAmfSINumCurrStandbyAssignments == 0)
				newState = SA_AMF_ASSIGNMENT_FULLY_ASSIGNED;
			else
				newState = SA_AMF_ASSIGNMENT_PARTIALLY_ASSIGNED;
			break;
		default:
			assert(0);
		}
	}

	if (newState != si->saAmfSIAssignmentState) {
		/* Log? Send notification? */
		si->saAmfSIAssignmentState = newState;
		avd_saImmOiRtObjectUpdate(&si->name, "saAmfSIAssignmentState",
								  SA_IMM_ATTR_SAUINT32T, &si->saAmfSIAssignmentState);
		m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, si, AVSV_CKPT_SI_ASSIGNMENT_STATE);
	}
}

void avd_si_inc_curr_act_ass(AVD_SI *si)
{
	si->saAmfSINumCurrActiveAssignments++;
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, si, AVSV_CKPT_SI_SU_CURR_ACTIVE);
	si_update_ass_state(si);
}

void avd_si_dec_curr_act_ass(AVD_SI *si)
{
	assert(si->saAmfSINumCurrActiveAssignments > 0);
	si->saAmfSINumCurrActiveAssignments--;
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, si, AVSV_CKPT_SI_SU_CURR_ACTIVE);
	si_update_ass_state(si);
}

void avd_si_inc_curr_stdby_ass(AVD_SI *si)
{
	si->saAmfSINumCurrStandbyAssignments++;
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, si, AVSV_CKPT_SI_SU_CURR_STBY);
	si_update_ass_state(si);
}

void avd_si_dec_curr_stdby_ass(AVD_SI *si)
{
	assert(si->saAmfSINumCurrStandbyAssignments > 0);
	si->saAmfSINumCurrStandbyAssignments--;
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, si, AVSV_CKPT_SI_SU_CURR_STBY);
	si_update_ass_state(si);
}

void avd_si_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	assert(ncs_patricia_tree_init(&si_db, &patricia_params) == NCSCC_RC_SUCCESS);
	avd_class_impl_set("SaAmfSI", si_rt_attr_cb, si_admin_op_cb, si_ccb_completed_cb, si_ccb_apply_cb);
}

