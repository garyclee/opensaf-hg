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

  DESCRIPTION:

  This file contains all Public APIs for the Flex Log server (DTS) portion
  of the Distributed Tracing Service (DTSv) subsystem.

  ..............................................................................

  FUNCTIONS INCLUDED in this module:

  dts_lm                       - DTS layer management API.
  dts_svc_create               - Create DTS service.
  dts_svc_destroy              - Distroy DTS service.
  ncs_dtsv_ascii_spec_api      - ASCII spec registration SE API.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/
#include <dlfcn.h>

#include "dts.h"

/*****************************************************************************

                     DTS LAYER MANAGEMENT IMPLEMENTAION

  PROCEDURE NAME:    dts_lm

  DESCRIPTION:       Core API for all Flex Log Server layer management 
                     operations used by a target system to instantiate and
                     control a DTS instance. Its operations include:

                     CREATE  a DTS instance
                     DESTROY a DTS instance

  RETURNS:           SUCCESS - All went well
                     FAILURE - internal processing didn't like something.
                               enable m_DTS_DBG_SINK() for details.

*****************************************************************************/

uint32_t dts_lm(DTS_LM_ARG *arg)
{
	switch (arg->i_op) {
	case DTS_LM_OP_CREATE:
		return dts_svc_create(&arg->info.create);

	case DTS_LM_OP_DESTROY:
		return dts_svc_destroy(&arg->info.destroy);

	default:
		break;
	}
	return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_lm: Incorrect operation value passed");
}

/*#############################################################################
 *
 *                   PRIVATE DTS LAYER MANAGEMENT IMPLEMENTAION
 *
 *############################################################################*/

/*****************************************************************************

  PROCEDURE NAME:    dts_svc_create

  DESCRIPTION:       Create an instance of DTS, set configuration profile to
                     default, install this DTS with MDS and subscribe to DTS
                     events.

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_DTS_DBG_SINK()
                               for details.

*****************************************************************************/

uint32_t dts_svc_create(DTS_CREATE *create)
{
	/* Initialize all the CB fields */
	DTS_CB *inst = &dts_cb;
	NCS_PATRICIA_PARAMS pt_params;

	m_DTS_LK_INIT;

	memset(&pt_params, 0, sizeof(NCS_PATRICIA_PARAMS));

	m_DTS_LK_CREATE(&inst->lock);

	m_DTS_LK(&inst->lock);

	/* inst->created is set to true in dts_mds_reg() */
	inst->created = false;

	/* review changes ; initialize pt_params to 0 */
	memset(&pt_params, 0, sizeof(NCS_PATRICIA_PARAMS));

	pt_params.key_size = sizeof(SVC_KEY);

	/* Create Patritia tree which will keep the DTS service registration 
	 * information.*/
	if (ncs_patricia_tree_init(&inst->svc_tbl, &pt_params) != NCSCC_RC_SUCCESS) {
		m_DTS_UNLK(&inst->lock);
		m_DTS_LK_DLT(&inst->lock);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_svc_create: Patricia tree init failed");
	}

	pt_params.key_size = sizeof(MDS_DEST);

	/*  Create Patritia tree which will keep the list of DTAs */
	if (ncs_patricia_tree_init(&inst->dta_list, &pt_params) != NCSCC_RC_SUCCESS) {
		ncs_patricia_tree_destroy(&inst->svc_tbl);
		m_DTS_UNLK(&inst->lock);
		m_DTS_LK_DLT(&inst->lock);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_svc_create: Patricia tree init failed");
	}

	pt_params.key_size = sizeof(ASCII_SPEC_INDEX);

	/* create patricia tree to keep ASCII_SPEC table indexed by svc_id */
	if (ncs_patricia_tree_init(&inst->svcid_asciispec_tree, &pt_params) != NCSCC_RC_SUCCESS) {
		ncs_patricia_tree_destroy(&inst->svc_tbl);
		ncs_patricia_tree_destroy(&inst->dta_list);
		m_DTS_UNLK(&inst->lock);
		m_DTS_LK_DLT(&inst->lock);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_svc_create: Patricia tree init failed");
	}

	pt_params.key_size = DTS_MAX_LIBNAME * sizeof(int8_t);

	/* create patricia tree keeping libnames of ASCII SPEC table registered */
	if (ncs_patricia_tree_init(&inst->libname_asciispec_tree, &pt_params) != NCSCC_RC_SUCCESS) {
		ncs_patricia_tree_destroy(&inst->svc_tbl);
		ncs_patricia_tree_destroy(&inst->dta_list);
		ncs_patricia_tree_destroy(&inst->svcid_asciispec_tree);
		m_DTS_UNLK(&inst->lock);
		m_DTS_LK_DLT(&inst->lock);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_svc_create: Patricia tree init failed");
	}

	/* Initialize global policy table */
	dts_global_policy_set(&inst->g_policy);
	/* Smik - Set cli_bit_map to 0 */
	inst->cli_bit_map = 0;

	inst->dts_enbl = true;
	inst->hmpool_id = create->i_hmpool_id;

	/* Versioning changes - Set the DTS MDS service sub-part version */
	inst->dts_mds_version = DTS_MDS_SUB_PART_VERSION;

	/* Intialize the default policy table */
	dts_default_policy_set(&inst->dflt_plcy);

	/* Create new selection object for signal handler */
	m_NCS_SEL_OBJ_CREATE(&inst->sighdlr_sel_obj);

	/* Register DTS with MDS */
	if (dts_mds_reg(inst) != NCSCC_RC_SUCCESS) {
		m_NCS_SEL_OBJ_DESTROY(inst->sighdlr_sel_obj);
		ncs_patricia_tree_destroy(&inst->svc_tbl);
		ncs_patricia_tree_destroy(&inst->dta_list);
		ncs_patricia_tree_destroy(&inst->libname_asciispec_tree);
		ncs_patricia_tree_destroy(&inst->svcid_asciispec_tree);

		m_DTS_UNLK(&inst->lock);
		m_DTS_LK_DLT(&inst->lock);
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_svc_create: MDS registration failed");
	}

	/* Initialize DTS version */
	inst->dts_mbcsv_version = DTS_MBCSV_VERSION;
	/* Register DTS with MBCSv */
	if (dtsv_mbcsv_register(inst) != NCSCC_RC_SUCCESS) {
		m_NCS_SEL_OBJ_DESTROY(inst->sighdlr_sel_obj);
		dts_mds_unreg(inst, true);
		inst->created = false;
		ncs_patricia_tree_destroy(&inst->svc_tbl);
		ncs_patricia_tree_destroy(&inst->dta_list);
		ncs_patricia_tree_destroy(&inst->libname_asciispec_tree);
		ncs_patricia_tree_destroy(&inst->svcid_asciispec_tree);

		m_DTS_UNLK(&inst->lock);
		m_DTS_LK_DLT(&inst->lock);

		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_svc_create: MBCSv registration failed");
	}

	strcpy(inst->log_path, getenv("DTSV_ROOT_DIRECTORY"));
	TRACE("\nMy log directory path = %s\n", inst->log_path);

	inst->imm_init_done = false;
	m_DTS_UNLK(&inst->lock);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    dts_svc_destroy

  DESCRIPTION:       Destroy an instance of DTS. Withdraw from MDS and free
                     this DTS_CB and tend to other resource recovery issues.

  RETURNS:           SUCCESS - all went well.
                     FAILURE - something went wrong. Turn on m_DTS_DBG_SINK()
                               for details.

*****************************************************************************/

uint32_t dts_svc_destroy(DTS_DESTROY *destroy)
{
	DTS_CB *inst = &dts_cb;
	uint32_t retval = NCSCC_RC_SUCCESS, i = 0;

	m_DTS_LK(&inst->lock);
#if (DTS_LOG == 1)
	if (dts_log_unbind() != NCSCC_RC_SUCCESS) {
		m_DTS_UNLK(&inst->lock);
		m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "dts_svc_destroy: Unable to unbind from DTSv");
	}
#endif

	/* Now walk through the sequencing buffer and free all the messages */
	/* Clear sequencing buffer only if DTS is Act */
	if ((true == inst->g_policy.g_enable_seq) && (inst->ha_state == SA_AMF_HA_ACTIVE)) {
		m_NCS_TMR_STOP(inst->tmr);
		m_NCS_TMR_DESTROY(inst->tmr);
		for (i = 0; i < inst->s_buffer.num_msgs; i++) {
			if (0 != inst->s_buffer.arr_ptr[i].msg) {
				m_MMGR_FREE_DTSV_MSG(inst->s_buffer.arr_ptr[i].msg);
				inst->s_buffer.arr_ptr[i].msg = NULL;
			}
		}

		m_MMGR_FREE_SEQ_BUFF(inst->s_buffer.arr_ptr);
	}

	/* Walk through entire paticia tree and free all the registration nodes */
	dtsv_clear_registration_table(inst);

	/* We no longer needed patricia tree; so destroy it */
	ncs_patricia_tree_destroy(&inst->svc_tbl);
	ncs_patricia_tree_destroy(&inst->dta_list);
	dtsv_clear_asciispec_tree(inst);
	dtsv_clear_libname_tree(inst);
	ncs_patricia_tree_destroy(&inst->svcid_asciispec_tree);
	ncs_patricia_tree_destroy(&inst->libname_asciispec_tree);

	m_NCS_SEL_OBJ_DESTROY(inst->sighdlr_sel_obj);

	/* Deregister frm AMF */
	dts_amf_finalize(inst);

	/*Un-install DTS from MBCSv */
	dtsv_mbcsv_deregister(inst);

	/* Un-install DTS from MDS */
	dts_mds_unreg(inst, true);

	inst->created = false;

	m_DTS_UNLK(&inst->lock);

	m_DTS_LK_DLT(&inst->lock);

	return retval;
}

/**************************************************************************
 Function: dtsv_clear_registration_table

 Purpose:  Function walks through the entire tree and free all the assigned memory
           close files and clear entire registration table.

 Input:    cb       : DTS_CB pointer

 Returns:  None

 Notes:  
**************************************************************************/
void dtsv_clear_registration_table(DTS_CB *inst)
{
	SVC_KEY nt_key;
	DTS_SVC_REG_TBL *service = NULL;
	DTA_DEST_LIST *dta = NULL, *dta_node = NULL;
	DTA_ENTRY *dta_entry = NULL;
	MDS_DEST vkey;
	OP_DEVICE *device = NULL;
	SVC_ENTRY *svc_entry = NULL;
	SPEC_ENTRY *spec_entry = NULL;

	m_LOG_DTS_API(DTS_REG_TBL_CLEAR);
	/* Search through registration table.
	 */
	service = (DTS_SVC_REG_TBL *)ncs_patricia_tree_getnext(&inst->svc_tbl, NULL);
	while (service != NULL) {
		/* Setup key for new search */
		/* Network order key added */
		nt_key.node = service->ntwk_key.node;
		nt_key.ss_svc_id = service->ntwk_key.ss_svc_id;

		/* Clear the circular buffer and close the files */
		dts_circular_buffer_free(&service->device.cir_buffer);

		if ((service->device.file_open == true) && (service->device.svc_fh != NULL)) {
			fclose(service->device.svc_fh);
			service->device.svc_fh = NULL;
			service->device.file_open = false;
			service->device.new_file = true;
			service->device.cur_file_size = 0;
		}

		/* Clear the log file datastructure associated with each svc device */
		device = &service->device;
		m_DTS_FREE_FILE_LIST(device);

		/* Cleanup the console devices associated with the node */
		m_DTS_RMV_ALL_CONS(device);

		/* For service nodes, walk through the link list and then free entire queue */
		if (nt_key.ss_svc_id != 0) {
			dta_entry = service->v_cd_list;
			while (dta_entry != NULL) {
				vkey = dta_entry->dta->dta_addr;

				/*Free the dta_list patricia tree before releasing memory */
				if ((dta_node =
				     (DTA_DEST_LIST *)ncs_patricia_tree_get(&inst->dta_list,
									    (const uint8_t *)&vkey)) == NULL) {
					m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
						       "dtsv_clear_registration_table: DTA entry doesn't exist in dta tree");
				}
				/* Adjust the pointer to to_reg with the offset */
				dta_node = (DTA_DEST_LIST *)((long)dta_node - DTA_DEST_LIST_OFFSET);

				/*Remove all svc entries from dta's svc_list */
				while (dta_node->svc_list != NULL) {
					svc_entry = dta_node->svc_list;
					dta_node->svc_list = dta_node->svc_list->next_in_dta_entry;
					m_MMGR_FREE_DTS_SVC_ENTRY(svc_entry);
				}

				dta_entry = dta_entry->next_in_svc_entry;
			}	/*end of while */

			/* Now remove all entries from service's v_cd_list */
			while (service->v_cd_list != NULL) {
				dta_entry = service->v_cd_list;
				service->v_cd_list = dta_entry->next_in_svc_entry;
				m_MMGR_FREE_DTS_DTA_ENTRY(dta_entry);
			}

			/* Versioning changes : Remove the spec_entries for the service */
			while (service->spec_list != NULL) {
				spec_entry = service->spec_list;
				service->spec_list = spec_entry->next_spec_entry;
				/* Decrement the use count of ASCII_SPEC ptr before deletion */
				if (spec_entry->spec_struct != NULL)
					spec_entry->spec_struct->use_count--;
				/* Decrement the use count for library having the spec */
				if (spec_entry->lib_struct != NULL)
					spec_entry->lib_struct->use_count--;
				m_MMGR_FREE_DTS_SVC_SPEC(spec_entry);
			}
		}

		/*end of ss_svc_id != 0 */
		/*Now remove entry from patricia tree and free the memory */
		if ((service->v_cd_list == NULL) || (service->dta_count == 0)) {
			/* No need of policy handles */
			/*ncshm_destroy_hdl(NCS_SERVICE_ID_DTSV, service->svc_hdl); */
			ncs_patricia_tree_del(&inst->svc_tbl, (NCS_PATRICIA_NODE *)service);
			if (service != NULL)
				m_MMGR_FREE_SVC_REG_TBL(service);
		} else {
			m_DTS_DBG_SINK(NCSCC_RC_FAILURE,
				       "dtsv_clear_registration_table: Failed to delete svc entry, svc_reg->v_cd_list is not empty");
		}

		service = (DTS_SVC_REG_TBL *)ncs_patricia_tree_getnext(&inst->svc_tbl, (const uint8_t *)&nt_key);
	}

	/* Now clear the dta patricia tree also, now that there are no dta_ptrs
	 * being referenced by any svc reg entries 
	 */
	while ((dta = (DTA_DEST_LIST *)ncs_patricia_tree_getnext(&inst->dta_list, NULL)) != NULL) {
		dta = (DTA_DEST_LIST *)((long)dta - DTA_DEST_LIST_OFFSET);
		/* Delete frm the patricia tree */
		ncs_patricia_tree_del(&inst->dta_list, (NCS_PATRICIA_NODE *)&dta->node);
		/* Now free the memory of dta */
		if (dta != NULL)
			m_MMGR_FREE_VCARD_TBL(dta);
	}

	/* Clear the circular buffer for global policy */
	dts_circular_buffer_free(&inst->g_policy.device.cir_buffer);
	/* Clear the log file list associated with global policy also */
	device = &inst->g_policy.device;
	m_DTS_FREE_FILE_LIST(device);
	/* Close the global level files if any open */
	if ((inst->g_policy.device.file_open == true) && (inst->g_policy.device.svc_fh != NULL)) {
		fclose(inst->g_policy.device.svc_fh);
		inst->g_policy.device.svc_fh = NULL;
		inst->g_policy.device.file_open = false;
		inst->g_policy.device.new_file = true;
		inst->g_policy.device.cur_file_size = 0;
	}

	/* Clear all the console devices for global policy */
	m_DTS_RMV_ALL_CONS(device);

	return;
}

/*****************************************************************************

  PROCEDURE NAME:    ncs_dtsv_ascii_spec_api

  DESCRIPTION:       Introduce subsystem specific ASCII printing info for
                     logging and bump the ref-count. If the subsystem has
                     already registered (presumably a different instance),
                     just bump the refcount.

*****************************************************************************/

uint32_t ncs_dtsv_ascii_spec_api(NCS_DTSV_REG_CANNED_STR *arg)
{
	DTS_CB *inst = &dts_cb;
	uint32_t rc = NCSCC_RC_SUCCESS;

	if (arg == NULL)
		return m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "ncs_dtsv_ascii_spec_api: NULLi pointer passed as argument");

	if (inst->created == false) {
		m_LOG_DTS_DBGSTR(DTS_SERVICE, "DTS is not created. Unable to register/de-register ASCII_SPEC",
				 0,
				 (arg->i_op ==
				  NCS_DTSV_OP_ASCII_SPEC_REGISTER) ? arg->info.reg_ascii_spec.spec->ss_id : arg->info.
				 dereg_ascii_spec.svc_id);

		return m_DTS_DBG_SINK_SVC(NCSCC_RC_FAILURE,
					  "ncs_dtsv_ascii_spec_api: DTS is not created. First create DTS before spec registration",
					  arg->info.dereg_ascii_spec.svc_id);
	}

	m_DTS_LK(&inst->lock);
	switch (arg->i_op) {
	case NCS_DTSV_OP_ASCII_SPEC_REGISTER:
		rc = dts_ascii_spec_register(arg->info.reg_ascii_spec.spec);
		break;

	case NCS_DTSV_OP_ASCII_SPEC_DEREGISTER:
		rc = dts_ascii_spec_deregister(arg->info.dereg_ascii_spec.svc_id, arg->info.dereg_ascii_spec.version);
		break;

	default:
		rc = m_DTS_DBG_SINK(NCSCC_RC_FAILURE, "ncs_dtsv_ascii_spec_api: Wrong operation type passed!!");
		break;
	}
	m_DTS_UNLK(&inst->lock);
	return rc;
}

/**************************************************************************
 Function: dtsv_clear_asciispec_tree

 Purpose:  Function walks through entire tree and free all the assigned memory
           clear entire tree and subsequent deletion

 Input:    cb       : DTS_CB pointer

 Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE  

 Notes:  
**************************************************************************/
uint32_t dtsv_clear_asciispec_tree(DTS_CB *cb)
{
	SYSF_ASCII_SPECS *spec_entry;

	while (NULL != (spec_entry = (SYSF_ASCII_SPECS *)ncs_patricia_tree_getnext(&cb->svcid_asciispec_tree, NULL))) {
		ncs_patricia_tree_del(&cb->svcid_asciispec_tree, (NCS_PATRICIA_NODE *)spec_entry);
		m_MMGR_FREE_DTS_SPEC_ENTRY(spec_entry);
	}

	return NCSCC_RC_SUCCESS;
}

/**************************************************************************
 Function: dtsv_clear_libname_tree

 Purpose:  Function walks through entire tree and free all the assigned memory
           clear entire tree and subsequent deletion

 Input:    cb       : DTS_CB pointer

 Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE 

 Notes:  
**************************************************************************/
uint32_t dtsv_clear_libname_tree(DTS_CB *cb)
{
	ASCII_SPEC_LIB *lib_entry;

	while (NULL != (lib_entry = (ASCII_SPEC_LIB *)ncs_patricia_tree_getnext(&cb->libname_asciispec_tree, NULL))) {
		if (lib_entry->lib_hdl != NULL)
			dlclose(lib_entry->lib_hdl);
		ncs_patricia_tree_del(&cb->libname_asciispec_tree, (NCS_PATRICIA_NODE *)lib_entry);
		m_MMGR_FREE_DTS_LIBNAME(lib_entry);
	}

	return NCSCC_RC_SUCCESS;
}
