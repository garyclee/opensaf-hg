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
  the component database on the AVD.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <saImmOm.h>
#include <immutil.h>
#include <avd_util.h>
#include <avd_dblog.h>
#include <avd_comp.h>
#include <avd_imm.h>
#include <avd_node.h>
#include <avd_csi.h>
#include <avd_proc.h>
#include <avd_ckpt_msg.h>

static SaAisErrorT avd_compcstype_config_get(SaNameT *comp_name, AVD_COMP *comp);
static void avd_comptype_del_comp(AVD_COMP* comp);

static NCS_PATRICIA_TREE avd_comp_db;
static NCS_PATRICIA_TREE avd_compcstype_db;

void avd_comp_pres_state_set(AVD_COMP *comp, SaAmfPresenceStateT pres_state)
{
	assert(pres_state <= SA_AMF_PRESENCE_TERMINATION_FAILED);
	avd_log(NCSFL_SEV_NOTICE, "'%s' %s => %s",
		comp->comp_info.name.value, avd_pres_state_name[comp->saAmfCompPresenceState],
		avd_pres_state_name[pres_state]);
	comp->saAmfCompPresenceState = pres_state;
	avd_saImmOiRtObjectUpdate(&comp->comp_info.name,
		"saAmfCompPresenceState", SA_IMM_ATTR_SAUINT32T, &comp->saAmfCompPresenceState);
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, comp, AVSV_CKPT_COMP_PRES_STATE);
}

void avd_comp_oper_state_set(AVD_COMP *comp, SaAmfOperationalStateT oper_state)
{
	assert(oper_state <= SA_AMF_OPERATIONAL_DISABLED);
	avd_log(NCSFL_SEV_NOTICE, "'%s' %s => %s",
		comp->comp_info.name.value, avd_oper_state_name[comp->saAmfCompOperState], avd_oper_state_name[oper_state]);
	comp->saAmfCompOperState = oper_state;
	avd_saImmOiRtObjectUpdate(&comp->comp_info.name,
		"saAmfCompOperState", SA_IMM_ATTR_SAUINT32T, &comp->saAmfCompOperState);
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, comp, AVSV_CKPT_COMP_OPER_STATE);
}

void avd_comp_readiness_state_set(AVD_COMP *comp, SaAmfReadinessStateT readiness_state)
{
	assert(readiness_state <= SA_AMF_READINESS_STOPPING);
	avd_log(NCSFL_SEV_NOTICE, "'%s' %s => %s",
		comp->comp_info.name.value,
		avd_readiness_state_name[comp->saAmfCompReadinessState], avd_readiness_state_name[readiness_state]);
	comp->saAmfCompReadinessState = readiness_state;
	avd_saImmOiRtObjectUpdate(&comp->comp_info.name,
		 "saAmfCompReadinessState", SA_IMM_ATTR_SAUINT32T, &comp->saAmfCompReadinessState);
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, comp, AVSV_CKPT_COMP_READINESS_STATE);
}

/*****************************************************************************
 * Function: avd_comp_find
 *
 * Purpose:  This function will find a AVD_COMP structure in the
 * tree with comp_name value as key.
 *
 * Input: dn - The name of the component.
 *
 * Returns: The pointer to AVD_COMP structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_COMP *avd_comp_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_COMP *)ncs_patricia_tree_get(&avd_comp_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_comp_getnext
 *
 * Purpose:  This function will find the next AVD_COMP structure in the
 * tree whose comp_name value is next of the given comp_name value.
 *
 * Input: dn - The name of the component.
 *
 * Returns: The pointer to AVD_COMP structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_COMP *avd_comp_getnext(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_COMP *)ncs_patricia_tree_getnext(&avd_comp_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_comp_delete
 *
 * Purpose:  This function will delete and free AVD_COMP structure from 
 * the tree. If the node need to be deleted with saNameT, first do a find
 * and call this function with the AVD_COMP pointer.
 *
 * Input: comp - The component structure that needs to be deleted.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_comp_delete(AVD_COMP* comp)
{
	avd_su_del_comp(comp);
	avd_comptype_del_comp(comp);
	(void)ncs_patricia_tree_del(&avd_comp_db, &comp->tree_node);
	free(comp);
}

/*****************************************************************************
 * Function: avd_comp_ack_msg
 *
 * Purpose:  This function processes the acknowledgment received for
 *          a non proxied component or proxied component related message 
 *          from AVND for operator related changes. If the message
 *          acknowledges success nothing to be done. If failure is
 *          reported a error is logged.
 *
 * Input: cb - the AVD control block
 *        ack_msg - The acknowledgement message received from the AVND.
 *
 * Returns: none.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_comp_ack_msg(AVD_CL_CB *cb, AVD_DND_MSG *ack_msg)
{

	AVD_COMP *comp, *i_comp;
	AVSV_PARAM_INFO param;
	AVD_AVND *avnd;
	uns32 min_si = 0;
	NCS_BOOL isPre;
	AVD_AVND *su_node_ptr = NULL;

	/* check the ack message for errors. If error find the component that
	 * has error.
	 */

	if (ack_msg->msg_info.n2d_reg_comp.error != NCSCC_RC_SUCCESS) {
		/* Find the component */
		comp = avd_comp_find(&ack_msg->msg_info.n2d_reg_comp.comp_name);
		if (comp == NULL) {
			/* The component has already been deleted. there is nothing
			 * that can be done.
			 */

			/* Log an information error that the component is
			 * deleted.
			 */
			return;
		}

		/* log an fatal error as normally this shouldnt happen.
		 */
		m_AVD_LOG_INVALID_VAL_ERROR(ack_msg->msg_info.n2d_reg_comp.error);

		/* Make the row status as not in service to indicate that. It is
		 * not active.
		 */

		/* verify if the max ACTIVE and STANDBY SIs of the SU 
		 ** need to be changed
		 */
		if (comp->max_num_csi_actv == comp->su->si_max_active) {
			/* find the number and set it */
			min_si = 0;
			i_comp = comp->su->list_of_comp;
			while (i_comp) {
				if (i_comp != comp) {
					if (min_si > i_comp->max_num_csi_actv)
						min_si = i_comp->max_num_csi_actv;
					else if (min_si == 0)
						min_si = i_comp->max_num_csi_actv;
				}
				i_comp = i_comp->su_comp_next;
			}
			/* Now we have the min value. set it */
			comp->su->si_max_active = min_si;
		}

		/* FOR STANDBY count */
		if (comp->max_num_csi_stdby == comp->su->si_max_standby) {
			/* find the number and set it */
			min_si = 0;
			i_comp = comp->su->list_of_comp;
			while (i_comp) {
				if (i_comp != comp) {
					if (min_si > i_comp->max_num_csi_stdby)
						min_si = i_comp->max_num_csi_stdby;
					else if (min_si == 0)
						min_si = i_comp->max_num_csi_stdby;
				}
				i_comp = i_comp->su_comp_next;
			}
			/* Now we have the min value. set it */
			comp->su->si_max_standby = min_si;
		}

		/* Verify if the SUs preinstan value need to be changed */
		if ((NCS_COMP_TYPE_SA_AWARE == comp->comp_info.category) ||
		    (NCS_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE == comp->comp_info.category) ||
		    (NCS_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE == comp->comp_info.category)) {
			isPre = FALSE;
			i_comp = comp->su->list_of_comp;
			while (i_comp) {
				if (((NCS_COMP_TYPE_SA_AWARE == i_comp->comp_info.category) ||
				     (NCS_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE == i_comp->comp_info.category) ||
				     (NCS_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE == i_comp->comp_info.category)) &&
				    (i_comp != comp)) {
					isPre = TRUE;
					break;
				}
				i_comp = i_comp->su_comp_next;
			}	/* end while */

			if (isPre == TRUE) {
				comp->su->saAmfSUPreInstantiable = TRUE;
			} else {
				comp->su->saAmfSUPreInstantiable = FALSE;
			}
		}

		if (comp->su->curr_num_comp == 1) {
			/* This comp will be deleted so revert these to def val */
			comp->su->si_max_active = 0;
			comp->su->si_max_standby = 0;
			comp->su->saAmfSUPreInstantiable = TRUE;
		}

		/* Set runtime cached attributes. */
		avd_saImmOiRtObjectUpdate(&comp->su->name,
				"saAmfSUPreInstantiable", SA_IMM_ATTR_SAUINT32T,
				&comp->su->saAmfSUPreInstantiable);

		/* decrement the active component number of this SU */
		comp->su->curr_num_comp--;

		m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (comp->su), AVSV_CKPT_AVD_SU_CONFIG);

		avd_su_del_comp(comp);

		m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, comp, AVSV_CKPT_AVD_COMP_CONFIG);
		return;
	}

	/* Log an information error that the node was updated with the
	 * information of the component.
	 */
	/* Find the component */
	comp = avd_comp_find(&ack_msg->msg_info.n2d_reg_comp.comp_name);
	if (comp == NULL) {
		/* The comp has already been deleted. there is nothing
		 * that can be done.
		 */

		/* Log an information error that the comp is
		 * deleted.
		 */
		/* send a delete message to the AvND for the comp. */
		memset(((uns8 *)&param), '\0', sizeof(AVSV_PARAM_INFO));
		param.act = AVSV_OBJ_OPR_DEL;
		param.name = ack_msg->msg_info.n2d_reg_comp.comp_name;
		param.class_id = AVSV_SA_AMF_COMP;
		avnd = avd_node_find_nodeid(ack_msg->msg_info.n2d_reg_comp.node_id);
		avd_snd_op_req_msg(cb, avnd, &param);
		return;
	}

	if (comp->su->curr_num_comp == comp->su->num_of_comp) {
		if (comp->su->saAmfSUPreInstantiable == TRUE) {
			avd_sg_app_su_inst_func(cb, comp->su->sg_of_su);
		} else {
			comp->saAmfCompOperState = SA_AMF_OPERATIONAL_ENABLED;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVSV_CKPT_COMP_OPER_STATE);
			avd_su_pres_state_set(comp->su, SA_AMF_OPERATIONAL_ENABLED);

			m_AVD_GET_SU_NODE_PTR(cb, comp->su, su_node_ptr);

			if (m_AVD_APP_SU_IS_INSVC(comp->su, su_node_ptr)) {
				avd_su_readiness_state_set(comp->su, SA_AMF_READINESS_IN_SERVICE);
				switch (comp->su->sg_of_su->sg_redundancy_model) {
				case SA_AMF_2N_REDUNDANCY_MODEL:
					avd_sg_2n_su_insvc_func(cb, comp->su);
					break;

				case SA_AMF_N_WAY_ACTIVE_REDUNDANCY_MODEL:
					avd_sg_nacvred_su_insvc_func(cb, comp->su);
					break;

				case SA_AMF_N_WAY_REDUNDANCY_MODEL:
					avd_sg_nway_su_insvc_func(cb, comp->su);
					break;

				case SA_AMF_NPM_REDUNDANCY_MODEL:
					avd_sg_npm_su_insvc_func(cb, comp->su);
					break;

				case SA_AMF_NO_REDUNDANCY_MODEL:
				default:
					avd_sg_nored_su_insvc_func(cb, comp->su);
					break;
				}
			}
		}
	}
	return;
}

/*****************************************************************************
 * Function: avd_comp_add_to_model
 * 
 * Purpose: This routine adds SaAmfComp objects to Patricia tree.
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
static void avd_comp_add_to_model(AVD_COMP *avd_comp)
{
	AVD_COMP *i_comp = NULL;
	AVD_AVND *su_node_ptr = NULL;
	NCS_BOOL isPre;

	/* add to the list of SU  */
	avd_su_add_comp(avd_comp);

	/* add to CompType list */
	avd_comptype_add_comp_list(avd_comp);

	/* check if the
	 * corresponding node is UP send the component information
	 * to the Node.
	 */
	if (FALSE == avd_comp->su->su_is_external) {
		su_node_ptr = avd_comp->su->su_on_node;
	} else {
		/* This is an external SU, so there is no node assigned to it.
		   For some purpose of validations and sending SU/Comps info to
		   hosting node (Controllers), we can take use of the hosting
		   node. */
		if ((NCS_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE ==
		     avd_comp->comp_info.category) ||
		    (NCS_COMP_TYPE_EXTERNAL_NON_PRE_INSTANTIABLE == avd_comp->comp_info.category)) {
			/* This is a valid external component. Ext comp is in ext
			   SU. */
		} else {
			/* This is not a valid external component. External SU has 
			   been assigned a cluster component. */
			avd_comp->su->curr_num_comp--;
			avd_su_del_comp(avd_comp);
			avd_log(NCSFL_SEV_ERROR, "Not A Valid External Component: Category '%u'configured",
				avd_comp->comp_info.category);
			return;
		}
		su_node_ptr = avd_cb->ext_comp_info.local_avnd_node;
	}			/* Else of if(FALSE == avd_comp->su->su_is_external). */
#if 0
	if ((su_node_ptr->node_state == AVD_AVND_STATE_PRESENT) ||
	    (su_node_ptr->node_state == AVD_AVND_STATE_NO_CONFIG) ||
	    (su_node_ptr->node_state == AVD_AVND_STATE_NCS_INIT)) {
		if (avd_snd_comp_msg(avd_cb, avd_comp) != NCSCC_RC_SUCCESS) {
			/* the SU will never get to readiness state in service */
			/* Log an internal error */
			avd_comp->su->curr_num_comp--;
			avd_su_del_comp(avd_comp);
			avd_log(NCSFL_SEV_ERROR, "Sending Comp Info to AvND failed: '%s'",
				avd_comp->comp_info.name.value);
			return;
		}
	}
#endif
	/* Verify if the SUs preinstan value need to be changed */
	if ((avd_comp->comp_info.category == NCS_COMP_TYPE_SA_AWARE) ||
	    (avd_comp->comp_info.category == NCS_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE) ||
	    (avd_comp->comp_info.category == NCS_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE)) {
		avd_comp->su->saAmfSUPreInstantiable = TRUE;
	} else {
		isPre = FALSE;
		i_comp = avd_comp->su->list_of_comp;
		while (i_comp) {
			if ((i_comp->comp_info.category == NCS_COMP_TYPE_SA_AWARE) ||
			    (i_comp->comp_info.category == NCS_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE) ||
			    (i_comp->comp_info.category == NCS_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE)) {
				isPre = TRUE;
				break;
			}
			i_comp = i_comp->su_comp_next;
		}
		if (isPre == FALSE) {
			avd_comp->su->saAmfSUPreInstantiable = FALSE;
		}
		avd_comp->max_num_csi_actv = 1;
		avd_comp->max_num_csi_stdby = 1;
	}

	if ((avd_comp->max_num_csi_actv < avd_comp->su->si_max_active) || (avd_comp->su->si_max_active == 0)) {
		avd_comp->su->si_max_active = avd_comp->max_num_csi_actv;
	}

	if ((avd_comp->max_num_csi_stdby < avd_comp->su->si_max_standby) || (avd_comp->su->si_max_standby == 0)) {
		avd_comp->su->si_max_standby = avd_comp->max_num_csi_stdby;
	}

        /* Set runtime cached attributes. */
        if (avd_cb->impl_set == TRUE) {
		avd_saImmOiRtObjectUpdate(&avd_comp->su->name,
				"saAmfSUPreInstantiable", SA_IMM_ATTR_SAUINT32T,
				&avd_comp->su->saAmfSUPreInstantiable);

		avd_saImmOiRtObjectUpdate(&avd_comp->comp_info.name,
				"saAmfCompReadinessState", SA_IMM_ATTR_SAUINT32T, &avd_comp->saAmfCompReadinessState);

		avd_saImmOiRtObjectUpdate(&avd_comp->comp_info.name,
				"saAmfCompOperState", SA_IMM_ATTR_SAUINT32T, &avd_comp->saAmfCompOperState);

		avd_saImmOiRtObjectUpdate(&avd_comp->comp_info.name,
				"saAmfCompPresenceState", SA_IMM_ATTR_SAUINT32T, &avd_comp->saAmfCompPresenceState);
	}

}

/**
 * Validate configuration attributes for an AMF Comp object
 * @param comp
 * 
 * @return int
 */
static int avd_comp_config_validate(AVD_COMP *avd_comp, const CcbUtilOperationData_t *opdata)
{
	AVD_SU *su = NULL;
	CcbUtilOperationData_t *ccb_object = NULL;
	saAmfRedundancyModelT sg_red_model;
	char *parent;
	char *dn = (char *)avd_comp->comp_info.name.value;
	SaNameT su_name;

	if ((parent = strchr(dn, ',')) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "No parent to '%s' ", dn);
		return -1;
	}

	parent++;

	/* Should be children to SU */
	if (strncmp(parent, "safSu=", 6) != 0) {
		avd_log(NCSFL_SEV_ERROR, "Wrong parent '%s' to '%s' ", parent, dn);
		return -1;
	}

	if ((avd_comp->comp_info.category == NCS_COMP_TYPE_SA_AWARE) && (avd_comp->comp_info.init_len == 0)) {
		avd_log(NCSFL_SEV_ERROR, "Sa Aware Component: instantiation command not configured");
		return -1;
	} else if ((avd_comp->comp_info.category == NCS_COMP_TYPE_NON_SAF) &&
		   ((avd_comp->comp_info.init_len == 0) || (avd_comp->comp_info.term_len == 0))) {
		avd_log(NCSFL_SEV_ERROR, "Non-SaAware Component: instantiation or termination command not configured");
		return -1;
	}

	if ((NCS_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE == avd_comp->comp_info.category) ||
	    (NCS_COMP_TYPE_EXTERNAL_NON_PRE_INSTANTIABLE == avd_comp->comp_info.category)) {
		if ((avd_comp->comp_info.init_len == 0) ||
		    (avd_comp->comp_info.term_len == 0) || (avd_comp->comp_info.clean_len == 0)) {
			/* For external component, the following fields should not be 
			   filled. */
		} else {
			avd_log(NCSFL_SEV_ERROR, "External Component: instantiation or termination not configured");
			return -1;
		}
	} else {
		if (avd_comp->comp_info.clean_len == 0) {
			avd_log(NCSFL_SEV_ERROR, "Cluster Component: Cleanup script not configured");
			return -1;
		}

		if (avd_comp->comp_info.max_num_inst == 0) {
			avd_log(NCSFL_SEV_ERROR, "Cluster Component: Max num inst not configured");
			return -1;
		}
	}

	if ((avd_comp->comp_info.category == NCS_COMP_TYPE_SA_AWARE) ||
	    (avd_comp->comp_info.category == NCS_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE) ||
	    (avd_comp->comp_info.category == NCS_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE)) {

		if (avd_comp->comp_info.cap == NCS_COMP_CAPABILITY_1_ACTIVE_OR_Y_STANDBY) {
			avd_comp->max_num_csi_actv = 1;
		} else if ((avd_comp->comp_info.cap == NCS_COMP_CAPABILITY_1_ACTIVE_OR_1_STANDBY) ||
			   (avd_comp->comp_info.cap == NCS_COMP_CAPABILITY_1_ACTIVE)) {
			avd_comp->max_num_csi_actv = 1;
			avd_comp->max_num_csi_stdby = 1;
		} else if (avd_comp->comp_info.cap == NCS_COMP_CAPABILITY_X_ACTIVE) {
			avd_comp->max_num_csi_stdby = avd_comp->max_num_csi_actv;
		}

		if ((avd_comp->max_num_csi_actv == 0) || (avd_comp->max_num_csi_stdby == 0)) {
			avd_log(NCSFL_SEV_ERROR, "Max Act Csi or Max Stdby Csi not configured");
			return -1;
		}
	}

	avsv_sanamet_init(&avd_comp->comp_info.name, &su_name, "safSu=");
	if (NULL == (su = avd_su_find(&su_name))) {
		if (opdata != NULL) {
			if ((ccb_object = ccbutil_getCcbOpDataByDN(opdata->ccbId, &su_name)) == NULL) {
				avd_log(NCSFL_SEV_ERROR, "SU '%s' does not exist in existing model or in CCB",
					su_name.value);
				return -1;

			} else {
				su = (AVD_SU *)(ccb_object->userData);
			}
		} else
			assert(0);
	}

	if (su == NULL) {
		avd_log(NCSFL_SEV_ERROR, "Su '%s'  Not Configured", su_name.value);
		return -1;
	}

	sg_red_model = su->sg_of_su->sg_redundancy_model;

	/* Check illegal component capability/category wrt SG red model */
	if (((sg_red_model == SA_AMF_N_WAY_REDUNDANCY_MODEL) &&
	     ((avd_comp->comp_info.cap != NCS_COMP_CAPABILITY_X_ACTIVE_AND_Y_STANDBY) ||
	      (avd_comp->comp_info.category == NCS_COMP_TYPE_NON_SAF)))) {
		avd_log(NCSFL_SEV_ERROR, "Illegal category %u or cap %u for SG red model %u",
			avd_comp->comp_info.category, avd_comp->comp_info.cap, sg_red_model);
		return -1;
	}

	/* Check illegal component capability wrt SG red model */
	if ((sg_red_model == SA_AMF_NPM_REDUNDANCY_MODEL) &&
	    (avd_comp->comp_info.cap != NCS_COMP_CAPABILITY_1_ACTIVE_OR_1_STANDBY)) {
		avd_log(NCSFL_SEV_ERROR, "Illegal capability %u for SG red model %u",
			avd_comp->comp_info.cap, sg_red_model);
		return -1;
	}

	/* Verify that the SU can contain this component */
	{
		AVD_SUTCOMP_TYPE *sutcomptype;
		SaNameT sutcomptype_name;

		avd_create_association_class_dn(&avd_comp->saAmfCompType, &su->saAmfSUType,
			"safMemberCompType", &sutcomptype_name);
		sutcomptype = avd_sutcomptype_find(&sutcomptype_name);
		if (sutcomptype == NULL) {
			avd_log(NCSFL_SEV_ERROR, "Not found '%s'", sutcomptype_name.value);
			return -1;
		}

		if (sutcomptype->curr_num_components == sutcomptype->saAmfSutMaxNumComponents) {
			avd_log(NCSFL_SEV_ERROR, "SU '%s' cannot contain more components of this type '%s*",
				su->name.value, avd_comp->saAmfCompType.value);
			return -1;
		}
	}

	if (TRUE == su->su_is_external) {
		if ((TRUE == avd_comp->comp_info.am_enable) ||
		    (0 != avd_comp->comp_info.amstart_len) || (0 != avd_comp->comp_info.amstop_len)) {
			avd_log(NCSFL_SEV_ERROR, "External Component: Active monitoring configured");
			return -1;
		} else {
			/* There are default values assigned to amstart_time, 
			   amstop_time and clean_time. Since these values are not 
			   used for external components, so we will reset it. */
			avd_comp->comp_info.amstart_time = 0;
			avd_comp->comp_info.amstop_time = 0;
			avd_comp->comp_info.clean_time = 0;
			avd_comp->comp_info.max_num_amstart = 0;
		}
	}

	return 0;
}

static NCS_COMP_TYPE_VAL avd_comp_category_to_ncs(SaUint32T saf_comp_category)
{
	NCS_COMP_TYPE_VAL ncs_comp_category = 0;

	/* Check for mandatory attr for SA_AWARE. */
	if (saf_comp_category & SA_AMF_COMP_SA_AWARE) {
		/* It shouldn't match with any of others, we don't care about SA_AMF_COMP_LOCAL as it is optional */
		if ((saf_comp_category & ~SA_AMF_COMP_LOCAL) == SA_AMF_COMP_SA_AWARE) {
			ncs_comp_category = NCS_COMP_TYPE_SA_AWARE;
		}
	} else {
		/* Not SA_AMF_COMP_PROXY, SA_AMF_COMP_CONTAINER, SA_AMF_COMP_CONTAINED and SA_AMF_COMP_SA_AWARE */
		if (saf_comp_category & SA_AMF_COMP_LOCAL) {
			/* It shouldn't match with any of others */
			if ((saf_comp_category & SA_AMF_COMP_PROXIED) ||
			    (!(saf_comp_category & SA_AMF_COMP_PROXIED_NPI))) {
				ncs_comp_category = NCS_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE;
			} else if ((saf_comp_category & SA_AMF_COMP_PROXIED_NPI) ||
				   (!(saf_comp_category & SA_AMF_COMP_PROXIED))) {
				ncs_comp_category = NCS_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE;
			} else if (!((saf_comp_category & SA_AMF_COMP_PROXIED)) ||
				   (!(saf_comp_category & SA_AMF_COMP_PROXIED_NPI))) {
				ncs_comp_category = NCS_COMP_TYPE_NON_SAF;
			}
		} else {
			/* Not SA_AMF_COMP_PROXY, SA_AMF_COMP_CONTAINER, SA_AMF_COMP_CONTAINED, SA_AMF_COMP_SA_AWARE and 
			   SA_AMF_COMP_LOCAL */
			if (saf_comp_category & SA_AMF_COMP_PROXIED_NPI) {
				ncs_comp_category = NCS_COMP_TYPE_EXTERNAL_NON_PRE_INSTANTIABLE;
			} else {
				/* Not SA_AMF_COMP_PROXY, SA_AMF_COMP_CONTAINER, SA_AMF_COMP_CONTAINED, SA_AMF_COMP_SA_AWARE, 
				   SA_AMF_COMP_LOCAL and SA_AMF_COMP_PROXIED_NPI. So only thing left is SA_AMF_COMP_PROXIED */
				/* Whether SA_AMF_COMP_PROXIED is set or not, we don't care, as it is optional */
				ncs_comp_category = NCS_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE;
			}

		}		/* else of if((comp_type->ct_comp_category & SA_AMF_COMP_PROXIED)  */
	}			/* else of if(comp_type->ct_comp_category & SA_AMF_COMP_SA_AWARE)  */

	return ncs_comp_category;
}

AVD_COMP *avd_comp_create(const SaNameT *comp_name, const SaImmAttrValuesT_2 **attributes)
{
	int rc = -1;
	AVD_COMP *comp;
	char *cmd_argv;
	const char *str;
	const AVD_COMP_TYPE *comptype;
	SaNameT su_name;

	/*
	** If called from cold sync, attributes==NULL.
	** If called at new active at failover, the object is found in the DB
	** but needs to get configuration attributes initialized.
	*/
	if (NULL == (comp = avd_comp_find(comp_name))) {
		if ((comp = calloc(1, sizeof(AVD_COMP))) == NULL) {
			avd_log(NCSFL_SEV_ERROR, "calloc FAILED");
			goto done;
		}

		memcpy(comp->comp_info.name.value, comp_name->value, comp_name->length);
		comp->comp_info.name.length = comp_name->length;
		comp->tree_node.key_info = (uns8 *)&(comp->comp_info.name);
		comp->comp_info.cap = SA_AMF_COMP_ONE_ACTIVE_OR_ONE_STANDBY;
		comp->comp_info.category = NCS_COMP_TYPE_NON_SAF;
		comp->comp_info.def_recvr = SA_AMF_COMPONENT_RESTART;
		comp->comp_info.inst_level = 1;
		comp->comp_info.comp_restart = TRUE;
		comp->nodefail_cleanfail = FALSE;
		comp->saAmfCompOperState = SA_AMF_OPERATIONAL_DISABLED;
		comp->saAmfCompReadinessState = SA_AMF_READINESS_OUT_OF_SERVICE;
		comp->saAmfCompPresenceState = SA_AMF_PRESENCE_UNINSTANTIATED;
	}

	/* If no attributes supplied, go direct and add to DB */
	if (NULL == attributes)
		goto add_to_db;

	avsv_sanamet_init(comp_name, &su_name, "safSu=");
	comp->su = avd_su_find(&su_name);

	if (immutil_getAttr("saAmfCompType", attributes, 0, &comp->saAmfCompType) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfCompType FAILED for '%s'", comp_name->value);
		goto done;
	}

	if ((comp->comp_type = avd_comptype_find(&comp->saAmfCompType)) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "Get '%s' FAILED for '%s'", comp->saAmfCompType.value, comp_name->value);
		goto done;
	}

	comptype = comp->comp_type;

	/*  TODO clean this up! */
	comp->comp_info.category = avd_comp_category_to_ncs(comptype->saAmfCtCompCategory);

	if (strlen(comptype->saAmfCtRelPathInstantiateCmd) > 0) {
		strcpy(comp->comp_info.init_info, comptype->saAmfCtRelPathInstantiateCmd);
		cmd_argv = comp->comp_info.init_info + strlen(comp->comp_info.init_info);
		*cmd_argv++ = 0x20;	/* Insert SPACE between cmd and args */

		if ((str = immutil_getStringAttr(attributes, "saAmfCompInstantiateCmdArgv", 0)) == NULL)
			str = comptype->saAmfCtDefInstantiateCmdArgv;

		if (str != NULL)
			strcpy(cmd_argv, str);

		comp->comp_info.init_len = strlen(comp->comp_info.init_info);
	}

	if (immutil_getAttr("saAmfCompInstantiateTimeout", attributes, 0, &comp->comp_info.init_time) != SA_AIS_OK)
		comp->comp_info.init_time = comptype->saAmfCtDefClcCliTimeout;

	if (immutil_getAttr("saAmfCompInstantiateLevel", attributes, 0, &comp->comp_info.inst_level) != SA_AIS_OK)
		comp->comp_info.inst_level = comptype->saAmfCtDefInstantiationLevel;

	if (immutil_getAttr("saAmfCompNumMaxInstantiateWithoutDelay", attributes,
			    0, &comp->comp_info.max_num_inst) != SA_AIS_OK)
		comp->comp_info.max_num_inst = avd_comp_global_attrs.saAmfNumMaxInstantiateWithoutDelay;

	/*  TODO what is implemented? With or without delay? */

#if 0
	if (immutil_getAttr("saAmfCompNumMaxInstantiateWithDelay", attributes,
			    0, &comp->max_num_inst_delay) != SA_AIS_OK)
		comp->comp_info.max_num_inst = avd_comp_global_attrs.saAmfNumMaxInstantiateWithDelay;
#endif

	if (immutil_getAttr("saAmfCompDelayBetweenInstantiateAttempts", attributes,
			    0, &comp->inst_retry_delay) != SA_AIS_OK)
		comp->inst_retry_delay = avd_comp_global_attrs.saAmfDelayBetweenInstantiateAttempts;

	if (strlen(comptype->saAmfCtRelPathTerminateCmd) > 0) {
		strcpy(comp->comp_info.term_info, comptype->saAmfCtRelPathTerminateCmd);
		cmd_argv = comp->comp_info.term_info + strlen(comp->comp_info.term_info);
		*cmd_argv++ = 0x20;	/* Insert SPACE between cmd and args */

		if ((str = immutil_getStringAttr(attributes, "saAmfCompTerminateCmdArgv", 0)) == NULL)
			str = comptype->saAmfCtDefTerminateCmdArgv;

		if (str != NULL)
			strcpy(cmd_argv, str);

		comp->comp_info.term_len = strlen(comp->comp_info.term_info);
	}

	if (immutil_getAttr("saAmfCompTerminateTimeout", attributes,
			    0, &comp->comp_info.terminate_callback_timeout) != SA_AIS_OK)
		comp->comp_info.terminate_callback_timeout = comptype->saAmfCtDefCallbackTimeout;

	if (strlen(comptype->saAmfCtRelPathCleanupCmd) > 0) {
		strcpy(comp->comp_info.clean_info, comptype->saAmfCtRelPathCleanupCmd);
		cmd_argv = comp->comp_info.clean_info + strlen(comp->comp_info.clean_info);
		*cmd_argv++ = 0x20;	/* Insert SPACE between cmd and args */

		if ((str = immutil_getStringAttr(attributes, "saAmfCompCleanupCmdArgv", 0)) == NULL)
			str = comptype->saAmfCtDefCleanupCmdArgv;

		if (str != NULL)
			strcpy(cmd_argv, str);

		comp->comp_info.clean_len = strlen(comp->comp_info.clean_info);
	}

	if (immutil_getAttr("saAmfCompCleanupTimeout", attributes, 0, &comp->comp_info.clean_time) != SA_AIS_OK)
		comp->comp_info.clean_time = comptype->saAmfCtDefCallbackTimeout;

	if (strlen(comptype->saAmfCtRelPathAmStartCmd) > 0) {
		strcpy(comp->comp_info.amstart_info, comptype->saAmfCtRelPathAmStartCmd);
		cmd_argv = comp->comp_info.amstart_info + strlen(comp->comp_info.amstart_info);
		*cmd_argv++ = 0x20;	/* Insert SPACE between cmd and args */

		if ((str = immutil_getStringAttr(attributes, "saAmfCompAmStartCmdArgv", 0)) == NULL)
			str = comptype->saAmfCtDefAmStartCmdArgv;

		if (str != NULL)
			strcpy(cmd_argv, str);

		comp->comp_info.amstart_len = strlen(comp->comp_info.amstart_info);
	}

	if (immutil_getAttr("saAmfCompAmStartTimeout", attributes, 0, &comp->comp_info.amstart_time) != SA_AIS_OK)
		comp->comp_info.amstart_time = comptype->saAmfCtDefClcCliTimeout;

	if (immutil_getAttr("saAmfCompNumMaxAmStartAttempts", attributes,
			    0, &comp->comp_info.max_num_amstart) != SA_AIS_OK)
		comp->comp_info.max_num_amstart = avd_comp_global_attrs.saAmfNumMaxAmStartAttempts;

	if (strlen(comptype->saAmfCtRelPathAmStopCmd) > 0) {
		strcpy(comp->comp_info.amstop_info, comptype->saAmfCtRelPathAmStopCmd);
		cmd_argv = comp->comp_info.amstop_info + strlen(comp->comp_info.amstop_info);
		*cmd_argv++ = 0x20;	/* Insert SPACE between cmd and args */

		if ((str = immutil_getStringAttr(attributes, "saAmfCompAmStopCmdArgv", 0)) == NULL)
			str = comptype->saAmfCtDefAmStopCmdArgv;

		if (str != NULL)
			strcpy(cmd_argv, str);
	}

	if (immutil_getAttr("saAmfCompAmStopTimeout", attributes, 0, &comp->comp_info.amstop_time) != SA_AIS_OK)
		comp->comp_info.amstop_time = comptype->saAmfCtDefClcCliTimeout;

	if (immutil_getAttr("saAmfCompNumMaxAmStopAttempts", attributes,
			    0, &comp->comp_info.max_num_amstop) != SA_AIS_OK)
		comp->comp_info.max_num_amstop = avd_comp_global_attrs.saAmfNumMaxAmStopAttempts;

	if (immutil_getAttr("saAmfCompCSISetCallbackTimeout", attributes,
			    0, &comp->comp_info.csi_set_callback_timeout) != SA_AIS_OK)
		comp->comp_info.csi_set_callback_timeout = comptype->saAmfCtDefCallbackTimeout;

	if (immutil_getAttr("saAmfCompCSIRmvCallbackTimeout", attributes,
			    0, &comp->comp_info.csi_rmv_callback_timeout) != SA_AIS_OK)
		comp->comp_info.csi_rmv_callback_timeout = comptype->saAmfCtDefCallbackTimeout;

	if (immutil_getAttr("saAmfCompQuiescingCompleteTimeout", attributes,
			    0, &comp->comp_info.quiescing_complete_timeout) != SA_AIS_OK)
		comp->comp_info.quiescing_complete_timeout = comptype->saAmfCompQuiescingCompleteTimeout;

	if (immutil_getAttr("saAmfCompRecoveryOnError", attributes, 0, &comp->comp_info.def_recvr) != SA_AIS_OK)
		comp->comp_info.def_recvr = comptype->saAmfCtDefRecoveryOnError;

	if (immutil_getAttr("saAmfCompDisableRestart", attributes, 0, &comp->comp_info.comp_restart) != SA_AIS_OK)
		comp->comp_info.comp_restart = comptype->saAmfCtDefDisableRestart;

	comp->max_num_csi_actv = -1;	// TODO
	comp->max_num_csi_stdby = -1;	// TODO

add_to_db:
	(void)ncs_patricia_tree_add(&avd_comp_db, &comp->tree_node);
	rc = 0;

done:
	if (rc != 0) {
		(void)avd_comp_delete(comp);
		comp = NULL;
	}

	return comp;
}

/**
 * Get configuration for all SaAmfComp objects from IMM and
 * create AVD internal objects.
 * @param cb
 * 
 * @return int
 */
SaAisErrorT avd_comp_config_get(const SaNameT *su_name, AVD_SU *su)
{
	SaAisErrorT rc, error = SA_AIS_ERR_FAILED_OPERATION;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT comp_name;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfComp";
	AVD_COMP *comp;

	assert(su != NULL);

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	if ((rc = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, su_name,
		SA_IMM_SUBTREE, SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR,
		&searchParam, NULL, &searchHandle)) != SA_AIS_OK) {

		avd_log(NCSFL_SEV_ERROR, "saImmOmSearchInitialize_2 failed: %u", rc);
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &comp_name, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {

		avd_log(NCSFL_SEV_NOTICE, "'%s'", comp_name.value);

		if ((comp = avd_comp_create(&comp_name, attributes)) == NULL)
			goto done2;

		if (avd_comp_config_validate(comp, NULL) != 0) {
			avd_comp_delete(comp);
			goto done2;
		}

		avd_comp_add_to_model(comp);

		if (avd_compcstype_config_get(&comp_name, comp) != SA_AIS_OK)
			goto done2;
	}

	error = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:
	return error;
}

static void avd_comp_admin_op_cb(SaImmOiHandleT immOiHandle,
				 SaInvocationT invocation,
				 const SaNameT *objectName,
				 SaImmAdminOperationIdT opId, const SaImmAdminOperationParamsT_2 **params)
{
	SaAisErrorT rc = SA_AIS_OK;
	AVD_COMP *comp = avd_comp_find(objectName);

	assert(comp != NULL);

	switch (opId) {
		/* Valid B.04 AMF comp admin operations */
	case SA_AMF_ADMIN_RESTART:
	case SA_AMF_ADMIN_EAM_START:
	case SA_AMF_ADMIN_EAM_STOP:
	default:
		rc = SA_AIS_ERR_NOT_SUPPORTED;
		break;
	}

	(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, rc);
}

static SaAisErrorT avd_comp_rt_attr_callback(SaImmOiHandleT immOiHandle,
					     const SaNameT *objectName, const SaImmAttrNameT *attributeNames)
{
	AVD_COMP *comp = avd_comp_find(objectName);
	SaImmAttrNameT attributeName;
	int i = 0;

	avd_trace("%s", objectName->value);
	assert(comp != NULL);

	while ((attributeName = attributeNames[i++]) != NULL) {
		if (!strcmp("saAmfCompRestartCount", attributeName)) {
			(void)avd_saImmOiRtObjectUpdate(objectName, attributeName,
						       SA_IMM_ATTR_SAUINT32T, &comp->saAmfCompRestartCount);
		} else if (!strcmp("saAmfCompCurrProxyName", attributeName)) {
			/* TODO */
		} else if (!strcmp("saAmfCompCurrProxiedNames", attributeName)) {
			/* TODO */
		} else
			assert(0);
	}

	return SA_AIS_OK;
}

/*****************************************************************************
 * Function: avd_comptype_del_comp
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
static void avd_comptype_del_comp(AVD_COMP* comp)
{
	AVD_COMP *i_comp = NULL;
	AVD_COMP *prev_comp = NULL;

	if (comp->comp_type != NULL) {
		i_comp = comp->comp_type->list_of_comp;

		while ((i_comp != NULL) && (i_comp != comp)) {
			prev_comp = i_comp;
			i_comp = i_comp->comp_type_list_comp_next;
		}

		if (i_comp == comp) {
			if (prev_comp == NULL) {
				comp->comp_type->list_of_comp = comp->comp_type_list_comp_next;
			} else {
				prev_comp->comp_type_list_comp_next = comp->comp_type_list_comp_next;
			}

			comp->comp_type_list_comp_next = NULL;
			comp->comp_type = NULL;
		}
	}

	return;
}

/*****************************************************************************
 * Function: avd_comp_ccb_completed_modify_hdlr
 * 
 * Purpose: This routine validates modify CCB operations on SaAmfComp objects.
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
static SaAisErrorT avd_comp_ccb_completed_modify_hdlr(CcbUtilOperationData_t *opdata)
{
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;
	AVD_COMP *comp;
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	comp = avd_comp_find(&opdata->objectName);

	while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
		const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;
		void *value = attribute->attrValues[0];

		if (!strcmp(attribute->attrName, "saAmfCompType")) {
			SaNameT dn = *((SaNameT*)value);
			if (NULL == avd_comptype_find(&dn)) {
				avd_log(NCSFL_SEV_ERROR, "Comp Type '%s' not found", dn.value);
				goto done;
			}

			if ((comp->su->saAmfSUAdminState != SA_AMF_ADMIN_LOCKED_INSTANTIATION) &&
			    (comp->su->sg_of_su->saAmfSGAdminState != SA_AMF_ADMIN_LOCKED_INSTANTIATION) &&
			    (comp->su->sg_of_su->sg_on_app->saAmfApplicationAdminState != SA_AMF_ADMIN_LOCKED_INSTANTIATION)){
				avd_log(NCSFL_SEV_ERROR, "A parent is not locked instantiation, '%s'", dn.value);
				goto done;
			}

			if (comp->saAmfCompPresenceState != SA_AMF_PRESENCE_UNINSTANTIATED) {
				avd_log(NCSFL_SEV_ERROR, "Comp '%s' has wrong presence state %u",
					dn.value, comp->saAmfCompPresenceState);
				goto done;
			}
		}
	}

	rc = SA_AIS_OK;
done:
	return rc;
}

/*****************************************************************************
 * Function: avd_comp_ccb_completed_delete_hdlr
 * 
 * Purpose: This routine validates delete CCB operations on SaAmfComp objects.
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
static SaAisErrorT avd_comp_ccb_completed_delete_hdlr(CcbUtilOperationData_t *opdata)
{
	AVD_COMP *comp = NULL;
	SaAisErrorT rc = SA_AIS_OK;

	avd_log(NCSFL_SEV_NOTICE, "'%s'", opdata->objectName.value);

	/* Find the comp name. */
	comp = avd_comp_find(&opdata->objectName);
	assert(comp != NULL);

	/* Check to see that the SU of which the component is a
	 * part is in admin locked state, in term state with
	 * no assignments before
	 * making the row status as not in service or delete 
	 */
	if ((comp->su->sg_of_su->sg_ncs_spec == TRUE) ||
	    (comp->su->saAmfSUAdminState != SA_AMF_ADMIN_LOCKED) ||
	    (comp->su->saAmfSUPresenceState != SA_AMF_PRESENCE_UNINSTANTIATED) ||
	    (comp->su->list_of_susi != AVD_SU_SI_REL_NULL) ||
	    ((comp->su->saAmfSUPreInstantiable == TRUE) && (comp->su->term_state != TRUE))) {
		/* log information error */
		return SA_AIS_ERR_BAD_OPERATION;
	}

	return rc;

}

/*****************************************************************************
 * Function: avd_comp_ccb_completed_cb
 * 
 * Purpose: This routine handles all CCB operations on SaAmfComp objects.
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
static SaAisErrorT avd_comp_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_COMP *comp;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if ((comp = avd_comp_create(&opdata->objectName, opdata->param.create.attrValues)) == NULL) {
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}

		if (avd_comp_config_validate(comp, opdata) != 0) {
			rc = SA_AIS_ERR_INVALID_PARAM;
			goto done;
		}
		opdata->userData = comp;	/* Save for later use in apply */
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		rc = avd_comp_ccb_completed_modify_hdlr(opdata);
		break;
	case CCBUTIL_DELETE:
		rc = avd_comp_ccb_completed_delete_hdlr(opdata);
		break;
	default:
		assert(0);
		break;
	}
 done:

	return rc;
}

/*****************************************************************************
 * Function: avd_comp_ccb_apply_create_hdlr
 * 
 * Purpose: This routine handles create operations on SaAmfComp objects.
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
static void avd_comp_ccb_apply_create_hdlr(CcbUtilOperationData_t *opdata)
{
	avd_comp_add_to_model(opdata->userData);
}

/*****************************************************************************
 * Function: avd_comp_ccb_apply_modify_hdlr
 * 
 * Purpose: This routine handles modify operations on SaAmfComp objects.
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
static void avd_comp_ccb_apply_modify_hdlr(struct CcbUtilOperationData *opdata)
{
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;
	AVD_COMP *avd_comp = NULL;
	NCS_BOOL node_present = FALSE;
	AVD_AVND *su_node_ptr = NULL;
	AVD_COMP_TYPE *avd_comp_type;
	unsigned int rc = NCSCC_RC_SUCCESS;
	AVSV_PARAM_INFO param;

	avd_log(NCSFL_SEV_NOTICE, "'%s'", opdata->objectName.value);

	memset(((uns8 *)&param), '\0', sizeof(AVSV_PARAM_INFO));
	param.class_id = AVSV_SA_AMF_COMP;
	param.act = AVSV_OBJ_OPR_MOD;

	avd_comp = avd_comp_find(&opdata->objectName);
	param.name = avd_comp->comp_info.name;
	avd_comp_type = avd_comptype_find(&avd_comp->saAmfCompType);

	m_AVD_GET_SU_NODE_PTR(avd_cb, avd_comp->su, su_node_ptr);

	if ((su_node_ptr->node_state == AVD_AVND_STATE_PRESENT) ||
	    (su_node_ptr->node_state == AVD_AVND_STATE_NO_CONFIG) ||
	    (su_node_ptr->node_state == AVD_AVND_STATE_NCS_INIT)) {
		node_present = TRUE;
	}

	while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
		const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;
		char *cmd_argv;
		void *value = attribute->attrValues[0];

		if (!strcmp(attribute->attrName, "saAmfCompType")) {
			SaNameT *dn = (SaNameT*) value;
			param.attr_id = saAmfCompType_ID;
			param.value_len = dn->length;
			memcpy(param.value, dn->value, param.value_len);
		} else if (!strcmp(attribute->attrName, "saAmfCompInstantiateCmdArgv")) {

			char *param_val = *((char **)value);
			param.attr_id = saAmfCompInstantiateCmd_ID;
			strcpy((char *)&param.value[0], (char *)&avd_comp_type->saAmfCtRelPathInstantiateCmd);
			/* We need to append the arg with the command. */
			cmd_argv = (char *)(param.value + strlen((char *)param.value));
			*cmd_argv++ = 0x20;	/* Insert SPACE between cmd and args */
			strncpy(cmd_argv, param_val, strlen(param_val));
			param.value_len = strlen((char *)param.value);

			memset(&avd_comp->comp_info.init_cmd_arg_info, 0, AVSV_MISC_STR_MAX_SIZE);
			strcpy((char *)&avd_comp->comp_info.init_cmd_arg_info, param_val);
			avd_comp->comp_info.init_len = param.value_len;
			memset(&avd_comp->comp_info.init_info, 0, AVSV_MISC_STR_MAX_SIZE);
			memcpy(avd_comp->comp_info.init_info, &param.value[0], avd_comp->comp_info.init_len);

		} else if (!strcmp(attribute->attrName, "saAmfCompInstantiateTimeout")) {
			SaTimeT timeout;
			SaTimeT temp_timeout;
			timeout = *((SaTimeT *)value);
			m_NCS_OS_HTONLL_P(&temp_timeout, timeout);

			param.attr_id = saAmfCompInstantiateTimeout_ID;
			param.value_len = sizeof(SaTimeT);
			memcpy(&param.value[0], &temp_timeout, param.value_len);
			avd_comp->comp_info.init_time = *((SaTimeT *)value);

		} else if (!strcmp(attribute->attrName, "saAmfCompInstantiationLevel")) {
			avd_comp->comp_info.inst_level = *((SaUint32T *)value);
		} else if (!strcmp(attribute->attrName, "saAmfCompNumMaxInstantiateWithoutDelay")) {

			uns32 num_inst;
			num_inst = *((SaUint32T *)value);
			param.attr_id = saAmfCompNumMaxInstantiate_ID;
			param.value_len = sizeof(uns32);
			num_inst = htonl(num_inst);
			memcpy(&param.value[0], &num_inst, param.value_len);
			avd_comp->comp_info.max_num_inst = *((SaUint32T *)value);

		} else if (!strcmp(attribute->attrName, "saAmfCompNumMaxInstantiateWithDelay")) {

			uns32 num_inst;
			num_inst = *((SaUint32T *)value);
			param.attr_id = saAmfCompNumMaxInstantiateWithDelay_ID;
			param.value_len = sizeof(uns32);
			num_inst = htonl(num_inst);
			memcpy(&param.value[0], &num_inst, param.value_len);
			avd_comp->max_num_inst_delay = *((SaUint32T *)value);

		} else if (!strcmp(attribute->attrName, "saAmfCompDelayBetweenInstantiateAttempts")) {

			SaTimeT timeout;
			SaTimeT temp_timeout;
			timeout = *((SaTimeT *)value);
			m_NCS_OS_HTONLL_P(&temp_timeout, timeout);

			param.attr_id = saAmfCompDelayBetweenInstantiateAttempts_ID;
			param.value_len = sizeof(SaTimeT);
			memcpy(&param.value[0], &temp_timeout, param.value_len);
			avd_comp->inst_retry_delay = *((SaTimeT *)value);

		} else if (!strcmp(attribute->attrName, "saAmfCompTerminateCmdArgv")) {

			char *param_val = *((char **)value);
			param.attr_id = saAmfCompTerminateCmd_ID;
			strcpy((char *)&param.value[0], (char *)&avd_comp_type->saAmfCtRelPathTerminateCmd);
			/* We need to append the arg with the command. */
			cmd_argv = (char *)(param.value + strlen((char *)param.value));
			*cmd_argv++ = 0x20;	/* Insert SPACE between cmd and args */
			strncpy(cmd_argv, param_val, strlen(param_val));
			param.value_len = strlen((char *)param.value);

			memset(&avd_comp->comp_info.term_cmd_arg_info, 0, AVSV_MISC_STR_MAX_SIZE);
			strcpy((char *)&avd_comp->comp_info.term_cmd_arg_info, param_val);
			avd_comp->comp_info.term_len = param.value_len;
			memset(&avd_comp->comp_info.term_info, 0, AVSV_MISC_STR_MAX_SIZE);
			memcpy(avd_comp->comp_info.term_info, &param.value[0], avd_comp->comp_info.term_len);

		} else if (!strcmp(attribute->attrName, "saAmfCompTerminateTimeout")) {

			SaTimeT timeout;
			SaTimeT temp_timeout;
			timeout = *((SaTimeT *)value);
			m_NCS_OS_HTONLL_P(&temp_timeout, timeout);

			param.attr_id = saAmfCompTerminateTimeout_ID;
			param.value_len = sizeof(SaTimeT);
			memcpy(&param.value[0], &temp_timeout, param.value_len);
			avd_comp->comp_info.term_time = *((SaTimeT *)value);

		} else if (!strcmp(attribute->attrName, "saAmfCompCleanupCmdArgv")) {

			char *param_val = *((char **)value);
			param.attr_id = saAmfCompCleanupCmd_ID;
			strcpy((char *)&param.value[0], (char *)&avd_comp_type->saAmfCtRelPathCleanupCmd);
			/* We need to append the arg with the command. */
			cmd_argv = (char *)(param.value + strlen((char *)param.value));
			*cmd_argv++ = 0x20;	/* Insert SPACE between cmd and args */
			strncpy(cmd_argv, param_val, strlen(param_val));
			param.value_len = strlen((char *)param.value);

			memset(&avd_comp->comp_info.clean_cmd_arg_info, 0, AVSV_MISC_STR_MAX_SIZE);
			strcpy((char *)&avd_comp->comp_info.clean_cmd_arg_info, param_val);
			avd_comp->comp_info.clean_len = param.value_len;
			memset(&avd_comp->comp_info.clean_info, 0, AVSV_MISC_STR_MAX_SIZE);
			memcpy(avd_comp->comp_info.clean_info, &param.value[0], avd_comp->comp_info.clean_len);

		} else if (!strcmp(attribute->attrName, "saAmfCompCleanupTimeout")) {
			SaTimeT timeout;
			SaTimeT temp_timeout;
			timeout = *((SaTimeT *)value);
			m_NCS_OS_HTONLL_P(&temp_timeout, timeout);

			param.attr_id = saAmfCompCleanupTimeout_ID;
			param.value_len = sizeof(SaTimeT);
			memcpy(&param.value[0], &temp_timeout, param.value_len);
			avd_comp->comp_info.clean_time = *((SaTimeT *)value);

		} else if (!strcmp(attribute->attrName, "saAmfCompAmStartCmdArgv")) {

			char *param_val = *((char **)value);

			if (TRUE == avd_comp->su->su_is_external) {
				rc = SA_AIS_ERR_INVALID_PARAM;
				goto done;
			}

			param.attr_id = saAmfCompAmStartCmd_ID;
			strcpy((char *)&param.value[0], (char *)&avd_comp_type->saAmfCtRelPathAmStartCmd);
			/* We need to append the arg with the command. */
			cmd_argv = (char *)(param.value + strlen((char *)param.value));
			*cmd_argv++ = 0x20;	/* Insert SPACE between cmd and args */
			strncpy(cmd_argv, param_val, strlen(param_val));
			param.value_len = strlen((char *)param.value);

			memset(&avd_comp->comp_info.amstart_cmd_arg_info, 0, AVSV_MISC_STR_MAX_SIZE);
			strcpy((char *)&avd_comp->comp_info.amstart_cmd_arg_info, param_val);
			avd_comp->comp_info.amstart_len = param.value_len;
			memset(&avd_comp->comp_info.amstart_info, 0, AVSV_MISC_STR_MAX_SIZE);
			memcpy(avd_comp->comp_info.amstart_info, &param.value[0], avd_comp->comp_info.amstart_len);

		} else if (!strcmp(attribute->attrName, "saAmfCompAmStartTimeout")) {

			SaTimeT timeout;
			SaTimeT temp_timeout;
			if (TRUE == avd_comp->su->su_is_external) {
				rc = SA_AIS_ERR_INVALID_PARAM;
				goto done;
			}

			timeout = *((SaTimeT *)value);
			m_NCS_OS_HTONLL_P(&temp_timeout, timeout);

			param.attr_id = saAmfCompAmStartTimeout_ID;
			param.value_len = sizeof(SaTimeT);
			memcpy(&param.value[0], &temp_timeout, param.value_len);
			avd_comp->comp_info.amstart_time = *((SaTimeT *)value);

		} else if (!strcmp(attribute->attrName, "saAmfCompNumMaxAmStartAttempt")) {

			uns32 num_am_start;
			if (TRUE == avd_comp->su->su_is_external) {
				rc = SA_AIS_ERR_INVALID_PARAM;
				goto done;
			}

			num_am_start = *((SaUint32T *)value);
			param.attr_id = saAmfCompNumMaxAmStartAttempts_ID;
			param.value_len = sizeof(uns32);
			num_am_start = htonl(num_am_start);
			memcpy(&param.value[0], &num_am_start, param.value_len);
			avd_comp->comp_info.max_num_amstart = *((SaUint32T *)value);

		} else if (!strcmp(attribute->attrName, "saAmfCompAmStopCmdArgv")) {

			char *param_val = *((char **)value);
			if (TRUE == avd_comp->su->su_is_external) {
				rc = SA_AIS_ERR_INVALID_PARAM;
				goto done;
			}

			memset(&(avd_comp->comp_info.amstop_cmd_arg_info), 0, AVSV_MISC_STR_MAX_SIZE);
			strcpy((char *)&(avd_comp->comp_info.amstop_cmd_arg_info), param_val);
			param.attr_id = saAmfCompAmStopCmd_ID;
			strcpy((char *)&param.value[0], (char *)&avd_comp_type->saAmfCtRelPathAmStartCmd);
			/* We need to append the arg with the command. */
			cmd_argv = (char *)(param.value + strlen((char *)param.value));
			*cmd_argv++ = 0x20;	/* Insert SPACE between cmd and args */
			strncpy(cmd_argv, param_val, strlen(param_val));
			param.value_len = strlen((char *)param.value);

			memset(&avd_comp->comp_info.amstop_cmd_arg_info, 0, AVSV_MISC_STR_MAX_SIZE);
			strcpy((char *)&avd_comp->comp_info.amstop_cmd_arg_info, param_val);
			avd_comp->comp_info.amstop_len = param.value_len;
			memset(&avd_comp->comp_info.amstop_info, 0, AVSV_MISC_STR_MAX_SIZE);
			memcpy((char *)&avd_comp->comp_info.amstop_info, &param.value[0],
			       avd_comp->comp_info.amstop_len);

		} else if (!strcmp(attribute->attrName, "saAmfCompAmStopTimeout")) {

			SaTimeT timeout;
			SaTimeT temp_timeout;
			if (TRUE == avd_comp->su->su_is_external) {
				rc = SA_AIS_ERR_INVALID_PARAM;
				goto done;
			}

			timeout = *((SaTimeT *)value);
			m_NCS_OS_HTONLL_P(&temp_timeout, timeout);

			param.attr_id = saAmfCompAmStopTimeout_ID;
			param.value_len = sizeof(SaTimeT);
			memcpy(&param.value[0], &temp_timeout, param.value_len);
			avd_comp->comp_info.amstop_time = *((SaTimeT *)value);

		} else if (!strcmp(attribute->attrName, "saAmfCompNumMaxAmStopAttempt")) {

			uns32 num_am_stop;
			if (TRUE == avd_comp->su->su_is_external) {
				rc = SA_AIS_ERR_INVALID_PARAM;
				goto done;
			}

			num_am_stop = *((SaUint32T *)value);
			param.attr_id = saAmfCompNumMaxAmStopAttempts_ID;
			param.value_len = sizeof(uns32);
			num_am_stop = htonl(num_am_stop);
			memcpy(&param.value[0], &num_am_stop, param.value_len);
			avd_comp->comp_info.max_num_amstop = *((SaUint32T *)value);

		} else if (!strcmp(attribute->attrName, "saAmfCompCSISetCallbackTimeout")) {

			SaTimeT timeout;
			SaTimeT temp_timeout;
			timeout = *((SaTimeT *)value);
			m_NCS_OS_HTONLL_P(&temp_timeout, timeout);

			param.attr_id = saAmfCompCSISetCallbackTimeout_ID;
			param.value_len = sizeof(SaTimeT);
			memcpy(&param.value[0], &temp_timeout, param.value_len);
			avd_comp->comp_info.csi_set_callback_timeout = *((SaTimeT *)value);

		} else if (!strcmp(attribute->attrName, "saAmfCompCSIRmvCallbackTimeout")) {

			SaTimeT timeout;
			SaTimeT temp_timeout;
			timeout = *((SaTimeT *)value);
			m_NCS_OS_HTONLL_P(&temp_timeout, timeout);

			param.attr_id = saAmfCompCSIRmvCallbackTimeout_ID;
			param.value_len = sizeof(SaTimeT);
			memcpy(&param.value[0], &temp_timeout, param.value_len);
			avd_comp->comp_info.csi_rmv_callback_timeout = *((SaTimeT *)value);

		} else if (!strcmp(attribute->attrName, "saAmfCompQuiescingCompleteTimeout")) {

			SaTimeT timeout;
			SaTimeT temp_timeout;
			timeout = *((SaTimeT *)value);
			m_NCS_OS_HTONLL_P(&temp_timeout, timeout);

			param.attr_id = saAmfCompQuiescingCompleteTimeout_ID;
			param.value_len = sizeof(SaTimeT);
			memcpy(&param.value[0], &temp_timeout, param.value_len);
			avd_comp->comp_info.quiescing_complete_timeout = *((SaTimeT *)value);
		} else if (!strcmp(attribute->attrName, "saAmfCompRecoveryOnError")) {
			uns32 recovery;
			recovery = *((SaUint32T *)value);
			param.attr_id = saAmfCompRecoveryOnError_ID;
			param.value_len = sizeof(uns32);
			recovery = htonl(recovery);
			memcpy(&param.value[0], &recovery, param.value_len);
			avd_comp->comp_info.def_recvr = *((SaUint32T *)value);

		} else if (!strcmp(attribute->attrName, "saAmfCompDisableRestart")) {
			avd_comp->comp_info.comp_restart = *((SaUint32T *)value);
		} else if (!strcmp(attribute->attrName, "saAmfCompProxyCsi")) {
			avd_comp->comp_proxy_csi = *((SaNameT *)value);
		} else if (!strcmp(attribute->attrName, "saAmfCompContainerCsi")) {
			avd_comp->comp_container_csi = *((SaNameT *)value);
		} else {
			assert(0);
		}

		if (TRUE == node_present) {
			rc = avd_snd_op_req_msg(avd_cb, su_node_ptr, &param);
			if (rc != NCSCC_RC_SUCCESS)
				goto done;
		}
	}
 done:
	return;
}

/*****************************************************************************
 * Function: avd_comp_ccb_apply_delete_hdlr
 * 
 * Purpose: This routine handles delete operations on SaAmfComp objects.
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
static void avd_comp_ccb_apply_delete_hdlr(struct CcbUtilOperationData *opdata)
{
	AVD_COMP *comp = NULL, *i_comp = NULL;
	uns32 min_si = 0;
	NCS_BOOL isPre;
	AVD_AVND *su_node_ptr = NULL;
	AVSV_PARAM_INFO param;

	comp = avd_comp_find(&opdata->objectName);

	/* verify if the max ACTIVE and STANDBY SIs of the SU 
	 ** need to be changed
	 */
	if (comp->max_num_csi_actv == comp->su->si_max_active) {
		/* find the number and set it */
		min_si = 0;
		i_comp = comp->su->list_of_comp;
		while (i_comp) {
			if (i_comp != comp) {
				if (min_si > i_comp->max_num_csi_actv)
					min_si = i_comp->max_num_csi_actv;
				else if (min_si == 0)
					min_si = i_comp->max_num_csi_actv;
			}
			i_comp = i_comp->su_comp_next;
		}
		/* Now we have the min value. set it */
		comp->su->si_max_active = min_si;
	}

	/* FOR STANDBY count */
	if (comp->max_num_csi_stdby == comp->su->si_max_standby) {
		/* find the number and set it */
		min_si = 0;
		i_comp = comp->su->list_of_comp;
		while (i_comp) {
			if (i_comp != comp) {
				if (min_si > i_comp->max_num_csi_stdby)
					min_si = i_comp->max_num_csi_stdby;
				else if (min_si == 0)
					min_si = i_comp->max_num_csi_stdby;
			}
			i_comp = i_comp->su_comp_next;
		}
		/* Now we have the min value. set it */
		comp->su->si_max_standby = min_si;
	}

	/* Verify if the SUs preinstan value need to be changed */
	if ((NCS_COMP_TYPE_SA_AWARE == comp->comp_info.category) ||
	    (NCS_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE == comp->comp_info.category) ||
	    (NCS_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE == comp->comp_info.category)) {
		isPre = FALSE;
		i_comp = comp->su->list_of_comp;
		while (i_comp) {
			if (((NCS_COMP_TYPE_SA_AWARE == i_comp->comp_info.category) ||
			     (NCS_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE == i_comp->comp_info.category) ||
			     (NCS_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE == i_comp->comp_info.category))
			    && (i_comp != comp)) {
				isPre = TRUE;
				break;
			}
			i_comp = i_comp->su_comp_next;
		}		/* end while */

		if (isPre == TRUE) {
			comp->su->saAmfSUPreInstantiable = TRUE;
		} else {
			comp->su->saAmfSUPreInstantiable = FALSE;
		}
	}

	if (comp->su->curr_num_comp == 1) {
		/* This comp will be deleted so revert these to def val */
		comp->su->si_max_active = 0;
		comp->su->si_max_standby = 0;
		comp->su->saAmfSUPreInstantiable = TRUE;
	}

	/* Set runtime cached attributes. */
	if (avd_cb->impl_set == TRUE) {

		avd_saImmOiRtObjectUpdate(&comp->su->name,
				"saAmfSUPreInstantiable", SA_IMM_ATTR_SAUINT32T,
				&comp->su->saAmfSUPreInstantiable);
	}

	/* send a message to the AVND deleting the
	 * component.
	 */
	m_AVD_GET_SU_NODE_PTR(avd_cb, comp->su, su_node_ptr);
	if ((su_node_ptr->node_state == AVD_AVND_STATE_PRESENT) ||
	    (su_node_ptr->node_state == AVD_AVND_STATE_NO_CONFIG) ||
	    (su_node_ptr->node_state == AVD_AVND_STATE_NCS_INIT)) {
		memset(((uns8 *)&param), '\0', sizeof(AVSV_PARAM_INFO));
		param.act = AVSV_OBJ_OPR_DEL;
		param.name = comp->comp_info.name;
		param.class_id = AVSV_SA_AMF_COMP;
		avd_snd_op_req_msg(avd_cb, su_node_ptr, &param);
	}

	avd_comp_delete(comp);
}

/*****************************************************************************
 * Function: avd_comp_ccb_apply_cb
 *
 * Purpose: This routine handles all CCB operations on SaAmfComp objects.
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
static void avd_comp_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		avd_comp_ccb_apply_create_hdlr(opdata);
		break;
	case CCBUTIL_MODIFY:
		avd_comp_ccb_apply_modify_hdlr(opdata);
		break;
	case CCBUTIL_DELETE:
		avd_comp_ccb_apply_delete_hdlr(opdata);
		break;
	default:
		assert(0);
		break;
	}
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************
 * Function: avd_compcstype_find_match
 *  
 * Purpose:  This function will verify the the component and CSI are related
 *  in the table.
 *
 * Input: csi -  The CSI whose type need to be matched with the components CSI types list
 *        comp - The component whose list need to be searched.
 *
 * Returns: NCSCC_RC_SUCCESS, NCS_RC_FAILURE.
 *
 * NOTES: This function will be used only while assigning new susi.
 *        In this funtion we will find a match only if the matching comp_cs_type
 *        row status is active.
 **************************************************************************/

uns32 avd_compcstype_find_match(const AVD_CSI *csi, const AVD_COMP *comp)
{
	AVD_COMPCS_TYPE *cst;
	SaNameT dn;

	avd_create_association_class_dn(&csi->saAmfCSType, &comp->comp_info.name, "safSupportedCsType", &dn);
	avd_trace("'%s'", dn.value);
	cst = (AVD_COMPCS_TYPE *)ncs_patricia_tree_get(&avd_compcstype_db, (uns8 *)&dn);

	if (cst != NULL)
		return NCSCC_RC_SUCCESS;
	else
		return NCSCC_RC_FAILURE;
}

AVD_COMPCS_TYPE *avd_compcstype_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_COMPCS_TYPE *)ncs_patricia_tree_get(&avd_compcstype_db, (uns8 *)&tmp);
}

AVD_COMPCS_TYPE *avd_compcstype_getnext(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, dn->length);

	return (AVD_COMPCS_TYPE *)ncs_patricia_tree_getnext(&avd_compcstype_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_compcstype_delete
 *
 * Purpose:  This function will delete and free AVD_COMP_CS_TYPE structure from 
 * the tree.
 *
 * Input: cst - 
 *
 * Returns: -
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static void avd_compcstype_delete(AVD_COMPCS_TYPE *cst)
{
	unsigned int rc;

	rc = ncs_patricia_tree_del(&avd_compcstype_db, &cst->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);

	free(cst);
}

/**
 * Validate configuration attributes for an SaAmfCompCsType object
 * @param cst
 * 
 * @return int
 */
static int avd_compcstype_config_validate(const AVD_COMPCS_TYPE *cst)
{
	char *parent;
	char *dn = (char *)cst->name.value;

	/* This is an association class, the parent (SaAmfComp) should come after the second comma */
	if ((parent = strchr(dn, ',')) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "No parent to '%s' ", dn);
		return -1;
	}

	parent++;

	/* Second comma should be the parent */
	if ((parent = strchr(parent, ',')) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "No parent to '%s' ", dn);
		return -1;
	}

	parent++;

	/* Should be children to SaAmfComp */
	if (strncmp(parent, "safComp=", 8) != 0) {
		avd_log(NCSFL_SEV_ERROR, "Wrong parent '%s'", parent);
		return -1;
	}

	return 0;
}

AVD_COMPCS_TYPE *avd_compcstype_create(const SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	int rc = -1;
	AVD_COMPCS_TYPE *compcstype;
	AVD_CTCS_TYPE *ctcstype;
	SaNameT ctcstype_name;
	char *rdnval = strdup((char*)dn->value);
	char *p;
	SaNameT comp_name;
	AVD_COMP *comp;

	/*
	** If called from cold sync, attributes==NULL.
	** If called at new active at failover, the object is found in the DB
	** but needs to get configuration attributes initialized.
	*/
	if (NULL == (compcstype = avd_compcstype_find(dn))) {
		if ((compcstype = calloc(1, sizeof(*compcstype))) == NULL) {
			avd_log(NCSFL_SEV_ERROR, "calloc FAILED");
			return NULL;
		}
		
		memcpy(compcstype->name.value, dn->value, dn->length);
		compcstype->name.length = dn->length;
		compcstype->tree_node.key_info = (uns8 *)&(compcstype->name);
	}

	/* If no attributes supplied, go direct and add to DB */
	if (NULL == attributes)
		goto add_to_db;

	avsv_sanamet_init(dn, &comp_name, "safComp=");
	comp = avd_comp_find(&comp_name);

	p = strchr(rdnval, ',') + 1;
	p = strchr(p, ',');
	*p = '\0';
	ctcstype_name.length = sprintf((char*)ctcstype_name.value,
		"%s,%s", rdnval, comp->comp_type->name.value);
	ctcstype = avd_ctcstype_find(&ctcstype_name);

	if (ctcstype == NULL) {
		avd_log(NCSFL_SEV_ERROR, "avd_ctcstype_find FAILED for %s", ctcstype_name.value);
		goto done;
	}

	if (immutil_getAttr("saAmfCompNumMaxActiveCSIs", attributes, 0,
		&compcstype->saAmfCompNumMaxActiveCSIs) != SA_AIS_OK) {

		compcstype->saAmfCompNumMaxActiveCSIs = ctcstype->saAmfCtDefNumMaxActiveCSIs;
	}


	if (immutil_getAttr("saAmfCompNumMaxStandbyCSIs", attributes, 0,
		&compcstype->saAmfCompNumMaxStandbyCSIs) != SA_AIS_OK) {

		compcstype->saAmfCompNumMaxStandbyCSIs = ctcstype->saAmfCtDefNumMaxStandbyCSIs;
	}

	/* add to list in comp */
	compcstype->comp_list_compcstype_next = comp->compcstype_list;
	comp->compcstype_list = compcstype;

add_to_db:
	(void)ncs_patricia_tree_add(&avd_compcstype_db, &compcstype->tree_node);
	rc = 0;

done:
	if (rc != 0) {
		free(compcstype);
		compcstype = NULL;
	}

	return compcstype;
}

/**
 * Get configuration for all AMF CompCsType objects from IMM and
 * create AVD internal objects.
 * 
 * @param cb
 * @param comp
 * 
 * @return int
 */
static SaAisErrorT avd_compcstype_config_get(SaNameT *comp_name, AVD_COMP *comp)
{
	SaAisErrorT error;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfCompCsType";
	AVD_COMPCS_TYPE *compcstype;
	SaImmAttrNameT attributeNames[] = {"saAmfCompNumMaxActiveCSIs", "saAmfCompNumMaxStandbyCSIs", NULL};

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, comp_name, SA_IMM_SUBTREE,
		SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
		attributeNames, &searchHandle);

	if (SA_AIS_OK != error) {
		avd_log(NCSFL_SEV_ERROR, "saImmOmSearchInitialize_2 failed: %u", error);
		goto done1;
	}

	while ((error = immutil_saImmOmSearchNext_2(searchHandle, &dn,
		(SaImmAttrValuesT_2 ***)&attributes)) == SA_AIS_OK) {

		avd_log(NCSFL_SEV_NOTICE, "'%s'", dn.value);

		if ((compcstype = avd_compcstype_create(&dn, attributes)) == NULL) {
			error = SA_AIS_ERR_FAILED_OPERATION;
			goto done2;
		}

		if (avd_compcstype_config_validate(compcstype) != 0) {
			avd_compcstype_delete(compcstype);
			goto done2;
		}
	}

	error = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:

	return error;
}

static SaAisErrorT avd_compcstype_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_COMPCS_TYPE *cst;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE: {
		cst = avd_compcstype_create(&opdata->objectName, opdata->param.create.attrValues);

		if (cst == NULL)
			goto done;

		if (avd_compcstype_config_validate(cst) != 0)
			goto done;

		opdata->userData = cst;	/* Save for later use in apply */
		rc = SA_AIS_OK;
		break;
	}
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfCompCsType not supported");
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

static void avd_compcstype_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		break;
	case CCBUTIL_DELETE:{
		AVD_COMPCS_TYPE *compcstype = avd_compcstype_find(&opdata->objectName);
		avd_compcstype_delete(compcstype);
		break;
	}
	default:
		assert(0);
		break;
	}
}

static SaAisErrorT avd_compcstype_rt_attr_callback(SaImmOiHandleT immOiHandle,
	const SaNameT *objectName, const SaImmAttrNameT *attributeNames)
{
	AVD_COMPCS_TYPE *cst = avd_compcstype_find(objectName);
	SaImmAttrNameT attributeName;
	int i = 0;

	avd_trace("%s", objectName->value);
	assert(cst != NULL);

	while ((attributeName = attributeNames[i++]) != NULL) {
		if (!strcmp("saAmfCompNumCurrActiveCSIs", attributeName)) {
			(void)avd_saImmOiRtObjectUpdate(objectName, attributeName,
						       SA_IMM_ATTR_SAUINT32T, &cst->saAmfCompNumCurrActiveCSIs);
		} else if (!strcmp("saAmfCompNumCurrStandbyCSIs", attributeName)) {
			(void)avd_saImmOiRtObjectUpdate(objectName, attributeName,
						       SA_IMM_ATTR_SAUINT32T, &cst->saAmfCompNumCurrStandbyCSIs);
		} else if (!strcmp("saAmfCompAssignedCsi", attributeName)) {
			/* TODO */
		} else
			assert(0);
	}

	return SA_AIS_OK;
}

void avd_comp_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	assert(ncs_patricia_tree_init(&avd_comp_db, &patricia_params) == NCSCC_RC_SUCCESS);

	avd_class_impl_set("SaAmfComp", avd_comp_rt_attr_callback,
		avd_comp_admin_op_cb, avd_comp_ccb_completed_cb, avd_comp_ccb_apply_cb);

	assert(ncs_patricia_tree_init(&avd_compcstype_db, &patricia_params) == NCSCC_RC_SUCCESS);

	avd_class_impl_set("SaAmfCompCsType", avd_compcstype_rt_attr_callback,
		NULL, avd_compcstype_ccb_completed_cb, avd_compcstype_ccb_apply_cb);
}

