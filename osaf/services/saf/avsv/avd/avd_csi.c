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
  the CSI database and component CSI relationship list on the AVD. It
  deals with all the MIB operations like set,get,getnext etc related to the
  component service instance table. 

..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_csi_struc_crt - creates and adds CSI structure to database.
  avd_csi_struc_find - Finds CSI structure from the database.
  avd_csi_struc_find_next - Finds the next CSI structure from the database.  
  avd_csi_struc_del - deletes and frees CSI structure from database.
  avd_csi_del_si_list - deletes the CSI from the SI list.
  avd_compcsi_struc_crt - creates and adds compCSI structure to the list of
                       compCSI in SUSI.
  avd_compcsi_list_del - deletes and frees all compCSI structure from
                         the list of compCSI in SUSI.
  saamfcsitableentry_get - This function is one of the get processing
                        routines for objects in SA_AMF_C_S_I_TABLE_ENTRY_ID table.
  saamfcsitableentry_extract - This function is one of the get processing
                            function for objects in SA_AMF_C_S_I_TABLE_ENTRY_ID
                            table.
  saamfcsitableentry_set - This function is the set processing for objects in
                         SA_AMF_C_S_I_TABLE_ENTRY_ID table.
  saamfcsitableentry_next - This function is the next processing for objects in
                         SA_AMF_C_S_I_TABLE_ENTRY_ID table.
  saamfcsitableentry_setrow - This function is the setrow processing for
                           objects in SA_AMF_C_S_I_TABLE_ENTRY_ID table.

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"

/*****************************************************************************
 * Function: avd_csi_struc_crt
 *
 * Purpose:  This function will create and add a AVD_CSI structure to the
 * tree if an element with csi_name key value doesn't exist in the tree.
 *
 * Input: cb - the AVD control block
 *        csi_name - The CSI name of the CSI that needs to be
 *                    added.
 *
 * Returns: The pointer to AVD_CSI structure allocated and added.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_CSI *avd_csi_struc_crt(AVD_CL_CB *cb, SaNameT csi_name, NCS_BOOL ckpt)
{
	AVD_CSI *csi;

	if ((csi_name.length <= 7) || (strncmp(csi_name.value, "safCsi=", 7) != 0)) {
		return AVD_CSI_NULL;
	}

	/* Allocate a new block structure now
	 */
	if ((csi = m_MMGR_ALLOC_AVD_CSI) == AVD_CSI_NULL) {
		/* log an error */
		m_AVD_LOG_MEM_FAIL(AVD_CSI_ALLOC_FAILED);
		return AVD_CSI_NULL;
	}

	memset((char *)csi, '\0', sizeof(AVD_CSI));

	if (ckpt) {
		memcpy(csi->name_net.value, csi_name.value, m_NCS_OS_NTOHS(csi_name.length));
		csi->name_net.length = csi_name.length;
	} else {
		memcpy(csi->name_net.value, csi_name.value, csi_name.length);
		csi->name_net.length = m_HTON_SANAMET_LEN(csi_name.length);

		csi->row_status = NCS_ROW_NOT_READY;
	}

	/* Init the pointers */
	csi->si = AVD_SI_NULL;
	csi->si_list_of_csi_next = AVD_CSI_NULL;
	csi->list_compcsi = AVD_COMP_CSI_REL_NULL;
	csi->list_param = AVD_CSI_PARAM_NULL;

	/* initialize the pg node-list */
	csi->pg_node_list.order = NCS_DBLIST_ANY_ORDER;
	csi->pg_node_list.cmp_cookie = avsv_dblist_uns32_cmp;
	csi->pg_node_list.free_cookie = 0;

	csi->tree_node.key_info = (uns8 *)&(csi->name_net);
	csi->tree_node.bit = 0;
	csi->tree_node.left = NCS_PATRICIA_NODE_NULL;
	csi->tree_node.right = NCS_PATRICIA_NODE_NULL;

	if (ncs_patricia_tree_add(&cb->csi_anchor, &csi->tree_node)
	    != NCSCC_RC_SUCCESS) {
		/* log an error */
		m_MMGR_FREE_AVD_CSI(csi);
		return AVD_CSI_NULL;
	}

	return csi;

}

/*****************************************************************************
 * Function: avd_csi_struc_find
 *
 * Purpose:  This function will find a AVD_CSI structure in the
 * tree with csi_name value as key.
 *
 * Input: cb - the AVD control block
 *        csi_name - The name of the CSI.
 *        host_order - Flag indicating if the length is host order
 *
 * Returns: The pointer to AVD_CSI structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_CSI *avd_csi_struc_find(AVD_CL_CB *cb, SaNameT csi_name, NCS_BOOL host_order)
{
	AVD_CSI *csi;
	SaNameT lcsi_name;

	memset((char *)&lcsi_name, '\0', sizeof(SaNameT));
	lcsi_name.length = (host_order == FALSE) ? csi_name.length : m_HTON_SANAMET_LEN(csi_name.length);
	memcpy(lcsi_name.value, csi_name.value, m_NCS_OS_NTOHS(lcsi_name.length));

	csi = (AVD_CSI *)ncs_patricia_tree_get(&cb->csi_anchor, (uns8 *)&lcsi_name);

	return csi;
}

/*****************************************************************************
 * Function: avd_csi_struc_find_next
 *
 * Purpose:  This function will find the next AVD_CSI structure in the
 * tree whose csi_name value is next of the given csi_name value.
 *
 * Input: cb - the AVD control block
 *        csi_name - The name of the CSI.
 *        host_order - Flag indicating if the length is host order
 *
 * Returns: The pointer to AVD_CSI structure found in the tree.  
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_CSI *avd_csi_struc_find_next(AVD_CL_CB *cb, SaNameT csi_name, NCS_BOOL host_order)
{
	AVD_CSI *csi;
	SaNameT lcsi_name;

	memset((char *)&lcsi_name, '\0', sizeof(SaNameT));
	lcsi_name.length = (host_order == FALSE) ? csi_name.length : m_HTON_SANAMET_LEN(csi_name.length);

	memcpy(lcsi_name.value, csi_name.value, m_NCS_OS_NTOHS(lcsi_name.length));

	csi = (AVD_CSI *)ncs_patricia_tree_getnext(&cb->csi_anchor, (uns8 *)&lcsi_name);

	return csi;
}

/*****************************************************************************
 * Function: avd_csi_struc_del
 *
 * Purpose:  This function will delete and free AVD_CSI structure from 
 * the tree.
 *
 * Input: cb - the AVD control block
 *        csi - The CSI structure that needs to be deleted.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_csi_struc_del(AVD_CL_CB *cb, AVD_CSI *csi)
{
	if (csi == AVD_CSI_NULL)
		return NCSCC_RC_FAILURE;

	if (ncs_patricia_tree_del(&cb->csi_anchor, &csi->tree_node)
	    != NCSCC_RC_SUCCESS) {
		/* log error */
		return NCSCC_RC_FAILURE;
	}

	m_MMGR_FREE_AVD_CSI(csi);
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_csi_param_crt
 *
 * Purpose:  This function will create and add a AVD_CSI_PARAM structure to the
 * list if an element with param_name key value doesn't exist in the tree.
 *
 * Input: cb - the AVD control block
 *        csi - The CSI to which the param needs to be added.
          param_name - The name of param to be created and added.
 *
 * Returns: The pointer to AVD_CSI structure allocated and added.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static AVD_CSI_PARAM *avd_csi_param_crt(AVD_CSI *csi, SaNameT param_name)
{
	AVD_CSI_PARAM *param;
	AVD_CSI_PARAM *ptr, *i_param;

	/* Allocate a new block structure now
	 */
	if ((param = m_MMGR_ALLOC_AVD_CSI_PARAM) == AVD_CSI_PARAM_NULL) {
		/* log an error */
		m_AVD_LOG_MEM_FAIL(AVD_CSI_ALLOC_FAILED);
		return AVD_CSI_PARAM_NULL;
	}

	memset((char *)param, '\0', sizeof(AVD_CSI_PARAM));

	memcpy(param->param.name.value, param_name.value, param_name.length);
	param->param.name.length = param_name.length;

	param->row_status = NCS_ROW_NOT_READY;

	/* Init the pointers */
	param->param_next = AVD_CSI_PARAM_NULL;

	/* keep the list in CSI in ascending order */
	if (csi->list_param == AVD_CSI_PARAM_NULL) {
		csi->list_param = param;
		param->param_next = AVD_CSI_PARAM_NULL;
		return param;
	}

	ptr = AVD_CSI_PARAM_NULL;
	i_param = csi->list_param;
	while ((i_param != AVD_CSI_PARAM_NULL) && (m_CMP_HORDER_SANAMET(i_param->param.name, param->param.name) < 0)) {
		ptr = i_param;
		i_param = i_param->param_next;
	}

	if (ptr == AVD_CSI_PARAM_NULL) {
		csi->list_param = param;
		param->param_next = i_param;
	} else {
		ptr->param_next = param;
		param->param_next = i_param;
	}

	return param;
}

/*****************************************************************************
 * Function: avd_csi_param_del 
 *
 * Purpose:  This function will find and delete the CSI_PARAM in the list.
 *
 * Input: cb - the AVD control block
 *        csi - The CSI in which the param needs to be deleted.
          param - The param to be deleted.
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_csi_param_del(AVD_CSI *csi, AVD_CSI_PARAM *param)
{
	AVD_CSI_PARAM *i_param = AVD_CSI_PARAM_NULL;
	AVD_CSI_PARAM *p_param = AVD_CSI_PARAM_NULL;

	if (!param)
		return NCSCC_RC_SUCCESS;

	/* remove PARAM from csi list */
	i_param = csi->list_param;

	while ((i_param != AVD_CSI_PARAM_NULL) && (i_param != param)) {
		p_param = i_param;
		i_param = i_param->param_next;
	}

	if (i_param != param) {
		/* Log a fatal error */
	} else {
		if (p_param == AVD_CSI_PARAM_NULL) {
			csi->list_param = i_param->param_next;
		} else {
			p_param->param_next = i_param->param_next;
		}
	}

	m_MMGR_FREE_AVD_CSI_PARAM(param);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_csi_find_param
 *
 * Purpose:  This function will find a AVD_CSI_PARAM structure in the
 * list with param_name value as key.
 *
 * Input: csi  - the CSI control block
 *        csi_name - The name of the CSI.
 *
 * Returns: The pointer to AVD_CSI_PARAM structure found in the list. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static AVD_CSI_PARAM *avd_csi_find_param(AVD_CSI *csi, SaNameT param_name)
{
	AVD_CSI_PARAM *param;

	if ((param_name.length == 0) || (csi->list_param == AVD_CSI_PARAM_NULL))
		return AVD_CSI_PARAM_NULL;

	param = csi->list_param;

	while (param != AVD_CSI_PARAM_NULL) {
		if (param_name.length == param->param.name.length) {
			if (memcmp(param_name.value, param->param.name.value, param_name.length) == 0) {
				return param;
			}
		}
		param = param->param_next;
	}
	return AVD_CSI_PARAM_NULL;
}

/*****************************************************************************
 * Function: avd_csi_find_next_param
 *
 * Purpose:  This function will find a AVD_CSI_PARAM structure in the
 * list with param_name value as key.
 *
 * Input: csi  - the CSI control block
 *        csi_name - The name of the CSI.
 *
 * Returns: The pointer to AVD_CSI_PARAM structure found in the list. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static AVD_CSI_PARAM *avd_csi_find_next_param(AVD_CL_CB *cb, AVD_CSI **csi_ptr, SaNameT csi_name, SaNameT param_name)
{
	AVD_CSI_PARAM *param = AVD_CSI_PARAM_NULL;
	AVD_CSI *csi = AVD_CSI_NULL;
	NCS_BOOL found = FALSE;
	int32 cmp = 0;
	SaNameT temp_name;

	/* First find the CSI */
	if (csi_name.length != 0) {
		csi = avd_csi_struc_find(cb, csi_name, TRUE);
		if (csi == AVD_CSI_NULL)
			param = AVD_CSI_PARAM_NULL;
		else
			param = csi->list_param;
	}

	if ((param_name.length == 0) && (param != AVD_CSI_PARAM_NULL)) {
		*csi_ptr = csi;
		return param;
	}

	while (param != AVD_CSI_PARAM_NULL) {
		if (param_name.length == param->param.name.length) {
			cmp = m_CMP_HORDER_SANAMET(param_name, param->param.name);
			if (cmp == 0) {
				found = TRUE;
				break;
			} else if (cmp < 0) {
				break;
			}
		}

		param = param->param_next;
	}

	/* If found the exact match get the next and if available return and DONE
	 * else assign null and move on to the next csi
	 */
	if (found) {
		if (param->param_next != AVD_CSI_PARAM_NULL) {
			*csi_ptr = csi;
			return param->param_next;
		} else
			param = AVD_CSI_PARAM_NULL;
	}

	/* Find the next csi which has an existing csi param */

	if (param == AVD_CSI_PARAM_NULL) {
		/* The length is in host order change it */
		temp_name.length = m_NTOH_SANAMET_LEN(csi_name.length);
		memcpy(temp_name.value, csi_name.value, csi_name.length);

		while ((csi = avd_csi_struc_find_next(cb, temp_name, FALSE)) != AVD_CSI_NULL) {
			if (csi->list_param != AVD_CSI_PARAM_NULL)
				break;

			temp_name = csi->name_net;
		}

		if (csi == AVD_CSI_NULL) {
			*csi_ptr = AVD_CSI_NULL;
			return AVD_CSI_PARAM_NULL;
		} else {
			*csi_ptr = csi;
			return csi->list_param;
		}
	}

	*csi_ptr = csi;
	return param;
}

/*****************************************************************************
 * Function: avd_compcsi_struc_crt
 *
 * Purpose:  This function will create and add a AVD_COMP_CSI_REL structure
 * to the list of component csi relationships of a SU SI relationship.
 * It will add the element to the head of the list.
 *
 * Input: cb - the AVD control block
 *        susi - The SU SI relationship structure that encompasses this
 *               component CSI relationship.
 *
 * Returns: Pointer to AVD_COMP_CSI_REL structure or AVD_COMP_CSI_REL_NULL
 *          if failure.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_COMP_CSI_REL *avd_compcsi_struc_crt(AVD_CL_CB *cb, AVD_SU_SI_REL *susi)
{
	AVD_COMP_CSI_REL *comp_csi;

	/* Allocate a new block structure now
	 */
	if ((comp_csi = m_MMGR_ALLOC_AVD_COMP_CSI_REL) == AVD_COMP_CSI_REL_NULL) {
		/* log an error */
		m_AVD_LOG_MEM_FAIL(AVD_COMPCSI_ALLOC_FAILED);
		return AVD_COMP_CSI_REL_NULL;
	}

	memset((char *)comp_csi, '\0', sizeof(AVD_COMP_CSI_REL));

	comp_csi->susi_csicomp_next = AVD_COMP_CSI_REL_NULL;
	if (susi->list_of_csicomp == AVD_COMP_CSI_REL_NULL) {
		susi->list_of_csicomp = comp_csi;
		return comp_csi;
	}

	comp_csi->susi_csicomp_next = susi->list_of_csicomp;
	susi->list_of_csicomp = comp_csi;

	return comp_csi;
}

/*****************************************************************************
 * Function: avd_compcsi_list_del
 *
 * Purpose:  This function will delete and free all the AVD_COMP_CSI_REL
 * structure from the list_of_csicomp in the SUSI relationship
 * 
 * Input: cb - the AVD control block
 *        susi - The SU SI relationship structure that encompasses this
 *               component CSI relationship.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE .
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_compcsi_list_del(AVD_CL_CB *cb, AVD_SU_SI_REL *susi, NCS_BOOL ckpt)
{
	AVD_COMP_CSI_REL *lcomp_csi;
	AVD_COMP_CSI_REL *i_compcsi, *prev_compcsi = AVD_COMP_CSI_REL_NULL;

	while (susi->list_of_csicomp != AVD_COMP_CSI_REL_NULL) {
		lcomp_csi = susi->list_of_csicomp;

		i_compcsi = lcomp_csi->csi->list_compcsi;
		while ((i_compcsi != AVD_COMP_CSI_REL_NULL) && (i_compcsi != lcomp_csi)) {
			prev_compcsi = i_compcsi;
			i_compcsi = i_compcsi->csi_csicomp_next;
		}
		if (i_compcsi != lcomp_csi) {
			/* not found */
		} else {
			if (prev_compcsi == AVD_COMP_CSI_REL_NULL) {
				lcomp_csi->csi->list_compcsi = i_compcsi->csi_csicomp_next;
			} else {
				prev_compcsi->csi_csicomp_next = i_compcsi->csi_csicomp_next;
			}
			lcomp_csi->csi->compcsi_cnt--;

			/* trigger pg upd */
			if (!ckpt) {
				avd_pg_compcsi_chg_prc(cb, lcomp_csi, TRUE);
			}

			i_compcsi->csi_csicomp_next = AVD_COMP_CSI_REL_NULL;
		}

		susi->list_of_csicomp = lcomp_csi->susi_csicomp_next;
		lcomp_csi->susi_csicomp_next = AVD_COMP_CSI_REL_NULL;
		m_MMGR_FREE_AVD_COMP_CSI_REL(lcomp_csi);

	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_csi_add_si_list
 *
 * Purpose:  This function will add the given CSI to the SI list, and fill
 * the CSIs pointers. 
 *
 * Input: cb - the AVD control block
 *        csi - The CSI pointer
 *
 * Returns: None. 
 *
 * NOTES: None
 *
 * 
 **************************************************************************/

void avd_csi_add_si_list(AVD_CL_CB *cb, AVD_CSI *csi)
{
	AVD_CSI *i_csi = AVD_CSI_NULL;
	AVD_CSI *prev_csi = AVD_CSI_NULL;

	if ((csi == AVD_CSI_NULL) || (csi->si == AVD_SI_NULL))
		return;

	i_csi = csi->si->list_of_csi;

	while ((i_csi != AVD_CSI_NULL) && (csi->rank <= i_csi->rank)) {
		prev_csi = i_csi;
		i_csi = i_csi->si_list_of_csi_next;
	}

	if (prev_csi == AVD_CSI_NULL) {
		csi->si_list_of_csi_next = csi->si->list_of_csi;
		csi->si->list_of_csi = csi;
	} else {
		prev_csi->si_list_of_csi_next = csi;
		csi->si_list_of_csi_next = i_csi;
	}
	return;
}

/*****************************************************************************
 * Function: avd_csi_del_si_list
 *
 * Purpose:  This function will del the given CSI from the SI list, and fill
 * the CSIs pointer with NULL
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

void avd_csi_del_si_list(AVD_CL_CB *cb, AVD_CSI *csi)
{
	AVD_CSI *i_csi = AVD_CSI_NULL;
	AVD_CSI *prev_csi = AVD_CSI_NULL;

	if (csi->si != AVD_SI_NULL) {
		/* remove CSI from the SI */
		prev_csi = AVD_CSI_NULL;
		i_csi = csi->si->list_of_csi;

		while ((i_csi != AVD_CSI_NULL) && (i_csi != csi)) {
			prev_csi = i_csi;
			i_csi = i_csi->si_list_of_csi_next;
		}

		if (i_csi != csi) {
			/* Log a fatal error */
		} else {
			if (prev_csi == AVD_CSI_NULL) {
				csi->si->list_of_csi = csi->si_list_of_csi_next;
			} else {
				prev_csi->si_list_of_csi_next = csi->si_list_of_csi_next;
			}
		}

		csi->si_list_of_csi_next = AVD_CSI_NULL;
		csi->si = AVD_SI_NULL;
	}
	/* if (csi->si != AVD_SI_NULL) */
}

/*****************************************************************************
 * Function: saamfcsitableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_C_S_I_TABLE_ENTRY_ID table. This is the CSI table. The
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

uns32 saamfcsitableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
	AVD_CL_CB *avd_cb = (AVD_CL_CB *)cb;
	AVD_CSI *csi;
	SaNameT csi_name;
	uns32 i;

	if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK) {
		/* Invalid operation */
		return NCSCC_RC_NO_INSTANCE;
	}

	memset(&csi_name, '\0', sizeof(SaNameT));

	/* Prepare the CSI database key from the instant ID */
	csi_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
	for (i = 0; i < csi_name.length; i++) {
		csi_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i + 1]);
	}

	csi = avd_csi_struc_find(avd_cb, csi_name, TRUE);

	if (csi == AVD_CSI_NULL) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}

	*data = (NCSCONTEXT)csi;

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfcsitableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_AMF_C_S_I_TABLE_ENTRY_ID table. This is the CSI table. The
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

uns32 saamfcsitableentry_extract(NCSMIB_PARAM_VAL *param, NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer)
{
	AVD_CSI *csi = (AVD_CSI *)data;

	if (csi == AVD_CSI_NULL) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}

	switch (param->i_param_id) {
	case saAmfCSType_ID:
		m_AVSV_LENVAL_TO_PARAM(param, buffer, csi->csi_type);
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
 * Function: saamfcsitableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_C_S_I_TABLE_ENTRY_ID table. This is the CSI table. The
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

uns32 saamfcsitableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag)
{
	AVD_CL_CB *avd_cb = (AVD_CL_CB *)cb;
	AVD_CSI *csi;
	SaNameT csi_name;
	SaNameT temp_name;
	uns32 i;
	NCS_BOOL val_same_flag = FALSE;
	uns32 rc;
	AVD_CSI_PARAM *i_csi_param = AVD_CSI_PARAM_NULL;
	AVD_CSI *i_csi = AVD_CSI_NULL;
	AVD_PG_CSI_NODE *curr = 0;
	NCSMIBLIB_REQ_INFO temp_mib_req;

	if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK) {
		/* Invalid operation */
		return NCSCC_RC_INV_VAL;
	}

	memset(&csi_name, '\0', sizeof(SaNameT));

	/* Prepare the CSI database key from the instant ID */
	csi_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
	for (i = 0; i < csi_name.length; i++) {
		csi_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i + 1]);
	}

	csi = avd_csi_struc_find(avd_cb, csi_name, TRUE);

	if (csi == AVD_CSI_NULL) {
		if ((arg->req.info.set_req.i_param_val.i_param_id == saAmfCSIRowStatus_ID)
		    && (arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT)) {
			/* Invalid row status object */
			return NCSCC_RC_INV_VAL;
		}

		if (test_flag == TRUE) {
			return NCSCC_RC_SUCCESS;
		}

		m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

		csi = avd_csi_struc_crt(avd_cb, csi_name, FALSE);

		m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

		if (csi == AVD_CSI_NULL) {
			/* Invalid instance object */
			return NCSCC_RC_NO_INSTANCE;
		}

	} else {		/* if (csi == AVD_CSI_NULL) */

		/* The record is already available */

		if (arg->req.info.set_req.i_param_val.i_param_id == saAmfCSIRowStatus_ID) {
			/* This is a row status operation */
			if (arg->req.info.set_req.i_param_val.info.i_int == (uns32)csi->row_status) {
				/* row status object is same so nothing to be done. */
				return NCSCC_RC_SUCCESS;
			}

			switch (arg->req.info.set_req.i_param_val.info.i_int) {
			case NCS_ROW_ACTIVE:
				/* validate the structure to see if the row can be made active */

				if ((csi->rank == 0) || (csi->csi_type.length == 0))
					return NCSCC_RC_INV_VAL;

				/* check that the SI is present and is active.
				 */
				/* First get the SI name */
				avsv_cpy_SI_DN_from_DN(&temp_name, &csi_name);

				if (temp_name.length == 0) {
					/* log information error */
					return NCSCC_RC_INV_VAL;
				}

				/* Find the SI and link it to the structure.
				 */

				if ((csi->si = avd_si_struc_find(avd_cb, temp_name, TRUE))
				    == AVD_SI_NULL)
					return NCSCC_RC_INV_VAL;

				if (csi->si->row_status != NCS_ROW_ACTIVE) {
					csi->si = AVD_SI_NULL;
					return NCSCC_RC_INV_VAL;
				}

				if (csi->si->max_num_csi < csi->si->num_csi + 1)
					return NCSCC_RC_INV_VAL;

				/* Check the list of CSI in the SI for any duplicate CSI */
				i_csi = csi->si->list_of_csi;
				while (i_csi) {
					if (m_CMP_HORDER_SANAMET(i_csi->csi_type, csi->csi_type) == 0) {
						return NCSCC_RC_INV_VAL;
					}
					i_csi = i_csi->si_list_of_csi_next;
				}	/* end check for duplicate CSI in SI */

				if (test_flag == TRUE) {
					csi->si = AVD_SI_NULL;
					return NCSCC_RC_SUCCESS;
				}

				/* add to the list of SI  */
				avd_csi_add_si_list(avd_cb, csi);

				/* set the value, checkpoint the entire record.
				 */
				csi->row_status = NCS_ROW_ACTIVE;

				m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(cb, csi, AVSV_CKPT_AVD_CSI_CONFIG);

				/* update the SI information about the CSI */
				csi->si->num_csi++;

				if ((csi->si->max_num_csi == csi->si->num_csi) &&
				    (csi->si->admin_state == NCS_ADMIN_STATE_UNLOCK) &&
				    (avd_cb->init_state == AVD_APP_STATE)) {
					switch (csi->si->sg_of_si->su_redundancy_model) {
					case AVSV_SG_RED_MODL_2N:
						avd_sg_2n_si_func(avd_cb, csi->si);
						break;

					case AVSV_SG_RED_MODL_NWAYACTV:
						avd_sg_nacvred_si_func(avd_cb, csi->si);
						break;

					case AVSV_SG_RED_MODL_NWAY:
						avd_sg_nway_si_func(avd_cb, csi->si);
						break;

					case AVSV_SG_RED_MODL_NPM:
						avd_sg_npm_si_func(avd_cb, csi->si);
						break;

					case AVSV_SG_RED_MODL_NORED:
					default:
						avd_sg_nored_si_func(avd_cb, csi->si);
						break;
					}
				}
				return NCSCC_RC_SUCCESS;
				break;

			case NCS_ROW_NOT_IN_SERVICE:
			case NCS_ROW_DESTROY:

				/* check if it is active currently */

				if (csi->row_status == NCS_ROW_ACTIVE) {

					/* Check to see that the SI of which the CSI is a
					 * part is in admin locked state before
					 * making the row status as not in service or delete 
					 */

					if ((csi->si->admin_state != NCS_ADMIN_STATE_LOCK) ||
					    (csi->si->list_of_sisu != AVD_SU_SI_REL_NULL) ||
					    (csi->list_compcsi != AVD_COMP_CSI_REL_NULL)) {
						/* log information error */
						return NCSCC_RC_INV_VAL;
					}

					if (test_flag == TRUE) {
						return NCSCC_RC_SUCCESS;
					}

					/* we need to delete this csi structure on the
					 * standby AVD.
					 */

					/* check point to the standby AVD that this
					 * record need to be deleted
					 */

					/* decrement the active csi number of this SI */
					csi->si->num_csi--;

					/* inform the avnds that track this csi */
					for (curr = (AVD_PG_CSI_NODE *)m_NCS_DBLIST_FIND_FIRST(&csi->pg_node_list);
					     curr;
					     curr = (AVD_PG_CSI_NODE *)m_NCS_DBLIST_FIND_NEXT(&curr->csi_dll_node))
						avd_snd_pg_upd_msg(cb, curr->node, 0, 0, &csi->name_net);

					m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

					/* remove the csi from the SI list.  */
					avd_csi_del_si_list(avd_cb, csi);

					/* delete the pg-node list */
					avd_pg_csi_node_del_all(cb, csi);

					m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

					m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(avd_cb, csi, AVSV_CKPT_AVD_CSI_CONFIG);

				}
				/* if(csi->row_status == NCS_ROW_ACTIVE) */
				if (test_flag == TRUE) {
					return NCSCC_RC_SUCCESS;
				}

				if (arg->req.info.set_req.i_param_val.info.i_int == NCS_ROW_DESTROY) {
					i_csi_param = csi->list_param;
					while (i_csi_param != AVD_CSI_PARAM_NULL) {
						avd_csi_param_del(csi, i_csi_param);
						i_csi_param = csi->list_param;
					}

					/* delete and free the structure */
					m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

					/* remove the csi from the SI list */
					avd_csi_struc_del(avd_cb, csi);

					m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

					return NCSCC_RC_SUCCESS;

				}
				/* if(arg->req.info.set_req.i_param_val.info.i_int
				   == NCS_ROW_DESTROY) */
				csi->row_status = arg->req.info.set_req.i_param_val.info.i_int;
				return NCSCC_RC_SUCCESS;

				break;
			default:
				m_AVD_LOG_INVALID_VAL_ERROR(arg->req.info.set_req.i_param_val.info.i_int);
				/* Invalid row status object */
				return NCSCC_RC_INV_VAL;
				break;
			}	/* switch(arg->req.info.set_req.i_param_val.info.i_int) */

		}
		/* if(arg->req.info.set_req.i_param_val.i_param_id == ncsCsiRowStatus_ID) */
	}			/* if (csi == AVD_CSI_NULL) */

	/* We now have the csi block */

	if (csi->row_status == NCS_ROW_ACTIVE) {
		/* when row status is active we don't allow any other MIB object to be
		 * modified.
		 */
		return NCSCC_RC_INV_VAL;
	}

	if (test_flag == TRUE) {
		return NCSCC_RC_SUCCESS;
	}

	switch (arg->req.info.set_req.i_param_val.i_param_id) {
	case saAmfCSIRowStatus_ID:
		/* fill the row status value */
		if (arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT) {
			csi->row_status = arg->req.info.set_req.i_param_val.info.i_int;
		}
		break;

	case saAmfCSType_ID:
		csi->csi_type.length = arg->req.info.set_req.i_param_val.i_length;
		memcpy(csi->csi_type.value, arg->req.info.set_req.i_param_val.info.i_oct, csi->csi_type.length);
		break;
	default:
		memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

		temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP;
		temp_mib_req.info.i_set_util_info.param = &(arg->req.info.set_req.i_param_val);
		temp_mib_req.info.i_set_util_info.var_info = var_info;
		temp_mib_req.info.i_set_util_info.data = csi;
		temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

		/* call the mib routine handler */
		if ((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) {
			return rc;
		}
		break;
	}			/* switch(arg->req.info.set_req.i_param_val.i_param_id) */

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfcsitableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_C_S_I_TABLE_ENTRY_ID table. This is the CSI table. The
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

uns32 saamfcsitableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
			      NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len)
{
	AVD_CL_CB *avd_cb = (AVD_CL_CB *)cb;
	AVD_CSI *csi;
	SaNameT csi_name;
	uns32 i;

	if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK) {
		/* Invalid operation */
		return NCSCC_RC_NO_INSTANCE;
	}

	memset(&csi_name, '\0', sizeof(SaNameT));

	/* Prepare the CSI database key from the instant ID */
	if (arg->i_idx.i_inst_len != 0) {
		csi_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
		for (i = 0; i < csi_name.length; i++) {
			csi_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i + 1]);
		}
	}

	csi = avd_csi_struc_find_next(avd_cb, csi_name, TRUE);

	if (csi == AVD_CSI_NULL) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}

	/* Prepare the instant ID from the CSI name */

	*next_inst_id_len = m_NCS_OS_NTOHS(csi->name_net.length) + 1;

	next_inst_id[0] = *next_inst_id_len - 1;
	for (i = 0; i < next_inst_id[0]; i++) {
		next_inst_id[i + 1] = (uns32)(csi->name_net.value[i]);
	}

	*data = (NCSCONTEXT)csi;

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfcsitableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_C_S_I_TABLE_ENTRY_ID table. This is the CSI table. The
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

uns32 saamfcsitableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
				NCSMIB_SETROW_PARAM_VAL *params,
				struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag)
{
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfcsinamevaluetableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_C_S_I_NAME_VALUE_TABLE_ENTRY_ID table. This is the CSI attribute value 
 * table. The name of this function is generated by the MIBLIB tool. This function
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

uns32 saamfcsinamevaluetableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
	AVD_CL_CB *avd_cb = (AVD_CL_CB *)cb;
	AVD_CSI *csi;
	SaNameT csi_name;
	SaNameT param_name;
	uns32 i;
	uns32 *inst_ptr;
	AVD_CSI_PARAM *param;

	if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK) {
		/* Invalid operation */
		return NCSCC_RC_NO_INSTANCE;
	}

	memset(&csi_name, '\0', sizeof(SaNameT));

	/* Prepare the CSI database key from the instant ID */
	csi_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];

	inst_ptr = (uns32 *)(arg->i_idx.i_inst_ids + 1);
	for (i = 0; i < csi_name.length; i++) {
		csi_name.value[i] = (uns8)(inst_ptr[i]);
	}
	inst_ptr = inst_ptr + csi_name.length;

	csi = avd_csi_struc_find(avd_cb, csi_name, TRUE);

	if (csi == AVD_CSI_NULL) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}

	/* Now find the param value structure for the CSI */

	/* prepare the param_name from the instance ID */
	memset(&param_name, '\0', sizeof(SaNameT));
	param_name.length = (SaUint16T)inst_ptr[0];
	inst_ptr = inst_ptr + 1;
	for (i = 0; i < param_name.length; i++) {
		param_name.value[i] = (uns8)(inst_ptr[i]);
	}

	param = avd_csi_find_param(csi, param_name);

	if (param == AVD_CSI_PARAM_NULL) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}
	*data = (NCSCONTEXT)param;

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfcsinamevaluetableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_AMF_C_S_I_NAME_VALUE_TABLE_ENTRY_ID table. This is the CSI attribute value 
 * table. The name of this function is generated by the MIBLIB tool. This function
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

uns32 saamfcsinamevaluetableentry_extract(NCSMIB_PARAM_VAL *param,
					  NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer)
{
	AVD_CSI_PARAM *csi_param = (AVD_CSI_PARAM *)data;

	if (csi_param == AVD_CSI_PARAM_NULL) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}

	switch (param->i_param_id) {
	case saAmfCSINameValueParamValue_ID:
		m_AVSV_LENVAL_TO_PARAM(param, buffer, csi_param->param.value);
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
 * Function: saamfcsinamevaluetableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_C_S_I_NAME_VALUE_TABLE_ENTRY_ID table. This is the CSI attribute value 
 * table. The name of this function is generated by the MIBLIB tool. This function
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

uns32 saamfcsinamevaluetableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag)
{
	AVD_CL_CB *avd_cb = (AVD_CL_CB *)cb;
	AVD_CSI *csi;
	SaNameT csi_name;
	SaNameT param_name;
	uns32 i;
	uns32 *inst_ptr;
	AVD_CSI_PARAM *param;

	if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK) {
		/* Invalid operation */
		return NCSCC_RC_NO_INSTANCE;
	}

	memset(&csi_name, '\0', sizeof(SaNameT));

	/* Prepare the CSI database key from the instant ID */
	csi_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];

	inst_ptr = (uns32 *)(arg->i_idx.i_inst_ids + 1);
	for (i = 0; i < csi_name.length; i++) {
		csi_name.value[i] = (uns8)(inst_ptr[i]);
	}

	csi = avd_csi_struc_find(avd_cb, csi_name, TRUE);

	if (csi == AVD_CSI_NULL) {
		/* The row was not found */
		if (avd_cb->init_state != AVD_CFG_READY) {
			return NCSCC_RC_NO_INSTANCE;
		}
		m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

		csi = avd_csi_struc_crt(avd_cb, csi_name, FALSE);

		m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);
		if (csi == AVD_CSI_NULL) {
			return NCSCC_RC_NO_INSTANCE;
		}
	}

	if (csi->row_status == NCS_ROW_ACTIVE)
		return NCSCC_RC_INV_VAL;

	/* Now find the param value structure for the CSI */

	inst_ptr = inst_ptr + csi_name.length;

	/* prepare the param_name from the instance ID */
	memset(&param_name, '\0', sizeof(SaNameT));
	param_name.length = (SaUint16T)inst_ptr[0];
	for (i = 0; i < param_name.length; i++) {
		param_name.value[i] = (uns8)(inst_ptr[i + 1]);
	}

	param = avd_csi_find_param(csi, param_name);

	if (param == AVD_CSI_PARAM_NULL) {
		if ((arg->req.info.set_req.i_param_val.i_param_id == saAmfCSINameValueRowStatus_ID)
		    && (arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT)) {
			/* Invalid row status object */
			return NCSCC_RC_INV_VAL;
		}

		if (test_flag == TRUE) {
			return NCSCC_RC_SUCCESS;
		}

		m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

		param = avd_csi_param_crt(csi, param_name);

		m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

		if (param == AVD_CSI_PARAM_NULL) {
			/* Invalid instance object */
			return NCSCC_RC_NO_INSTANCE;
		}

	} else {		/* if (param == AVD_CSI_PARAM_NULL) */

		/* The record is already available */

		if (arg->req.info.set_req.i_param_val.i_param_id == saAmfCSINameValueRowStatus_ID) {
			/* This is a row status operation */
			if (arg->req.info.set_req.i_param_val.info.i_int == (uns32)param->row_status) {
				/* row status object is same so nothing to be done. */
				return NCSCC_RC_SUCCESS;
			}

			switch (arg->req.info.set_req.i_param_val.info.i_int) {
			case NCS_ROW_ACTIVE:
				/* validate the structure to see if the row can be made active */

				if (param->param.value.length == 0) {
					/* All the mandatory fields are not filled
					 */
					/* log information error */
					return NCSCC_RC_INV_VAL;
				}

				if (avd_cb->init_state != AVD_CFG_READY) {
					if (avd_cs_type_param_find_match(avd_cb, csi->csi_type, param->param.name) ==
					    NCSCC_RC_FAILURE) {
						return NCSCC_RC_INV_VAL;
					}
				}

				if (test_flag == TRUE) {
					return NCSCC_RC_SUCCESS;
				}

				/* set the value, checkpoint the entire record.
				 */
				param->row_status = NCS_ROW_ACTIVE;
				csi->num_active_params++;

				return NCSCC_RC_SUCCESS;
				break;

			case NCS_ROW_NOT_IN_SERVICE:
			case NCS_ROW_DESTROY:

				if (test_flag == TRUE) {
					return NCSCC_RC_SUCCESS;
				}

				if (arg->req.info.set_req.i_param_val.info.i_int == NCS_ROW_DESTROY) {
					/* delete and free the structure */
					m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

					avd_csi_param_del(csi, param);

					m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

					return NCSCC_RC_SUCCESS;
				}

				csi->num_active_params--;
				param->row_status = arg->req.info.set_req.i_param_val.info.i_int;
				return NCSCC_RC_SUCCESS;

				break;
			default:
				m_AVD_LOG_INVALID_VAL_ERROR(arg->req.info.set_req.i_param_val.info.i_int);
				/* Invalid row status object */
				return NCSCC_RC_INV_VAL;
				break;
			}	/* switch(arg->req.info.set_req.i_param_val.info.i_int) */

		}		/* if(arg->req.info.set_req.i_param_val.i_param_id == ROWSTATUS) */
	}

	/* We now have the CSI_PARAM block */

	if (param->row_status == NCS_ROW_ACTIVE) {
		/* when row status is active we don't allow any other MIB object to be
		 * modified.
		 */
		return NCSCC_RC_INV_VAL;
	}

	if (test_flag == TRUE) {
		return NCSCC_RC_SUCCESS;
	}

	switch (arg->req.info.set_req.i_param_val.i_param_id) {
	case saAmfCSINameValueRowStatus_ID:
		/* fill the row status value */
		if (arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT) {
			param->row_status = arg->req.info.set_req.i_param_val.info.i_int;
		}
		break;
	case saAmfCSINameValueParamValue_ID:
		memset(&param->param.value, '\0', sizeof(SaNameT));
		param->param.value.length = arg->req.info.set_req.i_param_val.i_length;
		memcpy(param->param.value.value,
		       arg->req.info.set_req.i_param_val.info.i_oct, param->param.value.length);
		break;
	default:
		break;
	}			/* switch(arg->req.info.set_req.i_param_val.i_param_id) */

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfcsinamevaluetableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_C_S_I_NAME_VALUE_TABLE_ENTRY_ID table. This is the CSI attribute value 
 * table. The name of this function is generated by the MIBLIB tool. This function
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

uns32 saamfcsinamevaluetableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
				       NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len)
{
	AVD_CL_CB *avd_cb = (AVD_CL_CB *)cb;
	AVD_CSI *csi = AVD_CSI_NULL;
	SaNameT csi_name;
	SaNameT param_name;
	uns32 i = 0, x;
	uns32 *inst_ptr;
	AVD_CSI_PARAM *param;

	if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK) {
		/* Invalid operation */
		return NCSCC_RC_NO_INSTANCE;
	}

	memset(&csi_name, '\0', sizeof(SaNameT));
	memset(&param_name, '\0', sizeof(SaNameT));

	if (arg->i_idx.i_inst_len != 0) {
		/* Prepare the CSI database key from the instant ID */
		csi_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];

		inst_ptr = (uns32 *)(arg->i_idx.i_inst_ids + 1);
		for (i = 0; i < csi_name.length; i++) {
			csi_name.value[i] = (uns8)(inst_ptr[i]);
		}

		if (arg->i_idx.i_inst_len > csi_name.length + 1) {
			inst_ptr = inst_ptr + i;
			/* prepare the param_name from the instance ID */
			param_name.length = (SaUint16T)inst_ptr[0];
			inst_ptr = inst_ptr + 1;
			for (i = 0; i < param_name.length; i++) {
				param_name.value[i] = (uns8)(inst_ptr[i]);
			}
		}
	}
	param = avd_csi_find_next_param(cb, &csi, csi_name, param_name);

	if ((param == AVD_CSI_PARAM_NULL) || (csi == AVD_CSI_NULL)) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}

	*next_inst_id_len = m_NCS_OS_NTOHS(csi->name_net.length)
	    + param->param.name.length + 2;

	next_inst_id[0] = m_NCS_OS_NTOHS(csi->name_net.length);
	for (i = 0; i < next_inst_id[0]; i++) {
		next_inst_id[i + 1] = (uns32)(csi->name_net.value[i]);
	}

	next_inst_id[i + 1] = param->param.name.length;
	for (x = 0; x < param->param.name.length; x++) {
		next_inst_id[i + x + 2] = (uns32)(param->param.name.value[x]);
	}

	*data = (NCSCONTEXT)param;

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfcsinamevaluetableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_C_S_I_NAME_VALUE_TABLE_ENTRY_ID table. This is the CSI attribute value 
 * table. The name of this function is generated by the MIBLIB tool. This function
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

uns32 saamfcsinamevaluetableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
					 NCSMIB_SETROW_PARAM_VAL *params,
					 struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag)
{
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_cs_type_param_find_match
 *
 * Purpose:  This function will find a match of cst type name and param name in 
 *            CSTypeParam table. 
 *
 * Input: cb         - the AVD control block
 *        csi_type   - csi type name
 *        param_name - param name.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE  
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_cs_type_param_find_match(AVD_CL_CB *cb, SaNameT csi_type, SaNameT param_name)
{
	AVD_CS_TYPE_PARAM *type_param = AVD_CS_TYPE_PARAM_NULL;
	AVD_CS_TYPE_PARAM_INDX indx;

	memset((char *)&indx, '\0', sizeof(AVD_CS_TYPE_PARAM_INDX));

	indx.type_name_net.length = m_NCS_OS_HTONS(csi_type.length);

	memcpy(indx.type_name_net.value, csi_type.value, csi_type.length);

	indx.param_name_net.length = m_NCS_OS_HTONS(param_name.length);

	memcpy(indx.param_name_net.value, param_name.value, param_name.length);

	type_param = avd_cs_type_param_struc_find(cb, indx);

	if (type_param == AVD_CS_TYPE_PARAM_NULL) {
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;

}

/*****************************************************************************
 * Function: avd_cs_type_param_struc_crt
 *
 * Purpose:  This function will create and add a AVD_CS_TYPE_PARAM structure to the
 * tree if an element with with same csi type name and param name doesn't exist in 
 * the tree.
 *
 * Input: cb   - the AVD control block
 *        indx - the key information 
 *               
 *
 * Returns: The pointer to AVD_CS_TYPE_PARAM structure allocated and added.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_CS_TYPE_PARAM *avd_cs_type_param_struc_crt(AVD_CL_CB *cb, AVD_CS_TYPE_PARAM_INDX indx)
{
	AVD_CS_TYPE_PARAM *type_param = AVD_CS_TYPE_PARAM_NULL;

	/* Allocate a new block structure now
	 */
	if ((type_param = m_MMGR_ALLOC_AVD_CS_TYPE_PARAM) == AVD_CS_TYPE_PARAM_NULL) {
		/* log an error */
		m_AVD_LOG_MEM_FAIL(AVD_CS_TYPE_PARAM_ALLOC_FAILED);
		return AVD_CS_TYPE_PARAM_NULL;
	}

	memset((char *)type_param, '\0', sizeof(AVD_CS_TYPE_PARAM));

	type_param->indx.type_name_net.length = indx.type_name_net.length;
	memcpy(type_param->indx.type_name_net.value, indx.type_name_net.value,
	       m_NCS_OS_NTOHS(indx.type_name_net.length));

	type_param->indx.param_name_net.length = indx.param_name_net.length;
	memcpy(type_param->indx.param_name_net.value, indx.param_name_net.value,
	       m_NCS_OS_NTOHS(indx.param_name_net.length));

	type_param->row_status = NCS_ROW_NOT_READY;

	type_param->tree_node.key_info = (uns8 *)(&type_param->indx);
	type_param->tree_node.bit = 0;
	type_param->tree_node.left = NCS_PATRICIA_NODE_NULL;
	type_param->tree_node.right = NCS_PATRICIA_NODE_NULL;

	if (ncs_patricia_tree_add(&cb->cs_type_param_anchor, &type_param->tree_node)
	    != NCSCC_RC_SUCCESS) {
		/* log an error */
		m_MMGR_FREE_AVD_CS_TYPE_PARAM(type_param);
		return AVD_CS_TYPE_PARAM_NULL;
	}

	return type_param;

}

/*****************************************************************************
 * Function: avd_cs_type_param_struc_find
 *
 * Purpose:  This function will find a AVD_CS_TYPE_PARAM structure in the
 * tree with csi type name and param name as key.
 *
 * Input: cb   - the AVD control block
 *        indx - the key information.
 *
 * Returns: The pointer to AVD_CS_TYPE_PARAM structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_CS_TYPE_PARAM *avd_cs_type_param_struc_find(AVD_CL_CB *cb, AVD_CS_TYPE_PARAM_INDX indx)
{
	AVD_CS_TYPE_PARAM *type_param = AVD_CS_TYPE_PARAM_NULL;
	type_param = (AVD_CS_TYPE_PARAM *)ncs_patricia_tree_get(&cb->cs_type_param_anchor, (uns8 *)&indx);

	return type_param;
}

/*****************************************************************************
 * Function: avd_cs_type_param_struc_find_next
 *
 * Purpose:  This function will find the next AVD_CS_TYPE_PARAM structure in the
 * tree whose csi type name and param name is next of the given.
 *
 * Input: cb   - the AVD control block
 *        indx - the key information.
 *
 * Returns: The pointer to AVD_CS_TYPE_PARAM structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_CS_TYPE_PARAM *avd_cs_type_param_struc_find_next(AVD_CL_CB *cb, AVD_CS_TYPE_PARAM_INDX indx)
{
	AVD_CS_TYPE_PARAM *type_param = AVD_CS_TYPE_PARAM_NULL;
	type_param = (AVD_CS_TYPE_PARAM *)ncs_patricia_tree_getnext(&cb->cs_type_param_anchor, (uns8 *)&indx);

	return type_param;
}

/*****************************************************************************
 * Function: avd_cs_type_param_struc_del
 *
 * Purpose:  This function will delete and free AVD_CS_TYPE_PARAM structure from 
 * the tree.
 *
 * Input: cb         - the AVD control block
 *        type_param - The structure need to be deleted.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE  
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_cs_type_param_struc_del(AVD_CL_CB *cb, AVD_CS_TYPE_PARAM *type_param)
{
	if (type_param == AVD_CS_TYPE_PARAM_NULL)
		return NCSCC_RC_FAILURE;

	if (ncs_patricia_tree_del(&cb->cs_type_param_anchor, &type_param->tree_node)
	    != NCSCC_RC_SUCCESS) {
		/* log error */
		return NCSCC_RC_FAILURE;
	}

	m_MMGR_FREE_AVD_CS_TYPE_PARAM(type_param);
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfcstypeparamentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_C_S_TYPE_PARAM_ENTRY_ID table. This is the CSI Type Param table. 
 * The name of this function is generated by the MIBLIB tool. This function
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

uns32 saamfcstypeparamentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
	AVD_CL_CB *avd_cb = (AVD_CL_CB *)cb;
	AVD_CS_TYPE_PARAM_INDX indx;
	uns16 p_len;
	uns16 s_len;
	uns32 i;
	AVD_CS_TYPE_PARAM *type_param = AVD_CS_TYPE_PARAM_NULL;

	if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK) {
		/* Invalid operation */
		return NCSCC_RC_NO_INSTANCE;
	}

	memset(&indx, '\0', sizeof(AVD_CS_TYPE_PARAM_INDX));

	/* Prepare the cstype-param database key from the instant ID */

	if (arg->i_idx.i_inst_len != 0) {
		p_len = (SaUint16T)arg->i_idx.i_inst_ids[0];

		indx.type_name_net.length = m_NCS_OS_HTONS(p_len);

		for (i = 0; i < p_len; i++) {
			indx.type_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i + 1]);
		}

		if (arg->i_idx.i_inst_len > p_len + 1) {
			s_len = (SaUint16T)arg->i_idx.i_inst_ids[p_len + 1];

			indx.param_name_net.length = m_NCS_OS_HTONS(s_len);

			for (i = 0; i < s_len; i++) {
				indx.param_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[p_len + 1 + 1 + i]);
			}

		}
	}

	type_param = avd_cs_type_param_struc_find(avd_cb, indx);

	if (type_param == AVD_CS_TYPE_PARAM_NULL) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}

	*data = (NCSCONTEXT)type_param;

	return NCSCC_RC_SUCCESS;

}

/*****************************************************************************
 * Function: saamfcstypeparamentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects 
 * in SA_AMF_C_S_TYPE_PARAM_ENTRY_ID table. This is the CSI Type Param table. 
 * The name of this function is generated by the MIBLIB tool. This function
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

uns32 saamfcstypeparamentry_extract(NCSMIB_PARAM_VAL *param,
				    NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer)
{
	AVD_CS_TYPE_PARAM *type_param = (AVD_CS_TYPE_PARAM *)data;

	if (type_param == AVD_CS_TYPE_PARAM_NULL) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}

	/* call the MIBLIB utility routine for standfard object types */
	if ((var_info != NULL) && (var_info->offset != 0)) {
		return ncsmiblib_get_obj_val(param, var_info, data, buffer);
	}

	return NCSCC_RC_NO_OBJECT;
}

/*****************************************************************************
 * Function: saamfcstypeparamentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_C_S_TYPE_PARAM_ENTRY_ID table. This is the CSI Type Param table. 
 * The name of this function is generated by the MIBLIB tool. This function
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

uns32 saamfcstypeparamentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag)
{
	AVD_CL_CB *avd_cb = (AVD_CL_CB *)cb;
	AVD_CS_TYPE_PARAM_INDX indx;
	uns16 p_len;
	uns16 s_len;
	uns32 i;
	AVD_CS_TYPE_PARAM *type_param = AVD_CS_TYPE_PARAM_NULL;

	if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK) {
		/* Invalid operation */
		return NCSCC_RC_NO_INSTANCE;
	}

	memset(&indx, '\0', sizeof(AVD_CS_TYPE_PARAM_INDX));

	/* Prepare the cstype-param database key from the instant ID */

	if (arg->i_idx.i_inst_len != 0) {
		p_len = (SaUint16T)arg->i_idx.i_inst_ids[0];

		indx.type_name_net.length = m_NCS_OS_HTONS(p_len);

		for (i = 0; i < p_len; i++) {
			indx.type_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i + 1]);
		}

		if (arg->i_idx.i_inst_len > p_len + 1) {
			s_len = (SaUint16T)arg->i_idx.i_inst_ids[p_len + 1];

			indx.param_name_net.length = m_NCS_OS_HTONS(s_len);

			for (i = 0; i < s_len; i++) {
				indx.param_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[p_len + 1 + 1 + i]);
			}

		}
	}

	type_param = avd_cs_type_param_struc_find(avd_cb, indx);

	if (type_param == AVD_CS_TYPE_PARAM_NULL) {
		/* The row was not found */
		if ((arg->req.info.set_req.i_param_val.i_param_id == saAmfCSTypeParamRowStatus_ID)
		    && (arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT)
		    && (arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_GO)) {
			if ((arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_ACTIVE)
			    || (avd_cb->init_state >= AVD_CFG_DONE)) {
				/* Invalid row status object */
				return NCSCC_RC_INV_VAL;
			}
		}

		if (test_flag == TRUE) {
			return NCSCC_RC_SUCCESS;
		}

		m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

		type_param = avd_cs_type_param_struc_crt(avd_cb, indx);

		m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

		if (type_param == AVD_CS_TYPE_PARAM_NULL) {
			/* Invalid instance object */
			return NCSCC_RC_NO_INSTANCE;
		}

	} else {		/* The record is already available */

		if (arg->req.info.set_req.i_param_val.i_param_id == saAmfCSTypeParamRowStatus_ID) {
			/* This is a row status operation */
			if (arg->req.info.set_req.i_param_val.info.i_int == (uns32)type_param->row_status) {
				/* row status object is same so nothing to be done. */
				return NCSCC_RC_SUCCESS;
			}
			if (test_flag == TRUE) {
				return NCSCC_RC_SUCCESS;
			}

			switch (arg->req.info.set_req.i_param_val.info.i_int) {
			case NCS_ROW_ACTIVE:

				type_param->row_status = arg->req.info.set_req.i_param_val.info.i_int;
				m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(cb, type_param, AVSV_CKPT_AVD_CS_TYPE_PARAM_CONFIG);
				return NCSCC_RC_SUCCESS;
				break;

			case NCS_ROW_NOT_IN_SERVICE:

				if (type_param->row_status == NCS_ROW_ACTIVE) {
					m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, type_param,
									AVSV_CKPT_AVD_CS_TYPE_PARAM_CONFIG);
				}

				type_param->row_status = arg->req.info.set_req.i_param_val.info.i_int;

				return NCSCC_RC_SUCCESS;
				break;

			case NCS_ROW_DESTROY:

				if (type_param->row_status == NCS_ROW_ACTIVE) {
					m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, type_param,
									AVSV_CKPT_AVD_CS_TYPE_PARAM_CONFIG);
				}

				/* delete and free the structure */
				m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

				avd_cs_type_param_struc_del(avd_cb, type_param);

				m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

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

	if (test_flag == TRUE) {
		return NCSCC_RC_SUCCESS;
	}

	if (arg->req.info.set_req.i_param_val.i_param_id == saAmfCSTypeParamRowStatus_ID) {
		/* fill the row status value */
		if ((arg->req.info.set_req.i_param_val.info.i_int == NCS_ROW_CREATE_AND_GO)
		    || (arg->req.info.set_req.i_param_val.info.i_int == NCS_ROW_ACTIVE)) {
			type_param->row_status = NCS_ROW_ACTIVE;
			m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(cb, type_param, AVSV_CKPT_AVD_CS_TYPE_PARAM_CONFIG);
		}

	} else {
		/* Invalid Object ID */
		return NCSCC_RC_INV_VAL;
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfcstypeparamentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_C_S_TYPE_PARAM_ENTRY_ID table. This is the CSI Type Param table. 
 * The name of this function is generated by the MIBLIB tool. This function
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

uns32 saamfcstypeparamentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
				 NCSCONTEXT *data, uns32 *next_inst_id, uns32 *next_inst_id_len)
{
	AVD_CL_CB *avd_cb = (AVD_CL_CB *)cb;
	AVD_CS_TYPE_PARAM_INDX indx;
	uns16 p_len;
	uns16 s_len;
	uns32 i;
	AVD_CS_TYPE_PARAM *type_param = AVD_CS_TYPE_PARAM_NULL;

	if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK) {
		/* Invalid operation */
		return NCSCC_RC_NO_INSTANCE;
	}

	memset(&indx, '\0', sizeof(AVD_CS_TYPE_PARAM_INDX));

	/* Prepare the cstype-param database key from the instant ID */

	if (arg->i_idx.i_inst_len != 0) {
		p_len = (SaUint16T)arg->i_idx.i_inst_ids[0];

		indx.type_name_net.length = m_NCS_OS_HTONS(p_len);

		for (i = 0; i < p_len; i++) {
			indx.type_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i + 1]);
		}

		if (arg->i_idx.i_inst_len > p_len + 1) {
			s_len = (SaUint16T)arg->i_idx.i_inst_ids[p_len + 1];

			indx.param_name_net.length = m_NCS_OS_HTONS(s_len);

			for (i = 0; i < s_len; i++) {
				indx.param_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[p_len + 1 + 1 + i]);
			}

		}
	}

	type_param = avd_cs_type_param_struc_find_next(avd_cb, indx);

	if (type_param == AVD_CS_TYPE_PARAM_NULL) {
		/* The row was not found */
		return NCSCC_RC_NO_INSTANCE;
	}

	/* Prepare the instant ID from the csi type name and param name */

	p_len = m_NCS_OS_NTOHS(type_param->indx.type_name_net.length);
	s_len = m_NCS_OS_NTOHS(type_param->indx.param_name_net.length);

	*next_inst_id_len = p_len + 1 + s_len + 1;

	next_inst_id[0] = p_len;
	for (i = 0; i < p_len; i++) {
		next_inst_id[i + 1] = (uns32)(type_param->indx.type_name_net.value[i]);
	}

	next_inst_id[p_len + 1] = s_len;
	for (i = 0; i < s_len; i++) {
		next_inst_id[p_len + 1 + i + 1] = (uns32)(type_param->indx.param_name_net.value[i]);
	}

	*data = (NCSCONTEXT)type_param;

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfcstypeparamentry_setrow
 * 
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_C_S_TYPE_PARAM_ENTRY_ID table. This is the CSI Type Param table. 
 * The name of this function is generated by the MIBLIB tool. This function
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

uns32 saamfcstypeparamentry_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
				   NCSMIB_SETROW_PARAM_VAL *params,
				   struct ncsmib_obj_info *obj_info, NCS_BOOL testrow_flag)
{
	return NCSCC_RC_SUCCESS;
}
