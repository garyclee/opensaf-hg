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
..............................................................................

..............................................................................

  DESCRIPTION:This module deals with the creation, accessing and deletion of
  the SU SI relationship list on the AVD. It also deals with
  all the MIB operations like set,get,getnext etc related to the 
  service unit service instance relationship table.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_susi_struc_crt - creates and adds SUSI structure to the list of
                       susi in both si and su
  avd_susi_struc_find - Finds SUSI structure from the list in SU.
  avd_susi_struc_find_next - Finds the next SUSI structure in the AVD.  
  avd_susi_struc_del - deletes and frees SUSI structure from the list of
                       susi in both si and su.
  saamfsusitableentry_get - This function is one of the get processing
                        routines for objects in SA_AMF_S_U_S_I_TABLE_ENTRY_ID table.
  saamfsusitableentry_extract - This function is one of the get processing
                            function for objects in SA_AMF_S_U_S_I_TABLE_ENTRY_ID
                            table.
  saamfsusitableentry_set - This function is the set processing for objects in
                         SA_AMF_S_U_S_I_TABLE_ENTRY_ID table.
  saamfsusitableentry_next - This function is the next processing for objects in
                         SA_AMF_S_U_S_I_TABLE_ENTRY_ID table.
  saamfsusitableentry_setrow - This function is the setrow processing for
                           objects in SA_AMF_S_U_S_I_TABLE_ENTRY_ID table.
  saamfsusitableentry_setrow - This function is the setrow processing for
                           objects in SA_AMF_S_U_S_I_TABLE_ENTRY_ID table.

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"

/*****************************************************************************
 * Function: avd_susi_struc_crt
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

AVD_SU_SI_REL *avd_susi_struc_crt(AVD_CL_CB *cb, AVD_SI *si, AVD_SU *su)
{
	AVD_SU_SI_REL *su_si, *p_su_si, *i_su_si;
	AVD_SU *curr_su = 0;
	AVD_SUS_PER_SI_RANK_INDX i_idx;
	AVD_SUS_PER_SI_RANK *su_rank_rec = 0, *i_su_rank_rec = 0;
	uns32 rank1, rank2;

	/* Allocate a new block structure now
	 */
	if ((su_si = m_MMGR_ALLOC_AVD_SU_SI_REL) == AVD_SU_SI_REL_NULL) {
		/* log an error */
		m_AVD_LOG_MEM_FAIL(AVD_SUSI_ALLOC_FAILED);
		return AVD_SU_SI_REL_NULL;
	}

	memset((char *)su_si, '\0', sizeof(AVD_SU_SI_REL));

	su_si->state = SA_AMF_HA_STANDBY;
	su_si->fsm = AVD_SU_SI_STATE_ABSENT;
	su_si->list_of_csicomp = AVD_COMP_CSI_REL_NULL;
	su_si->si = si;
	su_si->su = su;

	/* 
	 * Add the susi rel rec to the ordered si-list
	 */

	/* determine if the su is ranked per si */
	memset((uns8 *)&i_idx, '\0', sizeof(i_idx));
	i_idx.si_name_net = si->name_net;
	i_idx.su_rank_net = 0;
	for (su_rank_rec = avd_sus_per_si_rank_struc_find_next(cb, i_idx);
	     su_rank_rec && (m_CMP_NORDER_SANAMET(su_rank_rec->indx.si_name_net, si->name_net) == 0);
	     su_rank_rec = avd_sus_per_si_rank_struc_find_next(cb, su_rank_rec->indx)) {
		curr_su = avd_su_struc_find(cb, su_rank_rec->su_name, TRUE);
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
			i_idx.si_name_net = si->name_net;
			i_idx.su_rank_net = 0;
			for (i_su_rank_rec = avd_sus_per_si_rank_struc_find_next(cb, i_idx);
			     i_su_rank_rec
			     && (m_CMP_NORDER_SANAMET(i_su_rank_rec->indx.si_name_net, si->name_net) == 0);
			     i_su_rank_rec = avd_sus_per_si_rank_struc_find_next(cb, i_su_rank_rec->indx)) {
				curr_su = avd_su_struc_find(cb, i_su_rank_rec->su_name, TRUE);
				if (curr_su == i_su_si->su)
					break;
			}

			m_AVSV_ASSERT(i_su_rank_rec);

			rank1 = su_rank_rec->indx.su_rank_net;
			rank2 = i_su_rank_rec->indx.su_rank_net;
			rank1 = m_NCS_OS_NTOHL(rank1);
			rank2 = m_NCS_OS_NTOHL(rank2);
			if (rank1 <= rank2)
				break;
		} else {
			if (TRUE == su_si->is_per_si)
				break;

			if (su->rank <= i_su_si->su->rank)
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
		return su_si;
	}

	p_su_si = AVD_SU_SI_REL_NULL;
	i_su_si = su->list_of_susi;
	while ((i_su_si != AVD_SU_SI_REL_NULL) &&
	       (m_CMP_NORDER_SANAMET(i_su_si->si->name_net, su_si->si->name_net) < 0)) {
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

	return su_si;
}

/*****************************************************************************
 * Function: avd_su_susi_struc_find
 *
 * Purpose:  This function will find a AVD_SU_SI_REL structure from the
 * list of susis in a su.
 *
 * Input: cb - the AVD control block
 *        su - The pointer to the SU . 
 *        si_name - The SI name of the SU SI relation.
 *        host_order - Flag indicating if the length is host order
 *
 * Returns: The AVD_SU_SI_REL structure that was found.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SU_SI_REL *avd_su_susi_struc_find(AVD_CL_CB *cb, AVD_SU *su, SaNameT si_name, NCS_BOOL host_order)
{

	AVD_SU_SI_REL *su_si;
	SaNameT si_name_net;

	su_si = su->list_of_susi;

	if (host_order == TRUE) {
		memset((char *)&si_name_net, '\0', sizeof(SaNameT));
		memcpy(si_name_net.value, si_name.value, si_name.length);
		si_name_net.length = m_HTON_SANAMET_LEN(si_name.length);

		while ((su_si != AVD_SU_SI_REL_NULL) && (m_CMP_NORDER_SANAMET(su_si->si->name_net, si_name_net) < 0)) {
			su_si = su_si->su_next;
		}

		if ((su_si != AVD_SU_SI_REL_NULL) && (m_CMP_NORDER_SANAMET(su_si->si->name_net, si_name_net) == 0)) {
			return su_si;
		}

	} else {
		while ((su_si != AVD_SU_SI_REL_NULL) && (m_CMP_NORDER_SANAMET(su_si->si->name_net, si_name) < 0)) {
			su_si = su_si->su_next;
		}

		if ((su_si != AVD_SU_SI_REL_NULL) && (m_CMP_NORDER_SANAMET(su_si->si->name_net, si_name) == 0)) {
			return su_si;
		}
	}

	return AVD_SU_SI_REL_NULL;
}

/*****************************************************************************
 * Function: avd_susi_struc_find
 *
 * Purpose:  This function will find a AVD_SU_SI_REL structure from the
 * list of susis in a su.
 *
 * Input: cb - the AVD control block
 *        su_name - The SU name of the SU SI relation. 
 *        si_name - The SI name of the SU SI relation.
 *        host_order - Flag indicating if the length is host order
 *
 * Returns: The AVD_SU_SI_REL structure that was found.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SU_SI_REL *avd_susi_struc_find(AVD_CL_CB *cb, SaNameT su_name, SaNameT si_name, NCS_BOOL host_order)
{
	AVD_SU *su;

	if ((su = avd_su_struc_find(cb, su_name, host_order)) == AVD_SU_NULL)
		return AVD_SU_SI_REL_NULL;

	return avd_su_susi_struc_find(cb, su, si_name, host_order);
}

/*****************************************************************************
 * Function: avd_susi_struc_find_next
 *
 * Purpose:  This function will find the next AVD_SU_SI_REL structure from the
 * list of susis in a su. If NULL, it will find the first SUSI for the next
 * SU in the tree.
 *
 * Input: cb - the AVD control block
 *        su_name - The SU name of the SU SI relation. 
 *        si_name - The SI name of the SU SI relation.
 *        host_order - Flag indicating if the length is host order
 *
 * Returns: The AVD_SU_SI_REL structure that was found.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SU_SI_REL *avd_susi_struc_find_next(AVD_CL_CB *cb, SaNameT su_name, SaNameT si_name, NCS_BOOL host_order)
{
	AVD_SU *su;
	AVD_SU_SI_REL *su_si = AVD_SU_SI_REL_NULL;
	SaNameT su_name_net, si_name_net;

	/* check if exact match of SU is found so that the next SU SI
	 * in the list of SU can be found. If not select the next SUs
	 * first SU SI relation
	 */
	if (su_name.length != 0) {
		su = avd_su_struc_find(cb, su_name, host_order);
		if (su == AVD_SU_NULL)
			su_si = AVD_SU_SI_REL_NULL;
		else
			su_si = su->list_of_susi;
	}

	if (host_order == TRUE) {
		memset((char *)&si_name_net, '\0', sizeof(SaNameT));
		memcpy(si_name_net.value, si_name.value, si_name.length);
		si_name_net.length = m_HTON_SANAMET_LEN(si_name.length);

		while ((su_si != AVD_SU_SI_REL_NULL) && (m_CMP_NORDER_SANAMET(su_si->si->name_net, si_name_net) <= 0)) {
			su_si = su_si->su_next;
		}

	} else {
		while ((su_si != AVD_SU_SI_REL_NULL) && (m_CMP_NORDER_SANAMET(su_si->si->name_net, si_name) <= 0)) {
			su_si = su_si->su_next;
		}

	}

	if (su_si != AVD_SU_SI_REL_NULL) {
		return su_si;
	}

	/* Find the the first SU SI relation in the next SU with
	 * a SU SI relation.
	 */
	su_name_net = su_name;
	if (host_order == TRUE)
		su_name_net.length = m_HTON_SANAMET_LEN(su_name.length);

	while ((su = avd_su_struc_find_next(cb, su_name_net, FALSE)) != AVD_SU_NULL) {
		if (su->list_of_susi != AVD_SU_SI_REL_NULL)
			break;

		su_name_net = su->name_net;
	}

	/* The given element didn't have a exact match but an element with
	 * a greater SI name was found in the list
	 */

	if (su == AVD_SU_NULL)
		return AVD_SU_SI_REL_NULL;
	else
		return su->list_of_susi;

	return su_si;
}

/*****************************************************************************
 * Function: avd_susi_struc_del
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

uns32 avd_susi_struc_del(AVD_CL_CB *cb, AVD_SU_SI_REL *susi, NCS_BOOL ckpt)
{
	AVD_SU_SI_REL *p_su_si, *p_si_su, *i_su_si;
	AVD_AVND *avnd = NULL;
	SaBoolT is_ncs = susi->su->sg_of_su->sg_ncs_spec;

	m_AVD_GET_SU_NODE_PTR(cb, susi->su, avnd);

	/* delete the comp-csi rels */
	avd_compcsi_list_del(cb, susi, ckpt);

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

	susi->si = AVD_SI_NULL;
	susi->su = AVD_SU_NULL;

	m_MMGR_FREE_AVD_SU_SI_REL(susi);

	/* call the func to check on the context for deletion */
	if (!ckpt) {
		avd_chk_failover_shutdown_cxt(cb, avnd, is_ncs);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfsusitableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_S_U_S_I_TABLE_ENTRY_ID table. This is the SUSI table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function finds the corresponding data structure for the given
 * instance and returns the pointer to the structure.
 *
 * Input:  cb        - AVD control block.
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 * NOTES: This function works in conjunction with extract function to provide the
 * get functionality.
 *
 * 
 **************************************************************************/

uns32 saamfsusitableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
	AVD_CL_CB *avd_cb = (AVD_CL_CB *)cb;
	SaNameT su_name;
	SaNameT si_name;
	uns32 *inst_ptr;
	AVD_SU_SI_REL *susi;
	uns32 i;

	if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK) {
		/* Invalid operation */
		return NCSCC_RC_NO_INSTANCE;
	}

	memset(&su_name, '\0', sizeof(SaNameT));
	memset(&si_name, '\0', sizeof(SaNameT));

	/* Prepare the SU name from the instant ID */
	su_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];

	inst_ptr = (uns32 *)(arg->i_idx.i_inst_ids + 1);
	for (i = 0; i < su_name.length; i++) {
		su_name.value[i] = (uns8)(inst_ptr[i]);
	}

	inst_ptr = inst_ptr + su_name.length;

	/* Prepare the SI name from the instant ID */
	si_name.length = (SaUint16T)inst_ptr[0];

	inst_ptr = inst_ptr + 1;
	for (i = 0; i < si_name.length; i++) {
		si_name.value[i] = (uns8)(inst_ptr[i]);
	}

	/* find the instance of the SU SI */
	susi = avd_susi_struc_find(avd_cb, su_name, si_name, TRUE);

	if (susi == AVD_SU_SI_REL_NULL) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}

	*data = (NCSCONTEXT)susi;

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfsusitableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_AMF_S_U_S_I_TABLE_ENTRY_ID table. This is the SUSI table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after calling the get call to get data structure.
 * This function fills the value information in the param filed structure. For
 * octate information the buffer field will be used for filling the information.
 * MIBLIB will provide the memory and pointer to the buffer. For only objects that
 * have a direct value(i.e their offset is not 0 in VAR INFO) in the structure
 * the data field is filled using the VAR INFO provided by MIBLIB, for others based
 * on the OID the value is filled accordingly.
 *
 * Input:  param     -  param->i_param_id indicates the parameter to extract
 *                      The remaining elements of the param need to be filled
 *                      by the subystem's extract function
 *         var_info  - Pointer to the var_info structure for the param.
 *         data      - The pointer to the data-structure containing the object
 *                     value which we have already provided to MIBLIB from get call.
 *         buffer    - The buffer pointer provided by MIBLIB for filling the octate
 *                     type data.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 *
 * NOTES:  This function works in conjunction with other functions to provide the
 * get,getnext and getrow functionality.
 *
 * 
 **************************************************************************/

uns32 saamfsusitableentry_extract(NCSMIB_PARAM_VAL *param,
				  NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer)
{

	AVD_SU_SI_REL *susi = (AVD_SU_SI_REL *)data;

	if (susi == AVD_SU_SI_REL_NULL) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}
	switch (param->i_param_id) {

	case saAmfSUSISGName_ID:
		m_AVSV_OCTVAL_TO_PARAM(param, buffer, susi->su->sg_name.length, susi->su->sg_name.value);
		break;

	default:
		/* call the MIBLIB utility routine for standfard object types */
		if ((var_info != NULL) && (var_info->offset != 0))
			return ncsmiblib_get_obj_val(param, var_info, data, buffer);
		else
			return NCSCC_RC_NO_OBJECT;
	}
	return NCSCC_RC_SUCCESS;

}

/*****************************************************************************
 * Function: saamfsusitableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_S_U_S_I_TABLE_ENTRY_ID table. This is the SUSI table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for the objects that are settable. This same function can be used for test
 * operation also.
 *
 * Input:  cb        - AVD control block
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         var_info  - The VAR INFO structure pointer generated by MIBLIB for
 *                     the objects in this table.
 *         test_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 saamfsusitableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag)
{
	/* Invalid operation */
	return NCSCC_RC_INV_VAL;
}

/*****************************************************************************
 * Function: saamfsusitableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_S_U_S_I_TABLE_ENTRY_ID table. This is the SUSI table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function gets the next valid instance and its data structure
 * and it passes to the MIBLIB the values.
 *
 * Input: cb        - AVD control block.
 *        arg       - The pointer to the MIB arg that was provided by the caller.
 *        data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 *     next_inst_id - The next instance id will be filled in this buffer
 *                     and returned by reference.
 * next_inst_id_len - The next instance id length.
 *
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context
 *
 * NOTES: This function works in conjunction with extract function to provide the
 * getnext functionality.
 *
 * 
 **************************************************************************/

uns32 saamfsusitableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
			       NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len)
{
	AVD_CL_CB *avd_cb = (AVD_CL_CB *)cb;
	SaNameT su_name;
	SaNameT si_name;
	uns32 *inst_ptr;
	AVD_SU_SI_REL *susi;
	uns32 i, i_si;

	if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK) {
		/* Invalid operation */
		return NCSCC_RC_NO_INSTANCE;
	}

	memset(&su_name, '\0', sizeof(SaNameT));
	memset(&si_name, '\0', sizeof(SaNameT));

	/* Prepare the SU name from the instant ID */
	if (arg->i_idx.i_inst_len != 0) {
		su_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];

		inst_ptr = (uns32 *)(arg->i_idx.i_inst_ids + 1);
		for (i = 0; i < su_name.length; i++) {
			su_name.value[i] = (uns8)(inst_ptr[i]);
		}

		if (arg->i_idx.i_inst_len > (uns32)(su_name.length + 1)) {

			inst_ptr = inst_ptr + su_name.length;

			/* Prepare the SI name from the instant ID */
			si_name.length = (SaUint16T)inst_ptr[0];

			inst_ptr = inst_ptr + 1;
			for (i = 0; i < si_name.length; i++) {
				si_name.value[i] = (uns8)(inst_ptr[i]);
			}
		}
		/* if(arg->i_idx.i_inst_len > (su_name.length + 1)) */
	}

	/* if (arg->i_idx.i_inst_len != 0) */
	/* find the instance of the SU SI */
	susi = avd_susi_struc_find_next(avd_cb, su_name, si_name, TRUE);

	if (susi == AVD_SU_SI_REL_NULL) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}

	/* Prepare the instant ID from the SU name and SI name */

	*next_inst_id_len = m_NCS_OS_NTOHS(susi->si->name_net.length) + m_NCS_OS_NTOHS(susi->su->name_net.length) + 2;

	next_inst_id[0] = m_NCS_OS_NTOHS(susi->su->name_net.length);
	for (i = 0; i < next_inst_id[0]; i++)
		next_inst_id[i + 1] = (uns32)(susi->su->name_net.value[i]);

	next_inst_id[i + 1] = m_NCS_OS_NTOHS(susi->si->name_net.length);
	i_si = i + 2;
	for (i = 0; i < m_NCS_OS_NTOHS(susi->si->name_net.length); i++)
		next_inst_id[i + i_si] = (uns32)(susi->si->name_net.value[i]);

	*data = (NCSCONTEXT)susi;

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfsusitableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_S_U_S_I_TABLE_ENTRY_ID table. This is the SUSI table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for all the objects that are settable as part of the setrow operation. 
 * This same function can be used for test row
 * operation also.
 *
 * Input:  cb        - AVD control block
 *         args      - The pointer to the MIB arg that was provided by the caller.
 *         params    - The List of object ids and their values.
 *         obj_info  - The VAR INFO structure array pointer generated by MIBLIB for
 *                     the objects in this table.
 *      testrow_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 saamfsusitableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
				 NCSMIB_SETROW_PARAM_VAL *params,
				 struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag)
{
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function:saamfsusitableentry_rmvrow 
 *
 * Purpose:  This function is one of the RMVROW processing routines for objects
 * in NCS_CLM_TABLE_ENTRY_ID table. 
 *
 * Input:  cb        - AVD control block.
 *         idx       - pointer to NCSMIB_IDX 
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 **************************************************************************/
uns32 saamfsusitableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx)
{
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_sus_per_si_rank_struc_crt
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

AVD_SUS_PER_SI_RANK *avd_sus_per_si_rank_struc_crt(AVD_CL_CB *cb, AVD_SUS_PER_SI_RANK_INDX indx)
{
	AVD_SUS_PER_SI_RANK *rank_elt = AVD_SU_PER_SI_RANK_NULL;

	/* Allocate a new block structure now
	 */
	if ((rank_elt = m_MMGR_ALLOC_AVD_SU_PER_SI_RANK) == AVD_SU_PER_SI_RANK_NULL) {
		/* log an error */
		m_AVD_LOG_MEM_FAIL(AVD_SU_PER_SI_RANK_ALLOC_FAILED);
		return AVD_SU_PER_SI_RANK_NULL;
	}

	memset((char *)rank_elt, '\0', sizeof(AVD_SUS_PER_SI_RANK));

	rank_elt->indx.si_name_net.length = indx.si_name_net.length;
	memcpy(rank_elt->indx.si_name_net.value, indx.si_name_net.value,
	       m_NCS_OS_NTOHS(rank_elt->indx.si_name_net.length));

	rank_elt->indx.su_rank_net = indx.su_rank_net;

	rank_elt->row_status = NCS_ROW_NOT_READY;

	rank_elt->tree_node.key_info = (uns8 *)(&rank_elt->indx);
	rank_elt->tree_node.bit = 0;
	rank_elt->tree_node.left = NCS_PATRICIA_NODE_NULL;
	rank_elt->tree_node.right = NCS_PATRICIA_NODE_NULL;

	if (ncs_patricia_tree_add(&cb->su_per_si_rank_anchor, &rank_elt->tree_node)
	    != NCSCC_RC_SUCCESS) {
		/* log an error */
		m_MMGR_FREE_AVD_SU_PER_SI_RANK(rank_elt);
		return AVD_SU_PER_SI_RANK_NULL;
	}

	return rank_elt;

}

/*****************************************************************************
 * Function: avd_sus_per_si_rank_struc_find
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

AVD_SUS_PER_SI_RANK *avd_sus_per_si_rank_struc_find(AVD_CL_CB *cb, AVD_SUS_PER_SI_RANK_INDX indx)
{
	AVD_SUS_PER_SI_RANK *rank_elt = AVD_SU_PER_SI_RANK_NULL;
	AVD_SUS_PER_SI_RANK_INDX rank_indx;

	memset(&rank_indx, '\0', sizeof(AVD_SUS_PER_SI_RANK_INDX));
	rank_indx.si_name_net.length = indx.si_name_net.length;
	memcpy(rank_indx.si_name_net.value, indx.si_name_net.value, m_NCS_OS_NTOHS(indx.si_name_net.length));
	rank_indx.su_rank_net = indx.su_rank_net;

	rank_elt = (AVD_SUS_PER_SI_RANK *)ncs_patricia_tree_get(&cb->su_per_si_rank_anchor, (uns8 *)&rank_indx);

	return rank_elt;
}

/*****************************************************************************
 * Function: avd_sus_per_si_rank_struc_find_next
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

AVD_SUS_PER_SI_RANK *avd_sus_per_si_rank_struc_find_next(AVD_CL_CB *cb, AVD_SUS_PER_SI_RANK_INDX indx)
{
	AVD_SUS_PER_SI_RANK *rank_elt = AVD_SU_PER_SI_RANK_NULL;
	AVD_SUS_PER_SI_RANK_INDX rank_indx;

	memset(&rank_indx, '\0', sizeof(AVD_SUS_PER_SI_RANK_INDX));
	rank_indx.si_name_net.length = indx.si_name_net.length;
	memcpy(rank_indx.si_name_net.value, indx.si_name_net.value, m_NCS_OS_NTOHS(indx.si_name_net.length));
	rank_indx.su_rank_net = indx.su_rank_net;

	rank_elt = (AVD_SUS_PER_SI_RANK *)ncs_patricia_tree_getnext(&cb->su_per_si_rank_anchor, (uns8 *)&rank_indx);

	return rank_elt;
}

/*****************************************************************************
 * Function: avd_sus_per_si_rank_struc_find_valid_next
 *
 * Purpose:  This function will find the next AVD_SUS_PER_SI_RANK structure in the
 * tree whose key value is next of the given key value. It also verifies if the 
 * row status is active and the si & su belong to the same sg.
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

AVD_SUS_PER_SI_RANK *avd_sus_per_si_rank_struc_find_valid_next(AVD_CL_CB *cb, AVD_SUS_PER_SI_RANK_INDX indx,
							       AVD_SU **o_su)
{
	AVD_SUS_PER_SI_RANK *rank_elt = AVD_SU_PER_SI_RANK_NULL;
	AVD_SUS_PER_SI_RANK_INDX rank_indx;
	AVD_SI *si = AVD_SI_NULL;
	AVD_SU *su = AVD_SU_NULL;

	memset(&rank_indx, '\0', sizeof(AVD_SUS_PER_SI_RANK_INDX));
	rank_indx.si_name_net.length = indx.si_name_net.length;
	memcpy(rank_indx.si_name_net.value, indx.si_name_net.value, m_NCS_OS_NTOHS(indx.si_name_net.length));
	rank_indx.su_rank_net = indx.su_rank_net;

	rank_elt = (AVD_SUS_PER_SI_RANK *)ncs_patricia_tree_getnext(&cb->su_per_si_rank_anchor, (uns8 *)&rank_indx);

	if (rank_elt == AVD_SU_PER_SI_RANK_NULL) {
		/*  return NULL */
		return rank_elt;
	}

	if (rank_elt->row_status != NCS_ROW_ACTIVE) {
		return avd_sus_per_si_rank_struc_find_valid_next(cb, rank_elt->indx, o_su);
	}

	/* get the su & si */
	su = avd_su_struc_find(cb, rank_elt->su_name, TRUE);
	si = avd_si_struc_find(cb, indx.si_name_net, FALSE);

	/* validate this entry */
	if ((si == AVD_SI_NULL) || (su == AVD_SU_NULL) || (si->row_status != NCS_ROW_ACTIVE) ||
	    (su->row_status != NCS_ROW_ACTIVE) || (si->sg_of_si != su->sg_of_su))
		return avd_sus_per_si_rank_struc_find_valid_next(cb, rank_elt->indx, o_su);

	*o_su = su;
	return rank_elt;
}

/*****************************************************************************
 * Function: avd_sus_per_si_rank_struc_del
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

uns32 avd_sus_per_si_rank_struc_del(AVD_CL_CB *cb, AVD_SUS_PER_SI_RANK *rank_elt)
{
	if (rank_elt == AVD_SU_PER_SI_RANK_NULL)
		return NCSCC_RC_FAILURE;

	if (ncs_patricia_tree_del(&cb->su_per_si_rank_anchor, &rank_elt->tree_node)
	    != NCSCC_RC_SUCCESS) {
		/* log error */
		return NCSCC_RC_FAILURE;
	}

	m_MMGR_FREE_AVD_SU_PER_SI_RANK(rank_elt);
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfsuspersirankentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_S_USPER_S_I_RANK_ENTRY_ID table. This is the SUsperSIRank table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function finds the corresponding data structure for the given
 * instance and returns the pointer to the structure.
 *
 * Input:  cb        - AVD control block.
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 * 
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 * NOTES: This function works in conjunction with extract function to provide the
 * get functionality.
 *
 * 
 **************************************************************************/

uns32 saamfsuspersirankentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
	AVD_CL_CB *avd_cb = (AVD_CL_CB *)cb;
	AVD_SUS_PER_SI_RANK_INDX indx;
	uns16 len;
	uns32 i;
	AVD_SUS_PER_SI_RANK *rank_elt = AVD_SU_PER_SI_RANK_NULL;

	if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK) {
		/* Invalid operation */
		return NCSCC_RC_NO_INSTANCE;
	}

	memset(&indx, '\0', sizeof(AVD_SUS_PER_SI_RANK_INDX));

	/* Prepare the SuperSiRank database key from the instant ID */
	len = (SaUint16T)arg->i_idx.i_inst_ids[0];

	indx.si_name_net.length = m_NCS_OS_HTONS(len);

	for (i = 0; i < len; i++) {
		indx.si_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i + 1]);
	}
	if (arg->i_idx.i_inst_len > len + 1) {
		indx.su_rank_net = m_NCS_OS_HTONL((SaUint32T)arg->i_idx.i_inst_ids[len + 1]);
	}

	rank_elt = avd_sus_per_si_rank_struc_find(avd_cb, indx);

	if (rank_elt == AVD_SU_PER_SI_RANK_NULL) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}

	*data = (NCSCONTEXT)rank_elt;

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfsuspersirankentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_AMF_S_USPER_S_I_RANK_ENTRY_ID table. This is the SUsperSIRank table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after calling the get call to get data structure.
 * This function fills the value information in the param filed structure. For
 * octate information the buffer field will be used for filling the information.
 * MIBLIB will provide the memory and pointer to the buffer. For only objects that
 * have a direct value(i.e their offset is not 0 in VAR INFO) in the structure
 * the data field is filled using the VAR INFO provided by MIBLIB, for others based
 * on the OID the value is filled accordingly.
 *
 * Input:  param     -  param->i_param_id indicates the parameter to extract
 *                      The remaining elements of the param need to be filled
 *                      by the subystem's extract function
 *         var_info  - Pointer to the var_info structure for the param.
 *         data      - The pointer to the data-structure containing the object
 *                     value which we have already provided to MIBLIB from get call.
 *         buffer    - The buffer pointer provided by MIBLIB for filling the octate
 *                     type data.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 *
 * NOTES:  This function works in conjunction with other functions to provide the
 * get,getnext and getrow functionality.
 *
 * 
 **************************************************************************/

uns32 saamfsuspersirankentry_extract(NCSMIB_PARAM_VAL *param,
				     NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer)
{
	AVD_SUS_PER_SI_RANK *rank_elt = (AVD_SUS_PER_SI_RANK *)data;

	if (rank_elt == AVD_SU_PER_SI_RANK_NULL) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}

	switch (param->i_param_id) {

	case saAmfSUsperSISUName_ID:
		m_AVSV_OCTVAL_TO_PARAM(param, buffer, rank_elt->su_name.length, rank_elt->su_name.value);
		break;

	default:
		/* call the MIBLIB utility routine for standfard object types */
		if ((var_info != NULL) && (var_info->offset != 0))
			return ncsmiblib_get_obj_val(param, var_info, data, buffer);
		else
			return NCSCC_RC_NO_OBJECT;
		break;

	}

	return NCSCC_RC_SUCCESS;

}

/*****************************************************************************
 * Function: saamfsuspersirankentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_S_USPER_S_I_RANK_ENTRY_ID table. This is the SUsperSIRank table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for the objects that are settable. This same function can be used for test
 * operation also.
 *
 * Input:  cb        - AVD control block
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         var_info  - The VAR INFO structure pointer generated by MIBLIB for
 *                     the objects in this table.
 *         test_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 saamfsuspersirankentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag)
{

	AVD_CL_CB *avd_cb = (AVD_CL_CB *)cb;
	AVD_SUS_PER_SI_RANK_INDX indx;
	uns16 len;
	uns32 i;
	AVD_SUS_PER_SI_RANK *rank_elt = AVD_SU_PER_SI_RANK_NULL;
	AVD_SU *su = AVD_SU_NULL;
	AVD_SI *si = AVD_SI_NULL;

	if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK) {
		/* Invalid operation */
		return NCSCC_RC_NO_INSTANCE;
	}

	memset(&indx, '\0', sizeof(AVD_SUS_PER_SI_RANK_INDX));

	/* Prepare the SuperSiRank database key from the instant ID */
	len = (SaUint16T)arg->i_idx.i_inst_ids[0];

	indx.si_name_net.length = m_NCS_OS_HTONS(len);

	for (i = 0; i < len; i++) {
		indx.si_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i + 1]);
	}
	if (arg->i_idx.i_inst_len > len + 1) {
		indx.su_rank_net = m_NCS_OS_HTONL((SaUint32T)arg->i_idx.i_inst_ids[len + 1]);
	}

	rank_elt = avd_sus_per_si_rank_struc_find(avd_cb, indx);

	if (rank_elt == AVD_SU_PER_SI_RANK_NULL) {
		/* The row was not found */
		if ((arg->req.info.set_req.i_param_val.i_param_id == saAmfSUsperSIRowStatus_ID)
		    && (arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT)) {
			/* Invalid row status object */
			return NCSCC_RC_INV_VAL;
		}

		if (test_flag == TRUE) {
			return NCSCC_RC_SUCCESS;
		}

		m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

		rank_elt = avd_sus_per_si_rank_struc_crt(avd_cb, indx);

		m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

		if (rank_elt == AVD_SU_PER_SI_RANK_NULL) {
			/* Invalid instance object */
			return NCSCC_RC_NO_INSTANCE;
		}

	} else {		/* The record is already available */

		if (arg->req.info.set_req.i_param_val.i_param_id == saAmfSUsperSIRowStatus_ID) {
			/* This is a row status operation */
			if (arg->req.info.set_req.i_param_val.info.i_int == (uns32)rank_elt->row_status) {
				/* row status object is same so nothing to be done. */
				return NCSCC_RC_SUCCESS;
			}

			switch (arg->req.info.set_req.i_param_val.info.i_int) {
			case NCS_ROW_ACTIVE:

				/* validate the structure to see if the row can be made active */
				if (rank_elt->su_name.length <= 0) {
					/* SU name must be present for the row to be made active. */
					return NCSCC_RC_INV_VAL;
				}
				if (test_flag == TRUE) {
					return NCSCC_RC_SUCCESS;
				}
				rank_elt->row_status = arg->req.info.set_req.i_param_val.info.i_int;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(cb, rank_elt, AVSV_CKPT_AVD_SUS_PER_SI_RANK_CONFIG);
				return NCSCC_RC_SUCCESS;
				break;

			case NCS_ROW_NOT_IN_SERVICE:
			case NCS_ROW_DESTROY:

				su = avd_su_struc_find(avd_cb, rank_elt->su_name, TRUE);
				si = avd_si_struc_find(avd_cb, rank_elt->indx.si_name_net, FALSE);

				if (su != AVD_SU_NULL) {
					if (su->admin_state != NCS_ADMIN_STATE_LOCK) {
						return NCSCC_RC_INV_VAL;
					}
				}
				if (si != AVD_SI_NULL) {
					if (si->admin_state != NCS_ADMIN_STATE_LOCK) {
						return NCSCC_RC_INV_VAL;
					}
				}
				if (test_flag == TRUE) {
					return NCSCC_RC_SUCCESS;
				}

				if (rank_elt->row_status == NCS_ROW_ACTIVE) {
					m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, rank_elt,
									AVSV_CKPT_AVD_SUS_PER_SI_RANK_CONFIG);
				}

				if (arg->req.info.set_req.i_param_val.info.i_int == NCS_ROW_DESTROY) {
					/* delete and free the structure */
					m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

					avd_sus_per_si_rank_struc_del(avd_cb, rank_elt);

					m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);
				} else {
					rank_elt->row_status = arg->req.info.set_req.i_param_val.info.i_int;
				}

				return NCSCC_RC_SUCCESS;
				break;

			default:

				m_AVD_LOG_INVALID_VAL_ERROR(arg->req.info.set_req.i_param_val.info.i_int);
				/* Invalid row status object */
				return NCSCC_RC_INV_VAL;
				break;
			}
		}
	}

	if (rank_elt->row_status == NCS_ROW_ACTIVE) {
		/* when row status is active we don't allow any MIB object to be
		 * modified.
		 */
		return NCSCC_RC_INV_VAL;
	}

	if (test_flag == TRUE) {
		return NCSCC_RC_SUCCESS;
	}

	if (arg->req.info.set_req.i_param_val.i_param_id == saAmfSUsperSIRowStatus_ID) {
		/* fill the row status value */
		if (arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT) {
			rank_elt->row_status = arg->req.info.set_req.i_param_val.info.i_int;
		}

	} else if (arg->req.info.set_req.i_param_val.i_param_id == saAmfSUsperSISUName_ID) {
		rank_elt->su_name.length = arg->req.info.set_req.i_param_val.i_length;
		memcpy(rank_elt->su_name.value, arg->req.info.set_req.i_param_val.info.i_oct, rank_elt->su_name.length);
	} else {
		/* Invalid Object ID */
		return NCSCC_RC_INV_VAL;
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfsuspersirankentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_S_USPER_S_I_RANK_ENTRY_ID table. This is the SUsperSIRank table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function gets the next valid instance and its data structure
 * and it passes to the MIBLIB the values.
 *
 * Input: cb        - AVD control block.
 *        arg       - The pointer to the MIB arg that was provided by the caller.
 *        data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 *     next_inst_id - The next instance id will be filled in this buffer
 *                     and returned by reference.
 * next_inst_id_len - The next instance id length.
 *
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context
 *
 * NOTES: This function works in conjunction with extract function to provide the
 * getnext functionality.
 *
 * 
 **************************************************************************/

uns32 saamfsuspersirankentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
				  NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len)
{
	AVD_CL_CB *avd_cb = (AVD_CL_CB *)cb;
	AVD_SUS_PER_SI_RANK_INDX indx;
	uns16 len;
	uns32 i;
	AVD_SUS_PER_SI_RANK *rank_elt = AVD_SU_PER_SI_RANK_NULL;

	if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK) {
		/* Invalid operation */
		return NCSCC_RC_NO_INSTANCE;
	}

	memset(&indx, '\0', sizeof(AVD_SUS_PER_SI_RANK_INDX));

	if (arg->i_idx.i_inst_len != 0) {

		/* Prepare the SuperSiRank database key from the instant ID */
		len = (SaUint16T)arg->i_idx.i_inst_ids[0];

		indx.si_name_net.length = m_NCS_OS_HTONS(len);

		for (i = 0; i < len; i++) {
			indx.si_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i + 1]);
		}
		if (arg->i_idx.i_inst_len > len + 1) {
			indx.su_rank_net = m_NCS_OS_HTONL((SaUint32T)arg->i_idx.i_inst_ids[len + 1]);
		}
	}

	rank_elt = avd_sus_per_si_rank_struc_find_next(avd_cb, indx);

	if (rank_elt == AVD_SU_PER_SI_RANK_NULL) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}

	*data = (NCSCONTEXT)rank_elt;

	/* Prepare the instant ID from the SI name and SU rank */
	len = m_NCS_OS_NTOHS(rank_elt->indx.si_name_net.length);

	*next_inst_id_len = len + 1 + 1;

	next_inst_id[0] = len;

	for (i = 0; i < len; i++) {
		next_inst_id[i + 1] = (uns32)(rank_elt->indx.si_name_net.value[i]);
	}

	next_inst_id[len + 1] = m_NCS_OS_NTOHL(rank_elt->indx.su_rank_net);

	*data = (NCSCONTEXT)rank_elt;

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfsuspersirankentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_S_USPER_S_I_RANK_ENTRY_ID table. This is the SUsperSIRank table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for all the objects that are settable as part of the setrow operation. 
 * This same function can be used for test row
 * operation also.
 *
 * Input:  cb        - AVD control block
 *         args      - The pointer to the MIB arg that was provided by the caller.
 *         params    - The List of object ids and their values.
 *         obj_info  - The VAR INFO structure array pointer generated by MIBLIB for
 *                     the objects in this table.
 *      testrow_flag - The flag that indicates if this is set or test.
 * 
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 saamfsuspersirankentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
				    NCSMIB_SETROW_PARAM_VAL *params,
				    struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag)
{
	return NCSCC_RC_SUCCESS;
}
