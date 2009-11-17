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
 *
 */

/*****************************************************************************

  DESCRIPTION:This module deals with the creation, accessing and deletion of
  the SU SI relationship list on the AVD.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include <avd_util.h>
#include <avd_susi.h>
#include <immutil.h>
#include <avd_imm.h>
#include <avd_dblog.h>
#include <avd_csi.h>

// TODO remove
extern void avd_chk_failover_shutdown_cxt(AVD_CL_CB *cb, AVD_AVND *avnd, SaBoolT is_ncs);

/**
 * Create an SaAmfSIAssignment runtime object in IMM.
 * @param ha_state
 * @param si_dn
 * @param su_dn
 */
static void avd_create_siassignment_in_imm(SaAmfHAStateT ha_state,
       const SaNameT *si_dn, const SaNameT *su_dn)
{
       SaAisErrorT rc;
       SaNameT dn;
       SaAmfReadinessStateT saAmfSISUHAReadinessState = SA_AMF_READINESS_IN_SERVICE;
       void *arr1[] = { &dn };
       void *arr2[] = { &ha_state };
       void *arr3[] = { &saAmfSISUHAReadinessState };
       const SaImmAttrValuesT_2 attr_safSISU = {
	       .attrName = "safSISU",
	       .attrValueType = SA_IMM_ATTR_SANAMET,
	       .attrValuesNumber = 1,
	       .attrValues = arr1
       };
       const SaImmAttrValuesT_2 attr_saAmfSISUHAState = {
	       .attrName = "saAmfSISUHAState",
	       .attrValueType = SA_IMM_ATTR_SAUINT32T,
	       .attrValuesNumber = 1,
	       .attrValues = arr2
       };
       const SaImmAttrValuesT_2 attr_saAmfSISUHAReadinessState = {
	       .attrName = "saAmfSISUHAReadinessState",
	       .attrValueType = SA_IMM_ATTR_SAUINT32T,
	       .attrValuesNumber = 1,
	       .attrValues = arr3
       };
       const SaImmAttrValuesT_2 *attrValues[] = {
	       &attr_safSISU,
	       &attr_saAmfSISUHAState,
	       &attr_saAmfSISUHAReadinessState,
	       NULL
       };

       if (avd_cb->avail_state_avd != SA_AMF_HA_ACTIVE)
	       return;

       avd_create_association_class_dn(su_dn, NULL, "safSISU", &dn);

       if ((rc = avd_saImmOiRtObjectCreate("SaAmfSIAssignment", si_dn, attrValues)) != SA_AIS_OK)
	   avd_log(NCSFL_SEV_ERROR, "rc=%u, '%s'", rc, dn.value);
}

/** Delete an SaAmfSIAssignment from IMM
 * 
 * @param si_dn
 * @param su_dn
 */
static void avd_delete_siassignment_from_imm(const SaNameT *si_dn, const SaNameT *su_dn)
{
       SaAisErrorT rc;
       SaNameT dn;

       if (avd_cb->avail_state_avd != SA_AMF_HA_ACTIVE)
	       return;

       avd_create_association_class_dn(su_dn, si_dn, "safSISU", &dn);

       if ((rc = avd_saImmOiRtObjectDelete(&dn)) != SA_AIS_OK)
	       avd_log(NCSFL_SEV_ERROR, "rc=%u, '%s'", rc, dn.value);
}

/**
 * Update an SaAmfSIAssignment runtime object in IMM.
 * @param ha_state
 * @param si_dn
 * @param su_dn
 */
void avd_susi_update(SaAmfHAStateT ha_state, const SaNameT *si_dn, const SaNameT *su_dn)
{
       SaAisErrorT rc;
       SaNameT dn;

       if (avd_cb->avail_state_avd != SA_AMF_HA_ACTIVE)
	       return;

       avd_create_association_class_dn(su_dn, si_dn, "safSISU", &dn);

       if ((rc = avd_saImmOiRtObjectUpdate(&dn,"saAmfSISUHAState", SA_IMM_ATTR_SAUINT32T, &ha_state)) != SA_AIS_OK)
	       avd_log(NCSFL_SEV_ERROR, "rc=%u, '%s'", rc, dn.value);
}

/*****************************************************************************
 * Function: avd_susi_create
 *
 * Purpose:  This function will create and add a AVD_SU_SI_REL structure to 
 * the list of susi in both si and su.
 *
 * Input: cb - the AVD control block
 *        su - The SU structure that needs to have the SU SI relation.
 *        si - The SI structure that needs to have the SU SI relation.
 *
 * Returns: The AVD_SU_SI_REL structure that was created and added
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SU_SI_REL *avd_susi_create(AVD_CL_CB *cb, AVD_SI *si, AVD_SU *su, SaAmfHAStateT state)
{
	AVD_SU_SI_REL *su_si, *p_su_si, *i_su_si;
	AVD_SU *curr_su = 0;
	AVD_SUS_PER_SI_RANK_INDX i_idx;
	AVD_SUS_PER_SI_RANK *su_rank_rec = 0, *i_su_rank_rec = 0;
	uns32 rank1, rank2;

	/* Allocate a new block structure now
	 */
	if ((su_si = calloc(1, sizeof(AVD_SU_SI_REL))) == NULL) {
		/* log an error */
		m_AVD_LOG_MEM_FAIL(AVD_SUSI_ALLOC_FAILED);
		return AVD_SU_SI_REL_NULL;
	}

	su_si->state = state;
	su_si->fsm = AVD_SU_SI_STATE_ABSENT;
	su_si->list_of_csicomp = NULL;
	su_si->si = si;
	su_si->su = su;

	/* 
	 * Add the susi rel rec to the ordered si-list
	 */

	/* determine if the su is ranked per si */
	memset((uns8 *)&i_idx, '\0', sizeof(i_idx));
	i_idx.si_name = si->name;
	i_idx.su_rank = 0;
	for (su_rank_rec = avd_sirankedsu_getnext(cb, i_idx);
	     su_rank_rec && (m_CMP_HORDER_SANAMET(su_rank_rec->indx.si_name, si->name) == 0);
	     su_rank_rec = avd_sirankedsu_getnext(cb, su_rank_rec->indx)) {
		curr_su = avd_su_find(&su_rank_rec->su_name);
		if (curr_su == su)
			break;
	}

	/* set the ranking flag */
	su_si->is_per_si = (curr_su == su) ? TRUE : FALSE;

	/* determine the insert position */
	for (p_su_si = AVD_SU_SI_REL_NULL, i_su_si = si->list_of_sisu;
	     i_su_si; p_su_si = i_su_si, i_su_si = i_su_si->si_next) {
		if (i_su_si->is_per_si == TRUE) {
			if (FALSE == su_si->is_per_si)
				continue;

			/* determine the su_rank rec for this rec */
			memset((uns8 *)&i_idx, '\0', sizeof(i_idx));
			i_idx.si_name = si->name;
			i_idx.su_rank = 0;
			for (i_su_rank_rec = avd_sirankedsu_getnext(cb, i_idx);
			     i_su_rank_rec
			     && (m_CMP_HORDER_SANAMET(i_su_rank_rec->indx.si_name, si->name) == 0);
			     i_su_rank_rec = avd_sirankedsu_getnext(cb, i_su_rank_rec->indx)) {
				curr_su = avd_su_find(&i_su_rank_rec->su_name);
				if (curr_su == i_su_si->su)
					break;
			}

			assert(i_su_rank_rec);

			rank1 = su_rank_rec->indx.su_rank;
			rank2 = i_su_rank_rec->indx.su_rank;
			if (rank1 <= rank2)
				break;
		} else {
			if (TRUE == su_si->is_per_si)
				break;

			if (su->saAmfSURank <= i_su_si->su->saAmfSURank)
				break;
		}
	}			/* for */

	/* now insert the susi rel at the correct position */
	if (p_su_si) {
		su_si->si_next = p_su_si->si_next;
		p_su_si->si_next = su_si;
	} else {
		su_si->si_next = si->list_of_sisu;
		si->list_of_sisu = su_si;
	}

	/* keep the list in su inascending order */
	if (su->list_of_susi == AVD_SU_SI_REL_NULL) {
		su->list_of_susi = su_si;
		su_si->su_next = AVD_SU_SI_REL_NULL;
		goto done;
	}

	p_su_si = AVD_SU_SI_REL_NULL;
	i_su_si = su->list_of_susi;
	while ((i_su_si != AVD_SU_SI_REL_NULL) &&
	       (m_CMP_HORDER_SANAMET(i_su_si->si->name, su_si->si->name) < 0)) {
		p_su_si = i_su_si;
		i_su_si = i_su_si->su_next;
	}

	if (p_su_si == AVD_SU_SI_REL_NULL) {
		su_si->su_next = su->list_of_susi;
		su->list_of_susi = su_si;
	} else {
		su_si->su_next = p_su_si->su_next;
		p_su_si->su_next = su_si;
	}

done:
	if (su_si != NULL)
		avd_create_siassignment_in_imm(state, &si->name, &su->name);

	return su_si;
}

/*****************************************************************************
 * Function: avd_su_susi_find
 *
 * Purpose:  This function will find a AVD_SU_SI_REL structure from the
 * list of susis in a su.
 *
 * Input: cb - the AVD control block
 *        su - The pointer to the SU . 
 *        si_name - The SI name of the SU SI relation.
 *        
 * Returns: The AVD_SU_SI_REL structure that was found.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SU_SI_REL *avd_su_susi_find(AVD_CL_CB *cb, AVD_SU *su, const SaNameT *si_name)
{
	AVD_SU_SI_REL *su_si;
	SaNameT lsi_name;

	su_si = su->list_of_susi;

	memset((char *)&lsi_name, '\0', sizeof(SaNameT));
	memcpy(lsi_name.value, si_name->value, si_name->length);
	lsi_name.length = si_name->length;

	while ((su_si != AVD_SU_SI_REL_NULL) && (m_CMP_HORDER_SANAMET(su_si->si->name, lsi_name) < 0)) {
		su_si = su_si->su_next;
	}

	if ((su_si != AVD_SU_SI_REL_NULL) && (m_CMP_HORDER_SANAMET(su_si->si->name, lsi_name) == 0)) {
		return su_si;
	}

	return AVD_SU_SI_REL_NULL;
}

/*****************************************************************************
 * Function: avd_susi_find
 *
 * Purpose:  This function will find a AVD_SU_SI_REL structure from the
 * list of susis in a su.
 *
 * Input: cb - the AVD control block
 *        su_name - The SU name of the SU SI relation. 
 *        si_name - The SI name of the SU SI relation.
 *        
 * Returns: The AVD_SU_SI_REL structure that was found.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SU_SI_REL *avd_susi_find(AVD_CL_CB *cb, const SaNameT *su_name, const SaNameT *si_name)
{
	AVD_SU *su;

	if ((su = avd_su_find(su_name)) == NULL)
		return AVD_SU_SI_REL_NULL;

	return avd_su_susi_find(cb, su, si_name);
}

/*****************************************************************************
 * Function: avd_susi_find_next
 *
 * Purpose:  This function will find the next AVD_SU_SI_REL structure from the
 * list of susis in a su. If NULL, it will find the first SUSI for the next
 * SU in the tree.
 *
 * Input: cb - the AVD control block
 *        su_name - The SU name of the SU SI relation. 
 *        si_name - The SI name of the SU SI relation.
 *        
 * Returns: The AVD_SU_SI_REL structure that was found.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SU_SI_REL *avd_susi_find_next(AVD_CL_CB *cb, SaNameT su_name, SaNameT si_name)
{
	AVD_SU *su;
	AVD_SU_SI_REL *su_si = AVD_SU_SI_REL_NULL;
	SaNameT lsu_name, lsi_name;

	/* check if exact match of SU is found so that the next SU SI
	 * in the list of SU can be found. If not select the next SUs
	 * first SU SI relation
	 */
	if (su_name.length != 0) {
		su = avd_su_find(&su_name);
		if (su == NULL)
			su_si = AVD_SU_SI_REL_NULL;
		else
			su_si = su->list_of_susi;
	}

	memset((char *)&lsi_name, '\0', sizeof(SaNameT));
	memcpy(lsi_name.value, si_name.value, si_name.length);
	lsi_name.length = si_name.length;
	
	while ((su_si != AVD_SU_SI_REL_NULL) && (m_CMP_HORDER_SANAMET(su_si->si->name, lsi_name) <= 0)) {
		su_si = su_si->su_next;
	}

	if (su_si != AVD_SU_SI_REL_NULL) {
		return su_si;
	}

	/* Find the the first SU SI relation in the next SU with
	 * a SU SI relation.
	 */
	lsu_name = su_name;

	while ((su = avd_su_getnext(&lsu_name)) != NULL) {
		if (su->list_of_susi != AVD_SU_SI_REL_NULL)
			break;

		lsu_name = su->name;
	}

	/* The given element didn't have a exact match but an element with
	 * a greater SI name was found in the list
	 */

	if (su == NULL)
		return AVD_SU_SI_REL_NULL;
	else
		return su->list_of_susi;

	return su_si;
}

/*****************************************************************************
 * Function: avd_susi_delete
 *
 * Purpose:  This function will delete and free AVD_SU_SI_REL structure both
 * the su and si list of susi structures.
 *
 * Input: cb - the AVD control block
 *        susi - The SU SI relation structure that needs to be deleted. 
 *
 * Returns:  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE 
 *
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_susi_delete(AVD_CL_CB *cb, AVD_SU_SI_REL *susi, NCS_BOOL ckpt)
{
	AVD_SU_SI_REL *p_su_si, *p_si_su, *i_su_si;
	AVD_AVND *avnd = NULL;
	SaBoolT is_ncs = susi->su->sg_of_su->sg_ncs_spec;

	m_AVD_GET_SU_NODE_PTR(cb, susi->su, avnd);

	/* delete the comp-csi rels */
	avd_compcsi_delete(cb, susi, ckpt);

	/* check the SU list to get the prev pointer */
	i_su_si = susi->su->list_of_susi;
	p_su_si = AVD_SU_SI_REL_NULL;
	while ((i_su_si != AVD_SU_SI_REL_NULL) && (i_su_si != susi)) {
		p_su_si = i_su_si;
		i_su_si = i_su_si->su_next;
	}

	if (i_su_si == AVD_SU_SI_REL_NULL) {
		/* problem it is mssing to delete */
		/* log error */
		return NCSCC_RC_FAILURE;
	}

	/* check the SI list to get the prev pointer */
	i_su_si = susi->si->list_of_sisu;
	p_si_su = AVD_SU_SI_REL_NULL;

	while ((i_su_si != AVD_SU_SI_REL_NULL) && (i_su_si != susi)) {
		p_si_su = i_su_si;
		i_su_si = i_su_si->si_next;
	}

	if (i_su_si == AVD_SU_SI_REL_NULL) {
		/* problem it is mssing to delete */
		/* log error */
		return NCSCC_RC_FAILURE;
	}

	/* now delete it from the SU list */
	if (p_su_si == AVD_SU_SI_REL_NULL) {
		susi->su->list_of_susi = susi->su_next;
		susi->su_next = AVD_SU_SI_REL_NULL;
	} else {
		p_su_si->su_next = susi->su_next;
		susi->su_next = AVD_SU_SI_REL_NULL;
	}

	/* now delete it from the SI list */
	if (p_si_su == AVD_SU_SI_REL_NULL) {
		susi->si->list_of_sisu = susi->si_next;
		susi->si_next = AVD_SU_SI_REL_NULL;
	} else {
		p_si_su->si_next = susi->si_next;
		susi->si_next = AVD_SU_SI_REL_NULL;
	}

	if (!ckpt) {
		/* update the si counters */
		if (SA_AMF_HA_STANDBY == susi->state) {
			m_AVD_SI_DEC_STDBY_CURR_SU(susi->si);
		} else {
			m_AVD_SI_DEC_ACTV_CURR_SU(susi->si);
		}
	}

	avd_delete_siassignment_from_imm(&susi->si->name, &susi->su->name);

	susi->si = AVD_SI_NULL;
	susi->su = NULL;

	/* call the func to check on the context for deletion */
	if (!ckpt) {
		avd_chk_failover_shutdown_cxt(cb, avnd, is_ncs);
	}

	free(susi);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_sirankedsu_create
 *
 * Purpose:  This function will create and add a AVD_SUS_PER_SI_RANK structure to the
 * tree if an element with given index value doesn't exist in the tree.
 *
 * Input: cb - the AVD control block
 *        indx - The key info  needs to add a element in the petricia tree 
 *
 * Returns: The pointer to AVD_SUS_PER_SI_RANK structure allocated and added.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static AVD_SUS_PER_SI_RANK *avd_sirankedsu_create(AVD_CL_CB *cb, AVD_SUS_PER_SI_RANK_INDX indx)
{
	AVD_SUS_PER_SI_RANK *rank_elt = AVD_SU_PER_SI_RANK_NULL;

	/* Allocate a new block structure now
	 */
	if ((rank_elt = calloc(1, sizeof(AVD_SUS_PER_SI_RANK))) == NULL) {
		/* log an error */
		m_AVD_LOG_MEM_FAIL(AVD_SU_PER_SI_RANK_ALLOC_FAILED);
		return AVD_SU_PER_SI_RANK_NULL;
	}

	rank_elt->indx.si_name.length = indx.si_name.length;
	memcpy(rank_elt->indx.si_name.value, indx.si_name.value,
		rank_elt->indx.si_name.length);

	rank_elt->indx.su_rank = indx.su_rank;

	rank_elt->tree_node.key_info = (uns8 *)(&rank_elt->indx);
	rank_elt->tree_node.bit = 0;
	rank_elt->tree_node.left = NCS_PATRICIA_NODE_NULL;
	rank_elt->tree_node.right = NCS_PATRICIA_NODE_NULL;

	if (ncs_patricia_tree_add(&cb->su_per_si_rank_anchor, &rank_elt->tree_node)
	    != NCSCC_RC_SUCCESS) {
		/* log an error */
		free(rank_elt);
		return AVD_SU_PER_SI_RANK_NULL;
	}

	return rank_elt;

}

/*****************************************************************************
 * Function: avd_sirankedsu_find
 *
 * Purpose:  This function will find a AVD_SUS_PER_SI_RANK structure in the
 * tree with indx value as key.
 *
 * Input: cb - the AVD control block
 *        indx - The key.
 *
 * Returns: The pointer to AVD_SUS_PER_SI_RANK structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static AVD_SUS_PER_SI_RANK *avd_sirankedsu_find(AVD_CL_CB *cb, AVD_SUS_PER_SI_RANK_INDX indx)
{
	AVD_SUS_PER_SI_RANK *rank_elt = AVD_SU_PER_SI_RANK_NULL;
	AVD_SUS_PER_SI_RANK_INDX rank_indx;

	memset(&rank_indx, '\0', sizeof(AVD_SUS_PER_SI_RANK_INDX));
	rank_indx.si_name.length = indx.si_name.length;
	memcpy(rank_indx.si_name.value, indx.si_name.value, indx.si_name.length);
	rank_indx.su_rank = indx.su_rank;

	rank_elt = (AVD_SUS_PER_SI_RANK *)ncs_patricia_tree_get(&cb->su_per_si_rank_anchor, (uns8 *)&rank_indx);

	return rank_elt;
}

/*****************************************************************************
 * Function: avd_sirankedsu_getnext
 *
 * Purpose:  This function will find the next AVD_SUS_PER_SI_RANK structure in the
 * tree whose key value is next of the given key value.
 *
 * Input: cb - the AVD control block
 *        indx - The key value.
 *
 * Returns: The pointer to AVD_SUS_PER_SI_RANK structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SUS_PER_SI_RANK *avd_sirankedsu_getnext(AVD_CL_CB *cb, AVD_SUS_PER_SI_RANK_INDX indx)
{
	AVD_SUS_PER_SI_RANK *rank_elt = AVD_SU_PER_SI_RANK_NULL;
	AVD_SUS_PER_SI_RANK_INDX rank_indx;

	memset(&rank_indx, '\0', sizeof(AVD_SUS_PER_SI_RANK_INDX));
	rank_indx.si_name.length = indx.si_name.length;
	memcpy(rank_indx.si_name.value, indx.si_name.value, indx.si_name.length);
	rank_indx.su_rank = indx.su_rank;

	rank_elt = (AVD_SUS_PER_SI_RANK *)ncs_patricia_tree_getnext(&cb->su_per_si_rank_anchor, (uns8 *)&rank_indx);

	return rank_elt;
}

/*****************************************************************************
 * Function: avd_sirankedsu_getnext_valid
 *
 * Purpose:  This function will find the next AVD_SUS_PER_SI_RANK structure in the
 * tree whose key value is next of the given key value. It also verifies if the 
 * the si & su belong to the same sg.
 *
 * Input: cb - the AVD control block
 *        indx - The key value.
 *       o_su - output field indicating the pointer to the pointer of
 *                the SU in the SISU rank list. Filled when return value is not
 *              NULL.
 *
 * Returns: The pointer to AVD_SUS_PER_SI_RANK structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SUS_PER_SI_RANK *avd_sirankedsu_getnext_valid(AVD_CL_CB *cb,
	AVD_SUS_PER_SI_RANK_INDX indx, AVD_SU **o_su)
{
	AVD_SUS_PER_SI_RANK *rank_elt = AVD_SU_PER_SI_RANK_NULL;
	AVD_SUS_PER_SI_RANK_INDX rank_indx;
	AVD_SI *si = AVD_SI_NULL;
	AVD_SU *su = NULL;

	memset(&rank_indx, '\0', sizeof(AVD_SUS_PER_SI_RANK_INDX));
	rank_indx.si_name.length = indx.si_name.length;
	memcpy(rank_indx.si_name.value, indx.si_name.value, indx.si_name.length);
	rank_indx.su_rank = indx.su_rank;

	rank_elt = (AVD_SUS_PER_SI_RANK *)ncs_patricia_tree_getnext(&cb->su_per_si_rank_anchor, (uns8 *)&rank_indx);

	if (rank_elt == AVD_SU_PER_SI_RANK_NULL) {
		/*  return NULL */
		return rank_elt;
	}

	/* get the su & si */
	su = avd_su_find(&rank_elt->su_name);
	si = avd_si_find(&indx.si_name);

	/* validate this entry */
	if ((si == AVD_SI_NULL) || (su == NULL) || (si->sg_of_si != su->sg_of_su))
		return avd_sirankedsu_getnext_valid(cb, rank_elt->indx, o_su);

	*o_su = su;
	return rank_elt;
}

/*****************************************************************************
 * Function: avd_sirankedsu_delete
 *
 * Purpose:  This function will delete and free AVD_SUS_PER_SI_RANK structure from 
 * the tree.
 *
 * Input: cb - the AVD control block
 *        rank_elt - The AVD_SUS_PER_SI_RANK structure that needs to be deleted.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE  
 *
 * NOTES:
 *
 * 
 **************************************************************************/
static uns32 avd_sirankedsu_delete(AVD_CL_CB *cb, AVD_SUS_PER_SI_RANK *rank_elt)
{
	if (rank_elt == AVD_SU_PER_SI_RANK_NULL)
		return NCSCC_RC_FAILURE;

	if (ncs_patricia_tree_del(&cb->su_per_si_rank_anchor, &rank_elt->tree_node)
	    != NCSCC_RC_SUCCESS) {
		/* log error */
		return NCSCC_RC_FAILURE;
	}

	free(rank_elt);
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_sirankedsu_ccb_apply_create_hdlr
 *
 * Purpose: This routine handles create operations on SaAmfSIRankedSU objects.
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
static SaAisErrorT avd_sirankedsu_ccb_apply_create_hdlr(
	struct CcbUtilOperationData *opdata)
{
	SaAisErrorT rc = SA_AIS_OK;
	AVD_SI *avd_si = NULL;
	AVD_SUS_PER_SI_RANK *avd_sus_per_si_rank = NULL;
	SaNameT su_name;
	AVD_SUS_PER_SI_RANK_INDX indx;
	SaNameT rdn_attr_value;
	NCS_BOOL found = FALSE;
	const SaImmAttrValuesT_2 *attribute;
	int i = 0;

	/* Find the si name. */
	avd_si = avd_si_find(opdata->param.create.parentName);

	if (avd_si == NULL) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	/* Find the rdn attribute value of the object */
	attribute = opdata->param.create.attrValues[i++];
	while (attribute != NULL) {
		void *value;

		value = attribute->attrValues[0];

		if (!strcmp(attribute->attrName, "safRankedSu")) {
			rdn_attr_value = *((SaNameT *)value);
			avsv_sanamet_init_from_association_dn(&rdn_attr_value, &su_name, "safSu", "safSi");
			found = TRUE;
		}
		attribute = opdata->param.create.attrValues[i++];
	}

	if (found == FALSE) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	/* Fill other atrributes values */
	i = 0;
	attribute = opdata->param.create.attrValues[i++];
	while (attribute != NULL) {
		void *value;

		value = attribute->attrValues[0];

		if (!strcmp(attribute->attrName, "safRankedSu")) {
			/*  Just for the sake of avoiding else. */
		} else if (!strcmp(attribute->attrName, "saAmfRank")) {
			uns32 rank;
			rank = *((SaUint32T *)value);

			/* Find the avd_sus_per_si_rank name. */
			memset(&indx, '\0', sizeof(AVD_SUS_PER_SI_RANK_INDX));
			indx.si_name = avd_si->name;
			indx.su_rank = rank;

			avd_sus_per_si_rank = avd_sirankedsu_find(avd_cb, indx);

			if (avd_sus_per_si_rank != NULL) {
				rc = SA_AIS_ERR_INVALID_PARAM;
				goto done;
			}

			m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

			avd_sus_per_si_rank = avd_sirankedsu_create(avd_cb, indx);

			m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

			if (avd_sus_per_si_rank == NULL) {
				/* Invalid instance object */
				rc = SA_AIS_ERR_INVALID_PARAM;
				goto done;
			}

			avd_sus_per_si_rank->su_name = su_name;
			avd_sus_per_si_rank->su_name.length = su_name.length;

		} else {
			avd_log(NCSFL_SEV_ERROR, "Amf avd_sus_per_si_rank Object Create:Invalid attribute");
			rc = SA_AIS_ERR_INVALID_PARAM;
			goto done;
		}

		attribute = opdata->param.create.attrValues[i++];
	}

	if (NULL == avd_sus_per_si_rank) {
		/* Mandatory params are not filled. */
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	/* Add sus_per_si_rank to si */
	avd_sus_per_si_rank->sus_per_si_rank_on_si = avd_si;
	avd_sus_per_si_rank->sus_per_si_rank_list_si_next =
	    avd_sus_per_si_rank->sus_per_si_rank_on_si->list_of_sus_per_si_rank;
	avd_sus_per_si_rank->sus_per_si_rank_on_si->list_of_sus_per_si_rank = avd_sus_per_si_rank;

 done:
	return rc;
}

/*****************************************************************************
 * Function: avd_sirankedsu_del_si_list
 *
 * Purpose:  This function will del the given sus_per_si_rank from si list.
 *
 * Input: cb - the AVD control block
 *        sus_per_si_rank - The sus_per_si_rank pointer
 *
 * Returns: None.
 *
 * NOTES: None
 *
 *
 **************************************************************************/
static void avd_sirankedsu_del_si_list(AVD_CL_CB *cb, AVD_SUS_PER_SI_RANK *sus_per_si_rank)
{
	AVD_SUS_PER_SI_RANK *i_sus_per_si_rank = NULL;
	AVD_SUS_PER_SI_RANK *prev_sus_per_si_rank = NULL;

	if (sus_per_si_rank->sus_per_si_rank_on_si != NULL) {
		i_sus_per_si_rank = sus_per_si_rank->sus_per_si_rank_on_si->list_of_sus_per_si_rank;

		while ((i_sus_per_si_rank != NULL) && (i_sus_per_si_rank != sus_per_si_rank)) {
			prev_sus_per_si_rank = i_sus_per_si_rank;
			i_sus_per_si_rank = i_sus_per_si_rank->sus_per_si_rank_list_si_next;
		}

		if (i_sus_per_si_rank != sus_per_si_rank) {
			/* Log a fatal error */
		} else {
			if (prev_sus_per_si_rank == NULL) {
				sus_per_si_rank->sus_per_si_rank_on_si->list_of_sus_per_si_rank =
				    sus_per_si_rank->sus_per_si_rank_list_si_next;
			} else {
				prev_sus_per_si_rank->sus_per_si_rank_list_si_next =
				    sus_per_si_rank->sus_per_si_rank_list_si_next;
			}
		}

		sus_per_si_rank->sus_per_si_rank_list_si_next = NULL;
		sus_per_si_rank->sus_per_si_rank_on_si = NULL;
	}

	return;
}

static void avd_susi_namet_init(const SaNameT *object_name, SaNameT *su_name, SaNameT *si_name)
{
	char *p = NULL;

	if (su_name) {
		SaNameT temp_name;
		int i;

		/* Take out Su Name. safRankedSu=safSu=SuName\,safSg=SgName\,
		safApp=AppName,safSi=SiName,safApp=AppName */
		temp_name = *object_name;
		p = strstr((char *)temp_name.value, "safSi=");
		*(--p) = '\0';	/* null terminate at comma before si name */

		/* Skip past the RDN tag */
		p = strchr((char *)temp_name.value, '=') + 1;
		assert(p);
		memset(su_name->value, 0, sizeof(SaNameT));
		/* Copy the RDN value which is a DN with escaped commas */
		i = 0;
		while (*p) {
			if (*p != '\\')
				su_name->value[i++] = *p;

			p++;
		}
		/* i Points just after SU name ends, so it will give the name length
		as it starts with zero. */
		su_name->length = i;
	}

	if (si_name) {
		p = strstr((char *)object_name->value, "safSi=");
		si_name->length = strlen(p);
		memcpy(si_name->value, p, si_name->length);
	}
}

static SaAisErrorT avd_sirankedsu_ccb_apply_delete_hdlr(struct CcbUtilOperationData *opdata)
{
	SaAisErrorT rc = SA_AIS_OK;
	SaNameT object_name;
	SaNameT su_name;
	SaNameT si_name;
	AVD_SUS_PER_SI_RANK_INDX indx;
	AVD_SU *su = NULL, *curr_su = NULL;
	AVD_SI *si = NULL;
	AVD_SUS_PER_SI_RANK *su_rank_rec = 0;
	NCS_BOOL found = FALSE;

	object_name = *opdata->param.deleteOp.objectName;
	avd_susi_namet_init(opdata->param.deleteOp.objectName, &su_name, &si_name);

	/* Find the su name. */
	su = avd_su_find(&su_name);

	if (su != NULL) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	/* determine if the su is ranked per si */
	memset((uns8 *)&indx, '\0', sizeof(indx));
	indx.si_name = si_name;
	indx.su_rank = 0;
	for (su_rank_rec = avd_sirankedsu_getnext(avd_cb, indx);
	     su_rank_rec && (m_CMP_HORDER_SANAMET(su_rank_rec->indx.si_name,
						  si_name) == 0);
	     su_rank_rec = avd_sirankedsu_getnext(avd_cb, su_rank_rec->indx)) {
		curr_su = avd_su_find(&su_rank_rec->su_name);
		if (curr_su == su) {
			found = TRUE;
			break;
		}
	}

	if (FALSE == found) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	/* Find the si name. */
	si = avd_si_find(&si_name);

	if (si == NULL) {
		rc = SA_AIS_ERR_INVALID_PARAM;
		goto done;
	}

	if (su != NULL) {
		if (su->saAmfSUAdminState != SA_AMF_ADMIN_LOCKED) {
			return NCSCC_RC_INV_VAL;
		}
	}
	if (si != AVD_SI_NULL) {
		if (si->saAmfSIAdminState != SA_AMF_ADMIN_LOCKED) {
			return NCSCC_RC_INV_VAL;
		}
	}

	/* delete and free the structure */
	m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);
	avd_sirankedsu_del_si_list(avd_cb, su_rank_rec);
	avd_sirankedsu_delete(avd_cb, su_rank_rec);
	m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

 done:
	return rc;
}

static void avd_sirankedsu_ccb_apply_modify_hdlr(CcbUtilOperationData_t *opdata)
{
	AVD_SUS_PER_SI_RANK_INDX indx;
	AVD_SUS_PER_SI_RANK *sirankedsu;
	const SaImmAttrModificationT_2 *attr_mod;
	int i = 0;

	indx.su_rank = 0;
	avsv_sanamet_init(&opdata->objectName, &indx.si_name, "safSI");
	sirankedsu = avd_sirankedsu_find(avd_cb, indx);

	while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
		if (!strcmp(attr_mod->modAttr.attrName, "saAmfSIPrefActiveAssignments")) {
			sirankedsu->indx.su_rank = *((SaUint32T *)attr_mod->modAttr.attrValues[0]);
		}
	}
}

static void avd_sirankedsu_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		avd_sirankedsu_ccb_apply_create_hdlr(opdata);
		break;
	case CCBUTIL_DELETE:
		avd_sirankedsu_ccb_apply_delete_hdlr(opdata);
		break;
	case CCBUTIL_MODIFY:
		avd_sirankedsu_ccb_apply_modify_hdlr(opdata);
		break;
	default:
		assert(0);
		break;
	}
}

static SaAisErrorT avd_sirankedsu_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;

	avd_log(NCSFL_SEV_NOTICE, "CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_DELETE:
		rc = SA_AIS_OK;
		break;
	default:
		assert(0);
		break;
	}

	return rc;
}

SaAisErrorT avd_sirankedsu_config_get(SaNameT *si_name, AVD_SI *si)
{
	SaAisErrorT error = SA_AIS_ERR_FAILED_OPERATION;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfSIRankedSU";
	AVD_SUS_PER_SI_RANK *avd_sus_per_si_rank;
	AVD_SUS_PER_SI_RANK_INDX indx;
	SaNameT dn;
	SaNameT su_name;

	searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	if (immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, si_name, SA_IMM_SUBTREE,
	      SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
	      NULL, &searchHandle) != SA_AIS_OK) {

		avd_log(NCSFL_SEV_ERROR, "No objects found (1)");
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {

		avd_log(NCSFL_SEV_NOTICE, "'%s'", dn.value);

		indx.si_name = *si_name;
		if (immutil_getAttr("saAmfRank", attributes, 0, &indx.su_rank) != SA_AIS_OK) {
			avd_log(NCSFL_SEV_ERROR, "Get saAmfRank FAILED for '%s'", dn.value);
			goto done1;
		}

		avd_susi_namet_init(&dn, &su_name, NULL);
		if ((avd_sus_per_si_rank = avd_sirankedsu_create(avd_cb, indx)) == NULL)
			goto done2;
		avd_sus_per_si_rank->su_name = su_name;

		avd_sus_per_si_rank->sus_per_si_rank_on_si = si;
		avd_sus_per_si_rank->sus_per_si_rank_list_si_next = si->list_of_sus_per_si_rank;
		si->list_of_sus_per_si_rank = avd_sus_per_si_rank;
	}

	error = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:

	return error;
}

__attribute__ ((constructor)) static void avd_sirankedsu_constructor(void)
{
	avd_class_impl_set("SaAmfSIRankedSU", NULL, NULL,
	       avd_sirankedsu_ccb_completed_cb, avd_sirankedsu_ccb_apply_cb);
}

