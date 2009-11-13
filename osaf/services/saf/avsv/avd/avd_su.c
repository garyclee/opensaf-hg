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
  the SU database on the AVD.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <avd_util.h>
#include <avd_su.h>
#include <avd_dblog.h>
#include <saImmOm.h>
#include <immutil.h>
#include <avd_imm.h>
#include <avd_cluster.h>
#include <avd_ntf.h>
#include <avd_proc.h>

static NCS_PATRICIA_TREE avd_su_db;
static NCS_PATRICIA_TREE avd_sutype_db;
static NCS_PATRICIA_TREE avd_sutcomptype_db;

static SaAisErrorT avd_sutcomptype_config_get(SaNameT *sutype_name, AVD_SU_TYPE *sut);

/*****************************************************************************
 * Function: avd_su_del_comp
 *
 * Purpose:  This function will del the given comp from the SU list, and fill
 * the components pointer with NULL
 *
 * Input: comp - The component pointer
 *
 * Returns: None. 
 *
 * NOTES: None
 *
 * 
 **************************************************************************/
void avd_su_del_comp(AVD_COMP *comp)
{
	AVD_COMP *i_comp = NULL;
	AVD_COMP *prev_comp = NULL;

	if (comp->su != NULL) {
		/* remove COMP from SU */
		i_comp = comp->su->list_of_comp;

		while ((i_comp != NULL) && (i_comp != comp)) {
			prev_comp = i_comp;
			i_comp = i_comp->su_comp_next;
		}

		if (i_comp == comp) {
			if (prev_comp == NULL) {
				comp->su->list_of_comp = comp->su_comp_next;
			} else {
				prev_comp->su_comp_next = comp->su_comp_next;
			}

			comp->su_comp_next = NULL;
			comp->su = NULL;

			/* decrement the active component number of this SU */
			comp->su->curr_num_comp--;
		}
	}
}

void avd_su_add_comp(AVD_COMP *comp)
{
	AVD_COMP *i_comp = comp->su->list_of_comp;

	if ((i_comp == NULL) || (i_comp->comp_info.inst_level >= comp->comp_info.inst_level)) {
		comp->su->list_of_comp = comp;
		comp->su_comp_next = i_comp;
	} else {
		while ((i_comp->su_comp_next != NULL) &&
		       (i_comp->su_comp_next->comp_info.inst_level < comp->comp_info.inst_level))
			i_comp = i_comp->su_comp_next;

		comp->su_comp_next = i_comp->su_comp_next;
		i_comp->su_comp_next = comp;
	}

	comp->su->curr_num_comp++;
	comp->su->num_of_comp++;	// TODO 
}

/*****************************************************************************
 * Function: avd_su_find
 *
 * Purpose:  This function will find a AVD_SU structure in the
 * tree with su_name value as key.
 *
 * Input: dn - The name of the SU.
 *        
 * Returns: The pointer to AVD_SU structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SU *avd_su_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SU *)ncs_patricia_tree_get(&avd_su_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_su_getnext
 *
 * Purpose:  This function will get the next AVD_SU structure in the
 * tree whose su_name value is next of the given su_name value.
 *
 * Input: dn - The name of the SU.
 *        
 * Returns: The pointer to AVD_SU structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SU *avd_su_getnext(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SU *)ncs_patricia_tree_getnext(&avd_su_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_su_delete
 *
 * Purpose:  This function will delete and free AVD_SU structure from 
 * the tree.
 *
 * Input: su - The SU structure that needs to be deleted.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_su_delete(AVD_SU *su)
{
	unsigned int rc = ncs_patricia_tree_del(&avd_su_db, &su->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	free(su);
}

/*****************************************************************************
 * Function: avd_su_del_avnd_list
 *
 * Purpose:  This function will del the given SU from the AVND list, and fill
 * the SUs pointer with NULL
 *
 * Input: cb - the AVD control block
 *        su - The SU pointer
 *
 * Returns: None. 
 *
 * NOTES: None
 *
 * 
 **************************************************************************/
void avd_su_del_avnd_list(AVD_CL_CB *cb, AVD_SU *su)
{
	AVD_SU *i_su = NULL;
	AVD_SU *prev_su = NULL;
	NCS_BOOL isNcs;

	if ((su->sg_of_su) && (su->sg_of_su->sg_ncs_spec == SA_TRUE))
		isNcs = TRUE;
	else
		isNcs = FALSE;

	/* For external component, there is no AvND attached, so let it return. */
	if (su->su_on_node != AVD_AVND_NULL) {
		/* remove SU from node */
		i_su = (isNcs) ? su->su_on_node->list_of_ncs_su : su->su_on_node->list_of_su;

		while ((i_su != NULL) && (i_su != su)) {
			prev_su = i_su;
			i_su = i_su->avnd_list_su_next;
		}

		if (i_su != su) {
			/* Log a fatal error */
		} else {
			if (prev_su == NULL) {
				if (isNcs)
					su->su_on_node->list_of_ncs_su = su->avnd_list_su_next;
				else
					su->su_on_node->list_of_su = su->avnd_list_su_next;
			} else {
				prev_su->avnd_list_su_next = su->avnd_list_su_next;
			}
		}

		su->avnd_list_su_next = NULL;
		su->su_on_node = AVD_AVND_NULL;
	}
	/* if (su->su_on_node != AVD_AVND_NULL) */
	return;
}

/*****************************************************************************
 * Function: avd_su_del_sg_list
 *
 * Purpose:  This function will del the given SU from the SG list, and fill
 * the SUs pointer with NULL
 *
 * Input: cb - the AVD control block
 *        su - The SU pointer
 *
 * Returns: None. 
 *
 * NOTES: None
 *
 * 
 **************************************************************************/

void avd_su_del_sg_list(AVD_CL_CB *cb, AVD_SU *su)
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

/*****************************************************************************
 * Function: avd_su_add_sg_list
 *
 * Purpose:  This function will add the given SU to the SG list, and fill
 * the SUs pointers. 
 *
 * Input: cb - the AVD control block
 *        su - The SU pointer
 *
 * Returns: None. 
 *
 * NOTES: None
 *
 * 
 **************************************************************************/

void avd_su_add_sg_list(AVD_CL_CB *cb, AVD_SU *su)
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

/*****************************************************************************
 * Function: avd_sutype_find
 *
 * Purpose:  This function will find a AVD_SU_TYPE structure in the
 *           tree with su_type_name value as key.
 *
 * Input: dn - The name of the su_type_name.
 *
 * Returns: The pointer to AVD_SU_TYPE structure found in the tree.
 *
 * NOTES:
 *
 *
 **************************************************************************/

AVD_SU_TYPE *avd_sutype_find(const SaNameT *dn)
{
	SaNameT tmp = {0};

	tmp.length = dn->length;
	memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SU_TYPE *)ncs_patricia_tree_get(&avd_sutype_db, (uns8 *)&tmp);
}

/*****************************************************************************
 * Function: avd_sutype_delete
 *
 * Purpose:  This function will delete a AVD_SU_TYPE structure from the
 *           tree. 
 *
 * Input: su_type - The su_type structure that needs to be deleted.
 *
 * Returns: Ok/Error.
 *
 * NOTES:
 *
 * 
 **************************************************************************/
void avd_sutype_delete(AVD_SU_TYPE *avd_su_type)
{
	uns32 rc;

	rc = ncs_patricia_tree_del(&avd_sutype_db, &avd_su_type->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);

	free(avd_su_type);
}

/*****************************************************************************
 * Function: avd_su_ack_msg
 *
 * Purpose:  This function processes the acknowledgment received for
 *          a SU related message from AVND for operator related changes.
 *          If the message acknowledges success change the row status of the
 *          SU to active, if failure change row status to not in service.
 *
 * Input: cb - the AVD control block
 *        ack_msg - The acknowledgement message received from the AVND.
 *
 * Returns: none.
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

void avd_su_ack_msg(AVD_CL_CB *cb, AVD_DND_MSG *ack_msg)
{
	AVD_SU *su;
	AVSV_PARAM_INFO param;
	AVD_AVND *avnd;

	/* check the ack message for errors. If error find the SU that
	 * has error.
	 */

	if (ack_msg->msg_info.n2d_reg_su.error != NCSCC_RC_SUCCESS) {
		/* Find the SU */
		su = avd_su_find(&ack_msg->msg_info.n2d_reg_su.su_name);
		if (su == NULL) {
			/* The SU has already been deleted. there is nothing
			 * that can be done.
			 */

			/* Log an information error that the SU is
			 * deleted.
			 */
			return;
		}

		/* log an fatal error as normally this shouldnt happen.
		 */

		m_AVD_LOG_INVALID_VAL_FATAL(ack_msg->msg_info.n2d_reg_su.error);

		m_AVD_GET_SU_NODE_PTR(cb, su, avnd);

		/* remove the SU from both the SG list and the
		 * AVND list if present.
		 */

		m_AVD_CB_LOCK(cb, NCS_LOCK_WRITE);

		avd_su_del_sg_list(cb, su);

		avd_su_del_avnd_list(cb, su);

		m_AVD_CB_UNLOCK(cb, NCS_LOCK_WRITE);

		if (su->list_of_comp != NULL) {
			/* Mark All the components as Not in service. Send
			 * the node delete request for all the components.
			 */

			while (su->list_of_comp != NULL) {
				m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, (su->list_of_comp), AVSV_CKPT_AVD_COMP_CONFIG);
				/* send a delete message to the AvND for the comp. */
				memset(((uns8 *)&param), '\0', sizeof(AVSV_PARAM_INFO));
				param.act = AVSV_OBJ_OPR_DEL;
				param.name = su->list_of_comp->comp_info.name;
				param.class_id = AVSV_SA_AMF_COMP;
				avd_snd_op_req_msg(cb, avnd, &param);
				/* delete from the SU list */
				avd_su_del_comp(su->list_of_comp);
			}

			su->si_max_active = 0;
			su->si_max_standby = 0;
			su->saAmfSUPreInstantiable = TRUE;
			su->curr_num_comp = 0;
			/* Set runtime cached attributes. */
			avd_saImmOiRtObjectUpdate(&su->name,
					"saAmfSUPreInstantiable", SA_IMM_ATTR_SAUINT32T,
					&su->saAmfSUPreInstantiable);
		}

		m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, su, AVSV_CKPT_AVD_SU_CONFIG);
		return;
	}

	/* Log an information error that the node was updated with the
	 * information of the SU.
	 */
	/* Find the SU */
	su = avd_su_find(&ack_msg->msg_info.n2d_reg_su.su_name);
	if (su == NULL) {
		/* The SU has already been deleted. there is nothing
		 * that can be done.
		 */

		/* Log an information error that the SU is
		 * deleted.
		 */
		/* send a delete message to the AvND for the SU. */
		memset(((uns8 *)&param), '\0', sizeof(AVSV_PARAM_INFO));
		param.act = AVSV_OBJ_OPR_DEL;
		param.name = ack_msg->msg_info.n2d_reg_su.su_name;
		param.class_id = AVSV_SA_AMF_SU;
		avnd = avd_node_find_nodeid(ack_msg->msg_info.n2d_reg_su.node_id);
		avd_snd_op_req_msg(cb, avnd, &param);
		return;
	}
}

/**
 * Validate configuration attributes for an AMF SU object
 * @param su
 * 
 * @return int
 */
static int avd_su_config_validate(const SaNameT *dn, const AVD_SU *su)
{
	if (avd_node_find(&su->saAmfSUHostNodeOrNodeGroup) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "Node '%s' does not exist", su->saAmfSUHostNodeOrNodeGroup.value);
		return -1;
	}

	if (su->saAmfSUFailover > SA_TRUE) {
		avd_log(NCSFL_SEV_ERROR, "Invalid saAmfSUFailover %u for '%s'", su->saAmfSUFailover, dn->value);
		return -1;
	}

	if (su->saAmfSUAdminState > SA_AMF_ADMIN_UNLOCKED) {
		avd_log(NCSFL_SEV_ERROR, "Invalid saAmfSUAdminState %u for '%s'", su->saAmfSUAdminState, dn->value);
		return -1;
	}

	return 0;
}

/**
 * Create a new SU object and initialize its attributes from the attribute list.
 * 
 * @param su_name
 * @param attributes
 * 
 * @return AVD_SU*
 */
/***/
AVD_SU *avd_su_create(const SaNameT *su_name, const SaImmAttrValuesT_2 **attributes)
{
	int rc = -1;
	AVD_SU *su;
	AVD_SU_TYPE *sut;
	SaNameT sg_name;

	/*
	** If called from cold sync, attributes==NULL.
	** If called at new active at failover, the object is found in the DB
	** but needs to get configuration attributes initialized.
	*/
	if (NULL == (su = avd_su_find(su_name))) {
		if ((su = calloc(1, sizeof(AVD_SU))) == NULL) {
			avd_log(NCSFL_SEV_ERROR, "calloc FAILED");
			goto done;
		}
		
		memcpy(su->name.value, su_name->value, su_name->length);
		su->name.length = su_name->length;
		su->tree_node.key_info = (uns8 *)&(su->name);
		avsv_sanamet_init(su_name, &sg_name, "safSg");
		su->sg_of_su = avd_sg_find(&sg_name);
		su->saAmfSUFailover = FALSE;
		su->term_state = FALSE;
		su->su_switch = AVSV_SI_TOGGLE_STABLE;
		su->saAmfSUPreInstantiable = TRUE;
		su->saAmfSUOperState = SA_AMF_OPERATIONAL_DISABLED;
		su->saAmfSUPresenceState = SA_AMF_PRESENCE_UNINSTANTIATED;
		su->saAmfSuReadinessState = SA_AMF_READINESS_OUT_OF_SERVICE;
		su->su_act_state = AVD_SU_NO_STATE;
		su->su_is_external = FALSE;
	}

	/* If no attributes supplied, go direct and add to DB */
	if (NULL == attributes)
		goto add_to_db;

	if (immutil_getAttr("saAmfSUType", attributes, 0, &su->saAmfSUType) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfSUType FAILED for '%s'", su_name->value);
		goto done;
	}

	if (immutil_getAttr("saAmfSURank", attributes, 0, &su->saAmfSURank) != SA_AIS_OK) {
		/* Empty, assign default value */
		su->saAmfSURank = 0;
	}

	if (immutil_getAttr("saAmfSUHostNodeOrNodeGroup", attributes, 0, &su->saAmfSUHostNodeOrNodeGroup) != SA_AIS_OK) {
		/* TODO: this is no error! */
		avd_log(NCSFL_SEV_ERROR, "Get saAmfSUHostNodeOrNodeGroup FAILED for '%s'", su_name->value);
		goto done;
	}

	memcpy(&su->saAmfSUHostedByNode, &su->saAmfSUHostNodeOrNodeGroup, sizeof(SaNameT));

	if ((sut = avd_sutype_find(&su->saAmfSUType)) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "saAmfSUType '%s' does not exist", su->saAmfSUType.value);
		goto done;
	}

	if (immutil_getAttr("saAmfSUFailover", attributes, 0, &su->saAmfSUFailover) != SA_AIS_OK) {
		su->saAmfSUFailover = sut->saAmfSutDefSUFailover;
	}

	(void)immutil_getAttr("saAmfSUMaintenanceCampaign", attributes, 0, &su->saAmfSUMaintenanceCampaign);

	if (immutil_getAttr("saAmfSUAdminState", attributes, 0, &su->saAmfSUAdminState) != SA_AIS_OK)
		su->saAmfSUAdminState = SA_AMF_ADMIN_UNLOCKED;

	su->su_on_su_type = sut;
	su->si_max_active = -1;	// TODO
	su->si_max_standby = -1;	// TODO

add_to_db:
	(void)ncs_patricia_tree_add(&avd_su_db, &su->tree_node);
	rc = 0;

done:
	if (rc != 0) {
		free(su);
		su = NULL;
	}
	return su;
}

/*****************************************************************************
 * Function: avd_su_add_to_model
 * 
 * Purpose: This routine adds SaAmfSU objects to Patricia tree.
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
static void avd_su_add_to_model(AVD_SU *su)
{
	unsigned int rc = NCSCC_RC_SUCCESS;
	AVD_SU_TYPE *sut = su->su_on_su_type;
	AVD_AVND *node = NULL;

	/* Add SU to list in SU Type */
	su->su_list_su_type_next = sut->list_of_su;
	sut->list_of_su = su;

	/* add to the list of SG  */
	avd_su_add_sg_list(avd_cb, su);

	if (FALSE == su->su_is_external) {
		/* Find the AVND node and link it to the structure.
		 */

		if ((su->su_on_node = avd_node_find(&su->saAmfSUHostNodeOrNodeGroup)) == AVD_AVND_NULL) {
			avd_su_del_sg_list(avd_cb, su);
			rc = SA_AIS_ERR_INVALID_PARAM;
			goto done;
		}

		if (strstr((char *)su->name.value, "safApp=OpenSAF") != NULL) {
			su->avnd_list_su_next = su->su_on_node->list_of_ncs_su;
			su->su_on_node->list_of_ncs_su = su;
		} else {
			su->avnd_list_su_next = su->su_on_node->list_of_su;
			su->su_on_node->list_of_su = su;
		}

		node = su->su_on_node;
	} /* if(FALSE == su->su_is_external)  */
	else {
		if (NULL == avd_cb->ext_comp_info.ext_comp_hlt_check) {
			/* This is an external SU and we need to create the 
			   supporting info. */
			avd_cb->ext_comp_info.ext_comp_hlt_check = malloc(sizeof(AVD_AVND));
			if (NULL == avd_cb->ext_comp_info.ext_comp_hlt_check) {
				avd_su_del_sg_list(avd_cb, su);
				avd_log(NCSFL_SEV_ERROR, "Memory Alloc Failure");
			}
			memset(avd_cb->ext_comp_info.ext_comp_hlt_check, 0, sizeof(AVD_AVND));
			avd_cb->ext_comp_info.local_avnd_node = avd_node_find_nodeid(avd_cb->node_id_avd);

			if (NULL == avd_cb->ext_comp_info.local_avnd_node) {
				m_AVD_PXY_PXD_ERR_LOG("sutableentry_set:Local node unavailable:SU Name,node id are",
						      &su->name, avd_cb->node_id_avd, 0, 0, 0);
				avd_su_del_sg_list(avd_cb, su);
				avd_log(NCSFL_SEV_ERROR, "Avnd Lookup failure, node id %u", avd_cb->node_id_avd);
			}

		}
		/* if(NULL == avd_cb->ext_comp_info.ext_comp_hlt_check) */
		node = avd_cb->ext_comp_info.local_avnd_node;

	}			/* else of if(FALSE == su->su_is_external)  */
#if 0
	if ((node->node_state == AVD_AVND_STATE_PRESENT) ||
	    (node->node_state == AVD_AVND_STATE_NO_CONFIG) ||
	    (node->node_state == AVD_AVND_STATE_NCS_INIT)) {
		if (avd_snd_su_msg(avd_cb, su) != NCSCC_RC_SUCCESS) {
			avd_su_del_avnd_list(avd_cb, su);
			avd_su_del_sg_list(avd_cb, su);

			rc = SA_AIS_ERR_INVALID_PARAM;
			goto done;
		}
	}
#endif

done:
	assert(rc == NCSCC_RC_SUCCESS);
	/* Set runtime cached attributes. */
	if (avd_cb->impl_set == TRUE) {
		avd_saImmOiRtObjectUpdate(&su->name,
				"saAmfSUPreInstantiable", SA_IMM_ATTR_SAUINT32T,
				&su->saAmfSUPreInstantiable);

		avd_saImmOiRtObjectUpdate(&su->name,
				"saAmfSUHostedByNode", SA_IMM_ATTR_SANAMET,
				&su->saAmfSUHostedByNode);

		avd_saImmOiRtObjectUpdate(&su->name,
				"saAmfSUPresenceState", SA_IMM_ATTR_SAUINT32T,
				&su->saAmfSUPresenceState);

		avd_saImmOiRtObjectUpdate(&su->name,
				"saAmfSUOperState", SA_IMM_ATTR_SAUINT32T,
				&su->saAmfSUOperState);

		avd_saImmOiRtObjectUpdate(&su->name,
				"saAmfSUReadinessState", SA_IMM_ATTR_SAUINT32T,
				&su->saAmfSuReadinessState);
	}

}

/**
 * Get configuration for all SaAmfSU objects from IMM and
 * create AVD internal objects.
 * 
 * @param sg_name
 * @param sg
 * 
 * @return int
 */
SaAisErrorT avd_su_config_get(const SaNameT *sg_name, AVD_SG *sg)
{
	SaAisErrorT error;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT su_name;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfSU";
	AVD_SU *su;

	assert(sg != NULL);

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, sg_name, SA_IMM_SUBTREE,
						  SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
						  NULL, &searchHandle);

	if (SA_AIS_OK != error) {
		avd_log(NCSFL_SEV_ERROR, "No objects found (1)");
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &su_name, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {
		avd_log(NCSFL_SEV_NOTICE, "'%s'", su_name.value);

		if ((su = avd_su_create(&su_name, attributes)) == NULL) {
			error = SA_AIS_ERR_FAILED_OPERATION;
			goto done2;
		}

		if (avd_su_config_validate(&su_name, su) != 0) {
			avd_log(NCSFL_SEV_ERROR, "SU '%s' validation error", su_name.value);
			avd_su_delete(su);
			goto done2;
		}

		avd_su_add_to_model(su);

		if (avd_comp_config_get(&su_name, su) != SA_AIS_OK) {
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
 * Function: avd_sutype_create
 * 
 * Purpose: This routine creates new SaAmfSUType objects.
 * 
 *
 * Input  : Ccb Util Oper Data
 *  
 * Returns: Pointer to SU Type structure.
 *  
 * NOTES  : None.
 *
 *
 **************************************************************************/
static AVD_SU_TYPE *avd_sutype_create(const SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	AVD_SU_TYPE *sut;
	int rc, i;

	if ((sut = calloc(1, sizeof(AVD_SU_TYPE))) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "calloc failed");
		return NULL;
	}
	memset(sut, 0, sizeof(AVD_SU_TYPE));
	memcpy(sut->su_type_name.value, dn->value, dn->length);
	sut->su_type_name.length = dn->length;
	sut->tree_node.key_info = (uns8 *)&(sut->su_type_name);

	if (immutil_getAttr("saAmfSutIsExternal", attributes, 0, &sut->saAmfSutIsExternal) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfSutIsExternal FAILED for '%s'", dn->value);
		goto done;
	}

	if (immutil_getAttr("saAmfSutDefSUFailover", attributes, 0, &sut->saAmfSutDefSUFailover) != SA_AIS_OK) {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfSutDefSUFailover FAILED for '%s'", dn->value);
		goto done;
	}

	if (immutil_getAttrValuesNumber("saAmfSutProvidesSvcTypes", attributes, &sut->number_svc_types) == SA_AIS_OK) {
		sut->saAmfSutProvidesSvcTypes = malloc(sut->number_svc_types * sizeof(SaNameT));
		for (i = 0; i < sut->number_svc_types; i++) {
			if (immutil_getAttr("saAmfSutProvidesSvcTypes", attributes, i,
					    &sut->saAmfSutProvidesSvcTypes[i]) != SA_AIS_OK) {
				avd_log(NCSFL_SEV_ERROR, "Get saAmfSutProvidesSvcTypes FAILED for '%s'", dn->value);
				goto done;
			}
		}
	} else {
		avd_log(NCSFL_SEV_ERROR, "Get saAmfSGtValidSuTypes FAILED for '%s'", dn->value);
		goto done;
	}

	rc = ncs_patricia_tree_add(&avd_sutype_db, &sut->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	rc = 0;
 done:
	if (rc != 0) {
		free(sut);
		sut = NULL;
	}

	return sut;
}

/*****************************************************************************
 * Function: avd_sutype_config_validate
 * 
 * Purpose: This routine handles all CCB operations on SaAmfSUType objects.
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
static SaAisErrorT avd_sutype_config_validate(const AVD_SU_TYPE *su_type, const CcbUtilOperationData_t *opdata)
{
	/* int i = 0; */
	char *parent;
	char *dn = (char *)su_type->su_type_name.value;

	if ((parent = strchr(dn, ',')) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "No parent to '%s' ", dn);
		return -1;
	}

	parent++;

	/* Should be children to the SU Base type */
	if (strncmp(parent, "safSuType=", 10) != 0) {
		avd_log(NCSFL_SEV_ERROR, "Wrong parent '%s' to '%s' ", parent, dn);
		return -1;
	}
#if 0
	/*  TODO we need proper Svc Type handling for this, revisit */

	/* Make sure all Svc types exist */
	for (i = 0; i < su_type->number_svc_types; i++) {
		AVD_AMF_SVC_TYPE *svc_type =
		    avd_svctype_find(avd_cb, su_type->saAmfSutProvidesSvcTypes[i], TRUE);
		if (svc_type == NULL) {
			/* Svc type does not exist in current model, check CCB */
			if ((opdata != NULL) &&
			    (ccbutil_getCcbOpDataByDN(opdata->ccbId, &su_type->saAmfSutProvidesSvcTypes[i]) == NULL)) {
				avd_log(NCSFL_SEV_ERROR, "Svc type '%s' does not exist either in model or CCB",
					su_type->saAmfSutProvidesSvcTypes[i]);
				return SA_AIS_ERR_BAD_OPERATION;
			}
			avd_log(NCSFL_SEV_ERROR, "AMF Svc type '%s' does not exist",
				su_type->saAmfSutProvidesSvcTypes[i].value);
			return -1;
		}
	}
#endif
	if (su_type->saAmfSutDefSUFailover > SA_TRUE) {
		avd_log(NCSFL_SEV_ERROR, "Wrong saAmfSutDefSUFailover value '%u'", su_type->saAmfSutDefSUFailover);
		return -1;
	}

	if (su_type->saAmfSutIsExternal > SA_TRUE) {
		avd_log(NCSFL_SEV_ERROR, "Wrong saAmfSutIsExternal value '%u'", su_type->saAmfSutIsExternal);
		return -1;
	}

	return 0;
}

/**
 * Get configuration for all SaAmfSUType objects from IMM and
 * create AVD internal objects.
 * @param cb
 * 
 * @return int
 */
SaAisErrorT avd_sutype_config_get(void)
{
	AVD_SU_TYPE *sut;
	SaAisErrorT error;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfSUType";

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

		if ((sut = avd_sutype_create(&dn, attributes)) == NULL) {
			error = SA_AIS_ERR_FAILED_OPERATION;
			goto done2;
		}

		if (avd_sutype_config_validate(sut, NULL) != 0) {
			avd_log(NCSFL_SEV_ERROR, "AMF SU type '%s' validation error", dn.value);
			goto done2;
		}

		if (avd_sutcomptype_config_get(&dn, sut) != SA_AIS_OK) {
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

void avd_su_pres_state_set(AVD_SU *su, SaAmfPresenceStateT pres_state)
{
	assert(su != NULL);
	assert(pres_state <= SA_AMF_PRESENCE_TERMINATION_FAILED);
	avd_log(NCSFL_SEV_NOTICE, "'%s' %s => %s",
		su->name.value, avd_pres_state_name[su->saAmfSUPresenceState], avd_pres_state_name[pres_state]);
	su->saAmfSUPresenceState = pres_state;
	avd_saImmOiRtObjectUpdate(&su->name,
				 "saAmfSUPresenceState", SA_IMM_ATTR_SAUINT32T, &su->saAmfSUPresenceState);
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, su, AVSV_CKPT_SU_PRES_STATE);
}

void avd_su_oper_state_set(AVD_SU *su, SaAmfOperationalStateT oper_state)
{
	assert(su != NULL);
	assert(oper_state <= SA_AMF_OPERATIONAL_DISABLED);
	avd_log(NCSFL_SEV_NOTICE, "'%s' %s => %s",
		su->name.value, avd_oper_state_name[su->saAmfSUOperState], avd_oper_state_name[oper_state]);
	su->saAmfSUOperState = oper_state;
	avd_saImmOiRtObjectUpdate(&su->name,
		"saAmfSUOperState", SA_IMM_ATTR_SAUINT32T, &su->saAmfSUOperState);
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, su, AVSV_CKPT_SU_OPER_STATE);
}

void avd_su_readiness_state_set(AVD_SU *su, SaAmfReadinessStateT readiness_state)
{
        AVD_COMP *comp = NULL;
	assert(su != NULL);
	assert(readiness_state <= SA_AMF_READINESS_STOPPING);
	avd_log(NCSFL_SEV_NOTICE, "'%s' %s => %s",
		su->name.value,
		avd_readiness_state_name[su->saAmfSuReadinessState], avd_readiness_state_name[readiness_state]);
	su->saAmfSuReadinessState = readiness_state;
	avd_saImmOiRtObjectUpdate(&su->name,
		 "saAmfSUReadinessState", SA_IMM_ATTR_SAUINT32T, &su->saAmfSuReadinessState);
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, su, AVSV_CKPT_SU_READINESS_STATE);

	/* Since Su readiness state has changed, we need to change it for all the component in this SU.*/
	comp = su->list_of_comp;
	while (comp != NULL) {
		SaAmfReadinessStateT saAmfCompReadinessState;
		if ((su->saAmfSuReadinessState == SA_AMF_READINESS_IN_SERVICE) &&
				(comp->saAmfCompOperState == SA_AMF_OPERATIONAL_ENABLED)) {
			saAmfCompReadinessState = SA_AMF_READINESS_IN_SERVICE;
		} else if((su->saAmfSuReadinessState == SA_AMF_READINESS_STOPPING) &&
				(comp->saAmfCompOperState == SA_AMF_OPERATIONAL_ENABLED)) {
			saAmfCompReadinessState = SA_AMF_READINESS_STOPPING;
		} else
			saAmfCompReadinessState = SA_AMF_READINESS_OUT_OF_SERVICE;

		avd_comp_readiness_state_set(comp, saAmfCompReadinessState);
		comp = comp->su_comp_next;
	}
}

static const char *admin_state_name[] = {
	"INVALID",
	"UNLOCKED",
	"LOCKED",
	"LOCKED_INSTANTIATION",
	"SHUTTING_DOWN"
};

void avd_su_admin_state_set(AVD_SU *su, SaAmfAdminStateT admin_state)
{
	assert(su != NULL);
	assert(admin_state <= SA_AMF_ADMIN_SHUTTING_DOWN);
	avd_log(NCSFL_SEV_NOTICE, "'%s' %s => %s",
		su->name.value, admin_state_name[su->saAmfSUAdminState], admin_state_name[admin_state]);
	su->saAmfSUAdminState = admin_state;
	avd_saImmOiRtObjectUpdate(&su->name,
		"saAmfSUAdminState", SA_IMM_ATTR_SAUINT32T, &su->saAmfSUAdminState);
	m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, su, AVSV_CKPT_SU_OPER_STATE);
	avd_gen_su_admin_state_changed_ntf(avd_cb, su);
}

/*****************************************************************************
 * Function: avd_sutype_ccb_apply_delete_hdlr
 * 
 * Purpose: This routine handles delete operations on SaAmfSUType objects.
 * 
 *
 * Input  : Ccb Util Oper Data
 *  
 * Returns: None.
 *  
 ****************************************************************************/
static void avd_sutype_ccb_apply_delete_hdlr(CcbUtilOperationData_t *opdata)
{
	AVD_SU_TYPE *avd_su_type;

	avd_su_type = avd_sutype_find(&opdata->objectName);
	assert(avd_su_type != NULL);

	if (NULL != avd_su_type->saAmfSutProvidesSvcTypes)
		free(avd_su_type->saAmfSutProvidesSvcTypes);

	assert(NULL == avd_su_type->list_of_su);
	avd_sutype_delete(avd_su_type);
}

/*****************************************************************************
 * Function: avd_sutype_ccb_apply_cb
 * 
 * Purpose: This routine handles all CCB apply operations on SaAmfSUType objects.
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
static void avd_sutype_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		break;
	case CCBUTIL_DELETE:
		avd_sutype_ccb_apply_delete_hdlr(opdata);
		break;
	default:
		assert(0);
		break;
	}
}

/*****************************************************************************
 * Function: avd_sutype_ccb_completed_cb
 * 
 * Purpose: This routine handles all CCB operations on SaAmfSUType objects.
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
static SaAisErrorT avd_sutype_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_SU_TYPE *su_type;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if ((su_type = avd_sutype_create(&opdata->objectName, opdata->param.create.attrValues)) == NULL) {
			goto done;
		}

		if (avd_sutype_config_validate(su_type, opdata) != 0)
			goto done;

		opdata->userData = su_type;	/* Save for later use in apply */
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfSUType not supported");
		break;
	case CCBUTIL_DELETE:
		su_type = avd_sutype_find(&opdata->objectName);
		if (NULL != su_type->list_of_su) {
			avd_log(NCSFL_SEV_ERROR, "SaAmfSUType is in use");
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
 * Function: avd_su_del_su_type_list
 *
 * Purpose:  This function will del the given su from su_type list.
 *
 * Input: cb - the AVD control block
 *        su - The su pointer
 *
 * Returns: None.
 *
 * NOTES: None
 *
 *
 **************************************************************************/
void avd_su_del_su_type_list(AVD_CL_CB *cb, AVD_SU *su)
{
	AVD_SU *i_su = NULL;
	AVD_SU *prev_su = NULL;

	if (su->su_on_su_type != NULL) {
		i_su = su->su_on_su_type->list_of_su;

		while ((i_su != NULL) && (i_su != su)) {
			prev_su = i_su;
			i_su = i_su->su_list_su_type_next;
		}

		if (i_su != su) {
			/* Log a fatal error */
		} else {
			if (prev_su == NULL) {
				su->su_on_su_type->list_of_su = su->su_list_su_type_next;
			} else {
				prev_su->su_list_su_type_next = su->su_list_su_type_next;
			}
		}

		su->su_list_su_type_next = NULL;
		su->su_on_su_type = NULL;
	}

	return;
}

/*****************************************************************************
 * Function: avd_su_admin_op_cb
 * 
 * Purpose: This routine handles all Admin operations on SaAmfSU objects.
 * 
 *
 * Input  : Oi handle, Invocation, Object name, oper id and params.
 *
 * Returns: None.
 *
 * NOTES  : None.
 *
 *
 **************************************************************************/
static void avd_su_admin_op_cb(SaImmOiHandleT immoi_handle,
			       SaInvocationT invocation,
			       const SaNameT *su_name,
			       SaImmAdminOperationIdT op_id, const SaImmAdminOperationParamsT_2 **params)
{
	AVD_CL_CB *cb = (AVD_CL_CB*) avd_cb;
	AVD_SU    *su = NULL;
	AVD_AVND  *su_node_ptr = NULL;
	SaBoolT   is_oper_successful = SA_TRUE;
	SaAmfAdminStateT adm_state = SA_AMF_ADMIN_LOCK;
	SaAmfReadinessStateT back_red_state;
	SaAmfAdminStateT back_admin_state;
	SaAisErrorT rc;

	/* Only unlock, shutdown, lock, lock-inst, unlock-inst are supported for SU */
	if ( op_id > SA_AMF_ADMIN_SHUTDOWN ) {
		immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_ERR_NOT_SUPPORTED);
		return;
	}

	/* Check if AvD is ready for doing an admin operation */
	if ( cb->init_state != AVD_APP_STATE ) {
		immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_ERR_NOT_READY);
		return;
	}

	/* Get the SU DB */
	su = (AVD_SU*)avd_su_find(su_name);

	if ( su == NULL ) {
		immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_ERR_NOT_EXIST);
		return;
	}

	/* Admin operation on OpenSAF service SU is not allowed. */
	if ( su->sg_of_su->sg_ncs_spec == SA_TRUE ) {
		immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_ERR_NOT_SUPPORTED);
		return;
	}

	/* Check if any admin operation is already going on the SU or SG of SU is not stable.*/
	if ( (su->pend_cbk.invocation != 0) || (su->sg_of_su->sg_fsm_state != AVD_SG_FSM_STABLE) ) {
		immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_ERR_TRY_AGAIN);
		return;
	}

	/* Check if admin state is already such that admin operation is not required */
	if ( ((su->saAmfSUAdminState == SA_AMF_ADMIN_UNLOCKED) && (op_id == SA_AMF_ADMIN_UNLOCK)) ||
	     ((su->saAmfSUAdminState == SA_AMF_ADMIN_LOCKED)   && (op_id == SA_AMF_ADMIN_LOCK))   ||
	     ((su->saAmfSUAdminState == SA_AMF_ADMIN_LOCKED_INSTANTIATION) &&
	      (op_id == SA_AMF_ADMIN_LOCK_INSTANTIATION))                                     ||
	     ((su->saAmfSUAdminState == SA_AMF_ADMIN_LOCKED)   &&
	      (op_id == SA_AMF_ADMIN_SHUTDOWN))   ) {
		immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_ERR_NO_OP);
		return;
	}

	/* Check if admin operation is valid against FSM transitions */
	if ( ((su->saAmfSUAdminState == SA_AMF_ADMIN_UNLOCKED) && (op_id != SA_AMF_ADMIN_LOCK))    ||
	     ((su->saAmfSUAdminState == SA_AMF_ADMIN_LOCKED)   &&
	      ((op_id != SA_AMF_ADMIN_UNLOCK) && (op_id != SA_AMF_ADMIN_LOCK_INSTANTIATION)))  ||
	     ((su->saAmfSUAdminState == SA_AMF_ADMIN_LOCKED_INSTANTIATION) &&
	      (op_id != SA_AMF_ADMIN_UNLOCK_INSTANTIATION))                                    ||
	     ((su->saAmfSUAdminState != SA_AMF_ADMIN_UNLOCKED) && (op_id == SA_AMF_ADMIN_SHUTDOWN))  ) {
		immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_ERR_BAD_OPERATION);
		return;
	}

	/* Validation has passed and admin operation should be done. Proceed with it... */
	switch (op_id) {
	/* Valid B.04 AMF node admin operations */
	case SA_AMF_ADMIN_UNLOCK:
		avd_su_admin_state_set(su,SA_AMF_ADMIN_UNLOCKED);
		if (su->num_of_comp == su->curr_num_comp) {
			m_AVD_GET_SU_NODE_PTR(cb,su,su_node_ptr);

			if (m_AVD_APP_SU_IS_INSVC(su,su_node_ptr)) {
				avd_su_readiness_state_set(su,SA_AMF_READINESS_IN_SERVICE);
				switch (su->sg_of_su->sg_redundancy_model) {
				case SA_AMF_2N_REDUNDANCY_MODEL:
					if (avd_sg_2n_su_insvc_func(cb, su) != NCSCC_RC_SUCCESS) {
						avd_su_readiness_state_set(su,SA_AMF_READINESS_OUT_OF_SERVICE);
						avd_su_admin_state_set(su,SA_AMF_ADMIN_LOCKED);
						is_oper_successful = SA_FALSE;
					}
					break;

				case SA_AMF_N_WAY_REDUNDANCY_MODEL:
					if (avd_sg_nway_su_insvc_func(cb, su) != NCSCC_RC_SUCCESS) {
						avd_su_readiness_state_set(su,SA_AMF_READINESS_OUT_OF_SERVICE);
						avd_su_admin_state_set(su,SA_AMF_ADMIN_LOCKED);
						is_oper_successful = SA_FALSE;
					}
					break;

				case SA_AMF_N_WAY_ACTIVE_REDUNDANCY_MODEL:
					if (avd_sg_nacvred_su_insvc_func(cb, su) != NCSCC_RC_SUCCESS) {

						avd_su_readiness_state_set(su,SA_AMF_READINESS_OUT_OF_SERVICE);
						avd_su_admin_state_set(su,SA_AMF_ADMIN_LOCKED);
						is_oper_successful = SA_FALSE;
					}
					break;

				case SA_AMF_NPM_REDUNDANCY_MODEL:
					if (avd_sg_npm_su_insvc_func(cb, su) != NCSCC_RC_SUCCESS) {
						avd_su_readiness_state_set(su,SA_AMF_READINESS_OUT_OF_SERVICE);
						avd_su_admin_state_set(su,SA_AMF_ADMIN_LOCKED);
						is_oper_successful = SA_FALSE;
					}
					break;

				case SA_AMF_NO_REDUNDANCY_MODEL:
				default:
					if (avd_sg_nored_su_insvc_func(cb, su) != NCSCC_RC_SUCCESS) {
						avd_su_readiness_state_set(su,SA_AMF_READINESS_OUT_OF_SERVICE);
						avd_su_admin_state_set(su,SA_AMF_ADMIN_LOCKED);
						is_oper_successful = SA_FALSE;
					}
					break;
				}

				avd_sg_app_su_inst_func(cb, su->sg_of_su);
			}
		}

		if ( is_oper_successful == SA_TRUE ) {
			if ( su->sg_of_su->sg_fsm_state == AVD_SG_FSM_SG_REALIGN ) {
				/* Store the callback parameters to send operation result later on. */
				su->pend_cbk.admin_oper = op_id;
				su->pend_cbk.invocation = invocation;
				return;
			} else {
				immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_OK);
				return;
			}
		} else {
			immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_ERR_FAILED_OPERATION);
			return;
		}

		break;

	case SA_AMF_ADMIN_SHUTDOWN:
		adm_state = SA_AMF_ADMIN_SHUTTING_DOWN;

	case SA_AMF_ADMIN_LOCK:

		if (su->list_of_susi == NULL) {
			avd_su_readiness_state_set(su,SA_AMF_READINESS_OUT_OF_SERVICE);
			avd_su_admin_state_set(su,SA_AMF_ADMIN_LOCKED);
			avd_sg_app_su_inst_func(cb, su->sg_of_su);
			immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_OK);
			return;
		}

		back_red_state = su->saAmfSuReadinessState;
		back_admin_state = su->saAmfSUAdminState;
		avd_su_readiness_state_set(su,SA_AMF_READINESS_OUT_OF_SERVICE);
		avd_su_admin_state_set(su,(adm_state));

		m_AVD_GET_SU_NODE_PTR(cb,su,su_node_ptr);

		switch (su->sg_of_su->sg_redundancy_model) {
		case SA_AMF_2N_REDUNDANCY_MODEL:
			if (avd_sg_2n_su_admin_fail(cb, su, su_node_ptr) != NCSCC_RC_SUCCESS) {
				avd_su_readiness_state_set(su,back_red_state);
				avd_su_admin_state_set(su,back_admin_state);
				is_oper_successful = SA_FALSE;
			}
			break;

		case SA_AMF_N_WAY_REDUNDANCY_MODEL:
			if (avd_sg_nway_su_admin_fail(cb, su, su_node_ptr) != NCSCC_RC_SUCCESS) {
				avd_su_readiness_state_set(su,back_red_state);
				avd_su_admin_state_set(su,back_admin_state);
				is_oper_successful = SA_FALSE;
			}
			break;

		case SA_AMF_NPM_REDUNDANCY_MODEL:
			if (avd_sg_npm_su_admin_fail(cb, su, su_node_ptr) != NCSCC_RC_SUCCESS) {
				avd_su_readiness_state_set(su,back_red_state);
				avd_su_admin_state_set(su,back_admin_state);
				is_oper_successful = SA_FALSE;
			}
			break;

		case SA_AMF_N_WAY_ACTIVE_REDUNDANCY_MODEL:
			if (avd_sg_nacvred_su_admin_fail(cb, su, su_node_ptr) != NCSCC_RC_SUCCESS) {
				avd_su_readiness_state_set(su,back_red_state);
				avd_su_admin_state_set(su,back_admin_state);
				is_oper_successful = SA_FALSE;
			}
			break;

		case SA_AMF_NO_REDUNDANCY_MODEL:

			if (avd_sg_nored_su_admin_fail(cb, su, su_node_ptr) != NCSCC_RC_SUCCESS) {
				avd_su_readiness_state_set(su,back_red_state);
				avd_su_admin_state_set(su,back_admin_state);
				is_oper_successful = SA_FALSE;
			}
			break;
		}

		avd_sg_app_su_inst_func(avd_cb, su->sg_of_su);

		if ( is_oper_successful == SA_TRUE ) {
			if ( (su->sg_of_su->sg_fsm_state == AVD_SG_FSM_SG_REALIGN) ||
			     (su->sg_of_su->sg_fsm_state == AVD_SG_FSM_SU_OPER) ) {
				/* Store the callback parameters to send operation result later on. */
				su->pend_cbk.admin_oper = op_id;
				su->pend_cbk.invocation = invocation;
				return;
			} else {
				immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_OK);
				return;
			}
		} else {
			immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_ERR_FAILED_OPERATION);
			return;
		}

		break;

	case SA_AMF_ADMIN_LOCK_INSTANTIATION:

		/* For non-preinstantiable SU lock-inst is same as lock */
		if ( su->saAmfSUPreInstantiable == FALSE ) {
			immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_OK);
			return;
		}

		/* If still there are SIs assigned to this SU then it can't be uninstantiated */
		if ( su->list_of_susi != NULL ) {
			immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_ERR_NOT_READY);
			return;
		}

		m_AVD_GET_SU_NODE_PTR(cb,su,su_node_ptr);

		if ( ( su_node_ptr->node_state == AVD_AVND_STATE_PRESENT )   ||
		     ( su_node_ptr->node_state == AVD_AVND_STATE_NO_CONFIG ) ||
		     ( su_node_ptr->node_state == AVD_AVND_STATE_NCS_INIT ) ) {
			/* When the SU will terminate then prescence state change message will come
			   and so store the callback parameters to send response later on. */
			if (avd_snd_presence_msg(cb, su, TRUE) == NCSCC_RC_SUCCESS) {
				m_AVD_SET_SU_TERM(cb,su,TRUE);
				avd_su_oper_state_set(su,SA_AMF_OPERATIONAL_DISABLED);
				avd_su_admin_state_set(su,SA_AMF_ADMIN_LOCKED_INSTANTIATION);

				su->pend_cbk.admin_oper = op_id;
				su->pend_cbk.invocation = invocation;

				return;
			}
			immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_ERR_MESSAGE_ERROR);
			return;
		} else {
			avd_su_admin_state_set(su,SA_AMF_ADMIN_LOCKED_INSTANTIATION);
			immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_OK);
			m_AVD_SET_SU_TERM(cb,su,TRUE);
			return;
		}

		break;

	case SA_AMF_ADMIN_UNLOCK_INSTANTIATION:

		/* For non-preinstantiable SU unlock-inst will not lead to its inst until unlock. */
		if ( su->saAmfSUPreInstantiable == FALSE ) {
			immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_OK);
			return;
		}

		m_AVD_GET_SU_NODE_PTR(cb,su,su_node_ptr);

		if ( ( su_node_ptr->node_state == AVD_AVND_STATE_PRESENT )   ||
		     ( su_node_ptr->node_state == AVD_AVND_STATE_NO_CONFIG ) ||
		     ( su_node_ptr->node_state == AVD_AVND_STATE_NCS_INIT ) ) {
			/* When the SU will instantiate then prescence state change message will come
			   and so store the callback parameters to send response later on. */
			if (avd_snd_presence_msg(cb, su, FALSE) == NCSCC_RC_SUCCESS) {
				m_AVD_SET_SU_TERM(cb,su,FALSE);
				avd_su_admin_state_set(su,SA_AMF_ADMIN_LOCKED);

				su->pend_cbk.admin_oper = op_id;
				su->pend_cbk.invocation = invocation;

				return;
			}
			immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_ERR_MESSAGE_ERROR);
			return;
		} else {
			avd_su_admin_state_set(su,SA_AMF_ADMIN_LOCKED);
			immutil_saImmOiAdminOperationResult(immoi_handle, invocation, SA_AIS_OK);
			m_AVD_SET_SU_TERM(cb,su,FALSE);
			return;
		}

		break;
	default:
		avd_log(NCSFL_SEV_ERROR, "Unsupported admin op");
		rc = SA_AIS_ERR_INVALID_PARAM;
		break;
	}
}

static SaAisErrorT avd_su_rt_attr_cb(SaImmOiHandleT immOiHandle,
				     const SaNameT *objectName, const SaImmAttrNameT *attributeNames)
{
	AVD_SU *su = avd_su_find(objectName);
	SaImmAttrNameT attributeName;
	int i = 0;

	avd_trace("%s", objectName->value);
	assert(su != NULL);

	while ((attributeName = attributeNames[i++]) != NULL) {
		if (!strcmp("saAmfSUAssignedSIs", attributeName)) {
#if 0
			/*  TODO */
			SaUint32T saAmfSUAssignedSIs = su->saAmfSUNumCurrActiveSIs + su->saAmfSUNumCurrStandbySIs;
			(void)avd_saImmOiRtObjectUpdate(immOiHandle, objectName,
						       attributeName, SA_IMM_ATTR_SAUINT32T, &saAmfSUAssignedSIs);
#endif
		} else if (!strcmp("saAmfSUNumCurrActiveSIs", attributeName)) {
			(void)avd_saImmOiRtObjectUpdate(objectName, attributeName,
						       SA_IMM_ATTR_SAUINT32T, &su->saAmfSUNumCurrActiveSIs);
		} else if (!strcmp("saAmfSUNumCurrStandbySIs", attributeName)) {
			(void)avd_saImmOiRtObjectUpdate(objectName, attributeName,
						       SA_IMM_ATTR_SAUINT32T, &su->saAmfSUNumCurrStandbySIs);
		} else if (!strcmp("saAmfSURestartCount", attributeName)) {
			(void)avd_saImmOiRtObjectUpdate(objectName, attributeName,
						       SA_IMM_ATTR_SAUINT32T, &su->saAmfSURestartCount);
		} else
			assert(0);
	}

	return SA_AIS_OK;
}

/*****************************************************************************
 * Function: avd_su_ccb_completed_modify_hdlr
 * 
 * Purpose: This routine validates modify CCB operations on SaAmfSU objects.
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
static SaAisErrorT avd_su_ccb_completed_modify_hdlr(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_OK;
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;

	while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
		if (!strcmp(attr_mod->modAttr.attrName, "saAmfSUFailover")) {
			NCS_BOOL su_failover = *((SaTimeT *)attr_mod->modAttr.attrValues[0]);
			if (su_failover > SA_TRUE) {
				avd_log(NCSFL_SEV_ERROR, "Invalid saAmfSUFailover %u", su_failover);
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
		} else {
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}
	}

 done:
	return rc;
}

/*****************************************************************************
 * Function: avd_su_ccb_completed_delete_hdlr
 * 
 * Purpose: This routine validates delete CCB operations on SaAmfSU objects.
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
static SaAisErrorT avd_su_ccb_completed_delete_hdlr(CcbUtilOperationData_t *opdata)
{
	AVD_SU *avd_su = NULL;
	SaAisErrorT rc = SA_AIS_OK;

	avd_log(NCSFL_SEV_NOTICE, "'%s'", opdata->objectName.value);

	/* Find the su name. */
	avd_su = avd_su_find(&opdata->objectName);
	assert(avd_su != NULL);

	if ((avd_su->saAmfSUPresenceState != SA_AMF_PRESENCE_UNINSTANTIATED) &&
	    (avd_su->saAmfSUPresenceState != SA_AMF_PRESENCE_INSTANTIATION_FAILED)) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	if (avd_su->saAmfSUAdminState != SA_AMF_ADMIN_LOCKED) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	/* Check to see that no components or SUSI exists on this component */
	if ((avd_su->list_of_comp != NULL) || (avd_su->list_of_susi != AVD_SU_SI_REL_NULL)) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

 done:

	return rc;

}

/*****************************************************************************
 * Function: avd_su_ccb_completed_cb
 * 
 * Purpose: This routine handles all CCB operations on SaAmfSU objects.
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
static SaAisErrorT avd_su_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_SU *su;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if ((su = avd_su_create(&opdata->objectName, opdata->param.create.attrValues)) == NULL)
			goto done;

		if (avd_su_config_validate(&opdata->objectName, su) != 0) {
			avd_log(NCSFL_SEV_ERROR, "AMF Su '%s' validation error", su->name.value);
			avd_su_delete(su);
			goto done;
		}
		
		opdata->userData = su; /* Save for later use in apply */
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		rc = avd_su_ccb_completed_modify_hdlr(opdata);
		break;
	case CCBUTIL_DELETE:
		rc = avd_su_ccb_completed_delete_hdlr(opdata);
		break;
	default:
		assert(0);
		break;
	}
 done:
	return rc;
}

/*****************************************************************************
 * Function: avd_su_ccb_apply_create_hdlr
 * 
 * Purpose: This routine handles create operations on SaAmfSU objects.
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
static void avd_su_ccb_apply_create_hdlr(CcbUtilOperationData_t *opdata)
{
	avd_su_add_to_model(opdata->userData);
}

/*****************************************************************************
 * Function: avd_su_ccb_apply_modify_hdlr
 * 
 * Purpose: This routine handles modify operations on SaAmfSU objects.
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
static void avd_su_ccb_apply_modify_hdlr(struct CcbUtilOperationData *opdata)
{
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;
	AVD_SU *avd_su = NULL;

	avd_log(NCSFL_SEV_NOTICE, "'%s'", opdata->objectName.value);
	avd_su = avd_su_find(&opdata->objectName);
	while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
		if (!strcmp(attr_mod->modAttr.attrName, "saAmfSUFailover")) {
			NCS_BOOL su_failover = *((SaUint32T *)attr_mod->modAttr.attrValues[0]);
			avd_su->saAmfSUFailover = su_failover;
		}
	}
}

/*****************************************************************************
 * Function: avd_su_ccb_apply_delete_hdlr
 * 
 * Purpose: This routine handles delete operations on SaAmfSU objects.
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
static void avd_su_ccb_apply_delete_hdlr(struct CcbUtilOperationData *opdata)
{
	AVD_SU *avd_su = NULL;
	AVD_AVND *su_node_ptr = NULL;
	AVSV_PARAM_INFO param;

	avd_log(NCSFL_SEV_NOTICE, "'%s'", opdata->objectName.value);
	avd_su = avd_su_find(&opdata->objectName);

	m_AVD_GET_SU_NODE_PTR(avd_cb, avd_su, su_node_ptr);

	if ((su_node_ptr->node_state == AVD_AVND_STATE_PRESENT) ||
	    (su_node_ptr->node_state == AVD_AVND_STATE_NO_CONFIG) ||
	    (su_node_ptr->node_state == AVD_AVND_STATE_NCS_INIT)) {
		memset(((uns8 *)&param), '\0', sizeof(AVSV_PARAM_INFO));
		param.class_id = AVSV_SA_AMF_SU;
		param.act = AVSV_OBJ_OPR_DEL;
		param.name = avd_su->name;
		avd_snd_op_req_msg(avd_cb, su_node_ptr, &param);
	}

	/* remove the SU from both the SG list and the
	 * AVND list if present.
	 */
	m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

	avd_su_del_sg_list(avd_cb, avd_su);
	avd_su_del_avnd_list(avd_cb, avd_su);

	m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

	/* check point to the standby AVD that this
	 * record need to be deleted
	 */
	m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(avd_cb, avd_su, AVSV_CKPT_AVD_SU_CONFIG);

	avd_su_del_su_type_list(avd_cb, avd_su);
	avd_su_delete(avd_su);

}

/*****************************************************************************
 * Function: avd_su_ccb_apply_cb
 *
 * Purpose: This routine handles all CCB operations on SaAmfSU objects.
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
static void avd_su_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		avd_su_ccb_apply_create_hdlr(opdata);
		break;
	case CCBUTIL_MODIFY:
		avd_su_ccb_apply_modify_hdlr(opdata);
		break;
	case CCBUTIL_DELETE:
		avd_su_ccb_apply_delete_hdlr(opdata);
		break;
	default:
		assert(0);
		break;
	}
}

static AVD_SUTCOMP_TYPE *avd_sutcomptype_create(SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	uns32 rc;
	AVD_SUTCOMP_TYPE *sutcomptype;

	if ((sutcomptype = calloc(1, sizeof(AVD_SUTCOMP_TYPE))) == NULL) {
		avd_log(NCSFL_SEV_ERROR, "calloc failed");
		return NULL;
	}

	memcpy(sutcomptype->name.value, dn->value, dn->length);
	sutcomptype->name.length = dn->length;
	sutcomptype->tree_node.key_info = (uns8 *)&(sutcomptype->name);

	if (immutil_getAttr("saAmfSutMaxNumComponents", attributes, 0, &sutcomptype->saAmfSutMaxNumComponents) != SA_AIS_OK)
		sutcomptype->saAmfSutMaxNumComponents = -1; /* no limit */

	if (immutil_getAttr("saAmfSutMinNumComponents", attributes, 0, &sutcomptype->saAmfSutMinNumComponents) != SA_AIS_OK)
		sutcomptype->saAmfSutMinNumComponents = 1;

	rc = ncs_patricia_tree_add(&avd_sutcomptype_db, &sutcomptype->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);

	return sutcomptype;
}

static void avd_sutcomptype_delete(AVD_SUTCOMP_TYPE *sutcomptype)
{
	uns32 rc;

	rc = ncs_patricia_tree_del(&avd_sutcomptype_db, &sutcomptype->tree_node);
	assert(rc == NCSCC_RC_SUCCESS);
	free(sutcomptype);
}

AVD_SUTCOMP_TYPE *avd_sutcomptype_find(const SaNameT *dn)
{
    SaNameT tmp = {0};

    tmp.length = dn->length;
    memcpy(tmp.value, dn->value, tmp.length);

	return (AVD_SUTCOMP_TYPE *)ncs_patricia_tree_get(&avd_sutcomptype_db, (uns8 *)&tmp);
}

/**
 * Get configuration for all SaAmfSutCompType objects from IMM and
 * create AVD internal objects.
 * @param cb
 * 
 * @return int
 */
static SaAisErrorT avd_sutcomptype_config_get(SaNameT *sutype_name, AVD_SU_TYPE *sut)
{
	AVD_SUTCOMP_TYPE *sutcomptype;
	SaAisErrorT error;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfSutCompType";

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, sutype_name, SA_IMM_SUBTREE,
		SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
		NULL, &searchHandle);
	
	if (SA_AIS_OK != error) {
		avd_log(NCSFL_SEV_ERROR, "saImmOmSearchInitialize_2 failed: %u", error);
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {

		avd_log(NCSFL_SEV_NOTICE, "'%s' (%u)", dn.value, dn.length);

		if ((sutcomptype = avd_sutcomptype_create(&dn, attributes)) == NULL) {
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
 * Function: avd_sutcomptype_ccb_completed_cb
 * 
 * Purpose: Handles the CCB completed operation for the SaAmfSutCompType class.
 *
 * Input  : Ccb Util Oper Data
 *  
 * Returns: None.
 *  
 * NOTES  : None.
 *
 *
 **************************************************************************/
static SaAisErrorT avd_sutcomptype_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_SUTCOMP_TYPE *sutcomptype;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		if ((sutcomptype = avd_sutcomptype_create(&opdata->objectName,
			opdata->param.create.attrValues)) == NULL) {
			goto done;
		}

		opdata->userData = sutcomptype;	/* Save for later use in apply */
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		avd_log(NCSFL_SEV_ERROR, "Modification of SaAmfSUType not supported");
		break;
	case CCBUTIL_DELETE:
		sutcomptype = avd_sutcomptype_find(&opdata->objectName);
		if (sutcomptype->curr_num_components == 0) {
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

/*****************************************************************************
 * Function: avd_sutcomptype_ccb_apply_cb
 * 
 * Purpose: Handles the CCB apply operation for the SaAmfSutCompType class.
 * 
 * Input  : Ccb Util Oper Data 
 *  
 * Returns: None.
 *  
 * NOTES  : None.
 *
 *
 **************************************************************************/
static void avd_sutcomptype_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	AVD_SUTCOMP_TYPE *sutcomptype;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		break;
	case CCBUTIL_DELETE:
		sutcomptype = avd_sutcomptype_find(&opdata->objectName);
		avd_sutcomptype_delete(sutcomptype);
		break;
	default:
		assert(0);
		break;
	}
}

void avd_su_constructor(void)
{
	NCS_PATRICIA_PARAMS patricia_params;

	patricia_params.key_size = sizeof(SaNameT);
	assert(ncs_patricia_tree_init(&avd_su_db, &patricia_params) == NCSCC_RC_SUCCESS);
	assert(ncs_patricia_tree_init(&avd_sutype_db, &patricia_params) == NCSCC_RC_SUCCESS);
	assert(ncs_patricia_tree_init(&avd_sutcomptype_db, &patricia_params) == NCSCC_RC_SUCCESS);

	avd_class_impl_set("SaAmfSU", avd_su_rt_attr_cb,
		avd_su_admin_op_cb, avd_su_ccb_completed_cb, avd_su_ccb_apply_cb);
	avd_class_impl_set("SaAmfSUType", NULL, NULL, avd_sutype_ccb_completed_cb, avd_sutype_ccb_apply_cb);
	avd_class_impl_set("SaAmfSUBaseType", NULL, NULL, avd_imm_default_OK_completed_cb, NULL);
	avd_class_impl_set("SaAmfSutCompType", NULL, NULL,
		avd_sutcomptype_ccb_completed_cb, avd_sutcomptype_ccb_apply_cb);
}
