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

  DESCRIPTION: This module contain all the supporting functions for the
               checkpointing operation. 

..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "dts.h"
#include "dts_imm.h"

static uint32_t dts_stby_global_filtering_policy_change(DTS_CB *cb);

/****************************************************************************\
 * Function: dtsv_ckpt_add_rmv_updt_dta_dest
 *
 * Purpose:  Add new DTA entry if action is ADD, remove node from the tree if 
 *           action is to remove and update data if request is to update.
 *
 * Input: cb  - CB pointer.
 *        avnd - Decoded structur.
 *        action - ADD/RMV/UPDT
 *        key - SVC_KEY of the service associated with DTA being added/updated
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: ADD will add to the Patricia tree. ADD also modifies svc_reg patricia 
 *        tree by adding to svc_reg's v_cd_list.
 *        UPDT is not used.
 *        RMV will remove dta frm patricia tree and modify svc_reg patricia tree *        only on DTA going down.         
 *        Service de-regs are handled by svc_reg async updates.
 *        
 *        As per this IR param key is not passed in network order    
\**************************************************************************/
uint32_t dtsv_ckpt_add_rmv_updt_dta_dest(DTS_CB *cb, DTA_DEST_LIST *dtadest, NCS_MBCSV_ACT_TYPE action, SVC_KEY key)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	DTA_DEST_LIST *to_reg = NULL, *del_reg = NULL;
	DTS_SVC_REG_TBL *svc = NULL;
	OP_DEVICE *device = NULL;
	MDS_DEST dta_key;
	SVC_ENTRY *svc_entry = NULL;
	SVC_KEY svc_key = {0, 0}, nt_key = {0, 0};

	if (dtadest == NULL) {
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_ckpt_add_rmv_updt_dta_dest: DTA_DEST_LIST ptr is NULL");
	}

	dta_key = dtadest->dta_addr;
	switch (action) {
	case NCS_MBCSV_ACT_ADD:
		{
			SYSF_ASCII_SPECS *spec_entry = NULL;
			ASCII_SPEC_INDEX spec_key;
			SPEC_ENTRY *per_dta_svc_spec = NULL;
			ASCII_SPEC_LIB *lib_hdl = NULL;

			/*  Network order key added */
			nt_key.node = m_NCS_OS_HTONL(key.node);
			nt_key.ss_svc_id = m_NCS_OS_HTONL(key.ss_svc_id);

			m_DTS_LK(&cb->lock);
			m_LOG_DTS_LOCK(DTS_LK_LOCKED, &cb->lock);

			/* Find svc entry in  svc_tbl patricia tree */
			if ((svc =
			     (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&cb->svc_tbl, (const uint8_t *)&nt_key)) == NULL) {
				m_DTS_UNLK(&cb->lock);
				m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
				m_LOG_DTS_EVT(DTS_EV_SVC_REG_NOTFOUND, key.ss_svc_id, key.node, (uint32_t)dta_key);
				/*Service should have already been added to patricia tree */
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dtsv_ckpt_add_rmv_updt_dta_dest: No service entry in Patricia Tree");
				/*For cold-sync also, svc_reg cold sync has to happen prior to this */
			}

			/* Check if dta is already present in dta_dest patricia tree */
			if ((to_reg =
			     (DTA_DEST_LIST *)ncs_patricia_tree_get(&cb->dta_list, (const uint8_t *)&dta_key)) == NULL) {
				to_reg = m_MMGR_ALLOC_VCARD_TBL;
				if (to_reg == NULL) {
					m_DTS_UNLK(&cb->lock);
					m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
					return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							      "dtsv_ckpt_add_rmv_updt_dta_dest: Memory allocation failed");
				}
				memset(to_reg, '\0', sizeof(DTA_DEST_LIST));
				/* Update the fields of DTA_DEST_LIST */
				to_reg->dta_addr = dtadest->dta_addr;
				to_reg->dta_up = dtadest->dta_up;
				to_reg->updt_req = dtadest->updt_req;
				to_reg->dta_num_svcs = 0;
				/*ncs_create_queue(&to_reg->svc_list); */
				to_reg->node.key_info = (uint8_t *)&to_reg->dta_addr;
				to_reg->svc_list = NULL;

				if (ncs_patricia_tree_add(&cb->dta_list, (NCS_PATRICIA_NODE *)&to_reg->node) !=
				    NCSCC_RC_SUCCESS) {
					m_DTS_UNLK(&cb->lock);
					m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
					m_LOG_DTS_EVT(DTS_EV_DTA_DEST_ADD_FAIL, 0, m_NCS_NODE_ID_FROM_MDS_DEST(dta_key),
						      (uint32_t)dta_key);
					if (NULL != to_reg)
						m_MMGR_FREE_VCARD_TBL(to_reg);
					return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							      "dtsv_ckpt_add_rmv_updt_dta_dest: Failed to add DTA list in Patricia tree");
				}
				m_LOG_DTS_EVT(DTS_EV_DTA_DEST_ADD_SUCC, 0, m_NCS_NODE_ID_FROM_MDS_DEST(dta_key),
					      (uint32_t)dta_key);
			} else {
				/* Not an error condition - DTA entry might already exist */
				/* Adjust the pointer to to_reg with the offset */
				to_reg = (DTA_DEST_LIST *)((long)to_reg - DTA_DEST_LIST_OFFSET);

				/* Also check if dta-svc relationship already exists */
				/* If it does then just break frm this case & return */
				if (dts_find_dta(svc, &dta_key) != NULL) {
					m_DTS_UNLK(&cb->lock);
					m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
					return status;
				}
			}

			/* If svc entry found, just add it to the dta */
			dts_add_svc_to_dta(to_reg, svc);
			m_LOG_DTS_EVT(DTS_EV_DTA_SVC_ADD, key.ss_svc_id, key.node, (uint32_t)dta_key);

			/* Now add dta into the svc->v_cd_list */
			dts_enqueue_dta(svc, to_reg);
			m_LOG_DTS_EVT(DTS_EV_SVC_DTA_ADD, key.ss_svc_id, key.node, (uint32_t)dta_key);

			/* Version support : Call to function to load ASCII_SPEC
			 * library. This function will load all versioning enabled
			 * ASCII_SPEC libraries.
			 */

			/* First check whether CB last_spec_loaded has valid svc_name or not
			 * If not, then break else continue to load library 
			 */
			if (*cb->last_spec_loaded.svc_name == 0) {
				m_DTS_UNLK(&cb->lock);
				m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
				break;
			}

			lib_hdl =
			    (ASCII_SPEC_LIB *)dts_ascii_spec_load(cb->last_spec_loaded.svc_name,
								  cb->last_spec_loaded.version, 1);

			/* memset the spec_key first before filling it up */
			memset(&spec_key, '\0', sizeof(ASCII_SPEC_INDEX));
			spec_key.svc_id = svc->ntwk_key.ss_svc_id;
			spec_key.ss_ver = cb->last_spec_loaded.version;
			/*Check if ASCII_SPEC table for this service is already loaded.
			 * If no, read the config file */
			if ((spec_entry =
			     (SYSF_ASCII_SPECS *)ncs_patricia_tree_get(&cb->svcid_asciispec_tree,
								       (const uint8_t *)&spec_key)) == NULL) {
				/* If ASCII_SPEC table is versioning enabled, this means
				 * service name & version specified was incorrect.
				 */
				dts_log(NCSFL_SEV_ERROR, "\n Service cribbing : %d\n", svc->my_key.ss_svc_id);
				fflush(stdout);
				m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					       "dtsv_ckpt_add_rmv_updt_dta_dest: ASCII_SPEC library couldn't be loaded. Check service name & version no. across ASCII_SPEC tables, registration parameters & library name.");
			} else {
				/* Add an ASCII_SPEC entry for each service registration.
				 * So any log message will directly index to this entry and 
				 * get the relevant spec pointer to use.
				 */
				per_dta_svc_spec = m_MMGR_ALLOC_DTS_SVC_SPEC;
				if (per_dta_svc_spec == NULL) {
					m_DTS_UNLK(&cb->lock);
					m_LOG_DTS_EVT(DTS_EV_DTA_DEST_ADD_FAIL, 0, m_NCS_NODE_ID_FROM_MDS_DEST(dta_key),
						      (uint32_t)dta_key);
					/* Do rest of cleanup, cleaning service regsitration table etc */
					return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							      "dtsv_ckpt_add_rmv_updt_dta_dest: Memory allocation failed");
				}
				memset(per_dta_svc_spec, '\0', sizeof(SPEC_ENTRY));
				per_dta_svc_spec->dta_addr = dta_key;
				per_dta_svc_spec->spec_struct = spec_entry;
				per_dta_svc_spec->lib_struct = lib_hdl;
				strcpy(per_dta_svc_spec->svc_name, cb->last_spec_loaded.svc_name);

				/* Add to the svc reg tbl's spec list */
				per_dta_svc_spec->next_spec_entry = svc->spec_list;	/* point next to the rest of list */
				svc->spec_list = per_dta_svc_spec;	/*Add new struct to start of list */

				/* Increment use count */
				spec_entry->use_count++;
			}

			m_DTS_UNLK(&cb->lock);
			m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
		}
		break;

		/* Yet not using UPDT for DTA_DEST_LIST */
	case NCS_MBCSV_ACT_UPDATE:
		{
			m_DTS_LK(&cb->lock);
			m_LOG_DTS_LOCK(DTS_LK_LOCKED, &cb->lock);
			if ((to_reg =
			     (DTA_DEST_LIST *)ncs_patricia_tree_get(&cb->dta_list, (const uint8_t *)&dta_key)) == NULL) {
				m_DTS_UNLK(&cb->lock);
				m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
				return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						      "dtsv_ckpt_add_rmv_updt_dta_dest: NCS_MBCSV_ACT_UPDATE - DTA entry does not exist in Patricia Tree");
			}
			to_reg = (DTA_DEST_LIST *)((long)to_reg - DTA_DEST_LIST_OFFSET);
			to_reg->dta_addr = dtadest->dta_addr;
			to_reg->dta_up = dtadest->dta_up;
			to_reg->updt_req = dtadest->updt_req;
			m_DTS_UNLK(&cb->lock);
			m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
		}
		break;

	case NCS_MBCSV_ACT_RMV:
		{
			m_DTS_LK(&cb->lock);
			m_LOG_DTS_LOCK(DTS_LK_LOCKED, &cb->lock);
			if ((to_reg =
			     (DTA_DEST_LIST *)ncs_patricia_tree_get(&cb->dta_list, (const uint8_t *)&dta_key)) != NULL) {
				to_reg = (DTA_DEST_LIST *)((long)to_reg - DTA_DEST_LIST_OFFSET);
				svc_entry = to_reg->svc_list;
				/* Remove dta entry frm the svc->v_cd_lists for all svcs */
				while (svc_entry != NULL) {
					svc = svc_entry->svc;
					if (svc != NULL) {
						svc_key = svc->my_key;
						/*  Network order key added */
						nt_key = svc->ntwk_key;
					}
					if ((svc =
					     (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&cb->svc_tbl,
										      (const uint8_t *)&nt_key)) == NULL) {
						m_DTS_UNLK(&cb->lock);
						m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
						m_LOG_DTS_EVT(DTS_EV_SVC_REG_NOTFOUND, svc_key.ss_svc_id, svc_key.node,
							      (uint32_t)to_reg->dta_addr);
						return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
								      "dtsv_ckpt_add_rmv_updt_dta_dest: Service entry not found in Patricia tree");
					}
					/* remove dta from svc->v_cd_list */
					if ((del_reg = (DTA_DEST_LIST *)dts_dequeue_dta(svc, to_reg)) == NULL) {
						m_LOG_DTS_EVT(DTS_EV_SVC_DTA_RMV_FAIL, svc_key.ss_svc_id, svc_key.node,
							      (uint32_t)to_reg->dta_addr);
						/* Don't return failure, continue rmeoving service frm dta */
						m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							       "dtsv_ckpt_add_rmv_updt_dta_dest: Failed to remove dta entry frm svc->v_cd_list");
					} else
						m_LOG_DTS_EVT(DTS_EV_SVC_DTA_RMV, svc_key.ss_svc_id, svc_key.node,
							      (uint32_t)to_reg->dta_addr);

					/* Versioning support: Remove spec entry corresponding to the 
					 * DTA from svc's spec_list. 
					 */
					if (dts_del_spec_frm_svc(svc, dta_key, NULL) != NCSCC_RC_SUCCESS) {
						m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							       "dtsv_ckpt_add_rmv_updt_dta_dest: Unable to remove spec entry");
					}

					/*Point to next entry before deleting svc frm svc_list */
					svc_entry = svc_entry->next_in_dta_entry;

					/* remove svc from dta->svc_list */
					if ((svc = (DTS_SVC_REG_TBL *)dts_del_svc_frm_dta(to_reg, svc)) == NULL) {
						m_LOG_DTS_EVT(DTS_EV_DTA_SVC_RMV_FAIL, svc_key.ss_svc_id, svc_key.node,
							      (uint32_t)to_reg->dta_addr);
						/* Don't return, continue */
						m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							       "dtsv_ckpt_add_rmv_updt_dta_dest: Failed to delete svc from dta->svc_list");
					} else
						m_LOG_DTS_EVT(DTS_EV_DTA_SVC_RMV, svc_key.ss_svc_id, svc_key.node,
							      (uint32_t)to_reg->dta_addr);

					/* Check if svc entry is required or not */
					if (svc->dta_count == 0) {
						if (&svc->device.cir_buffer != NULL)
							dts_circular_buffer_free(&svc->device.cir_buffer);
						device = &svc->device;
						m_DTS_FREE_FILE_LIST(device);
						ncs_patricia_tree_del(&cb->svc_tbl, (NCS_PATRICIA_NODE *)svc);
						m_LOG_DTS_EVT(DTS_EV_SVC_REG_ENT_RMVD, svc_key.ss_svc_id, svc_key.node,
							      (uint32_t)to_reg->dta_addr);
						if (svc != NULL)
							m_MMGR_FREE_SVC_REG_TBL(svc);
					}
				}	/*end of while svc!=NULL */

				/* Now delete the dta entry */
				if (to_reg->svc_list == NULL) {
					ncs_patricia_tree_del(&cb->dta_list, (NCS_PATRICIA_NODE *)&to_reg->node);
					m_LOG_DTS_EVT(DTS_EV_DTA_DEST_RMV_SUCC, 0,
						      m_NCS_NODE_ID_FROM_MDS_DEST(to_reg->dta_addr),
						      (uint32_t)to_reg->dta_addr);
					if (NULL != to_reg)
						m_MMGR_FREE_VCARD_TBL(to_reg);
				}
			} else {
				/* Not an error condition */
			}
			m_DTS_UNLK(&cb->lock);
			m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
		}
		break;

	default:
		/* Log error */
		status = NCSCC_RC_FAILURE;
	}
	return status;
}

/****************************************************************************\
 * Function: dtsv_ckpt_add_rmv_updt_svc_reg
 *
 * Purpose:  Add new entry if action is ADD, remove from the table if 
 *           action is to remove and update data if request is to update.
 *
 * Input: cb  - CB pointer.
 *        svcreg - pointer to SVC REG table structure.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES: ADD takes care of forming the node entry for DTS_SVC_REG_TBL as well
 *        as forming the individual service node.
 *        ADD just adds svc_reg to DTA Patricia tree.
 *        Hence, DTS_SVC_REG_TBL Async update(ADD) needs to be sent after 
 *        modification on the DTA Patricia tree in active DTS.
 *        UPDT takes care of the changes in policy/filtering settings.
 *        DTS_SVC_REG_TBL ADD happens before DTA_DEST_LIST ADD.
 *        DTS_SVC_REG_TBL RMV takes care of removing svc entries from all DTAs 
 *        in its v_cd_list. 
\**************************************************************************/
uint32_t dtsv_ckpt_add_rmv_updt_svc_reg(DTS_CB *cb, DTS_SVC_REG_TBL *svcreg,
				     NCS_MBCSV_ACT_TYPE action)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	DTS_SVC_REG_TBL *svc_ptr = NULL, *node_reg_ptr = NULL;
	DTA_DEST_LIST *to_reg = NULL;
	SVC_KEY nt_key, key;
	OP_DEVICE *device = NULL;
	DTA_ENTRY *dta_entry = NULL;
	MDS_DEST *vkey = NULL;

	switch (action) {
	case NCS_MBCSV_ACT_ADD:
		{
			/*
			 * add new SVC_REG entry into pat tree.
			 */
			key.node = svcreg->my_key.node;
			key.ss_svc_id = 0;
			/*  Network order key added */
			nt_key.node = m_NCS_OS_HTONL(svcreg->my_key.node);
			nt_key.ss_svc_id = 0;

			m_DTS_LK(&cb->lock);
			m_LOG_DTS_LOCK(DTS_LK_LOCKED, &cb->lock);

			/* Check whether the Node exist in the node registration table
			 * if yes, do nothing else create new entry in table and 
			 * fill the datastructure*/
			if ((node_reg_ptr =
			     (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&cb->svc_tbl, (const uint8_t *)&nt_key)) == NULL) {
				node_reg_ptr = m_MMGR_ALLOC_SVC_REG_TBL;
				if (node_reg_ptr == NULL) {
					m_DTS_UNLK(&cb->lock);
					m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
					return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							      "dtsv_ckpt_add_rmv_updt_svc_reg: Memory allocation failed");
				}
				memset(node_reg_ptr, '\0', sizeof(DTS_SVC_REG_TBL));
				/* Check for NULL value of svcreg parameter, if not NULL 
				   Copy the attributes of the node passed by decoder */
				node_reg_ptr->my_key.node = key.node;
				node_reg_ptr->my_key.ss_svc_id = 0;

				/*  Network order key added */
				node_reg_ptr->ntwk_key = nt_key;
				node_reg_ptr->node.key_info = (uint8_t *)&node_reg_ptr->ntwk_key;
				node_reg_ptr->v_cd_list = NULL;
				node_reg_ptr->dta_count = 0;
				node_reg_ptr->my_node = node_reg_ptr;

				if (svcreg->my_key.ss_svc_id == 0) {	/*updt for node entry */
					/*cold sync update for node entry, set attributes accordingly */
					m_DTS_SET_SVC_REG_TBL(node_reg_ptr, svcreg);
				} else {
					/*async update for new svc_reg, so initialize node param */
					node_reg_ptr->per_node_logging = NODE_LOGGING;

					/* Copy attributes of node policy & op device */
					node_reg_ptr->svc_policy.enable = NODE_ENABLE;
					node_reg_ptr->svc_policy.category_bit_map = NODE_CATEGORY_FILTER;
					node_reg_ptr->svc_policy.severity_bit_map = NODE_SEVERITY_FILTER;
					node_reg_ptr->svc_policy.log_dev = NODE_LOG_DEV;
					node_reg_ptr->svc_policy.log_file_size = NODE_LOGFILE_SIZE;
					node_reg_ptr->svc_policy.file_log_fmt = NODE_FILE_LOG_FMT;
					node_reg_ptr->svc_policy.cir_buff_size = NODE_CIR_BUFF_SIZE;
					node_reg_ptr->svc_policy.buff_log_fmt = NODE_BUFF_LOG_FMT;
					node_reg_ptr->device.new_file = TRUE;
					node_reg_ptr->device.file_open = FALSE;
					node_reg_ptr->device.last_rec_id = 0;
				}

				/* Add the node to the patricia tree */
				if (ncs_patricia_tree_add(&cb->svc_tbl, (NCS_PATRICIA_NODE *)node_reg_ptr) !=
				    NCSCC_RC_SUCCESS) {
					/* Attempt to add node was unsuccessful */
					if (NULL != node_reg_ptr)
						m_MMGR_FREE_SVC_REG_TBL(node_reg_ptr);

					m_DTS_UNLK(&cb->lock);
					m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
					m_LOG_DTS_SVCREG_ADD_FAIL(DTS_EV_SVC_REG_ENT_ADD_FAIL, key.ss_svc_id, key.node,
								  0);
					m_LOG_DTS_EVT(DTS_EV_SVC_REG_FAILED, key.ss_svc_id, key.node, 0);
					return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							      "dtsv_ckpt_add_rmv_updt_svc_reg : Failed to add node registration in Patricia tree");
				}
				m_LOG_DTS_EVT(DTS_EV_SVC_REG_ENT_ADD, key.ss_svc_id, key.node, 0);
			}
			/*end of creating new entry for node */
			key.ss_svc_id = svcreg->my_key.ss_svc_id;
			/*  Network order key added */
			nt_key.ss_svc_id = m_NCS_OS_HTONL(svcreg->my_key.ss_svc_id);

			/* Check whether the Service exist in the service registration
			 * table. If yes, then add the DTA v-card in the v-card queue.
			 * If NO then create new entry in the table. Initialize it with
			 * the default.Enqueue the v-card in the v-card table. */
			if ((svc_ptr =
			     (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&cb->svc_tbl, (const uint8_t *)&nt_key)) != NULL) {
				/* Do nothing */
				m_LOG_DTS_EVT(DTS_EV_SVC_ALREADY_REG, key.ss_svc_id, key.node, 0);
			} /* end of if svc_ptr */
			else {
				/* Add to patricia tree; then create link list and add the new
				 * element to it */
				svc_ptr = m_MMGR_ALLOC_SVC_REG_TBL;
				if (svc_ptr == NULL) {
					m_DTS_UNLK(&cb->lock);
					m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
					return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							      "dtsv_ckpt_add_rmv_updt_svc_reg : Memory allocation failed");
				}
				memset(svc_ptr, '\0', sizeof(DTS_SVC_REG_TBL));
				svc_ptr->my_key.ss_svc_id = key.ss_svc_id;
				svc_ptr->my_key.node = key.node;

				/*  Network order key added */
				svc_ptr->ntwk_key = nt_key;
				svc_ptr->node.key_info = (uint8_t *)&svc_ptr->ntwk_key;

				svc_ptr->v_cd_list = NULL;
				svc_ptr->dta_count = 0;
				svc_ptr->my_node = node_reg_ptr;

				m_DTS_SET_SVC_REG_TBL(svc_ptr, svcreg);

				if (ncs_patricia_tree_add(&cb->svc_tbl, (NCS_PATRICIA_NODE *)svc_ptr) !=
				    NCSCC_RC_SUCCESS) {
					/* Attempt to add node was unsuccessful */
					if (NULL != svc_ptr)
						m_MMGR_FREE_SVC_REG_TBL(svc_ptr);
					m_DTS_UNLK(&cb->lock);
					m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
					m_LOG_DTS_SVCREG_ADD_FAIL(DTS_EV_SVC_REG_ENT_ADD_FAIL, key.ss_svc_id, key.node,
								  0);
					m_LOG_DTS_EVT(DTS_EV_SVC_REG_FAILED, key.ss_svc_id, key.node, 0);
					return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
							      "dtsv_ckpt_add_rmv_updt_svc_reg : Failed to add service entry in patricia tree");
				}
				m_LOG_DTS_EVT(DTS_EV_SVC_REG_ENT_ADD, key.ss_svc_id, key.node, 0);
				svc_ptr->device.last_rec_id = svcreg->device.last_rec_id;

				m_LOG_DTS_EVT(DTS_EV_SVC_REG_SUCCESSFUL, key.ss_svc_id, key.node, 0);
			}	/*end of else */
			m_DTS_UNLK(&cb->lock);
			m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
		}
		break;

	case NCS_MBCSV_ACT_UPDATE:
		{
			if (NULL != svcreg) {
				key = svcreg->my_key;
				/*  Network order key added */
				nt_key.node = m_NCS_OS_HTONL(key.node);
				nt_key.ss_svc_id = m_NCS_OS_HTONL(key.ss_svc_id);

				m_DTS_LK(&cb->lock);
				m_LOG_DTS_LOCK(DTS_LK_LOCKED, &cb->lock);
				if (svcreg->my_key.ss_svc_id == 0) {	/*Node policy change */
					/* update the policy for all services in the node */
					if ((svc_ptr =
					     (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&cb->svc_tbl,
										      (const uint8_t *)&nt_key)) == NULL) {
						m_DTS_UNLK(&cb->lock);
						m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
						m_LOG_DTS_EVT(DTS_EV_SVC_REG_NOTFOUND, key.ss_svc_id, key.node, 0);
						return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
								      "dtsv_ckpt_add_rmv_updt_svc_reg : Node entry not found in the patricia tree");
					}

					/* Update this node first */
					m_DTS_SET_SVC_REG_TBL(svc_ptr, svcreg);
					m_LOG_DTS_EVT(DTS_EV_SVC_REG_ENT_UPDT, key.ss_svc_id, key.node, 0);
					if (cb->cli_bit_map != 0) {
						while ((svc_ptr != NULL) && (svc_ptr->my_key.node == key.node)) {
							switch (cb->cli_bit_map) {
							case osafDtsvNodeMessageLogging_ID:
								if (svcreg->per_node_logging == FALSE)
									svc_ptr->device.new_file = TRUE;
								break;

							case osafDtsvNodeCategoryBitMap_ID:
									svc_ptr->svc_policy.category_bit_map =
									    svcreg->svc_policy.category_bit_map;
								break;

							case osafDtsvNodeLoggingState_ID:
									svc_ptr->svc_policy.enable =
									    svcreg->svc_policy.enable;
								break;

							case osafDtsvNodeSeverityBitMap_ID:
									svc_ptr->svc_policy.severity_bit_map =
									    svcreg->svc_policy.severity_bit_map;
								break;

							default:
								/* Do nothing, it should not hit this */
								break;
							}	/*end of switch */
							nt_key = svc_ptr->ntwk_key;
							/*m_DTS_SET_SVC_REG_TBL(svc_ptr, svcreg); */
							m_LOG_DTS_EVT(DTS_EV_SVC_REG_ENT_UPDT, key.ss_svc_id, key.node,
								      0);

							svc_ptr =
							    (DTS_SVC_REG_TBL *)dts_get_next_svc_entry(&cb->svc_tbl,
												      (SVC_KEY *)
												      &nt_key);
						}	/*end of while */
						/* Re-set the cli_bit_map for subsequent policy changes */
						cb->cli_bit_map = 0;
					}	/*end of if cli_bit_map */
				} else {	/*Service policy change */

					if ((svc_ptr =
					     (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&cb->svc_tbl,
										      (const uint8_t *)&nt_key)) == NULL) {
						m_DTS_UNLK(&cb->lock);
						m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
						m_LOG_DTS_EVT(DTS_EV_SVC_REG_NOTFOUND, key.ss_svc_id, key.node, 0);
						return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
								      "dtsv_ckpt_add_rmv_updt_svc_reg : Service entry not found in the patricia tree");
					} else {
						m_DTS_SET_SVC_REG_TBL(svc_ptr, svcreg);
						m_LOG_DTS_EVT(DTS_EV_SVC_REG_ENT_UPDT, key.ss_svc_id, key.node, 0);
					}
				}
				m_DTS_UNLK(&cb->lock);
				m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
			} else {
				return NCSCC_RC_FAILURE;
			}
		}
		break;

	case NCS_MBCSV_ACT_RMV:
		{
			SPEC_CKPT spec_info;

			if (NULL != svcreg) {
				key.node = svcreg->my_key.node;
				key.ss_svc_id = svcreg->my_key.ss_svc_id;

				/*  Network order key added */
				nt_key.node = m_NCS_OS_HTONL(key.node);
				nt_key.ss_svc_id = m_NCS_OS_HTONL(key.ss_svc_id);

				m_DTS_LK(&cb->lock);
				m_LOG_DTS_LOCK(DTS_LK_LOCKED, &cb->lock);
				if ((svc_ptr =
				     (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&cb->svc_tbl,
									      (const uint8_t *)&nt_key)) == NULL) {
					m_DTS_UNLK(&cb->lock);
					m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
					m_LOG_DTS_EVT(DTS_EV_SVC_REG_NOTFOUND, key.ss_svc_id, key.node, 0);
					/* Return Success, since svc is already not present */
					return m_DTS_DBG_SINK(NCSCC_RC_SUCCESS,
							      "dtsv_ckpt_add_rmv_updt_svc_reg : Service entry not found in patricia tree");
				}

				/*The DTA to be removed from svc_reg->v_cd_list is stored in
				 * DTS_CB's svc_rmv_mds_dest.
				 * If svc_rmv_mds_dest is 0, then the svc->dta_count should already
				 * be zero else there's an error.
				 * If svc_rmv_mds_dest is not zero, then it is async update for
				 * svc unregister. In this case, remove the DTA with MDS_DEST 
				 * the same as cb->svc_rmv_mds_dest from 
				 * svc_ptr->v-cd_list, also remove svc entry from dta->svc_list.
				 */
				if (cb->svc_rmv_mds_dest == 0) {
					if (svc_ptr->dta_count == 0) {
						if (&svc_ptr->device.cir_buffer != NULL)
							dts_circular_buffer_free(&svc_ptr->device.cir_buffer);
						device = &svc_ptr->device;
						/* Cleanup the DTS_FILE_LIST datastructure for svc */
						m_DTS_FREE_FILE_LIST(device);
						ncs_patricia_tree_del(&cb->svc_tbl, (NCS_PATRICIA_NODE *)svc_ptr);
						m_LOG_DTS_EVT(DTS_EV_SVC_REG_ENT_RMVD, key.ss_svc_id, key.node, 0);
						if (NULL != svc_ptr)
							m_MMGR_FREE_SVC_REG_TBL(svc_ptr);

						m_DTS_UNLK(&cb->lock);
						m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
						return NCSCC_RC_SUCCESS;
					} else {
						m_LOG_DTS_EVT(DTS_EV_SVC_DEREG_FAILED, key.ss_svc_id, key.node, 0);
						m_DTS_UNLK(&cb->lock);
						m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
						return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
								      "dtsv_ckpt_add_rmv_updt_svc_reg: Either MDS_DEST for async update is wrong or database is inconsistent");
					}
				} /*end of if(cb->svc_rmv_mds_dest == 0) */
				else {
					vkey = &cb->svc_rmv_mds_dest;

					dta_entry = svc_ptr->v_cd_list;
					/* iterate through v_cd_list to find the dta to be removed */
					while (dta_entry != NULL) {
						to_reg = dta_entry->dta;

						/* Point to next dta entry before deletion */
						dta_entry = dta_entry->next_in_svc_entry;

						/*Check if the MDS_DEST for the dta to be removed matches */
						if (dts_find_reg(vkey, to_reg) == TRUE) {
							if ((svc_ptr =
							     (DTS_SVC_REG_TBL *)dts_del_svc_frm_dta(to_reg,
												    svc_ptr)) == NULL) {
								m_DTS_UNLK(&cb->lock);
								m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
								m_LOG_DTS_EVT(DTS_EV_DTA_SVC_RMV_FAIL, key.ss_svc_id,
									      key.node, (uint32_t)to_reg->dta_addr);
								m_LOG_DTS_EVT(DTS_EV_SVC_DEREG_FAILED, key.ss_svc_id,
									      key.node, (uint32_t)to_reg->dta_addr);
								return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
										      "dtsv_ckpt_add_rmv_updt_svc_reg: Unable to remove svc reg entry from dta's list");
							}
							m_LOG_DTS_EVT(DTS_EV_DTA_SVC_RMV, key.ss_svc_id, key.node,
								      (uint32_t)to_reg->dta_addr);

							/* Remove dta entry frm svc_ptr->v_cd_list */
							if ((to_reg =
							     (DTA_DEST_LIST *)dts_dequeue_dta(svc_ptr,
											      to_reg)) == NULL) {
								m_DTS_UNLK(&cb->lock);
								m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
								m_LOG_DTS_EVT(DTS_EV_SVC_DTA_RMV_FAIL, key.ss_svc_id,
									      key.node, (uint32_t)to_reg->dta_addr);
								m_LOG_DTS_EVT(DTS_EV_SVC_DEREG_FAILED, key.ss_svc_id,
									      key.node, (uint32_t)to_reg->dta_addr);
								return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
										      "dtsv_ckpt_add_rmv_updt_svc_reg: Unable to remove adest entry");
							}
							m_LOG_DTS_EVT(DTS_EV_SVC_DTA_RMV, key.ss_svc_id, key.node,
								      (uint32_t)to_reg->dta_addr);

							memset(&spec_info, '\0', sizeof(SPEC_CKPT));
							/* Versioning support : Remove spec entry corresponding to
							 * the DTA from svc's spec_list. */
							if (dts_del_spec_frm_svc(svc_ptr, *vkey, &spec_info) !=
							    NCSCC_RC_SUCCESS) {
								m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
									       "dtsv_ckpt_add_rmv_updt_svc_reg: Unable to remove spec entry");
							}

							/* Call function to unload the ASCII_SPEC library if 
							 * use_count of the library has become 0.
							 */
							dts_ascii_spec_load(spec_info.svc_name, spec_info.version, 0);

							/* If dta svc queue is empty delete dta frm patricia tree */
							if (to_reg->svc_list == NULL) {
								/*ncs_destroy_queue(&to_reg->svc_list); */
								ncs_patricia_tree_del(&cb->dta_list,
										      (NCS_PATRICIA_NODE *)
										      &to_reg->node);
								m_LOG_DTS_EVT(DTS_EV_DTA_DEST_RMV_SUCC, 0, key.node,
									      (uint32_t)to_reg->dta_addr);
								if (NULL != to_reg)
									m_MMGR_FREE_VCARD_TBL(to_reg);
							}

							/*Since the dta is found and deleted exit the while loop */
							break;
						}	/*end of if(dts_find_reg() == TRUE) */
					}	/*end of while */
				}	/*end of else (cb->svc_rmv_mds_dest == 0) */

				/* Do NOT attempt deletion of service entries while 
				 * it's still syncing with Active DTS esp. if its still yet to 
				 * get the data for DTA LIST. Because until then SVC<->DTA
				 * mapping won't be established.
				 * In that case all services which have dta_count = 0 will be 
				 * cleaned at the end of completion of DTA list data-sync.
				 */
				if ((cb->in_sync == FALSE) && (cb->cold_sync_done < DTSV_CKPT_DTA_DEST_LIST_CONFIG)) {
					/* Whatever is needed has been done so return success */
					m_DTS_UNLK(&cb->lock);
					m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
					m_LOG_DTS_EVT(DTS_EV_SVC_DEREG_SUCCESSFUL, key.ss_svc_id, key.node, 0);
					return NCSCC_RC_SUCCESS;
				}

				/* Delete svc entry when dta_count is zero 
				 */
				else if (svc_ptr->dta_count == 0) {
					/* No need of policy handles */
					/* Now delete the svc entry from the patricia tree */
					/*ncshm_destroy_hdl(NCS_SERVICE_ID_DTSV, svc_ptr->svc_hdl); 
					   m_LOG_DTS_EVT(DTS_EV_SVC_POLCY_HDL_DES, key.ss_svc_id, key.node, 0); */
					if (&svc_ptr->device.cir_buffer != NULL)
						dts_circular_buffer_free(&svc_ptr->device.cir_buffer);
					device = &svc_ptr->device;
					/* Cleanup the DTS_FILE_LIST datastructure for svc */
					m_DTS_FREE_FILE_LIST(device);

					ncs_patricia_tree_del(&cb->svc_tbl, (NCS_PATRICIA_NODE *)svc_ptr);
					m_LOG_DTS_EVT(DTS_EV_SVC_REG_ENT_RMVD, key.ss_svc_id, key.node, 0);
					if (NULL != svc_ptr)
						m_MMGR_FREE_SVC_REG_TBL(svc_ptr);
				}
				m_DTS_UNLK(&cb->lock);
				m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
				m_LOG_DTS_EVT(DTS_EV_SVC_DEREG_SUCCESSFUL, key.ss_svc_id, key.node, 0);
			} else {
				return NCSCC_RC_FAILURE;
			}
		}
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}
	return status;
}

/****************************************************************************\
 * Function: dtsv_ckpt_add_rmv_updt_dts_log
 *
 * Purpose:  Add new entry to device's DTS_FILE_LIST if action is ADD, 
 *           remove if action is to remove.
 *
 * Input: cb  - CB pointer.
 *        su - Decoded structur.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 * 
\**************************************************************************/
uint32_t dtsv_ckpt_add_rmv_updt_dts_log(DTS_CB *cb, DTS_LOG_CKPT_DATA *data, NCS_MBCSV_ACT_TYPE action)
{
	DTS_SVC_REG_TBL *svc = NULL;
	OP_DEVICE *device = NULL;
	SVC_KEY nt_key;

	if (data == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dtsv_ckpt_add_rmv_updt_dts_log: NULL pointer passed");

	/*  Network order key added */
	nt_key.node = m_NCS_OS_HTONL(data->key.node);
	nt_key.ss_svc_id = m_NCS_OS_HTONL(data->key.ss_svc_id);

	m_DTS_LK(&cb->lock);
	m_LOG_DTS_LOCK(DTS_LK_LOCKED, &cb->lock);
	if (data->key.node == 0)	/*global policy device */
		device = &cb->g_policy.device;
	else if (data->key.ss_svc_id == 0) {	/*node policy device */
		if ((svc = (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&cb->svc_tbl, (const uint8_t *)&nt_key)) == NULL) {
			m_DTS_UNLK(&cb->lock);
			m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_ckpt_add_rmv_updt_dts_log: Node entry doesn't exist");
		}
		device = &svc->device;
	} else {		/*service policy device */

		if ((svc = (DTS_SVC_REG_TBL *)ncs_patricia_tree_get(&cb->svc_tbl, (const uint8_t *)&nt_key)) == NULL) {
			m_DTS_UNLK(&cb->lock);
			m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
			return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
					      "dtsv_ckpt_add_rmv_updt_dts_log: Service entry doesn't exist");
		}
		device = &svc->device;
	}

	switch (action) {
	case NCS_MBCSV_ACT_ADD:
		break;
	case NCS_MBCSV_ACT_RMV:
		{
			m_LOG_DTS_LOGDEL(data->key.node, data->key.ss_svc_id, device->log_file_list.num_of_files);
			m_DTS_RMV_LL_ENTRY(device);
		}
		break;
	default:
		/* Log error */
		m_DTS_UNLK(&cb->lock);
		m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
		return NCSCC_RC_FAILURE;
	}
	m_DTS_UNLK(&cb->lock);
	m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
 * Function: dtsv_ckpt_add_rmv_updt_global_policy
 *
 * Purpose:  Add new entry if action is ADD, remove if 
 *           action is to remove and update data if request is to update.
 *
 * Input: cb  - CB pointer.
 *        su - Decoded structur.
 *        action - ADD/RMV/UPDT
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
uint32_t dtsv_ckpt_add_rmv_updt_global_policy(DTS_CB *cb, GLOBAL_POLICY *gp, DTS_FILE_LIST *file_list,
					   NCS_MBCSV_ACT_TYPE action)
{
	uint32_t status = NCSCC_RC_SUCCESS;

	/* Here we don't need to explicitly handle ADD and RMV functionality 
	 * we only handle the UPDT functionality 
	 */
	switch (action) {
	case NCS_MBCSV_ACT_ADD:
	case NCS_MBCSV_ACT_RMV:
		/* 
		 * Don't break...continue updating data to this  
		 * as done during normal update request.
		 */

	case NCS_MBCSV_ACT_UPDATE:
		if (NULL != gp) {
			/*
			 * Update all the data. 
			 */
			m_DTS_LK(&cb->lock);
			m_LOG_DTS_LOCK(DTS_LK_LOCKED, &cb->lock);
			cb->g_policy.global_logging = gp->global_logging;
			cb->g_policy.g_policy.enable = gp->g_policy.enable;
			cb->g_policy.g_policy.category_bit_map = gp->g_policy.category_bit_map;
			cb->g_policy.g_policy.severity_bit_map = gp->g_policy.severity_bit_map;
			cb->g_policy.g_policy.log_dev = gp->g_policy.log_dev;
			cb->g_policy.g_policy.log_file_size = gp->g_policy.log_file_size;
			cb->g_policy.g_policy.file_log_fmt = gp->g_policy.file_log_fmt;
			cb->g_policy.g_policy.cir_buff_size = gp->g_policy.cir_buff_size;
			cb->g_policy.g_policy.buff_log_fmt = gp->g_policy.buff_log_fmt;
			cb->g_policy.g_enable_seq = gp->g_enable_seq;
			cb->g_policy.g_num_log_files = gp->g_num_log_files;
			cb->g_policy.device.new_file = TRUE;
			cb->g_policy.device.file_open = FALSE;
			cb->g_policy.device.last_rec_id = gp->device.last_rec_id;

			/* Check for cb->cli_bit_map, if it is set to non-zero,
			   update the node/service policy with the changes */
			if (cb->cli_bit_map != 0) {
				switch (cb->cli_bit_map) {
				case osafDtsvGlobalMessageLogging_ID:
					cb->g_policy.device.new_file = TRUE;
					/*dts_circular_buffer_clear(&cb->g_policy.device.cir_buffer); */
				case osafDtsvGlobalCategoryBitMap_ID:
				case osafDtsvGlobalLoggingState_ID:
				case osafDtsvGlobalSeverityBitMap_ID:
					/* This case changes have to propagated to all svcs */
					if (NCSCC_RC_SUCCESS != dts_stby_global_filtering_policy_change(cb)) {
						m_DTS_UNLK(&cb->lock);
						m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
						return m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
								      "dtsv_ckpt_add_rmv_updt_global_policy: Unable to apply global policy changes to all services");
					}
					break;

				default:
					/* Do nothing, shouldn't hit this part */
					break;
				}
				/* Smik - Reset cb->cli_bit_map to 0 for future updates */
				cb->cli_bit_map = 0;
			}
			/* end of if cb->cli_bit_map */
			m_DTS_UNLK(&cb->lock);
			m_LOG_DTS_LOCK(DTS_LK_UNLOCKED, &cb->lock);
		} else {
			return NCSCC_RC_FAILURE;
		}
		break;

	default:
		/* Log error */
		return NCSCC_RC_FAILURE;
	}
	/* Smik - End of implementation */
	return status;
}

/**************************************************************************
 Function: dts_stby_global_filtering_policy_change

 Purpose:  Change in the Global Filtering policy will affect the entire system
           This function will set the filtering policies of all the nodes and 
           all the services which are currently present in the node and the 
           service registration table respectively.
           The above changes are for stby DTS only.

 Input:    cb       : DTS_CB pointer
           param_id : Parameter for which change is done.

 Returns:  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

 Notes:  
**************************************************************************/
static uint32_t dts_stby_global_filtering_policy_change(DTS_CB *cb)
{
	uint32_t status = NCSCC_RC_SUCCESS;
	SVC_KEY nt_key;
	DTS_SVC_REG_TBL *service = NULL;

	/* Search through registration table, Set all the policies,
	 * configure all the DTA's using this policy. */
	service = (DTS_SVC_REG_TBL *)ncs_patricia_tree_getnext(&cb->svc_tbl, NULL);
	while (service != NULL) {
		/* Setup key for new search */
		/*  Network order key added */
		nt_key.node = m_NCS_OS_HTONL(service->my_key.node);
		nt_key.ss_svc_id = m_NCS_OS_HTONL(service->my_key.ss_svc_id);

		/* Set the Node Policy as per the Global Policy */
		if (cb->cli_bit_map == osafDtsvGlobalLoggingState_ID)
			service->svc_policy.enable = cb->g_policy.g_policy.enable;
		if (cb->cli_bit_map == osafDtsvGlobalCategoryBitMap_ID)
			service->svc_policy.category_bit_map = cb->g_policy.g_policy.category_bit_map;
		if (cb->cli_bit_map == osafDtsvGlobalSeverityBitMap_ID)
			service->svc_policy.severity_bit_map = cb->g_policy.g_policy.severity_bit_map;
		if (cb->cli_bit_map == osafDtsvGlobalMessageLogging_ID) {
			service->device.new_file = TRUE;
		}
		service = (DTS_SVC_REG_TBL *)ncs_patricia_tree_getnext(&cb->svc_tbl, (const uint8_t *)&nt_key);
	}			/* end of while */
	return status;
}

/****************************************************************************\
 * Function: dts_data_clean_up
 *
 * Purpose:  Function is used for cleaning the entire DTS data structure.
 *           Will be called from the standby DTS on failure of cold sync.
 *
 * Input: cb  - CB pointer.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * NOTES:
 *
 * 
\**************************************************************************/
/*uint32_t dts_data_clean_up(DTS_CB *cb)
{
   uint32_t status = NCSCC_RC_SUCCESS;
   TRACE("\n***Inside dts_data_clean_up***\n"); 
   m_DTS_LK(&cb->lock);
   dtsv_clear_registration_table(cb);
   status = dtsv_delete_asciispec_tree(cb);
   status = dtsv_delete_libname_tree(cb);
   ncs_patricia_tree_destroy(&cb->svcid_asciispec_tree);
   ncs_patricia_tree_destroy(&cb->libname_asciispec_tree);
   m_DTS_UNLK(&cb->lock);
   return status;
}*/
