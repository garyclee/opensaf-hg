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

   Vinay Khanna
..............................................................................

  DESCRIPTION:
   This module deals with the creation, accessing and deletion of the CLM
   database on the AVND.
..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

#include "avnd.h"

/* static function declarations */
static uns32 avnd_clmdb_rec_free(NCS_DB_LINK_LIST_NODE *);

/****************************************************************************
  Name          : avnd_clmdb_init
 
  Description   : This routine initializes the CLM database.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_clmdb_init(AVND_CB *cb)
{
	AVND_CLM_DB *clmdb = &cb->clmdb;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* timestamp this node */
	clmdb->node_info.bootTimestamp = time(NULL) * SA_TIME_ONE_SECOND;

	/* initialize the clm dll list */
	clmdb->clm_list.order = NCS_DBLIST_ASSCEND_ORDER;
	clmdb->clm_list.cmp_cookie = avsv_dblist_uns32_cmp;
	clmdb->clm_list.free_cookie = avnd_clmdb_rec_free;

	m_AVND_LOG_CLM_DB(AVND_LOG_CLM_DB_CREATE, AVND_LOG_CLM_DB_SUCCESS, 0, NCSFL_SEV_INFO);

	return rc;
}

/****************************************************************************
  Name          : avnd_clmdb_destroy
 
  Description   : This routine destroys the CLM database. It deletes all the 
                  records in the CLM database.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_clmdb_destroy(AVND_CB *cb)
{
	AVND_CLM_DB *clmdb = &cb->clmdb;
	AVND_CLM_REC *rec = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* traverse & delete all the CLM records */
	while (0 != (rec = (AVND_CLM_REC *)m_NCS_DBLIST_FIND_FIRST(&clmdb->clm_list))) {
		rc = avnd_clmdb_rec_del(cb, rec->info.node_id);
		if (NCSCC_RC_SUCCESS != rc)
			goto err;
	}

	/* traverse & delete all the CLM track request records TBD */

	m_AVND_LOG_CLM_DB(AVND_LOG_CLM_DB_DESTROY, AVND_LOG_CLM_DB_SUCCESS, 0, NCSFL_SEV_INFO);
	return rc;

 err:
	m_AVND_LOG_CLM_DB(AVND_LOG_CLM_DB_DESTROY, AVND_LOG_CLM_DB_FAILURE, 0, NCSFL_SEV_CRITICAL);
	return rc;
}

// TODO remove later when we have a new CLM...
static void amf_to_clm_node(const SaNameT *amf_node, SaNameT *clm_node)
{
	#include <immutil.h>
        SaAisErrorT rc;
        SaImmAccessorHandleT accessorHandle;
        const SaImmAttrValuesT_2 **attributes;
	SaImmAttrNameT attributeNames[] = {"saAmfNodeClmNode", NULL};

        immutil_saImmOmAccessorInitialize(avnd_cb->immOmHandle, &accessorHandle);

        rc = immutil_saImmOmAccessorGet_2(accessorHandle, amf_node,
                attributeNames, (SaImmAttrValuesT_2 ***)&attributes);
	assert(rc == SA_AIS_OK);

	rc = immutil_getAttr("saAmfNodeClmNode", attributes, 0, clm_node);
	assert(rc == SA_AIS_OK);

        immutil_saImmOmAccessorFinalize(accessorHandle);
}

/****************************************************************************
  Name          : avnd_clmdb_rec_add
 
  Description   : This routine adds a record (node) to the CLM database. If 
                  the record is already present, it is modified with the new 
                  parameters.
 
  Arguments     : cb        - ptr to the AvND control block
                  node_info - ptr to the node params
 
  Return Values : ptr to the newly added/modified record
 
  Notes         : None.
******************************************************************************/
AVND_CLM_REC *avnd_clmdb_rec_add(AVND_CB *cb, AVSV_CLM_INFO *node_info)
{
	AVND_CLM_DB *clmdb = &cb->clmdb;
	AVND_CLM_REC *rec = 0;
	uns32 rc = NCSCC_RC_SUCCESS;

	/* get the record, if any */
	rec = avnd_clmdb_rec_get(cb, node_info->node_id);
	if (!rec) {
		/* a new record.. alloc & link it to the dll */
		rec = calloc(1, sizeof(AVND_CLM_REC));
		if (rec) {
			/* update the record key */
			rec->info.node_id = node_info->node_id;
			rec->clm_dll_node.key = (uns8 *)&rec->info.node_id;

			rc = ncs_db_link_list_add(&clmdb->clm_list, &rec->clm_dll_node);
			if (NCSCC_RC_SUCCESS != rc)
				goto done;
		} else {
			rc = NCSCC_RC_FAILURE;
			goto done;
		}
	}

	/* update the params */
	rec->info.node_address = node_info->node_address;
	amf_to_clm_node(&node_info->node_name, &rec->info.node_name);
	rec->info.member = node_info->member;
	rec->info.boot_timestamp = node_info->boot_timestamp;
	rec->info.view_number = node_info->view_number;

	m_AVND_LOG_CLM_DB(AVND_LOG_CLM_DB_REC_ADD, AVND_LOG_CLM_DB_SUCCESS, node_info->node_id, NCSFL_SEV_INFO);

 done:
	if (NCSCC_RC_SUCCESS != rc) {
		m_AVND_LOG_CLM_DB(AVND_LOG_CLM_DB_REC_ADD, AVND_LOG_CLM_DB_FAILURE, node_info->node_id, NCSFL_SEV_INFO);
		if (rec) {
			avnd_clmdb_rec_free(&rec->clm_dll_node);
			rec = 0;
		}
	}

	return rec;
}

/****************************************************************************
  Name          : avnd_clmdb_rec_del
 
  Description   : This routine deletes (unlinks & frees) the specified record
                  (node) from the CLM database.
 
  Arguments     : cb      - ptr to the AvND control block
                  node_id - node-id of the node that is to be deleted
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_clmdb_rec_del(AVND_CB *cb, SaClmNodeIdT node_id)
{
	AVND_CLM_DB *clmdb = &cb->clmdb;
	uns32 rc = NCSCC_RC_SUCCESS;

	rc = ncs_db_link_list_del(&clmdb->clm_list, (uns8 *)&node_id);

	if (NCSCC_RC_SUCCESS == rc)
		m_AVND_LOG_CLM_DB(AVND_LOG_CLM_DB_REC_DEL, AVND_LOG_CLM_DB_SUCCESS, node_id, NCSFL_SEV_INFO);
	else
		m_AVND_LOG_CLM_DB(AVND_LOG_CLM_DB_REC_DEL, AVND_LOG_CLM_DB_FAILURE, node_id, NCSFL_SEV_INFO);

	return rc;
}

/****************************************************************************
  Name          : avnd_clmdb_rec_get
 
  Description   : This routine retrives the specified record (node) from the 
                  CLM database.
 
  Arguments     : cb      - ptr to the AvND control block
                  node_id - node-id of the node that is to be retrived
 
  Return Values : ptr to the specified record (if present)
 
  Notes         : None.
******************************************************************************/
AVND_CLM_REC *avnd_clmdb_rec_get(AVND_CB *cb, SaClmNodeIdT node_id)
{
	AVND_CLM_DB *clmdb = &cb->clmdb;

	return (AVND_CLM_REC *)ncs_db_link_list_find(&clmdb->clm_list, (uns8 *)&node_id);
}

/****************************************************************************
  Name          : avnd_clmdb_rec_free
 
  Description   : This routine free the memory alloced to the specified 
                  record in the CLM database.
 
  Arguments     : node - ptr to the dll node
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_clmdb_rec_free(NCS_DB_LINK_LIST_NODE *node)
{
	AVND_CLM_REC *rec = (AVND_CLM_REC *)node;

	free(rec);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : avnd_clm_trkinfo_list_add
 
  Description   : This routine adds avnd_clm_track_info structure to the list 
                  in AVND_CLM_DB
 
  Arguments     : cb - AVND control block
                  trk_info - pointer to AVND_CLM_TRK_INFO structure.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : Add the node in the beginning (order is not imp).
******************************************************************************/
uns32 avnd_clm_trkinfo_list_add(AVND_CB *cb, AVND_CLM_TRK_INFO *trk_info)
{
	AVND_CLM_DB *db = &cb->clmdb;

	if (!trk_info) {
		/* LOG ERROR */
		return NCSCC_RC_FAILURE;
	}

	if (db->clm_trk_info == NULL) {
		/* This is the first track info add it in the front */
		db->clm_trk_info = trk_info;
		trk_info->next = NULL;
	} else {
		trk_info->next = db->clm_trk_info;
		db->clm_trk_info = trk_info;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : avnd_clm_trkinfo_list_del
 
  Description   : This routine removes avnd_clm_track_info structure from the list 
                  in AVND_CLM_DB
 
  Arguments     : cb - AVND control block
                  hdl - clm handle (index )
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : Search and remove the node This routine doesnt free memory
******************************************************************************/

AVND_CLM_TRK_INFO *avnd_clm_trkinfo_list_del(AVND_CB *cb, SaClmHandleT hdl, MDS_DEST *dest)
{
	AVND_CLM_TRK_INFO *i_ptr = NULL;
	AVND_CLM_TRK_INFO **p_ptr;
	AVND_CLM_DB *db = &cb->clmdb;

	if (!hdl) {
		/* LOG ERROR */
		return NULL;
	}

	i_ptr = db->clm_trk_info;
	p_ptr = &(db->clm_trk_info);
	while (i_ptr != NULL) {
		if ((i_ptr->req_hdl == hdl) && (memcmp(&i_ptr->mds_dest, dest, sizeof(MDS_DEST)) == 0)) {
			*p_ptr = i_ptr->next;
			return i_ptr;
		}
		p_ptr = &(i_ptr->next);
		i_ptr = i_ptr->next;
	}

	return NULL;
}

/****************************************************************************
  Name          : avnd_clm_trkinfo_list_find
 
  Description   : This routine finds avnd_clm_track_info structure from the list 
                  in AVND_CLM_DB
 
  Arguments     : cb - AVND control block
                  hdl - clm handle
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : 
******************************************************************************/

AVND_CLM_TRK_INFO *avnd_clm_trkinfo_list_find(AVND_CB *cb, SaClmHandleT hdl, MDS_DEST *dest)
{
	AVND_CLM_TRK_INFO *i_ptr = NULL;
	AVND_CLM_DB *db = &cb->clmdb;

	if ((!db->clm_trk_info) || (!hdl)) {
		/* LOG ERROR */
		return NULL;
	}

	i_ptr = db->clm_trk_info;
	while ((i_ptr != NULL) && (i_ptr->req_hdl != hdl) && (memcmp(&i_ptr->mds_dest, dest, sizeof(MDS_DEST)) != 0)) {
		i_ptr = i_ptr->next;
	}

	if ((i_ptr != NULL) && (i_ptr->req_hdl == hdl) && (memcmp(&i_ptr->mds_dest, dest, sizeof(MDS_DEST)) == 0)) {
		return i_ptr;
	}

	return NULL;
}

