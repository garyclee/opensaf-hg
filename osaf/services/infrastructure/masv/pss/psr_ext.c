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

  DESCRIPTION:

  This file contains additional routines of PSS.
  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#if (NCS_MAB == 1)
#if (NCS_PSR == 1)
#include "psr.h"
#include "psr_rfmt.h"

/*****************************************************************************

  PROCEDURE NAME:    pss_process_tbl_bind

  DESCRIPTION:       Process the Table-Bind request received from the OAA

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_process_tbl_bind(MAB_MSG *msg)
{
	PSS_PWE_CB *pwe_cb = NULL;
	PSS_OAA_ENTRY *oaa_node = NULL;
	PSS_CLIENT_ENTRY *client_node = NULL;
	MAB_PSS_TBL_BIND_EVT *bind_evt = NULL;
	MAB_PSS_TBL_LIST *tbl_list = NULL;
	PSS_TBL_REC *trec = NULL;
	uns32 retval = NCSCC_RC_SUCCESS;

	pwe_cb = (PSS_PWE_CB *)msg->yr_hdl;
	if (pwe_cb == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_BIND_INVLD_PWE_HDL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_PSS_RE_TBL_BIND_SYNC(pwe_cb, msg);	/* Send async-update to Standby */

	{
		char addr_str[255] = { 0 };

		if (m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card) == 0)
			sprintf(addr_str, "VDEST:%d", m_MDS_GET_VDEST_ID_FROM_MDS_DEST(msg->fr_card));
		else
			sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%d",
				m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card), 0, (uns32)(msg->fr_card));
		/* This MIB table or PCN is not available with PSS. */
		ncs_logmsg(NCS_SERVICE_ID_PSS, PSS_LID_HDLN_C, PSS_FC_HDLN,
			   NCSFL_LC_HEADLINE, NCSFL_SEV_DEBUG,
			   NCSFL_TYPE_TIC, PSS_HDLN_TBL_BIND_RCVD_FROM_MDSDEST, addr_str);
	}

	if ((msg->data.data.oac_pss_tbl_bind.pcn_list.pcn == NULL) ||
	    (msg->data.data.oac_pss_tbl_bind.pcn_list.tbl_list == NULL)) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_NULL_TBL_BIND);
		return NCSCC_RC_SUCCESS;
	}

	for (tbl_list = msg->data.data.oac_pss_tbl_bind.pcn_list.tbl_list; tbl_list != NULL; tbl_list = tbl_list->next) {
		m_LOG_PSS_TBL_BIND_EVT(NCSFL_SEV_DEBUG, PSS_TBL_BIND_RCVD, msg->data.data.oac_pss_tbl_bind.pcn_list.pcn,
				       tbl_list->tbl_id);
	}

	/* Lookup MDS_DEST of the OAA from the oaa_tree */
	oaa_node = pss_findadd_entry_in_oaa_tree(pwe_cb, &msg->fr_card, TRUE);
	if (oaa_node == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_BIND_FAIL);
		return NCSCC_RC_FAILURE;
	}

	bind_evt = &msg->data.data.oac_pss_tbl_bind;
	while (bind_evt != NULL) {
		/* Get PCN from the client_table */
		client_node = pss_find_client_entry(pwe_cb, bind_evt->pcn_list.pcn, TRUE);
		if (client_node == NULL) {
			m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_BIND_FAIL);
			return NCSCC_RC_FAILURE;
		}

		tbl_list = bind_evt->pcn_list.tbl_list;
		while (tbl_list != NULL) {
			if ((tbl_list->tbl_id < MIB_UD_TBL_ID_END) &&
			    (pwe_cb->p_pss_cb->mib_tbl_desc[tbl_list->tbl_id] != NULL)) {
				/* Only MIBS whose definitions are available with PSS */
				trec = pss_find_table_tree(pwe_cb, client_node, oaa_node, tbl_list->tbl_id, TRUE, NULL);
				if (trec == NULL) {
					m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_TBL_REC_ADD_FAIL, tbl_list->tbl_id);
					return NCSCC_RC_FAILURE;
				}
				m_LOG_PSS_TBL_BIND_EVT(NCSFL_SEV_DEBUG, PSS_TBL_BIND_DONE, bind_evt->pcn_list.pcn,
						       tbl_list->tbl_id);
			} else {
				m_LOG_PSS_TBL_BIND_EVT(NCSFL_SEV_ERROR, PSS_TBL_BIND_MIB_DESC_NULL,
						       msg->data.data.oac_pss_tbl_bind.pcn_list.pcn, tbl_list->tbl_id);
			}
			tbl_list = tbl_list->next;
		}
		bind_evt = bind_evt->next;
	}

	if (retval != NCSCC_RC_SUCCESS)
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_REQUEST_PROCESS_ERR);
	else
		m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_TBL_BIND_SUCCESS);

	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_process_tbl_unbind

  DESCRIPTION:       Process the Table-Unbind request received from the OAA

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 pss_process_tbl_unbind(MAB_MSG *msg)
{
	PSS_PWE_CB *pwe_cb = NULL;
	PSS_OAA_ENTRY *oaa_node = NULL;
	PSS_CLIENT_ENTRY *client_node = NULL;
	PSS_OAA_CLT_ID *oaa_clt_node = NULL, *prv_oaa_clt_node = NULL;
	PSS_TBL_REC *trec = NULL, *prv_trec = NULL, *del_trec = NULL;
	uns32 retval = NCSCC_RC_SUCCESS, bucket = 0;

	pwe_cb = (PSS_PWE_CB *)msg->yr_hdl;
	if (pwe_cb == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_UNBIND_INVLD_PWE_HDL);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	m_LOG_PSS_TBL_UNBIND_EVT(NCSFL_SEV_NOTICE, PSS_TBL_UNBIND_RCVD, msg->data.data.oac_pss_tbl_unbind.tbl_id);

	m_PSS_RE_TBL_UNBIND_SYNC(pwe_cb, msg);	/* Send async-update to Standby */

	if ((msg->data.data.oac_pss_tbl_unbind.tbl_id >= MIB_UD_TBL_ID_END) ||
	    (pwe_cb->p_pss_cb->mib_tbl_desc[msg->data.data.oac_pss_tbl_unbind.tbl_id] == NULL)) {
		m_LOG_PSS_TBL_UNBIND_EVT(NCSFL_SEV_ERROR, PSS_TBL_UNBIND_IS_NOT_PERSISTENT_TABLE,
					 msg->data.data.oac_pss_tbl_unbind.tbl_id);
		return NCSCC_RC_SUCCESS;
	}

	/* Lookup MDS_DEST of the OAA from the oaa_tree */
	oaa_node = pss_findadd_entry_in_oaa_tree(pwe_cb, &msg->fr_card, FALSE);
	if (oaa_node == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_UNBIND_FAIL);
		return NCSCC_RC_SUCCESS;
	}

	bucket = msg->data.data.oac_pss_tbl_unbind.tbl_id % MAB_MIB_ID_HASH_TBL_SIZE;
	oaa_clt_node = oaa_node->hash[bucket];
	while (oaa_clt_node != NULL) {
		if (oaa_clt_node->tbl_rec->tbl_id == msg->data.data.oac_pss_tbl_unbind.tbl_id) {
			m_LOG_PSS_TBL_UNBIND_EVT(NCSFL_SEV_INFO, PSS_TBL_UNBIND_FND_OAA_CLT_NODE,
						 msg->data.data.oac_pss_tbl_unbind.tbl_id);
			break;
		}
		prv_oaa_clt_node = oaa_clt_node;
		oaa_clt_node = oaa_clt_node->next;
	}
	if (oaa_clt_node == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_UNBIND_OAA_CLT_NODE_NOT_FOUND);
		return NCSCC_RC_SUCCESS;
	}
	del_trec = oaa_clt_node->tbl_rec;
	if (prv_oaa_clt_node != NULL)
		prv_oaa_clt_node->next = oaa_clt_node->next;
	else
		oaa_node->hash[bucket] = oaa_clt_node->next;
	m_MMGR_FREE_MAB_PSS_OAA_CLT_ID(oaa_clt_node);
	--oaa_node->tbl_cnt;
	if (oaa_node->tbl_cnt == 0) {
		/* Now delete and free the OAA-node */
		NCS_PATRICIA_NODE *pNode = &oaa_node->node;
		ncs_patricia_tree_del(&pwe_cb->oaa_tree, pNode);

		m_MMGR_FREE_PSS_OAA_ENTRY(oaa_node);
		oaa_node = NULL;
	}

	/* Get PCN from the client_table */
	client_node = pss_find_client_entry(pwe_cb, (char *)&del_trec->pss_client_key->pcn, FALSE);
	if (client_node == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_TBL_UNBIND_PCN_NODE_NOT_FOUND);
		return NCSCC_RC_SUCCESS;
	}

	bucket = msg->data.data.oac_pss_tbl_unbind.tbl_id % MAB_MIB_ID_HASH_TBL_SIZE;
	trec = client_node->hash[bucket];
	while (trec != NULL) {
		if (trec->tbl_id == msg->data.data.oac_pss_tbl_unbind.tbl_id) {
			if (prv_trec == NULL) {
				client_node->hash[bucket] = trec->next;
			} else
				prv_trec->next = trec->next;

			/* Commit the tree onto the persistent store. */
			if (trec->is_scalar) {
				retval = pss_save_to_sclr_store(pwe_cb, trec,
								NULL, (char *)&trec->pss_client_key->pcn, trec->tbl_id);
				if (trec->info.scalar.data != NULL) {
					m_MMGR_FREE_PSS_OCT(trec->info.scalar.data);
					trec->info.scalar.data = NULL;
				}
			} else {
				if (trec->info.other.tree_inited == TRUE) {
					retval = pss_save_to_store(pwe_cb, &trec->info.other.data, NULL,
								   (char *)&trec->pss_client_key->pcn, trec->tbl_id);
					ncs_patricia_tree_destroy(&trec->info.other.data);
					trec->info.other.tree_inited = FALSE;
				}
			}
			if (pwe_cb->p_pss_cb->save_type == PSS_SAVE_TYPE_ON_DEMAND) {
				/* Delete "trec" from pwe_cb->refresh_tbl_list */
				pss_del_rec_frm_refresh_tbl_list(pwe_cb, trec);
			}
			client_node->tbl_cnt--;
			if (client_node->tbl_cnt == 0) {
				NCS_PATRICIA_NODE *pNode = &client_node->pat_node;

				m_LOG_PSS_CLIENT_ENTRY(NCSFL_SEV_INFO, PSS_CLIENT_ENTRY_DEL,
						       (char *)&trec->pss_client_key->pcn, pwe_cb->pwe_id);
				ncs_patricia_tree_del(&pwe_cb->client_table, pNode);
				m_MMGR_FREE_PSS_CLIENT_ENTRY(client_node);
				client_node = NULL;
			}
			m_MMGR_FREE_PSS_TBL_REC(trec);
			break;
		}
		prv_trec = trec;
		trec = trec->next;
	}

	if (retval != NCSCC_RC_SUCCESS)
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_REQUEST_PROCESS_ERR);
	else
		m_LOG_PSS_TBL_UNBIND_EVT(NCSFL_SEV_NOTICE, PSS_TBL_UNBIND_SUCCESS,
					 msg->data.data.oac_pss_tbl_unbind.tbl_id);

	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_findadd_entry_frm_spcnlist

  DESCRIPTION:       Get entry of the client from the SPCN-list.

  RETURNS:           Pointer to PSS_SPCN_LIST entry, if search successful.
                     NULL, if entry not found.

*****************************************************************************/
PSS_SPCN_LIST *pss_findadd_entry_frm_spcnlist(PSS_CB *inst, char *p_pcn, NCS_BOOL add)
{
	PSS_SPCN_LIST *list = inst->spcn_list, *prv_list = NULL;
	uns32 str_len = strlen(p_pcn);

	for (; list != NULL; list = list->next) {
		if (strcmp(list->pcn, p_pcn) == 0) {
			return list;
		}
		prv_list = list;
	}
	if (add == FALSE) {
		return NULL;
	}

	if ((list = m_MMGR_ALLOC_MAB_PSS_SPCN_LIST) == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_SPCN_LIST_ALLOC_FAIL, "pss_findadd_entry_frm_spcnlist()");
		return NULL;
	}
	memset(list, '\0', sizeof(PSS_SPCN_LIST));
	if ((list->pcn = m_MMGR_ALLOC_MAB_PCN_STRING(str_len + 1)) == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_PCN_STRING_ALLOC_FAIL, "pss_findadd_entry_frm_spcnlist()");
		m_MMGR_FREE_MAB_PSS_SPCN_LIST(list);
		return NULL;
	}
	memset(list->pcn, '\0', str_len + 1);
	strcpy(list->pcn, p_pcn);
	if (prv_list != NULL)
		prv_list->next = list;
	else
		inst->spcn_list = list;

	return list;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_free_wbreq_list

  DESCRIPTION:       Free contents of warmboot request from client.

  RETURNS:           void

*****************************************************************************/
void pss_free_wbreq_list(MAB_MSG *msg)
{
	MAB_PSS_WARMBOOT_REQ *req = &msg->data.data.oac_pss_warmboot_req, *nxt_req = NULL;

	if (req->pcn_list.pcn != NULL) {
		m_MMGR_FREE_MAB_PCN_STRING(req->pcn_list.pcn);
		req->pcn_list.pcn = NULL;
	}
	if (req->pcn_list.tbl_list != NULL) {
		pss_free_tbl_list(req->pcn_list.tbl_list);
		req->pcn_list.tbl_list = NULL;
	}

	req = req->next;	/* First element is not an allocated memory element. */
	while (req != NULL) {
		nxt_req = req->next;

		if (req->pcn_list.pcn != NULL) {
			m_MMGR_FREE_MAB_PCN_STRING(req->pcn_list.pcn);
			req->pcn_list.pcn = NULL;
		}
		if (req->pcn_list.tbl_list != NULL) {
			pss_free_tbl_list(req->pcn_list.tbl_list);
			req->pcn_list.tbl_list = NULL;
		}

		m_MMGR_FREE_MAB_PSS_WARMBOOT_REQ(req);
		req = nxt_req;
	}
	return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_free_tbl_list

  DESCRIPTION:       Free the table-list of type MAB_PSS_TBL_LIST.

  RETURNS:           void

*****************************************************************************/
void pss_free_tbl_list(MAB_PSS_TBL_LIST *tbl_list)
{
	MAB_PSS_TBL_LIST *tmp = NULL;
	while (tbl_list != NULL) {
		tmp = tbl_list;
		tbl_list = tbl_list->next;
		m_MMGR_FREE_MAB_PSS_TBL_LIST(tmp);
	}
	return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_sort_wbreq_instore_tables_with_rank

  DESCRIPTION:       Sort the In-memory MIB tables in the sort-database, 
                     according to the MIB rank. If any MIB table does not
                     have data to be played-back, set the "*snd_evt_to_bam"
                     boolean to TRUE, and return NCSCC_RC_SUCCESS.

  RETURNS:           NCSCC_RC_SUCCESS, if operation successful.
                     Appropriate error-value, if any error encountered.

*****************************************************************************/
uns32 pss_sort_wbreq_instore_tables_with_rank(PSS_PWE_CB *pwe_cb,
					      PSS_WBPLAYBACK_SORT_TABLE *sortdb,
					      PSS_CLIENT_ENTRY *client_node,
					      PSS_SPCN_LIST *spcn_entry, NCS_BOOL *snd_evt_to_bam, NCS_UBAID *uba)
{
	uns32 i = 0, cnt = 0, no_data_cnt = 0;
	uns8 *p8 = NULL, *p8_cnt = NULL;
	PSS_TBL_REC *rec = NULL;
	PSS_WBSORT_ENTRY *entry = NULL;
	NCS_PATRICIA_TREE *sort_db = &sortdb->sort_db;
	NCS_BOOL init_done = FALSE;

	memset(uba, '\0', sizeof(NCS_UBAID));
	for (i = 0; i < MAB_MIB_ID_HASH_TBL_SIZE; i++) {
		rec = client_node->hash[i];
		while (rec != NULL) {
			if (pwe_cb->p_pss_cb->mib_tbl_desc[rec->tbl_id] == NULL) {
				rec = rec->next;
				continue;
			}
			cnt++;
			if (pss_data_available_for_table(pwe_cb, (char *)&client_node->key.pcn, rec) == TRUE) {
				m_LOG_PSS_WBREQ_II(NCSFL_SEV_INFO, PSS_WBREQ_II_SORT_FNC_TBL_DATA_AVAILABLE,
						   rec->tbl_id);

				if ((entry = m_MMGR_ALLOC_PSS_WBSORT_ENTRY) == NULL) {
					m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_PSS_WBSORT_ENTRY_ALLOC_FAIL,
							  "pss_sort_wbreq_instore_tables_with_rank()");
					if (uba->start != NULL) {
						m_MMGR_FREE_BUFR_LIST(uba->start);
						uba->start = NULL;
					}
					memset(uba, '\0', sizeof(NCS_UBAID));
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				}
				memset(entry, '\0', sizeof(PSS_WBSORT_ENTRY));
				entry->key.rank = pwe_cb->p_pss_cb->mib_tbl_desc[rec->tbl_id]->ptbl_info->table_rank;
				entry->key.tbl_id = rec->tbl_id;
				entry->tbl_rec = rec;
				entry->pat_node.key_info = (uns8 *)&entry->key;

				/* Insert in the patricia tree */
				if (ncs_patricia_tree_add(sort_db, &entry->pat_node) != NCSCC_RC_SUCCESS) {
					m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_WBSORTDB_ADD_ENTRY_FAIL);
					m_MMGR_FREE_PSS_WBSORT_ENTRY(entry);
					if (uba->start != NULL) {
						m_MMGR_FREE_BUFR_LIST(uba->start);
						uba->start = NULL;
					}
					memset(uba, '\0', sizeof(NCS_UBAID));
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				}
				++sortdb->num_entries;
				m_LOG_PSS_WBREQ_II(NCSFL_SEV_INFO, PSS_WBREQ_II_TBL_ADDED_TO_SORT_DB, rec->tbl_id);
				/* Encode table-id into uba to be sent to all MAAs before starting playback. */
				if (init_done == FALSE) {
					if (ncs_enc_init_space(uba) != NCSCC_RC_SUCCESS) {
						if (uba->start != NULL) {
							m_MMGR_FREE_BUFR_LIST(uba->start);
							uba->start = NULL;
						}
						memset(uba, '\0', sizeof(NCS_UBAID));
						ncs_patricia_tree_del(sort_db, &entry->pat_node);
						m_MMGR_FREE_PSS_WBSORT_ENTRY(entry);
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}
					init_done = TRUE;
					p8_cnt = ncs_enc_reserve_space(uba, sizeof(uns16));
					if (p8_cnt == NULL) {
						if (uba->start != NULL) {
							m_MMGR_FREE_BUFR_LIST(uba->start);
							uba->start = NULL;
						}
						memset(uba, '\0', sizeof(NCS_UBAID));
						ncs_patricia_tree_del(sort_db, &entry->pat_node);
						m_MMGR_FREE_PSS_WBSORT_ENTRY(entry);
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}
					ncs_enc_claim_space(uba, sizeof(uns16));
				}
				p8 = ncs_enc_reserve_space(uba, sizeof(uns16));
				if (p8 == NULL) {
					if (uba->start != NULL) {
						m_MMGR_FREE_BUFR_LIST(uba->start);
						uba->start = NULL;
					}
					memset(uba, '\0', sizeof(NCS_UBAID));
					ncs_patricia_tree_del(sort_db, &entry->pat_node);
					m_MMGR_FREE_PSS_WBSORT_ENTRY(entry);
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				}
				ncs_encode_16bit(&p8, (uns16)rec->tbl_id);
				ncs_enc_claim_space(uba, sizeof(uns16));
			} else {
				/* If data not available in persistent store for any MIB of the SPCN,
				   forward the request to BAM */
				if (spcn_entry != NULL) {
					no_data_cnt++;
					m_LOG_PSS_WBREQ_II(NCSFL_SEV_DEBUG,
							   PSS_WBREQ_II_SORT_FNC_SPCN_TBL_DATA_NOT_AVAILABLE,
							   rec->tbl_id);
				} else {
					m_LOG_PSS_WBREQ_II(NCSFL_SEV_INFO, PSS_WBREQ_II_SORT_FNC_TBL_DATA_NOT_AVAILABLE,
							   rec->tbl_id);
				}
			}
			rec = rec->next;
		}
	}
	if (no_data_cnt == cnt) {
		*snd_evt_to_bam = TRUE;
		if (uba->start != NULL) {
			m_MMGR_FREE_BUFR_LIST(uba->start);
			uba->start = NULL;
		}
		memset(uba, '\0', sizeof(NCS_UBAID));
	}
	if (p8_cnt != NULL)
		ncs_encode_16bit(&p8_cnt, (uns16)sortdb->num_entries);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_sort_wbreq_tables_with_rank

  DESCRIPTION:       Sort the MIB tables(specified in the input warmboot request)
                     in the sort-database, according to the MIB rank. If any 
                     MIB table does not have data to be played-back, set the 
                     "*snd_evt_to_bam" boolean to TRUE, and return 
                     NCSCC_RC_SUCCESS.

  RETURNS:           NCSCC_RC_SUCCESS, if operation successful.
                     Appropriate error-value, if any error encountered.

*****************************************************************************/
uns32 pss_sort_wbreq_tables_with_rank(PSS_PWE_CB *pwe_cb,
				      PSS_WBPLAYBACK_SORT_TABLE *sortdb,
				      PSS_CLIENT_ENTRY *client_node,
				      PSS_SPCN_LIST *spcn_entry,
				      MAB_PSS_TBL_LIST *wbreq_head, NCS_BOOL *snd_evt_to_bam, NCS_UBAID *uba)
{
	PSS_TBL_REC *rec = NULL;
	PSS_WBSORT_ENTRY *entry = NULL;
	MAB_PSS_TBL_LIST *wbreq = NULL;
	NCS_PATRICIA_TREE *sort_db = &sortdb->sort_db;
	NCS_BOOL init_done = FALSE;
	uns8 *p8 = NULL, *p8_cnt = NULL;

	memset(uba, '\0', sizeof(NCS_UBAID));
	for (wbreq = wbreq_head; wbreq != NULL; wbreq = wbreq->next) {
		if (pwe_cb->p_pss_cb->mib_tbl_desc[wbreq->tbl_id] == NULL) {
			m_LOG_PSS_WBREQ_II(NCSFL_SEV_NOTICE, PSS_WBREQ_II_INPUT_TBL_NOT_PERSISTENT, wbreq->tbl_id);
			continue;
		}
		m_LOG_PSS_WBREQ_II(NCSFL_SEV_INFO, PSS_WBREQ_II_SORTING_INPUT_TBL, wbreq->tbl_id);

		for (rec = client_node->hash[wbreq->tbl_id % MAB_MIB_ID_HASH_TBL_SIZE]; rec != NULL; rec = rec->next) {
			if (rec->tbl_id == wbreq->tbl_id) {
				m_LOG_PSS_WBREQ_II(NCSFL_SEV_DEBUG, PSS_WBREQ_II_SORT_FNC_INPUT_TBL_FND_IN_PCN_NODE,
						   wbreq->tbl_id);
				break;
			}
		}
		if (rec != NULL) {
			if (pss_data_available_for_table(pwe_cb, (char *)&client_node->key.pcn, rec) == TRUE) {
				m_LOG_PSS_WBREQ_II(NCSFL_SEV_DEBUG, PSS_WBREQ_II_SORT_FNC_TBL_DATA_AVAILABLE,
						   wbreq->tbl_id);
				if ((entry = m_MMGR_ALLOC_PSS_WBSORT_ENTRY) == NULL) {
					m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_PSS_WBSORT_ENTRY_ALLOC_FAIL,
							  "pss_sort_wbreq_tables_with_rank()");
					if (uba->start != NULL) {
						m_MMGR_FREE_BUFR_LIST(uba->start);
						uba->start = NULL;
					}
					memset(uba, '\0', sizeof(NCS_UBAID));
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				}

				memset(entry, '\0', sizeof(PSS_WBSORT_ENTRY));
				entry->key.rank = pwe_cb->p_pss_cb->mib_tbl_desc[rec->tbl_id]->ptbl_info->table_rank;
				entry->key.tbl_id = rec->tbl_id;
				entry->tbl_rec = rec;
				entry->pat_node.key_info = (uns8 *)&entry->key;

				/* Insert in the patricia tree */
				if (ncs_patricia_tree_add(sort_db, &entry->pat_node) != NCSCC_RC_SUCCESS) {
					m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_WBSORTDB_ADD_ENTRY_FAIL);
					m_MMGR_FREE_PSS_WBSORT_ENTRY(entry);
					if (uba->start != NULL) {
						m_MMGR_FREE_BUFR_LIST(uba->start);
						uba->start = NULL;
					}
					memset(uba, '\0', sizeof(NCS_UBAID));
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				}
				++sortdb->num_entries;
				m_LOG_PSS_WBREQ_II(NCSFL_SEV_INFO, PSS_WBREQ_II_TBL_ADDED_TO_SORT_DB, wbreq->tbl_id);

				/* Encode table-id into uba to be sent to all MAAs before starting playback. */
				if (init_done == FALSE) {
					if (ncs_enc_init_space(uba) != NCSCC_RC_SUCCESS) {
						if (uba->start != NULL) {
							m_MMGR_FREE_BUFR_LIST(uba->start);
							uba->start = NULL;
						}
						memset(uba, '\0', sizeof(NCS_UBAID));
						ncs_patricia_tree_del(sort_db, &entry->pat_node);
						m_MMGR_FREE_PSS_WBSORT_ENTRY(entry);
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}
					init_done = TRUE;
					p8_cnt = ncs_enc_reserve_space(uba, sizeof(uns16));
					if (p8_cnt == NULL) {
						if (uba->start != NULL) {
							m_MMGR_FREE_BUFR_LIST(uba->start);
							uba->start = NULL;
						}
						memset(uba, '\0', sizeof(NCS_UBAID));
						ncs_patricia_tree_del(sort_db, &entry->pat_node);
						m_MMGR_FREE_PSS_WBSORT_ENTRY(entry);
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}
					ncs_enc_claim_space(uba, sizeof(uns16));
				}
				p8 = ncs_enc_reserve_space(uba, sizeof(uns16));
				if (p8 == NULL) {
					if (uba->start != NULL) {
						m_MMGR_FREE_BUFR_LIST(uba->start);
						uba->start = NULL;
					}
					memset(uba, '\0', sizeof(NCS_UBAID));
					ncs_patricia_tree_del(sort_db, &entry->pat_node);
					m_MMGR_FREE_PSS_WBSORT_ENTRY(entry);
					return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				}
				ncs_encode_16bit(&p8, (uns16)rec->tbl_id);
				ncs_enc_claim_space(uba, sizeof(uns16));
			} else {
				/* If data not available in persistent store for any MIB of the SPCN,
				   forward the request to BAM */
				if (spcn_entry != NULL) {
					m_LOG_PSS_WBREQ_II(NCSFL_SEV_DEBUG,
							   PSS_WBREQ_II_SORT_FNC_SPCN_TBL_DATA_NOT_AVAILABLE,
							   wbreq->tbl_id);
					*snd_evt_to_bam = TRUE;
					if (uba->start != NULL) {
						m_MMGR_FREE_BUFR_LIST(uba->start);
						uba->start = NULL;
					}
					memset(uba, '\0', sizeof(NCS_UBAID));
					return NCSCC_RC_SUCCESS;
				} else {
					m_LOG_PSS_WBREQ_II(NCSFL_SEV_DEBUG,
							   PSS_WBREQ_II_SORT_FNC_TBL_DATA_NOT_AVAILABLE, wbreq->tbl_id);
				}
			}
		} else {
			m_LOG_PSS_WBREQ_II(NCSFL_SEV_ERROR, PSS_WBREQ_II_SORT_FNC_INPUT_TBL_NOT_FND_IN_PCN_NODE,
					   wbreq->tbl_id);
		}
	}
	if (p8_cnt != NULL)
		ncs_encode_16bit(&p8_cnt, (uns16)sortdb->num_entries);
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_send_wbreq_to_bam

  DESCRIPTION:       Send warmboot request to BAM instance, for the particular
                     system-client.

  RETURNS:           NCSCC_RC_SUCCESS, if operation successful.
                     Appropriate error-value, if any error encountered.

*****************************************************************************/
uns32 pss_send_wbreq_to_bam(PSS_PWE_CB *pwe_cb, char *pcn)
{
	PSS_BAM_MSG pmsg;
	uns32 code = NCSCC_RC_SUCCESS;

	if (!pwe_cb->p_pss_cb->is_bam_alive) {
		pss_add_entry_to_spcn_wbreq_pend_list(pwe_cb, pcn);
		/* BAM is not alive in the system. Log error and return. */
		m_LOG_PSS_BAM_REQ(NCSFL_SEV_ERROR, PSS_BAM_REQ_FAIL_BAM_NOT_ALIVE, pcn);
		return NCSCC_RC_FAILURE;
	}

	memset(&pmsg, '\0', sizeof(pmsg));
	pmsg.i_evt = PSS_BAM_EVT_WARMBOOT_REQ;
	pmsg.info.warmboot_req.pcn = pcn;	/* Only pointer is sufficient here. */

	m_LOG_PSS_PLBCK_INFO(NCSFL_SEV_NOTICE, PSS_PLBCK_REQ_TO_BAM, pcn, pwe_cb->pwe_id);

	code = mab_mds_snd(pwe_cb->mds_pwe_handle, &pmsg, NCSMDS_SVC_ID_PSS,
			   NCSMDS_SVC_ID_BAM, pwe_cb->p_pss_cb->bam_address);
	if (code == NCSCC_RC_SUCCESS) {
		m_LOG_PSS_BAM_REQ(NCSFL_SEV_NOTICE, PSS_BAM_REQ_SENT, pcn);
		/* Saving the MIB sets in-memory till the conf_done is received */
		pwe_cb->p_pss_cb->bam_req_cnt++;
		pwe_cb->p_pss_cb->save_type = PSS_SAVE_TYPE_ON_DEMAND;
	} else {
		m_LOG_PSS_BAM_REQ(NCSFL_SEV_ERROR, PSS_BAM_REQ_FAIL_MDS_ERROR, pcn);
	}

	/* If there is any pending list now, what action should PSS take? */

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_sort_odreq_instore_tables_with_rank

  DESCRIPTION:       Sort all the In-memory MIB tables in the sort-database, 
                     according to the MIB rank.

  RETURNS:           NCSCC_RC_SUCCESS, if operation successful.
                     Appropriate error-value, if any error encountered.

*****************************************************************************/
uns32 pss_sort_odreq_instore_tables_with_rank(PSS_PWE_CB *pwe_cb, PSS_ODPLAYBACK_SORT_TABLE *sortdb)
{
	PSS_ODSORT_ENTRY *entry = NULL;
	PSS_ODSORT_TBL_REC *od_tblrec = NULL;
	PSS_OAA_KEY oaa_key;
	PSS_OAA_ENTRY *oaa_entry = NULL;
	PSS_OAA_CLT_ID *oaa_clt_entry = NULL;
	NCS_PATRICIA_NODE *pNode = NULL, *pat_node = NULL;
	PSS_TBL_REC *rec = NULL;
	uns32 i = 0;

	/* For all clients active in the system, get the list of tables that need
	   be played back, and insert the list into the sort-database.
	   The index to this sort-database is <rank><table-id>.
	   All instances of the same MIB table, belonging to different clients are
	   arranged in the form of a linked-list.
	 */
	memset(&oaa_key, '\0', sizeof(oaa_key));

	while ((pNode = ncs_patricia_tree_getnext(&pwe_cb->oaa_tree, (const uns8 *)&oaa_key)) != NULL) {
		oaa_entry = (PSS_OAA_ENTRY *)pNode;
		oaa_key = oaa_entry->key;

		for (i = 0; i < MAB_MIB_ID_HASH_TBL_SIZE; i++) {
			for (oaa_clt_entry = oaa_entry->hash[i]; oaa_clt_entry != NULL;
			     oaa_clt_entry = oaa_clt_entry->next) {
				rec = oaa_clt_entry->tbl_rec;

				/* Add table record pointer into sort-db */
				/* if(pss_data_available_for_table(pwe_cb,
				   (char*)&oaa_clt_entry->tbl_rec->pss_client_key->pcn, rec) == TRUE) */
				if (pwe_cb->p_pss_cb->mib_tbl_desc[rec->tbl_id] != NULL) {
					PSS_ODSORT_KEY lcl_key;

					memset(&lcl_key, '\0', sizeof(lcl_key));
					lcl_key.rank =
					    pwe_cb->p_pss_cb->mib_tbl_desc[rec->tbl_id]->ptbl_info->table_rank;
					lcl_key.tbl_id = rec->tbl_id;
					pat_node = ncs_patricia_tree_get(&sortdb->sort_db, (uns8 *)&lcl_key);
					if (pat_node != NULL) {
						/* Entry is already there. Now, add the table-rec pointer */
						entry = (PSS_ODSORT_ENTRY *)pat_node;
						if (entry == NULL) {
							/* Should not hit this, unless PSS blew itself up!!! */
							return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
						}
					} /* if (pat_node != NULL) */
					else {
						/* Create a new node in the patricia-tree */
						if ((entry = m_MMGR_ALLOC_PSS_ODSORT_ENTRY) == NULL) {
							m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL,
									  PSS_MF_PSS_ODSORT_ENTRY_ALLOC_FAIL,
									  "pss_sort_odreq_instore_tables_with_rank()");
							return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
						}

						memset(entry, '\0', sizeof(PSS_ODSORT_ENTRY));
						entry->key = lcl_key;
						entry->pat_node.key_info = (uns8 *)&entry->key;

						/* Insert in the patricia tree */
						if (ncs_patricia_tree_add(&sortdb->sort_db, &entry->pat_node) !=
						    NCSCC_RC_SUCCESS) {
							m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL,
									   PSS_HDLN_ODSORTDB_ADD_ENTRY_FAIL);
							m_MMGR_FREE_PSS_WBSORT_ENTRY(entry);
							return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
						}
					}
					if ((od_tblrec = m_MMGR_ALLOC_PSS_ODSORT_TBL_REC) == NULL) {
						m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL,
								  PSS_MF_PSS_ODSORT_TBL_REC_ALLOC_FAIL,
								  "pss_sort_odreq_instore_tables_with_rank()");
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}

					memset(od_tblrec, '\0', sizeof(PSS_ODSORT_TBL_REC));

					/* Update the linked list */
					od_tblrec->tbl_rec = rec;
					od_tblrec->next = entry->tbl_rec_list;
					entry->tbl_rec_list = od_tblrec;
					sortdb->num_entries++;
				}
			}
		}
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_ondemand_playback_for_sorted_list

  DESCRIPTION:       Perform playback of the table entries in the 
                     sort-database. The entries are available sorted according
                     to <mib-rank> <table-id>.

  RETURNS:           NCSCC_RC_SUCCESS, if operation successful.
                     Appropriate error-value, if any error encountered.

*****************************************************************************/
uns32 pss_ondemand_playback_for_sorted_list(PSS_PWE_CB *pwe_cb, PSS_ODPLAYBACK_SORT_TABLE *sortdb, uns8 *profile)
{
	PSS_ODSORT_KEY lcl_key;
	PSS_ODSORT_ENTRY *entry = NULL;
	PSS_ODSORT_TBL_REC *od_tblrec = NULL;
	NCS_PATRICIA_NODE *pNode = NULL;
	PSS_TBL_REC *rec = NULL;
	NCS_QUEUE add_q, chg_q;
	uns32 retval, sort_cnt = 0;
	NCS_BOOL last_plbck_evt = FALSE;
#if (NCS_PSS_RED == 1)
	NCS_BOOL lcl_cont_ssn = FALSE;
#endif

	memset(&lcl_key, '\0', sizeof(lcl_key));

#if (NCS_PSS_RED == 1)
	if (pwe_cb->processing_pending_active_events == FALSE) {
		lcl_cont_ssn = TRUE;
	}
#endif

	while ((pNode = ncs_patricia_tree_getnext(&sortdb->sort_db, (const uns8 *)&lcl_key)) != NULL) {
		entry = (PSS_ODSORT_ENTRY *)pNode;
		lcl_key = entry->key;

		for (od_tblrec = entry->tbl_rec_list, sort_cnt = sortdb->num_entries;
		     od_tblrec != NULL; od_tblrec = od_tblrec->next, --sort_cnt) {
			rec = od_tblrec->tbl_rec;

			/* if(pss_data_available_for_table(pwe_cb,
			   (char*)&rec->pss_client_key->pcn, rec) == TRUE) */
			if (pwe_cb->p_pss_cb->mib_tbl_desc[rec->tbl_id] != NULL) {
				/* Perform playback for this MIB table. */
				if (sortdb->num_entries == 1) {
					last_plbck_evt = TRUE;
				}
				if (pwe_cb->p_pss_cb->mib_tbl_desc[rec->tbl_id]->ptbl_info->table_of_scalars) {
					pss_playback_process_sclr_tbl(pwe_cb, profile, rec,
#if (NCS_PSS_RED == 1)
								      &lcl_cont_ssn,
#endif
								      last_plbck_evt);
				} else {
					ncs_create_queue(&add_q);
					ncs_create_queue(&chg_q);

					if (pss_playback_process_tbl(pwe_cb, profile, rec, &add_q, &chg_q,
#if (NCS_PSS_RED == 1)
								     &lcl_cont_ssn,
#endif
								     last_plbck_evt) != NCSCC_RC_SUCCESS) {
						pss_destroy_queue(&add_q);
						pss_destroy_queue(&chg_q);
#if (NCS_PSS_RED == 1)
						pss_indicate_end_of_playback_to_standby(pwe_cb, TRUE);
#endif
						m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_ODPLBCK_OP_FAIL);
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}

					/* Now operate the add/modify queue-lists */
					retval = pss_playback_process_queue(pwe_cb,
#if (NCS_PSS_RED == 1)
									    &lcl_cont_ssn,
#endif
									    &add_q, &chg_q);
					pss_destroy_queue(&add_q);
					pss_destroy_queue(&chg_q);
					if (retval != NCSCC_RC_SUCCESS) {
#if (NCS_PSS_RED == 1)
						pss_indicate_end_of_playback_to_standby(pwe_cb, TRUE);
#endif
						m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_ODPLBCK_OP_FAIL);
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}
				}
			}
#if (NCS_PSS_RED == 1)
			if (pwe_cb->processing_pending_active_events == TRUE) {
				if ((lcl_cont_ssn == FALSE) && (pwe_cb->curr_plbck_ssn_info.tbl_id == rec->tbl_id)) {
					lcl_cont_ssn = TRUE;	/* This is the table which was last completed in the previous playback session */
				}
			}
#endif
		}
		od_tblrec = entry->tbl_rec_list;
		while (od_tblrec != NULL) {
			PSS_ODSORT_TBL_REC *next_od_tblrec = od_tblrec->next;
			m_MMGR_FREE_PSS_ODSORT_TBL_REC(od_tblrec);
			od_tblrec = next_od_tblrec;
		}
		retval = ncs_patricia_tree_del(&sortdb->sort_db, pNode);
		if (retval != NCSCC_RC_SUCCESS)
			return retval;
		m_MMGR_FREE_PSS_ODSORT_ENTRY(entry);
	}
	sortdb->num_entries = sort_cnt;
#if (NCS_PSS_RED == 1)
	pss_indicate_end_of_playback_to_standby(pwe_cb, FALSE);
#endif

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_destroy_odsort_db

  DESCRIPTION:       Destroy the on-demand sort-database.

  RETURNS:           void.

*****************************************************************************/
void pss_destroy_odsort_db(PSS_ODPLAYBACK_SORT_TABLE *sort_db)
{
	PSS_ODSORT_KEY lcl_key;
	NCS_PATRICIA_NODE *pNode = NULL;
	PSS_ODSORT_ENTRY *entry = NULL;
	PSS_ODSORT_TBL_REC *od_tblrec = NULL;

	memset(&lcl_key, '\0', sizeof(lcl_key));
	while ((pNode = ncs_patricia_tree_getnext(&sort_db->sort_db, (const uns8 *)&lcl_key)) != NULL) {
		entry = (PSS_ODSORT_ENTRY *)pNode;
		lcl_key = entry->key;

		od_tblrec = entry->tbl_rec_list;
		while (od_tblrec != NULL) {
			PSS_ODSORT_TBL_REC *next_tbl_rec = od_tblrec->next;
			m_MMGR_FREE_PSS_ODSORT_TBL_REC(od_tblrec);
			od_tblrec = next_tbl_rec;
		}
		if (ncs_patricia_tree_del(&sort_db->sort_db, pNode) != NCSCC_RC_SUCCESS)
			return;
		m_MMGR_FREE_PSS_ODSORT_ENTRY(entry);
	}
	sort_db->num_entries = 0;
	ncs_patricia_tree_destroy(&sort_db->sort_db);
	return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_destroy_wbsort_db

  DESCRIPTION:       Destroy the warmboot sort-database.

  RETURNS:           void.

*****************************************************************************/
void pss_destroy_wbsort_db(PSS_WBPLAYBACK_SORT_TABLE *sort_db)
{
	PSS_WBSORT_KEY lcl_key;
	NCS_PATRICIA_NODE *pNode = NULL;
	PSS_WBSORT_ENTRY *entry = NULL;

	memset(&lcl_key, '\0', sizeof(lcl_key));
	while ((pNode = ncs_patricia_tree_getnext(&sort_db->sort_db, (const uns8 *)&lcl_key)) != NULL) {
		entry = (PSS_WBSORT_ENTRY *)pNode;
		lcl_key = entry->key;

		if (ncs_patricia_tree_del(&sort_db->sort_db, pNode) != NCSCC_RC_SUCCESS)
			return;
		m_MMGR_FREE_PSS_WBSORT_ENTRY(entry);
		sort_db->num_entries--;
	}
	ncs_patricia_tree_destroy(&sort_db->sort_db);
	return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_reset_boolean_in_spcn_entry

  DESCRIPTION:       Resets the boolean in the spcn_entry. Also, updates the
                     boolean in the config file pwe_cb->p_pss_cb->spcn_list_file

  RETURNS:           void.

*****************************************************************************/
void pss_reset_boolean_in_spcn_entry(PSS_PWE_CB *pwe_cb, PSS_SPCN_LIST *spcn_entry)
{
	spcn_entry->plbck_frm_bam = FALSE;	/* Further Playback requests to be done from 
						   the persistent store. */

	/* Update to the config file pwe_cb->p_pss_cb->spcn_list_file */
	if (pss_update_entry_in_spcn_conf_file(pwe_cb->p_pss_cb, spcn_entry)
	    != NCSCC_RC_SUCCESS) {
		/* Log error. */
		return;

	}

	return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_flush_mib_entries_from_curr_profile

  DESCRIPTION:       Flush all MIB entries of the specified system-client both
                     from the in-memory store, and also the persistent-store.

  RETURNS:           NCSCC_RC_SUCCESS, if operation successful.
                     NCSCC_RC_FAILURE, if error encountered.

*****************************************************************************/
uns32 pss_flush_mib_entries_from_curr_profile(PSS_PWE_CB *pwe_cb, PSS_SPCN_LIST *spcn_entry, PSS_CLIENT_ENTRY *clt_node)
{
	uns32 retval = NCSCC_RC_SUCCESS;
	uns32 i = 0, cnt = 0;
	PSS_TBL_REC *trec = NULL;

	/* Look into the persistent store, and delete non-recent/non-registerd MIBs of this client. */
	m_NCS_PSSTS_PCN_DELETE(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl,
			       retval, pwe_cb->p_pss_cb->current_profile, pwe_cb->pwe_id, spcn_entry->pcn);

	if ((cnt = clt_node->tbl_cnt) == 0)
		return NCSCC_RC_SUCCESS;

	/* Flush the MIB entries of all MIBs of this client, from the 
	   in-memory store and also the persistent store. */
	for (i = 0; i < MAB_MIB_ID_HASH_TBL_SIZE; i++) {
		trec = clt_node->hash[i];
		while (trec != NULL) {
			/* Commit the tree onto the persistent store. */
			if (!trec->is_scalar) {
				(void)pss_table_tree_destroy(&trec->info.other.data, TRUE);
				/* Flush MIB entries from persistent store */
				m_NCS_PSSTS_FILE_DELETE(pwe_cb->p_pss_cb->pssts_api,
							pwe_cb->p_pss_cb->pssts_hdl, retval,
							pwe_cb->p_pss_cb->current_profile, pwe_cb->pwe_id,
							spcn_entry->pcn, trec->tbl_id);
			}
			--cnt;
			trec = trec->next;
		}
		if (cnt == 0)
			break;
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_wb_playback_for_sorted_list

  DESCRIPTION:       Perform playback for MIB tables present in the warmboot
                     sort-database.

  RETURNS:           NCSCC_RC_SUCCESS, if operation successful.
                     NCSCC_RC_FAILURE, if error encountered.

*****************************************************************************/
uns32 pss_wb_playback_for_sorted_list(PSS_PWE_CB *pwe_cb, PSS_WBPLAYBACK_SORT_TABLE *sort_db,
				      PSS_CLIENT_ENTRY *node, char *p_pcn)
{
	NCS_PATRICIA_TREE *pTree = &sort_db->sort_db;
	NCS_PATRICIA_NODE *pNode = NULL;
	PSS_WBSORT_ENTRY *entry = NULL;
	uns32 retval, start_xch_id = 0;
	NCS_BOOL is_end_of_playback = FALSE;
#if (NCS_PSS_RED == 1)
	NCS_BOOL lcl_cont_ssn = FALSE;
#endif

	m_LOG_PSS_WBREQ_I(NCSFL_SEV_INFO, PSS_WBREQ_I_PROCESS_SORTED_LIST_START, p_pcn, pwe_cb->pwe_id);

#if (NCS_PSS_RED == 1)
	if (pwe_cb->processing_pending_active_events == FALSE) {
		lcl_cont_ssn = TRUE;
	}
#endif

	/* Walk through each element of the tree and free the memory */
	while (sort_db->num_entries != 0) {
		pNode = ncs_patricia_tree_getnext(pTree, NULL);
		if ((entry = (PSS_WBSORT_ENTRY *)pNode) == NULL) {	/* No more nodes */
			m_LOG_PSS_WBREQ_I(NCSFL_SEV_INFO, PSS_WBREQ_I_PROCESS_SORTED_LIST_END, p_pcn, pwe_cb->pwe_id);
			ncs_patricia_tree_destroy(pTree);
			return NCSCC_RC_SUCCESS;
		}

		if (sort_db->num_entries == 1)
			is_end_of_playback = TRUE;

		start_xch_id = pwe_cb->p_pss_cb->xch_id;

		m_LOG_PSS_WBREQ_II(NCSFL_SEV_INFO, PSS_WBREQ_II_PROCESS_TBL_OF_SORT_DB, entry->tbl_rec->tbl_id);

		retval = NCSCC_RC_SUCCESS;
		if (pwe_cb->p_pss_cb->mib_tbl_desc[entry->tbl_rec->tbl_id] != NULL) {
			if (pwe_cb->p_pss_cb->mib_tbl_desc[entry->tbl_rec->tbl_id]->ptbl_info->table_of_scalars)
				retval = pss_oac_warmboot_process_sclr_tbl(pwe_cb, p_pcn, entry->tbl_rec, node,
#if (NCS_PSS_RED == 1)
									   &lcl_cont_ssn,
#endif
									   is_end_of_playback);
			else
				retval = pss_oac_warmboot_process_tbl(pwe_cb, p_pcn, entry->tbl_rec, node,
#if (NCS_PSS_RED == 1)
								      &lcl_cont_ssn,
#endif
								      is_end_of_playback);
		}
#if (NCS_PSS_RED == 1)
		if (pwe_cb->processing_pending_active_events == TRUE) {
			if ((lcl_cont_ssn == FALSE) && (pwe_cb->curr_plbck_ssn_info.tbl_id == entry->tbl_rec->tbl_id)) {
				lcl_cont_ssn = TRUE;	/* This is the table which was last completed in the previous playback session */
			}
		}
#endif
		if (retval != NCSCC_RC_SUCCESS) {
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_WARMBOOT_TBL_ERR);
		}
		if (start_xch_id != pwe_cb->p_pss_cb->xch_id) {
			m_LOG_PSS_PLBCK_SET_COUNT(NCSFL_SEV_NOTICE, PSS_PLBCK_SET_COUNT,
						  p_pcn, entry->tbl_rec->tbl_id,
						  pwe_cb->p_pss_cb->mib_tbl_desc[entry->tbl_rec->tbl_id]->capability,
						  (uns32)(pwe_cb->p_pss_cb->xch_id - start_xch_id));
		}

		retval = ncs_patricia_tree_del(pTree, pNode);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_WBREQ_I(NCSFL_SEV_ERROR, PSS_WBREQ_I_PROCESS_SORTED_LIST_ABORT,
					  p_pcn, pwe_cb->pwe_id);
			return retval;
		}

		m_MMGR_FREE_PSS_WBSORT_ENTRY(pNode);
		sort_db->num_entries--;
	}

	pNode = ncs_patricia_tree_getnext(pTree, NULL);
	if (pNode != NULL)
		return NCSCC_RC_FAILURE;

	ncs_patricia_tree_destroy(pTree);
	m_LOG_PSS_WBREQ_I(NCSFL_SEV_INFO, PSS_WBREQ_I_PROCESS_SORTED_LIST_END, p_pcn, pwe_cb->pwe_id);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_data_available_for_table

  DESCRIPTION:       Determine whether there is any MIB data to be played-back
                     for the specified MIB table of the particular client.

  RETURNS:           TRUE, if data available to be played-back.
                     FALSE, otherwise.

*****************************************************************************/
NCS_BOOL pss_data_available_for_table(PSS_PWE_CB *pwe_cb, char *p_pcn, PSS_TBL_REC *trec)
{
	NCS_PATRICIA_TREE *pTree;
	NCS_PATRICIA_NODE *pNode;
	PSS_MIB_TBL_DATA *pData;
	uns32 retval = NCSCC_RC_SUCCESS, bytes_read = 0;
	uns32 buf_size = 0, read_offset = 0;
	uns8 *in_buf = NULL;
	NCS_BOOL file_exists = FALSE;
	long file_hdl = 0;

	if (pwe_cb->p_pss_cb->mib_tbl_desc[trec->tbl_id]->ptbl_info->table_of_scalars) {
		if (trec->dirty == TRUE) {
			/* Data is present. */
			m_LOG_PSS_STORE(NCSFL_SEV_DEBUG, PSS_MIB_EXISTS_IN_INMEMORY_STORE,
					pwe_cb->pwe_id, p_pcn, trec->tbl_id);
			return TRUE;
		}
	} else {
		pTree = &trec->info.other.data;
		pNode = ncs_patricia_tree_getnext(pTree, NULL);
		pData = (PSS_MIB_TBL_DATA *)pNode;
		if (pData != NULL) {
			/* Data is present. */
			m_LOG_PSS_STORE(NCSFL_SEV_DEBUG, PSS_MIB_EXISTS_IN_INMEMORY_STORE,
					pwe_cb->pwe_id, p_pcn, trec->tbl_id);
			return TRUE;
		}
	}

	/* Check if the table file exists */
	m_NCS_PSSTS_FILE_EXISTS(pwe_cb->p_pss_cb->pssts_api,
				pwe_cb->p_pss_cb->pssts_hdl, retval,
				pwe_cb->p_pss_cb->current_profile, pwe_cb->pwe_id, p_pcn, trec->tbl_id, file_exists);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_EXISTENCE_IN_STORE_OP_FAIL,
				pwe_cb->pwe_id, p_pcn, trec->tbl_id);
		return FALSE;
	}
	if (file_exists == FALSE) {
		return FALSE;
	}

	/* Open the table file for reading */
	m_NCS_PSSTS_FILE_OPEN(pwe_cb->p_pss_cb->pssts_api,
			      pwe_cb->p_pss_cb->pssts_hdl, retval,
			      pwe_cb->p_pss_cb->current_profile, pwe_cb->pwe_id, p_pcn,
			      trec->tbl_id, NCS_PSSTS_FILE_PERM_READ, file_hdl);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_OPEN_FAIL, pwe_cb->pwe_id, p_pcn, trec->tbl_id);
		return FALSE;
	}

	buf_size = pwe_cb->p_pss_cb->mib_tbl_desc[trec->tbl_id]->max_row_length;
	in_buf = m_MMGR_ALLOC_PSS_OCT(buf_size);
	if (in_buf == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_data_available_for_table()");
		m_NCS_PSSTS_FILE_CLOSE(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl, retval, file_hdl);
		return FALSE;
	}
	memset(in_buf, '\0', buf_size);
	read_offset = PSS_TABLE_DETAILS_HEADER_LEN;
	m_NCS_PSSTS_FILE_READ(pwe_cb->p_pss_cb->pssts_api,
			      pwe_cb->p_pss_cb->pssts_hdl, retval, file_hdl, buf_size, read_offset, in_buf, bytes_read);
	if (retval != NCSCC_RC_SUCCESS) {
		m_MMGR_FREE_PSS_OCT(in_buf);
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_READ_FAIL, pwe_cb->pwe_id, p_pcn, trec->tbl_id);
		m_NCS_PSSTS_FILE_CLOSE(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl, retval, file_hdl);
		return FALSE;
	}
	m_NCS_PSSTS_FILE_CLOSE(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl, retval, file_hdl);

	m_LOG_PSS_STORE(NCSFL_SEV_DEBUG, PSS_MIB_EXISTS_ON_PERSISTENT_STORE, pwe_cb->pwe_id, p_pcn, trec->tbl_id);
	/* The buffer gets allocated by PSSv-Target-Service API. So, to be freed now, since it
	   is not required any longer. */
	if (in_buf != NULL)
		m_MMGR_FREE_PSS_OCT(in_buf);

	if (bytes_read != 0)
		return TRUE;

	return FALSE;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_handle_oaa_down_event

  DESCRIPTION:       This function deletes all patricia trees associated with 
                     an OAA that just went down

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_handle_oaa_down_event(PSS_PWE_CB *pwe_cb, MDS_DEST *fr_card)
{
	PSS_OAA_ENTRY *oaa_node = NULL;
	PSS_OAA_CLT_ID *oaa_clt_node = NULL, *prv_oaa_clt_node = NULL;
	PSS_TBL_REC *rec = NULL, *trec = NULL, *prv_trec = NULL;
	PSS_CLIENT_ENTRY *client_node = NULL;
	uns32 retval = NCSCC_RC_SUCCESS, i;
	char addr_str[255] = { 0 };

	oaa_node = pss_findadd_entry_in_oaa_tree(pwe_cb, fr_card, FALSE);
	if ((oaa_node == NULL) || (oaa_node->tbl_cnt == 0)) {
		/* No valid tables from this OAA. So, peace!!!! */
		return NCSCC_RC_SUCCESS;
	}

	/* convert the MDS_DEST into a string */
	if (m_NCS_NODE_ID_FROM_MDS_DEST(*fr_card) == 0)
		sprintf(addr_str, "VDEST:%d", m_MDS_GET_VDEST_ID_FROM_MDS_DEST(*fr_card));
	else
		sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%d",
			m_NCS_NODE_ID_FROM_MDS_DEST(*fr_card), 0, (uns32)(*fr_card));

	/* now log the message */
	ncs_logmsg(NCS_SERVICE_ID_PSS, PSS_LID_NO_CB, PSS_FC_HDLN,
		   NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, NCSFL_TYPE_TIC, PSS_HDLN_OAC_DATA_DELETING, addr_str);

	{
		MAB_MSG lcl_msg;
		memset(&lcl_msg, '\0', sizeof(lcl_msg));
		lcl_msg.fr_card = *fr_card;
		m_PSS_RE_OAA_DOWN_SYNC(pwe_cb, &lcl_msg);
	}

	for (i = 0; i < MAB_MIB_ID_HASH_TBL_SIZE; i++) {
		for (oaa_clt_node = oaa_node->hash[i]; oaa_clt_node != NULL;
		     prv_oaa_clt_node = oaa_clt_node, oaa_clt_node = oaa_clt_node->next) {
			if (prv_oaa_clt_node != NULL) {
				m_MMGR_FREE_MAB_PSS_OAA_CLT_ID(prv_oaa_clt_node);
				prv_oaa_clt_node = NULL;
			}
			rec = oaa_clt_node->tbl_rec;

			if (rec->is_scalar) {
				retval = pss_save_to_sclr_store(pwe_cb, rec, NULL,
								(char *)&rec->pss_client_key->pcn, rec->tbl_id);
				if (rec->info.scalar.data != NULL) {
					m_MMGR_FREE_PSS_OCT(rec->info.scalar.data);
					rec->info.scalar.data = NULL;
				}
			} else {
				if (rec->info.other.tree_inited == TRUE) {
					retval = pss_save_to_store(pwe_cb, &rec->info.other.data, NULL,
								   (char *)&rec->pss_client_key->pcn, rec->tbl_id);
					ncs_patricia_tree_destroy(&rec->info.other.data);
					rec->info.other.tree_inited = FALSE;
				}
			}
			if (retval != NCSCC_RC_SUCCESS) {
				m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_SAVE_TBL_ERROR);
				retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}

			client_node = pss_find_client_entry(pwe_cb, (char *)&rec->pss_client_key->pcn, FALSE);
			for (prv_trec = NULL, trec = client_node->hash[rec->tbl_id % MAB_MIB_ID_HASH_TBL_SIZE];
			     trec != NULL; prv_trec = trec, trec = trec->next) {
				if (trec->tbl_id == rec->tbl_id) {
					/* Increment the pointer to the next node. */
					if (prv_trec == NULL)
						client_node->hash[rec->tbl_id % MAB_MIB_ID_HASH_TBL_SIZE] = trec->next;
					else
						prv_trec->next = trec->next;
					break;
				}
			}
			--client_node->tbl_cnt;
			if (client_node->tbl_cnt == 0) {
				/* Last MIB table of the client is gone now. Delete the client_node */
				NCS_PATRICIA_NODE *pNode = &client_node->pat_node;
				ncs_patricia_tree_del(&pwe_cb->client_table, pNode);
				m_MMGR_FREE_PSS_CLIENT_ENTRY(client_node);
				client_node = NULL;
			}

			if (pwe_cb->p_pss_cb->save_type == PSS_SAVE_TYPE_ON_DEMAND) {
				/* Delete "rec" from pwe_cb->refresh_tbl_list */
				pss_del_rec_frm_refresh_tbl_list(pwe_cb, rec);
			}

			/* Free the table-record now */
			m_MMGR_FREE_PSS_TBL_REC(rec);
		}
		if (prv_oaa_clt_node != NULL) {
			m_MMGR_FREE_MAB_PSS_OAA_CLT_ID(prv_oaa_clt_node);
			prv_oaa_clt_node = NULL;
		}
	}
	if (prv_oaa_clt_node != NULL) {
		m_MMGR_FREE_MAB_PSS_OAA_CLT_ID(prv_oaa_clt_node);
		prv_oaa_clt_node = NULL;
	}

	/* Now delete and free the OAA-node */
	{
		NCS_PATRICIA_NODE *pNode = &oaa_node->node;
		ncs_patricia_tree_del(&pwe_cb->oaa_tree, pNode);

		m_MMGR_FREE_PSS_OAA_ENTRY(oaa_node);
		oaa_node = NULL;
	}

	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_OAC_DATA_DELETED);
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_process_bam_conf_done

  DESCRIPTION:       This function handles BAM conf-done message.

  RETURNS:           SUCCESS - All went well
                     FAILURE - Something went wrong. Turn on m_MAB_DBG_SINK()
                                for details

*****************************************************************************/
uns32 pss_process_bam_conf_done(MAB_MSG *msg)
{
	PSS_PWE_CB *pwe_cb = NULL;
	PSS_SPCN_LIST *spcn_entry = NULL;
	uns32 retval = NCSCC_RC_SUCCESS;

	if (msg->data.data.bam_conf_done.pcn_list.pcn == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVLD_CONF_DONE_MSG_RCVD);
		return NCSCC_RC_FAILURE;
	}
	m_LOG_PSS_CONF_DONE(NCSFL_SEV_NOTICE, PSS_CONF_DONE_RCVD, msg->data.data.bam_conf_done.pcn_list.pcn);
	pwe_cb = (PSS_PWE_CB *)msg->yr_hdl;
	if (pwe_cb == NULL) {
		pss_free_tbl_list(msg->data.data.bam_conf_done.pcn_list.tbl_list);
		m_MMGR_FREE_MAB_PCN_STRING(msg->data.data.bam_conf_done.pcn_list.pcn);
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PWE_HDL);
		return NCSCC_RC_FAILURE;
	}

	m_PSS_RE_BAM_CONF_DONE(pwe_cb, msg);

	/* Now, search the spcn_list, and update the boolean value. */
	if ((spcn_entry = pss_findadd_entry_frm_spcnlist(pwe_cb->p_pss_cb,
							 (char *)msg->data.data.bam_conf_done.pcn_list.pcn,
							 FALSE)) == NULL) {
		m_LOG_PSS_CONF_DONE(NCSFL_SEV_ERROR, PSS_CONF_DONE_FAIL, msg->data.data.bam_conf_done.pcn_list.pcn);
		pss_free_tbl_list(msg->data.data.bam_conf_done.pcn_list.tbl_list);
		m_MMGR_FREE_MAB_PCN_STRING(msg->data.data.bam_conf_done.pcn_list.pcn);
		return NCSCC_RC_FAILURE;
	}

	pss_reset_boolean_in_spcn_entry(pwe_cb, spcn_entry);

	m_LOG_PSS_CONF_DONE(NCSFL_SEV_INFO, PSS_CONF_DONE_FINISHED, msg->data.data.bam_conf_done.pcn_list.pcn);

	pwe_cb->p_pss_cb->bam_req_cnt--;

	/* Only modify persistent store file system if we are active. */
	if (gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE) {
		if (pwe_cb->p_pss_cb->bam_req_cnt == 0) {
			pwe_cb->p_pss_cb->save_type = PSS_SAVE_TYPE_IMMEDIATE;
			if (NCSCC_RC_SUCCESS != pss_save_current_configuration(pwe_cb->p_pss_cb)) {
				pss_free_tbl_list(msg->data.data.bam_conf_done.pcn_list.tbl_list);
				m_MMGR_FREE_MAB_PCN_STRING(msg->data.data.bam_conf_done.pcn_list.pcn);
				return NCSCC_RC_FAILURE;
			}
		}
	}

	pss_free_tbl_list(msg->data.data.bam_conf_done.pcn_list.tbl_list);
	m_MMGR_FREE_MAB_PCN_STRING(msg->data.data.bam_conf_done.pcn_list.pcn);
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_findadd_entry_in_oaa_tree

  DESCRIPTION:       This function returns the pointer to PSS_OAA_ENTRY node
                     in the "oaa_tree" patricia tree of the PSS_PWE_CB., for
                     the particular <MDS_DEST> address.

  RETURNS:           Pointer to PSS_OAA_ENTRY
                     NULL, if not found or error encountered.
*****************************************************************************/
PSS_OAA_ENTRY *pss_findadd_entry_in_oaa_tree(PSS_PWE_CB *pwe_cb, MDS_DEST *mdest, NCS_BOOL add)
{
	NCS_PATRICIA_NODE *pat_node = NULL;
	PSS_OAA_KEY oaa_key;
	PSS_OAA_ENTRY *oaa_node = NULL;

	if ((pwe_cb == NULL) || (mdest == NULL)) {
		return (PSS_OAA_ENTRY *)m_MAB_DBG_SINK(NULL);
	}

	if (*mdest == 0) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_NOTICE, PSS_HDLN_ZERO_MDS_DEST);
		return NULL;
	}

	memset(&oaa_key, '\0', sizeof(oaa_key));
	oaa_key.mds_dest = *mdest;
	pat_node = ncs_patricia_tree_get(&pwe_cb->oaa_tree, (uns8 *)&oaa_key);
	if (pat_node != NULL) {
		oaa_node = (PSS_OAA_ENTRY *)pat_node;
	} else if (add) {
		if ((oaa_node = m_MMGR_ALLOC_PSS_OAA_ENTRY) == NULL) {
			m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OAA_ENTRY_ALLOC_FAIL,
					  "pss_findadd_entry_in_oaa_tree()");
			return (PSS_OAA_ENTRY *)m_MAB_DBG_SINK(NULL);
		}

		memset(oaa_node, '\0', sizeof(PSS_OAA_ENTRY));
		oaa_node->key = oaa_key;
		oaa_node->node.key_info = (uns8 *)&oaa_node->key;

		/* Insert in the patricia tree */
		if (ncs_patricia_tree_add(&pwe_cb->oaa_tree, &oaa_node->node) != NCSCC_RC_SUCCESS) {
			m_MMGR_FREE_PSS_OAA_ENTRY(oaa_node);
			char addr_str[255] = { 0 };

			/* convert the MDS_DEST into a string */
			if (m_NCS_NODE_ID_FROM_MDS_DEST(oaa_key.mds_dest) == 0)
				sprintf(addr_str, "VDEST:%d", m_MDS_GET_VDEST_ID_FROM_MDS_DEST(oaa_key.mds_dest));
			else
				sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%d",
					m_NCS_NODE_ID_FROM_MDS_DEST(oaa_key.mds_dest), 0, (uns32)(oaa_key.mds_dest));
			/* now log the message */
			ncs_logmsg(NCS_SERVICE_ID_PSS, PSS_LID_NO_CB, PSS_FC_HDLN,
				   NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
				   NCSFL_TYPE_TIC, PSS_HDLN_OAA_ENTRY_ADD_FAIL, addr_str);

			return (PSS_OAA_ENTRY *)m_MAB_DBG_SINK(NULL);
		}
	}
	return oaa_node;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_add_tbl_in_oaa_node

  DESCRIPTION:       This function returns the pointer to PSS_OAA_CLT_ID node
                     of the particular "tbl_id" in the "oaa_tree" patricia tree 
                     of the PSS_PWE_CB.

  RETURNS:           Pointer to PSS_OAA_CLT_ID
                     NULL, if not found or error encountered.
*****************************************************************************/
PSS_OAA_CLT_ID *pss_add_tbl_in_oaa_node(PSS_PWE_CB *pwe_cb, PSS_OAA_ENTRY *node, PSS_TBL_REC *trec)
{
	PSS_OAA_CLT_ID *oaa_clt_node = NULL, *prv_oaa_clt_node = NULL;

	for (oaa_clt_node = node->hash[trec->tbl_id % MAB_MIB_ID_HASH_TBL_SIZE];
	     oaa_clt_node != NULL; prv_oaa_clt_node = oaa_clt_node, oaa_clt_node = oaa_clt_node->next) {
		if (oaa_clt_node->tbl_rec->tbl_id == trec->tbl_id) {
			return oaa_clt_node;
		}
	}

	if ((oaa_clt_node = m_MMGR_ALLOC_MAB_PSS_OAA_CLT_ID) == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OAA_CLT_ID_ALLOC_FAIL, "pss_add_tbl_in_oaa_node()");
		return (PSS_OAA_CLT_ID *)m_MAB_DBG_SINK(NULL);
	}

	memset(oaa_clt_node, '\0', sizeof(PSS_OAA_CLT_ID));
	oaa_clt_node->tbl_rec = trec;
	if (prv_oaa_clt_node == NULL) {
		node->hash[trec->tbl_id % MAB_MIB_ID_HASH_TBL_SIZE] = oaa_clt_node;
	} else {
		prv_oaa_clt_node->next = oaa_clt_node;
	}
	++node->tbl_cnt;
	return oaa_clt_node;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_find_tbl_in_oaa_node

  DESCRIPTION:       This function returns the pointer to PSS_OAA_CLT_ID node
                     of the particular "tbl_id" in the "oaa_tree" patricia tree 
                     of the PSS_PWE_CB.

  RETURNS:           Pointer to PSS_OAA_CLT_ID
                     NULL, if not found or error encountered.
*****************************************************************************/
PSS_OAA_CLT_ID *pss_find_tbl_in_oaa_node(PSS_PWE_CB *pwe_cb, PSS_OAA_ENTRY *node, uns32 tbl_id)
{
	PSS_OAA_CLT_ID *oaa_clt_node = NULL;

	for (oaa_clt_node = node->hash[tbl_id % MAB_MIB_ID_HASH_TBL_SIZE];
	     oaa_clt_node != NULL; oaa_clt_node = oaa_clt_node->next) {
		if (oaa_clt_node->tbl_rec->tbl_id == tbl_id) {
			break;
		}
	}
	return oaa_clt_node;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_get_pcn_from_mdsdest

  DESCRIPTION:       This function returns the pointer to PCN name for this
                     MIB table in the particular PWE running on the specified
                     MDS_DEST address.

  RETURNS:           Pointer to PCN name(character string)
                     NULL, if not found or error encountered.
*****************************************************************************/
char *pss_get_pcn_from_mdsdest(PSS_PWE_CB *pwe_cb, MDS_DEST *mdest, uns32 tbl_id)
{
	PSS_OAA_ENTRY *oaa_node = NULL;
	PSS_OAA_CLT_ID *oaa_clt_node = NULL;

	if (pwe_cb == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PWE_CB);
		return (char *)m_MAB_DBG_SINK(NULL);
	}

	oaa_node = pss_findadd_entry_in_oaa_tree(pwe_cb, mdest, FALSE);
	if (oaa_node == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_OAA_NODE_NOT_FND_FOR_MDEST);
		return (char *)m_MAB_DBG_SINK(NULL);
	}

	oaa_clt_node = pss_find_tbl_in_oaa_node(pwe_cb, oaa_node, tbl_id);
	if (oaa_clt_node == NULL) {
		m_LOG_PSS_HDLN_I(NCSFL_SEV_ERROR, PSS_HDLN_TBL_NODE_NOT_FND_FOR_TBL_ID, tbl_id);
		return (char *)m_MAB_DBG_SINK(NULL);
	}

	return (char *)&oaa_clt_node->tbl_rec->pss_client_key->pcn;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_on_demand_playback

  DESCRIPTION:       This function processes an on-demand playback request
                     from a given profile

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                               for more details

*****************************************************************************/
uns32 pss_on_demand_playback(PSS_CB *inst, PSS_CSI_NODE *csi_list, uns8 *profile)
{
	uns32 retval = NCSCC_RC_SUCCESS;
	NCS_BOOL profile_exists = FALSE;
	PSS_PWE_CB *pwe_cb = NULL;
	PSS_CSI_NODE *t_csi = NULL;

	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_START_OD_PLAYBACK);

	m_NCS_PSSTS_PROFILE_EXISTS(inst->pssts_api, inst->pssts_hdl, retval, profile, profile_exists);
	if ((retval != NCSCC_RC_SUCCESS) || (profile_exists == FALSE)) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PROFILE_SPCFD);
		return NCSCC_RC_FAILURE;
	}

	for (t_csi = csi_list; t_csi; t_csi = t_csi->next) {
		PSS_ODPLAYBACK_SORT_TABLE lcl_sort_db;

		pwe_cb = t_csi->pwe_cb;
		memset(&lcl_sort_db, '\0', sizeof(lcl_sort_db));
		lcl_sort_db.sort_params.key_size = sizeof(PSS_WBSORT_KEY);
		if (ncs_patricia_tree_init(&lcl_sort_db.sort_db, &lcl_sort_db.sort_params)
		    != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_ODSORTDB_INIT_FAIL);
			goto end;
		}
		retval = pss_sort_odreq_instore_tables_with_rank(pwe_cb, &lcl_sort_db);
		if (retval != NCSCC_RC_SUCCESS) {
			pss_destroy_odsort_db(&lcl_sort_db);
			goto end;
		} else {
			if (pss_broadcast_playback_signal(t_csi, TRUE) != NCSCC_RC_SUCCESS) {
				goto end;
			}
			if (pss_ondemand_playback_for_sorted_list(pwe_cb, &lcl_sort_db, profile)
			    != NCSCC_RC_SUCCESS) {
				pss_broadcast_playback_signal(t_csi, FALSE);
				pss_destroy_odsort_db(&lcl_sort_db);
				goto end;
			}
			if (pss_broadcast_playback_signal(t_csi, FALSE) != NCSCC_RC_SUCCESS) {
				goto end;
			}
			pss_destroy_odsort_db(&lcl_sort_db);
		}
	}

	/* Delete the current configuration and overwrite it with the alternate profile */
	m_NCS_PSSTS_PROFILE_DELETE(inst->pssts_api, inst->pssts_hdl, retval, inst->current_profile);
	if (retval != NCSCC_RC_SUCCESS) {
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto end;
	}

	m_NCS_PSSTS_PROFILE_COPY(inst->pssts_api, inst->pssts_hdl, retval, inst->current_profile, profile);
	if (retval != NCSCC_RC_SUCCESS) {
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto end;
	}

 end:
	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_END_OD_PLAYBACK);
	return retval;
}

    /* These macros are used in PSS internal processing flows */
#define m_READ_DATA(inst, retval, file_hdl, buf_size, file_offset, buf, ptr, bytes_read, rows_left, max_row_length) \
    { \
        m_NCS_PSSTS_FILE_READ(inst->pssts_api, inst->pssts_hdl, retval, file_hdl, buf_size, file_offset, buf, bytes_read); \
        if (retval != NCSCC_RC_SUCCESS) \
        { \
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
            goto cleanup; \
        } \
        rows_left = bytes_read / max_row_length; \
        file_offset += (rows_left * max_row_length); \
        ptr = buf; \
    }

#define m_READ_DATA2(inst, retval, file_hdl, buf_size, file_offset, buf, ptr, bytes_read, rows_left, max_row_length) \
    { \
        m_NCS_PSSTS_FILE_READ(inst->pssts_api, inst->pssts_hdl, retval, file_hdl, buf_size, file_offset, buf, bytes_read); \
        if (retval != NCSCC_RC_SUCCESS) \
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
        rows_left = bytes_read / max_row_length; \
        file_offset += (rows_left * max_row_length); \
        ptr = buf; \
    }

#if (NCS_PSS_RED == 1)
    /* These macros are used in PSSv's redundancy operations. */
#define m_RE_REMROW_REQUEST(pwe_cb) \
        { \
           /* Send Async updates to Standby */ \
           pwe_cb->curr_plbck_ssn_info.tbl_id = tbl; \
           pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len = idx->i_inst_len; \
           memcpy(pinst_re_ids, idx->i_inst_ids, tbl_info->num_inst_ids * sizeof(uns32)); \
           pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids = pinst_re_ids; \
           pwe_cb->curr_plbck_ssn_info.mib_obj_id = 0; \
           m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info); \
        }
#define m_RE_N_REMROW_REQUEST(pwe_cb) \
        { \
           /* Send Async updates to Standby */ \
           pwe_cb->curr_plbck_ssn_info.tbl_id = tbl; \
           pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len = idx.i_inst_len; \
           memcpy(pinst_re_ids, idx.i_inst_ids, tbl_info->num_inst_ids * sizeof(uns32)); \
           pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids = pinst_re_ids; \
           pwe_cb->curr_plbck_ssn_info.mib_obj_id = 0; \
           m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info); \
        }
#define m_RE_REMROW_REQUEST2(pwe_cb) m_RE_REMROW_REQUEST(pwe_cb)
#define m_RE_SINGLE_REM_REQUEST(pwe_cb) \
        { \
           /* Send Async updates to Standby */ \
           pwe_cb->curr_plbck_ssn_info.tbl_id = tbl; \
           pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len = idx->i_inst_len; \
           memcpy(pinst_re_ids, idx->i_inst_ids, tbl_info->num_inst_ids * sizeof(uns32)); \
           pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids = pinst_re_ids; \
           pwe_cb->curr_plbck_ssn_info.mib_obj_id = status_param; \
           m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info); \
        }
#define m_RE_SINGLE_REM_REQUEST2(pwe_cb) \
        { \
           /* Send Async updates to Standby */ \
           pwe_cb->curr_plbck_ssn_info.tbl_id = tbl; \
           pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len = idx.i_inst_len; \
           memcpy(pinst_re_ids, idx.i_inst_ids, tbl_info->num_inst_ids * sizeof(uns32)); \
           pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids = pinst_re_ids; \
           pwe_cb->curr_plbck_ssn_info.mib_obj_id = 0; \
           m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info); \
        }
#endif

#define m_REMROW_REQUEST(pwe_cb) \
    { \
        ub = ncsremrow_enc_done(&rra); \
        rra_inited = FALSE; \
        if (ub == NULL) \
        { \
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
            goto cleanup; \
        } \
        m_BUFR_STUFF_OWNER(ub); \
        retval = pss_send_remrow_request(pwe_cb, ub, tbl, &first_idx); \
        ub = NULL; \
        if (retval != NCSCC_RC_SUCCESS) \
        { \
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
            /* Log an error here */ \
        } \
    }

#define m_REMROW_REQUEST2(pwe_cb) \
    { \
        USRBUF *ub = ncsremrow_enc_done(rra); \
        *rra_inited = FALSE; \
        if (ub == NULL) \
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
        m_BUFR_STUFF_OWNER(ub); \
        retval = pss_send_remrow_request(pwe_cb, ub, tbl, first_idx); \
        ub = NULL; \
        if (retval != NCSCC_RC_SUCCESS) \
        { \
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
            /* Log an error here */ \
        } \
    }

#define m_SINGLE_REM_REQUEST(pwe_cb) \
    { \
        retval = pss_send_remove_request(pwe_cb, tbl, idx, status_param); \
        if(retval != NCSCC_RC_SUCCESS) \
        { \
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
            /* Log an error here */ \
        } \
    }

#define m_SINGLE_REM_REQUEST2(pwe_cb) \
    { \
        retval = pss_send_remove_request(pwe_cb, tbl, &idx, status_param); \
        if(retval != NCSCC_RC_SUCCESS) \
        { \
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE); \
            /* Log an error here */ \
        } \
    }

/*****************************************************************************

  PROCEDURE NAME:    pss_playback_process_tbl_curprofile

  DESCRIPTION:       This is just a helper function to pss_playback_process_tbl
                     This function processes the remaining data in the current
                     profile, both in the patricia tree and in the table file.
                     It sends remove row requests for all the entries.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                               for more details

*****************************************************************************/
uns32 pss_playback_process_tbl_curprofile(PSS_PWE_CB *pwe_cb, NCS_PATRICIA_TREE *pTree,
					  NCS_PATRICIA_NODE *pNode, uns8 *cur_key,
					  uns8 *cur_data, uns32 cur_rows_left,
					  uns8 *cur_buf, uns8 *cur_ptr,
					  long cur_file_hdl, uns32 buf_size,
					  uns32 cur_file_offset, NCSMIB_IDX *first_idx, NCSMIB_IDX *idx,
#if (NCS_PSS_RED == 1)
					  uns32 *pinst_re_ids, NCS_BOOL *i_continue_session,
#endif
					  NCSREMROW_AID *rra, NCS_BOOL *rra_inited,
					  uns32 tbl, uns16 pwe, char *p_pcn, NCS_BOOL cur_file_exists)
{
	int res;
	PSS_CB *inst = pwe_cb->p_pss_cb;
	PSS_MIB_TBL_DATA *pData;
	PSS_MIB_TBL_INFO *tbl_info = inst->mib_tbl_desc[tbl];
	NCS_BOOL cur_data_valid = FALSE;
	uns32 retval = NCSCC_RC_SUCCESS, bytes_read;

#if (NCS_PSS_RED == 1)
	if ((pwe_cb->processing_pending_active_events == TRUE) && ((*i_continue_session) == FALSE)) {
		if (pwe_cb->curr_plbck_ssn_info.tbl_id == 0)
			(*i_continue_session) = TRUE;	/* This is the first table in the playback session */
		else if ((pwe_cb->curr_plbck_ssn_info.tbl_id == tbl) &&
			 (tbl_info->capability == NCSMIB_CAPABILITY_SETALLROWS)) {
			(*i_continue_session) = TRUE;	/* This table was already played-back in previous playback session */
			return NCSCC_RC_SUCCESS;
		}
	}
	if ((*i_continue_session) == TRUE) {
		if (pwe_cb->processing_pending_active_events == TRUE) {
			pwe_cb->curr_plbck_ssn_info.tbl_id = tbl;
		}
	} else {
		if (pwe_cb->curr_plbck_ssn_info.tbl_id != tbl)
			return NCSCC_RC_SUCCESS;
	}
#endif

	while ((pTree->n_nodes != 0) && (pNode != NULL) && (cur_rows_left > 0)) {
		pData = (PSS_MIB_TBL_DATA *)pNode;
		pss_get_key_from_data(tbl_info, cur_key, cur_ptr);
		res = memcmp(pData->key, cur_key, tbl_info->max_key_length);

		if (res > 0) {
			cur_data_valid = TRUE;
			memcpy(cur_data, cur_ptr, tbl_info->max_row_length);
			cur_ptr += tbl_info->max_row_length;
			cur_rows_left--;
		} else if (res == 0) {
			if (pData->deleted == FALSE) {
				cur_data_valid = TRUE;
				memcpy(cur_data, pData->data, tbl_info->max_row_length);
			}
			cur_ptr += tbl_info->max_row_length;
			cur_rows_left--;
			pss_delete_inst_node_from_tree(pTree, pNode);
			pNode = ncs_patricia_tree_getnext(pTree, NULL);
		} else if (res < 0) {
			if (pData->deleted == FALSE) {
				cur_data_valid = TRUE;
				memcpy(cur_data, pData->data, tbl_info->max_row_length);
			}
			pss_delete_inst_node_from_tree(pTree, pNode);
			pNode = ncs_patricia_tree_getnext(pTree, NULL);
		}

		if ((cur_rows_left == 0) && (cur_file_exists == TRUE))
			/* Read in some more data for the current profile */
			m_READ_DATA2(inst, retval, cur_file_hdl, buf_size,
				     cur_file_offset, cur_buf, cur_ptr, bytes_read,
				     cur_rows_left, tbl_info->max_row_length);

		if (cur_data_valid == TRUE) {
			idx->i_inst_len = pss_get_inst_ids_from_data(tbl_info, (uns32 *)idx->i_inst_ids, cur_data);
			if (idx->i_inst_len == 0)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

			if (inst->mib_tbl_desc[tbl]->ptbl_info->status_field == 0) {
				/* SP-MIB */
				if (*rra_inited == FALSE) {
					ncsremrow_enc_init(rra);
					*rra_inited = TRUE;
					first_idx->i_inst_len = idx->i_inst_len;
					memcpy((uns32 *)first_idx->i_inst_ids, idx->i_inst_ids,
					       idx->i_inst_len * sizeof(uns32));
				}

				ncsremrow_enc_inst_ids(rra, idx);
				if (rra->len >= NCS_PSS_MAX_CHUNK_SIZE) {
					NCS_BOOL send_evt = TRUE;

#if (NCS_PSS_RED == 1)
					if (((pwe_cb->processing_pending_active_events == TRUE) &&
					     (((*i_continue_session) == TRUE) ||
					      (((pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len == 0) ||
						((idx->i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len) &&
						 (memcmp
						  (idx->i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						   idx->i_inst_len) == 0))))))
					    || (pwe_cb->processing_pending_active_events == FALSE)) {
						if ((pwe_cb->processing_pending_active_events == TRUE)
						    && ((*i_continue_session) == FALSE)) {
							if ((idx->i_inst_len ==
							     pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len)
							    &&
							    (memcmp
							     (idx->i_inst_ids,
							      pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
							      idx->i_inst_len) == 0)) {
								/* Found the last sent MIB object */
								(*i_continue_session) = TRUE;
								/* Skip to the next MIB object to send. */
								send_evt = FALSE;
							}
						}
					}
#endif
					if (send_evt) {
						m_REMROW_REQUEST2(pwe_cb);
#if (NCS_PSS_RED == 1)
						m_RE_REMROW_REQUEST2(pwe_cb);
#endif
					}
				}
			} else {
				NCS_BOOL send_evt = TRUE;
				NCSMIB_PARAM_ID status_param = tbl_info->ptbl_info->status_field;

#if (NCS_PSS_RED == 1)
				if (((pwe_cb->processing_pending_active_events == TRUE) &&
				     (((*i_continue_session) == TRUE) ||
				      (((pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len == 0) ||
					((idx->i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len) &&
					 (memcmp(idx->i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						 idx->i_inst_len) == 0)))))) ||
				    (pwe_cb->processing_pending_active_events == FALSE)) {
					if ((pwe_cb->processing_pending_active_events == TRUE) &&
					    ((*i_continue_session) == FALSE)) {
						if ((idx->i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len)
						    &&
						    (memcmp
						     (idx->i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						      idx->i_inst_len) == 0)) {
							/* Found the last sent MIB object */
							(*i_continue_session) = TRUE;
							/* Skip to the next MIB object to send. */
							send_evt = FALSE;
						}
					}
				}
#endif
				if (send_evt) {
					m_SINGLE_REM_REQUEST(pwe_cb);
#if (NCS_PSS_RED == 1)
					m_RE_SINGLE_REM_REQUEST(pwe_cb);
#endif
				}
			}
			cur_data_valid = FALSE;
		}		/* if (cur_data_valid == TRUE) */
	}			/* while ((pNode != NULL) && (cur_rows_left > 0)) */

	while ((pTree->n_nodes != 0) && (pNode != NULL)) {
		pData = (PSS_MIB_TBL_DATA *)pNode;
		if (pData->deleted == FALSE) {
			idx->i_inst_len = pss_get_inst_ids_from_data(tbl_info, (uns32 *)idx->i_inst_ids, pData->data);
			if (idx->i_inst_len == 0)
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

			if (inst->mib_tbl_desc[tbl]->ptbl_info->status_field == 0) {
				/* SP-MIB */
				if (*rra_inited == FALSE) {
					ncsremrow_enc_init(rra);
					*rra_inited = TRUE;
					first_idx->i_inst_len = idx->i_inst_len;
					memcpy((uns32 *)first_idx->i_inst_ids, idx->i_inst_ids,
					       idx->i_inst_len * sizeof(uns32));
				}
				ncsremrow_enc_inst_ids(rra, idx);
				if (rra->len >= NCS_PSS_MAX_CHUNK_SIZE) {
					NCS_BOOL send_evt = TRUE;

#if (NCS_PSS_RED == 1)
					if (((pwe_cb->processing_pending_active_events == TRUE) &&
					     (((*i_continue_session) == TRUE) ||
					      (((pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len == 0) ||
						((idx->i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len) &&
						 (memcmp
						  (idx->i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						   idx->i_inst_len) == 0))))))
					    || (pwe_cb->processing_pending_active_events == FALSE)) {
						if ((pwe_cb->processing_pending_active_events == TRUE)
						    && ((*i_continue_session) == FALSE)) {
							if ((idx->i_inst_len ==
							     pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len)
							    &&
							    (memcmp
							     (idx->i_inst_ids,
							      pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
							      idx->i_inst_len) == 0)) {
								/* Found the last sent MIB object */
								(*i_continue_session) = TRUE;
								/* Skip to the next MIB object to send. */
								send_evt = FALSE;
							}
						}
					}
#endif
					if (send_evt) {
						m_REMROW_REQUEST2(pwe_cb);
#if (NCS_PSS_RED == 1)
						m_RE_REMROW_REQUEST2(pwe_cb);
#endif
					}
				}
			} else {
				NCSMIB_PARAM_ID status_param = tbl_info->ptbl_info->status_field;
				NCS_BOOL send_evt = TRUE;

#if (NCS_PSS_RED == 1)
				if (((pwe_cb->processing_pending_active_events == TRUE) &&
				     (((*i_continue_session) == TRUE) ||
				      (((pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len == 0) ||
					((idx->i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len) &&
					 (memcmp(idx->i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						 idx->i_inst_len) == 0)))))) ||
				    (pwe_cb->processing_pending_active_events == FALSE)) {
					if ((pwe_cb->processing_pending_active_events == TRUE) &&
					    ((*i_continue_session) == FALSE)) {
						if ((idx->i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len)
						    &&
						    (memcmp
						     (idx->i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						      idx->i_inst_len) == 0)) {
							/* Found the last sent MIB object */
							(*i_continue_session) = TRUE;
							/* Skip to the next MIB object to send. */
							send_evt = FALSE;
						}
					}
				}
#endif
				if (send_evt) {
					m_SINGLE_REM_REQUEST(pwe_cb);
#if (NCS_PSS_RED == 1)
					m_RE_SINGLE_REM_REQUEST(pwe_cb);
#endif
				}
			}
			cur_data_valid = FALSE;
		}
		/* if (pData->deleted == FALSE) */
		pss_delete_inst_node_from_tree(pTree, pNode);
		pNode = ncs_patricia_tree_getnext(pTree, NULL);
	}			/* while (pNode != NULL) */

	while (cur_rows_left > 0) {
		idx->i_inst_len = pss_get_inst_ids_from_data(tbl_info, (uns32 *)idx->i_inst_ids, cur_ptr);
		cur_ptr += tbl_info->max_row_length;
		cur_rows_left--;
		if (idx->i_inst_len == 0)
			return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

		if (inst->mib_tbl_desc[tbl]->ptbl_info->status_field == 0) {
			/* SP-MIB */
			if (*rra_inited == FALSE) {
				ncsremrow_enc_init(rra);
				*rra_inited = TRUE;
				first_idx->i_inst_len = idx->i_inst_len;
				memcpy((uns32 *)first_idx->i_inst_ids, idx->i_inst_ids,
				       idx->i_inst_len * sizeof(uns32));
			}
			ncsremrow_enc_inst_ids(rra, idx);
			if (rra->len >= NCS_PSS_MAX_CHUNK_SIZE) {
				NCS_BOOL send_evt = TRUE;

#if (NCS_PSS_RED == 1)
				if (((pwe_cb->processing_pending_active_events == TRUE) &&
				     (((*i_continue_session) == TRUE) ||
				      (((pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len == 0) ||
					((idx->i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len) &&
					 (memcmp(idx->i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						 idx->i_inst_len) == 0)))))) ||
				    (pwe_cb->processing_pending_active_events == FALSE)) {
					if ((pwe_cb->processing_pending_active_events == TRUE) &&
					    ((*i_continue_session) == FALSE)) {
						if ((idx->i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len)
						    &&
						    (memcmp
						     (idx->i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						      idx->i_inst_len) == 0)) {
							/* Found the last sent MIB object */
							(*i_continue_session) = TRUE;
							/* Skip to the next MIB object to send. */
							send_evt = FALSE;
						}
					}
				}
#endif
				if (send_evt) {
					m_REMROW_REQUEST2(pwe_cb);
#if (NCS_PSS_RED == 1)
					m_RE_REMROW_REQUEST2(pwe_cb);
#endif
				}
			}
		} else {
			NCSMIB_PARAM_ID status_param = tbl_info->ptbl_info->status_field;
			NCS_BOOL send_evt = TRUE;

#if (NCS_PSS_RED == 1)
			if (((pwe_cb->processing_pending_active_events == TRUE) &&
			     (((*i_continue_session) == TRUE) ||
			      (((pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len == 0) ||
				((idx->i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len) &&
				 (memcmp(idx->i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
					 idx->i_inst_len) == 0)))))) ||
			    (pwe_cb->processing_pending_active_events == FALSE)) {
				if ((pwe_cb->processing_pending_active_events == TRUE) &&
				    ((*i_continue_session) == FALSE)) {
					if ((idx->i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len) &&
					    (memcmp(idx->i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						    idx->i_inst_len) == 0)) {
						/* Found the last sent MIB object */
						(*i_continue_session) = TRUE;
						/* Skip to the next MIB object to send. */
						send_evt = FALSE;
					}
				}
			}
#endif
			if (send_evt) {
				m_SINGLE_REM_REQUEST(pwe_cb);
#if (NCS_PSS_RED == 1)
				m_RE_SINGLE_REM_REQUEST(pwe_cb);
#endif
			}
		}

		if ((cur_rows_left == 0) && (cur_file_exists == TRUE)) {
			/* Read in some more data for the current profile */
			m_READ_DATA2(inst, retval, cur_file_hdl, buf_size,
				     cur_file_offset, cur_buf, cur_ptr,
				     bytes_read, cur_rows_left, tbl_info->max_row_length);
		}
	}			/* while (cur_rows_left > 0) */

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_playback_process_tbl

  DESCRIPTION:       This function computes the differences between the current
                     profile and the new profile and issues appropriate SNMP
                     requests.
                     For entries present in the current profile, but
                     not in the new profile, it sends REMOVEROW requests.
                     For entries present in the new profile, but not in the
                     current profile, it puts them into the add queue, which 
                     is processed in ascending order of table ranks.
                     For entries present in both the profiles, but which have
                     different values, it puts them into the change queue, which
                     again is processed in ascending order of table ranks.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                               for more details

*****************************************************************************/
uns32 pss_playback_process_tbl(PSS_PWE_CB *pwe_cb, uns8 *profile, PSS_TBL_REC *rec, NCS_QUEUE *add_q, NCS_QUEUE *chg_q,
#if (NCS_PSS_RED == 1)
			       NCS_BOOL *i_continue_session,
#endif
			       NCS_BOOL last_plbck_evt)
{
	uns32 retval = NCSCC_RC_SUCCESS;
	char *p_pcn = (char *)&rec->pss_client_key->pcn;
	NCS_PATRICIA_TREE *pTree = &rec->info.other.data;
	uns8 *cur_key = NULL, *alt_key = NULL;
	uns32 tbl = rec->tbl_id, cur_rows_left = 0, alt_rows_left = 0;
	uns8 *cur_buf = NULL, *alt_buf = NULL;
	uns8 *cur_ptr, *alt_ptr;
	NCS_BOOL cur_file_exists = FALSE, alt_file_exists = FALSE;
	long cur_file_hdl = 0, alt_file_hdl = 0;
	uns32 cur_file_offset = 0, alt_file_offset = 0;
	uns32 buf_size = 0, max_num_rows = 0, bytes_read = 0;
	uns8 *cur_data = NULL;
	NCS_BOOL cur_data_valid = FALSE;
	NCS_PATRICIA_NODE *pNode = NULL;
	PSS_MIB_TBL_DATA *pData = NULL;
	PSS_MIB_TBL_INFO *tbl_info = pwe_cb->p_pss_cb->mib_tbl_desc[tbl];
	NCSREMROW_AID rra;
	USRBUF *ub = NULL;
	int res;
	NCSMIB_IDX idx, first_idx;
	uns32 *inst_ids = NULL, *first_inst_ids = NULL;
#if (NCS_PSS_RED == 1)
	uns32 *pinst_re_ids = NULL;
#endif
	NCS_BOOL rra_inited = FALSE;

#if (NCS_PSS_RED == 1)
	if ((pwe_cb->processing_pending_active_events == TRUE) && ((*i_continue_session) == FALSE)) {
		if (pwe_cb->curr_plbck_ssn_info.tbl_id == 0)
			(*i_continue_session) = TRUE;	/* This is the first table in the playback session */
		else if ((pwe_cb->curr_plbck_ssn_info.tbl_id == tbl) &&
			 (tbl_info->capability == NCSMIB_CAPABILITY_SETALLROWS)) {
			(*i_continue_session) = TRUE;	/* This table was already played-back in previous playback session */
			return NCSCC_RC_SUCCESS;
		}
	}
	if ((*i_continue_session) == TRUE) {
		if (pwe_cb->processing_pending_active_events == TRUE) {
			pwe_cb->curr_plbck_ssn_info.tbl_id = tbl;
		}
	} else {
		if (pwe_cb->curr_plbck_ssn_info.tbl_id != tbl)
			return NCSCC_RC_SUCCESS;
	}
#endif

	/* First check if there is any data to be processed */
	m_NCS_PSSTS_FILE_EXISTS(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl,
				retval, profile, pwe_cb->pwe_id, p_pcn, tbl, alt_file_exists);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_EXISTENCE_IN_STORE_OP_FAIL, pwe_cb->pwe_id, p_pcn, tbl);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	m_NCS_PSSTS_FILE_EXISTS(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl,
				retval, pwe_cb->p_pss_cb->current_profile, pwe_cb->pwe_id, p_pcn, tbl, cur_file_exists);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_EXISTENCE_IN_STORE_OP_FAIL, pwe_cb->pwe_id, p_pcn, tbl);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	if ((alt_file_exists == FALSE) &&
	    (cur_file_exists == FALSE) && ((pTree == NULL) || (ncs_patricia_tree_size(pTree) == 0))) {
#if (NCS_PSS_RED == 1)
		if (pwe_cb->processing_pending_active_events == TRUE) {
			if ((pwe_cb->curr_plbck_ssn_info.tbl_id == tbl) && ((*i_continue_session) == FALSE)) {
				/* Ask PSS to continue playback for the next table */
				(*i_continue_session) = TRUE;
				return NCSCC_RC_SUCCESS;
			}
		}
#endif
		return NCSCC_RC_SUCCESS;
	}

	/* Allocate memory for the buffers and keys */
	max_num_rows = NCS_PSS_MAX_CHUNK_SIZE / tbl_info->max_row_length;
	if (max_num_rows == 0)
		max_num_rows = 1;

	buf_size = max_num_rows * tbl_info->max_row_length;
	cur_buf = m_MMGR_ALLOC_PSS_OCT(buf_size);
	if (cur_buf == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_playback_process_tbl()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	memset(cur_buf, '\0', buf_size);
	cur_ptr = cur_buf;

	alt_buf = m_MMGR_ALLOC_PSS_OCT(buf_size);
	if (alt_buf == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_playback_process_tbl()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	memset(alt_buf, '\0', buf_size);
	alt_ptr = alt_buf;

	cur_key = m_MMGR_ALLOC_PSS_OCT(tbl_info->max_key_length);
	if (cur_key == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_playback_process_tbl()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	memset(cur_key, '\0', tbl_info->max_key_length);

	alt_key = m_MMGR_ALLOC_PSS_OCT(tbl_info->max_key_length);
	if (alt_key == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_playback_process_tbl()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	memset(alt_key, '\0', tbl_info->max_key_length);

	cur_data = m_MMGR_ALLOC_PSS_OCT(tbl_info->max_row_length);
	if (cur_data == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_playback_process_tbl()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	memset(cur_data, '\0', tbl_info->max_row_length);

	inst_ids = m_MMGR_ALLOC_MIB_INST_IDS(tbl_info->num_inst_ids);
	if (inst_ids == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MIB_INST_IDS_ALLOC_FAIL, "pss_playback_process_tbl()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	idx.i_inst_ids = (const uns32 *)inst_ids;

	first_inst_ids = m_MMGR_ALLOC_MIB_INST_IDS(tbl_info->num_inst_ids);
	if (first_inst_ids == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MIB_INST_IDS_ALLOC_FAIL, "pss_playback_process_tbl()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	first_idx.i_inst_ids = (const uns32 *)first_inst_ids;
#if (NCS_PSS_RED == 1)
	/* if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) */
	{
		pinst_re_ids = m_MMGR_ALLOC_MIB_INST_IDS(tbl_info->num_inst_ids);
		if (pinst_re_ids == NULL) {
			m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MIB_INST_IDS_ALLOC_FAIL,
					  "pss_playback_process_tbl()");
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			goto cleanup;
		}
		memset(pinst_re_ids, '\0', tbl_info->num_inst_ids * sizeof(uns32));
	}
#endif

	alt_file_offset = cur_file_offset = PSS_TABLE_DETAILS_HEADER_LEN;

	/* Now open the files and read in data from them */
	if (alt_file_exists == TRUE) {
		m_NCS_PSSTS_FILE_OPEN(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl,
				      retval, profile, pwe_cb->pwe_id, p_pcn, tbl, NCS_PSSTS_FILE_PERM_READ,
				      alt_file_hdl);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_OPEN_FAIL, pwe_cb->pwe_id, p_pcn, tbl);
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			goto cleanup;
		}

		m_READ_DATA(pwe_cb->p_pss_cb, retval, alt_file_hdl, buf_size, alt_file_offset, alt_buf, alt_ptr,
			    bytes_read, alt_rows_left, tbl_info->max_row_length);
	}

	if (cur_file_exists == TRUE) {
		m_NCS_PSSTS_FILE_OPEN(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl,
				      retval, pwe_cb->p_pss_cb->current_profile, pwe_cb->pwe_id, p_pcn,
				      tbl, NCS_PSSTS_FILE_PERM_READ, cur_file_hdl);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_OPEN_FAIL, pwe_cb->pwe_id, p_pcn, tbl);
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			goto cleanup;
		}

		m_READ_DATA(pwe_cb->p_pss_cb, retval, cur_file_hdl, buf_size, cur_file_offset, cur_buf, cur_ptr,
			    bytes_read, cur_rows_left, tbl_info->max_row_length);
	}

	if ((pTree != NULL) && (pTree->n_nodes != 0))
		pNode = ncs_patricia_tree_getnext(pTree, NULL);

	/* Now the initializations have been done, so start the comparisons */
	while (((pNode != NULL) || (cur_rows_left > 0) || (cur_data_valid == TRUE)) && (alt_rows_left > 0)) {
		/* Get the appropriate row from the current profile */
		if (cur_data_valid == FALSE) {
			while (((pTree->n_nodes != 0) && (pNode != NULL)) || (cur_rows_left > 0)) {
				while ((pTree->n_nodes != 0) && (pNode != NULL) && (cur_rows_left > 0)) {
					pData = (PSS_MIB_TBL_DATA *)pNode;
					pss_get_key_from_data(tbl_info, cur_key, cur_ptr);
					res = memcmp(pData->key, cur_key, tbl_info->max_key_length);

					if (res > 0) {
						memcpy(cur_data, cur_ptr, tbl_info->max_row_length);
						cur_rows_left--;
						cur_ptr += tbl_info->max_row_length;
						cur_data_valid = TRUE;
						break;
					}
					/* if (res > 0) */
					if (res < 0) {
						if (pData->deleted == FALSE) {
							memcpy(cur_data, pData->data, tbl_info->max_row_length);
							memcpy(cur_key, pData->key, tbl_info->max_key_length);
							cur_data_valid = TRUE;
							pss_delete_inst_node_from_tree(pTree, pNode);

							pNode = NULL;
							if (pTree->n_nodes != 0)
								pNode = ncs_patricia_tree_getnext(pTree, NULL);
							break;
						}

						pss_delete_inst_node_from_tree(pTree, pNode);
						pNode = NULL;
						if (pTree->n_nodes != 0)
							pNode = ncs_patricia_tree_getnext(pTree, NULL);
						continue;
					}
					/* if (res < 0) */
					if (res == 0) {
						if (pData->deleted == FALSE) {
							memcpy(cur_data, pData->data, tbl_info->max_row_length);
							memcpy(cur_key, pData->key, tbl_info->max_key_length);
							cur_data_valid = TRUE;
							pss_delete_inst_node_from_tree(pTree, pNode);

							pNode = NULL;
							if (pTree->n_nodes != 0)
								pNode = ncs_patricia_tree_getnext(pTree, NULL);
							cur_rows_left--;
							cur_ptr += tbl_info->max_row_length;
							break;
						}

						pss_delete_inst_node_from_tree(pTree, pNode);
						pNode = NULL;
						if (pTree->n_nodes != 0)
							pNode = ncs_patricia_tree_getnext(pTree, NULL);
						cur_rows_left--;
						cur_ptr += tbl_info->max_row_length;

						/* If buffer is empty, read in some more data from the disk */
						if ((cur_rows_left == 0) && (cur_file_exists == TRUE)) {
							m_READ_DATA(pwe_cb->p_pss_cb, retval, cur_file_hdl, buf_size,
								    cur_file_offset, cur_buf, cur_ptr,
								    bytes_read, cur_rows_left,
								    tbl_info->max_row_length);
						}
					}	/* if (res == 0) */
				}	/* while ((pNode != NULL) && (cur_rows_left > 0)) */

				if (cur_data_valid == TRUE)
					break;

				/* If the tree contains no data, there should be some data in the buffer */
				if (cur_rows_left > 0) {
					assert(pNode == NULL);
					memcpy(cur_data, cur_ptr, tbl_info->max_row_length);
					pss_get_key_from_data(tbl_info, cur_key, cur_data);
					cur_ptr += tbl_info->max_row_length;
					cur_rows_left--;
					cur_data_valid = TRUE;
					break;
				}

				/* There is no more data on the disk, so the tree contains all the data */
				if (pNode != NULL) {
					assert(cur_rows_left == 0);

					while (pNode != NULL) {
						pData = (PSS_MIB_TBL_DATA *)pNode;
						if (pData->deleted == FALSE) {
							memcpy(cur_data, pData->data, tbl_info->max_row_length);
							memcpy(cur_key, pData->key, tbl_info->max_key_length);
							cur_data_valid = TRUE;
							pss_delete_inst_node_from_tree(pTree, pNode);
							pNode = NULL;
							if (pTree->n_nodes != 0)
								pNode = ncs_patricia_tree_getnext(pTree, NULL);
							break;
						}

						pss_delete_inst_node_from_tree(pTree, pNode);
						pNode = NULL;
						if (pTree->n_nodes != 0)
							pNode = ncs_patricia_tree_getnext(pTree, NULL);
					}	/* while (pNode != NULL) */

					break;
				}	/* if (pNode != NULL) */
			}	/* while ((pNode != NULL) || (cur_rows_left > 0)) */
		}
		/* if (cur_data_valid == FALSE) */
		if (cur_data_valid == FALSE)
			break;

		/* Now make the comparisons between the alternate profile and the current profile data */
		pss_get_key_from_data(tbl_info, alt_key, alt_ptr);
		res = memcmp(alt_key, cur_key, tbl_info->max_key_length);

		if (res < 0) {
			/* add to add_q */
			retval = pss_add_to_diff_q(add_q, tbl, p_pcn, alt_ptr, tbl_info->max_row_length);
			if (retval != NCSCC_RC_SUCCESS) {
				retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				goto cleanup;
			}
			alt_ptr += tbl_info->max_row_length;
			alt_rows_left--;
		}

		if (res == 0) {
			/* add to chg_q if data is different */
			if (memcmp(alt_ptr, cur_data, tbl_info->max_row_length) != 0) {
				retval = pss_add_to_diff_q(chg_q, tbl, p_pcn, alt_ptr, tbl_info->max_row_length);
				if (retval != NCSCC_RC_SUCCESS) {
					retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					goto cleanup;
				}
			}

			cur_data_valid = FALSE;
			alt_ptr += tbl_info->max_row_length;
			alt_rows_left--;
		}

		if (res > 0) {
			/* put cur_data in remove rows request */
			idx.i_inst_len = pss_get_inst_ids_from_data(tbl_info, inst_ids, cur_data);
			if (idx.i_inst_len == 0) {
				retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				goto cleanup;
			}

			if (tbl_info->ptbl_info->status_field == 0) {
				/* This is SP-MIB. Populate REMOVEROWS request. */
				if (rra_inited == FALSE) {
					ncsremrow_enc_init(&rra);
					first_idx.i_inst_len = idx.i_inst_len;
					memcpy((uns32 *)first_idx.i_inst_ids, idx.i_inst_ids,
					       idx.i_inst_len * sizeof(uns32));
					rra_inited = TRUE;
				}
				ncsremrow_enc_inst_ids(&rra, &idx);

				if (rra.len >= NCS_PSS_MAX_CHUNK_SIZE) {
					NCS_BOOL send_evt = TRUE;

#if (NCS_PSS_RED == 1)
					if (((pwe_cb->processing_pending_active_events == TRUE) &&
					     (((*i_continue_session) == TRUE) ||
					      (((pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len == 0) ||
						((idx.i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len) &&
						 (memcmp(idx.i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
							 idx.i_inst_len) == 0)))))) ||
					    (pwe_cb->processing_pending_active_events == FALSE)) {
						if ((pwe_cb->processing_pending_active_events == TRUE) &&
						    ((*i_continue_session) == FALSE)) {
							if ((idx.i_inst_len ==
							     pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len)
							    &&
							    (memcmp
							     (idx.i_inst_ids,
							      pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
							      idx.i_inst_len) == 0)) {
								/* Found the last sent MIB object */
								(*i_continue_session) = TRUE;
								/* Skip to the next MIB object to send. */
								send_evt = FALSE;
							}
						}
					}
#endif
					if (send_evt) {
						m_REMROW_REQUEST(pwe_cb);
#if (NCS_PSS_RED == 1)
						m_RE_N_REMROW_REQUEST(pwe_cb);
#endif
					}
				}
			} else {
				NCS_BOOL send_evt = TRUE;

#if (NCS_PSS_RED == 1)
				if (((pwe_cb->processing_pending_active_events == TRUE) &&
				     (((*i_continue_session) == TRUE) ||
				      (((pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len == 0) ||
					((idx.i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len) &&
					 (memcmp(idx.i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						 idx.i_inst_len) == 0)))))) ||
				    (pwe_cb->processing_pending_active_events == FALSE)) {
					if ((pwe_cb->processing_pending_active_events == TRUE) &&
					    ((*i_continue_session) == FALSE)) {
						if ((idx.i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len)
						    &&
						    (memcmp
						     (idx.i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						      idx.i_inst_len) == 0)) {
							/* Found the last sent MIB object */
							(*i_continue_session) = TRUE;
							/* Skip to the next MIB object to send. */
							send_evt = FALSE;
						}
					}
				}
#endif
				if (send_evt) {
					/* Individual row-delete events via SET are sent now. */
					NCSMIB_PARAM_ID status_param = tbl_info->ptbl_info->status_field;
					m_SINGLE_REM_REQUEST2(pwe_cb);
#if (NCS_PSS_RED == 1)
					m_RE_SINGLE_REM_REQUEST2(pwe_cb);
#endif
				}
			}

			cur_data_valid = FALSE;
		}

		if ((cur_rows_left == 0) && (cur_file_exists == TRUE)) {
			/* Read in some more data for the current profile */
			m_READ_DATA(pwe_cb->p_pss_cb, retval, cur_file_hdl, buf_size,
				    cur_file_offset, cur_buf, cur_ptr,
				    bytes_read, cur_rows_left, tbl_info->max_row_length);
		}

		if ((alt_rows_left == 0) && (alt_file_exists == TRUE)) {
			/* Read in some more data for the alternate profile */
			m_READ_DATA(pwe_cb->p_pss_cb, retval, alt_file_hdl, buf_size,
				    alt_file_offset, alt_buf, alt_ptr,
				    bytes_read, alt_rows_left, tbl_info->max_row_length);
		}
	}			/* while (((pNode != NULL) || (cur_rows_left > 0) || ... */

	/* Now process add row entries from the alternate profile if any */
	while (alt_rows_left > 0) {
		/* add to add_q */
		retval = pss_add_to_diff_q(add_q, tbl, p_pcn, alt_ptr, tbl_info->max_row_length);
		if (retval != NCSCC_RC_SUCCESS) {
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			goto cleanup;
		}
		alt_ptr += tbl_info->max_row_length;
		alt_rows_left--;

		if (alt_rows_left == 0) {
			/* Read in some more data for the alternate profile */
			m_READ_DATA(pwe_cb->p_pss_cb, retval, alt_file_hdl, buf_size,
				    alt_file_offset, alt_buf, alt_ptr,
				    bytes_read, alt_rows_left, tbl_info->max_row_length);
		}
	}			/* while (alt_rows_left > 0) */

	if (cur_data_valid == TRUE) {
		idx.i_inst_len = pss_get_inst_ids_from_data(tbl_info, inst_ids, cur_data);
		if (idx.i_inst_len == 0) {
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			goto cleanup;
		}

		if (tbl_info->ptbl_info->status_field == 0) {
			/* For SP-MIB */
			if (rra_inited == FALSE) {
				ncsremrow_enc_init(&rra);
				rra_inited = TRUE;
				first_idx.i_inst_len = idx.i_inst_len;
				memcpy((uns32 *)first_idx.i_inst_ids, idx.i_inst_ids, idx.i_inst_len * sizeof(uns32));
			}
			ncsremrow_enc_inst_ids(&rra, &idx);
		} else {
			/* Individual row-delete events via SET are sent now. */
			NCSMIB_PARAM_ID status_param = tbl_info->ptbl_info->status_field;
			NCS_BOOL send_evt = TRUE;

#if (NCS_PSS_RED == 1)
			if (((pwe_cb->processing_pending_active_events == TRUE) &&
			     (((*i_continue_session) == TRUE) ||
			      (((pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len == 0) ||
				((idx.i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len) &&
				 (memcmp(idx.i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
					 idx.i_inst_len) == 0)))))) ||
			    (pwe_cb->processing_pending_active_events == FALSE)) {
				if ((pwe_cb->processing_pending_active_events == TRUE) &&
				    ((*i_continue_session) == FALSE)) {
					if ((idx.i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len) &&
					    (memcmp(idx.i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						    idx.i_inst_len) == 0)) {
						/* Found the last sent MIB object */
						(*i_continue_session) = TRUE;
						/* Skip to the next MIB object to send. */
						send_evt = FALSE;
					}
				}
			}
#endif
			if (send_evt) {
				m_SINGLE_REM_REQUEST2(pwe_cb);
#if (NCS_PSS_RED == 1)
				m_RE_SINGLE_REM_REQUEST2(pwe_cb);
#endif
			}
		}
		cur_data_valid = FALSE;
	}

	/* Now process the current profile for remove row requests */
	retval = pss_playback_process_tbl_curprofile(pwe_cb, pTree,
						     pNode, cur_key, cur_data, cur_rows_left, cur_buf, cur_ptr,
						     cur_file_hdl, buf_size, cur_file_offset, &first_idx, &idx,
#if (NCS_PSS_RED == 1)
						     pinst_re_ids, i_continue_session,
#endif
						     &rra, &rra_inited, tbl, pwe_cb->pwe_id, p_pcn, cur_file_exists);
	if (retval != NCSCC_RC_SUCCESS) {
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	if ((pwe_cb->p_pss_cb->mib_tbl_desc[tbl]->ptbl_info->status_field == 0) && (rra_inited == TRUE)) {
		m_REMROW_REQUEST(pwe_cb);
#if (NCS_PSS_RED == 1)
		m_RE_N_REMROW_REQUEST(pwe_cb);
#endif
	}

 cleanup:
	if (cur_key != NULL)
		m_MMGR_FREE_PSS_OCT(cur_key);
	if (alt_key != NULL)
		m_MMGR_FREE_PSS_OCT(alt_key);

	if (cur_buf != NULL)
		m_MMGR_FREE_PSS_OCT(cur_buf);
	if (alt_buf != NULL)
		m_MMGR_FREE_PSS_OCT(alt_buf);

	if (cur_data != NULL)
		m_MMGR_FREE_PSS_OCT(cur_data);

	if (cur_file_hdl != 0)
		m_NCS_PSSTS_FILE_CLOSE(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl, retval, cur_file_hdl);
	if (alt_file_hdl != 0)
		m_NCS_PSSTS_FILE_CLOSE(pwe_cb->p_pss_cb->pssts_api, pwe_cb->p_pss_cb->pssts_hdl, retval, alt_file_hdl);

	if (ub != NULL)
		m_MMGR_FREE_BUFR_LIST(ub);

	if (inst_ids != NULL)
		m_MMGR_FREE_MIB_INST_IDS(inst_ids);

	if (first_inst_ids != NULL)
		m_MMGR_FREE_MIB_INST_IDS(first_inst_ids);

#if (NCS_PSS_RED == 1)
	if (pinst_re_ids != NULL)
		m_MMGR_FREE_MIB_INST_IDS(pinst_re_ids);
	pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids = NULL;
#endif

	if (rra_inited == TRUE) {
		ub = ncsremrow_enc_done(&rra);
		if (ub != NULL)
			m_MMGR_FREE_BUFR_LIST(ub);
	}

	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_playback_process_queue

  DESCRIPTION:       This function processes the add/modify/del queues the 
                     sort-database is parsed.
                     
                     For Row-Add/Modify updates, depending on the MIB-capability,
                     PSS would send in SETALLROWS/SETROW/SET MIB events.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                               for more details

*****************************************************************************/
uns32 pss_playback_process_queue(PSS_PWE_CB *pwe_cb,
#if (NCS_PSS_RED == 1)
				 NCS_BOOL *i_continue_session,
#endif
				 NCS_QUEUE *add_queue, NCS_QUEUE *diff_queue)
{
	NCS_QUEUE *queue = NULL;
	uns32 j, retval = NCSCC_RC_SUCCESS;
	PSS_QELEM *elem;
	NCSROW_AID ra;
	NCSPARM_AID pa;
	NCS_BOOL ra_inited = FALSE, is_add = TRUE, pa_inited = FALSE;
	NCSMIB_PARAM_VAL pv;
	PSS_MIB_TBL_INFO *tbl_info = NULL;
	uns32 tbl, add_cnt = 0, diff_cnt = 0, lcl_params_max = 0;
	uns32 *inst_ids = NULL;
	uns32 *first_inst_ids = NULL;
#if (NCS_PSS_RED == 1)
	uns32 *pinst_re_ids = NULL;
#endif
	NCSMIB_IDX idx, first_idx;

	add_cnt = add_queue->count;
	diff_cnt = diff_queue->count;
	queue = add_queue;
	elem = (PSS_QELEM *)ncs_dequeue(queue);
	if (elem == NULL) {
		queue = diff_queue;
		is_add = FALSE;
		elem = (PSS_QELEM *)ncs_dequeue(queue);
		if (elem == NULL)
			return NCSCC_RC_SUCCESS;
	}

	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_PROCESS_DIFFQ_START);
	tbl = elem->tbl_id;
	first_idx.i_inst_len = 0;	/* Just to remove a compiler warning */

	while (elem != NULL) {	/* Main loop for both add_queue and diff_queue */
		while (elem != NULL) {
			while (elem != NULL) {
				tbl_info = pwe_cb->p_pss_cb->mib_tbl_desc[tbl];
#if (NCS_PSS_RED == 1)
				if ((pwe_cb->processing_pending_active_events == TRUE) &&
				    ((*i_continue_session) == FALSE)) {
					if (pwe_cb->curr_plbck_ssn_info.tbl_id == 0)
						(*i_continue_session) = TRUE;	/* This is the first table in the playback session */
					else if ((pwe_cb->curr_plbck_ssn_info.tbl_id == tbl) &&
						 (tbl_info->capability == NCSMIB_CAPABILITY_SETALLROWS)) {
						(*i_continue_session) = TRUE;	/* This table was already played-back in previous playback session */
						return NCSCC_RC_SUCCESS;
					}
				}
				if ((*i_continue_session) == TRUE) {
					if (pwe_cb->processing_pending_active_events == TRUE) {
						pwe_cb->curr_plbck_ssn_info.tbl_id = tbl;
					}
				} else {
					if (pwe_cb->curr_plbck_ssn_info.tbl_id != tbl)
						return NCSCC_RC_SUCCESS;
				}
#endif

				if (ra_inited == FALSE) {
					inst_ids = m_MMGR_ALLOC_MIB_INST_IDS(tbl_info->num_inst_ids);
					if (inst_ids == NULL) {
						m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MIB_INST_IDS_ALLOC_FAIL,
								  "pss_playback_process_queue()");
						retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
						goto cleanup;
					}
					idx.i_inst_ids = inst_ids;
#if (NCS_PSS_RED == 1)
					/* if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) */
					{
						pinst_re_ids = m_MMGR_ALLOC_MIB_INST_IDS(tbl_info->num_inst_ids);
						if (pinst_re_ids == NULL) {
							m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL,
									  PSS_MF_MIB_INST_IDS_ALLOC_FAIL,
									  "pss_playback_process_queue()");
							retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
							goto cleanup;
						}
						memset(pinst_re_ids, '\0', tbl_info->num_inst_ids * sizeof(uns32));
					}
#endif
					if ((tbl_info->capability == NCSMIB_CAPABILITY_SETALLROWS) || (is_add == FALSE)) {
						first_inst_ids = m_MMGR_ALLOC_MIB_INST_IDS(tbl_info->num_inst_ids);
						if (first_inst_ids == NULL) {
							m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL,
									  PSS_MF_MIB_INST_IDS_ALLOC_FAIL,
									  "pss_playback_process_queue()");
							retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
							goto cleanup;
						}
						first_idx.i_inst_ids = first_inst_ids;
					}
					if (tbl_info->capability == NCSMIB_CAPABILITY_SETALLROWS) {
						memset(&ra, '\0', sizeof(ra));
						ncssetallrows_enc_init(&ra);
					}

					ra_inited = TRUE;
					idx.i_inst_len = pss_get_inst_ids_from_data(tbl_info, inst_ids, elem->data);
					if (idx.i_inst_len == 0) {
						retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
						goto cleanup;
					}

					if ((tbl_info->capability == NCSMIB_CAPABILITY_SETALLROWS) || (is_add == FALSE)) {
						first_idx.i_inst_len = idx.i_inst_len;
						memcpy((uns32 *)first_idx.i_inst_ids, idx.i_inst_ids,
						       idx.i_inst_len * sizeof(uns32));
					}
				} /* if (ra_inited == FALSE) */
				else {
					idx.i_inst_len = pss_get_inst_ids_from_data(tbl_info, inst_ids, elem->data);
					if (idx.i_inst_len == 0) {
						retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
						goto cleanup;
					}
				}

				if (is_add == FALSE) {
					/* Send Row-delete in case of diff_q elements */
					if (tbl_info->ptbl_info->status_field == 0) {
						NCSREMROW_AID rra;
						USRBUF *ub = NULL;
						NCS_BOOL rra_inited = TRUE;

						/* This is SP-MIB. Populate REMOVEROWS request. */
						ncsremrow_enc_init(&rra);
						first_idx.i_inst_len = idx.i_inst_len;
						first_idx.i_inst_ids = first_inst_ids;
						memcpy((uns32 *)first_idx.i_inst_ids, idx.i_inst_ids,
						       idx.i_inst_len * sizeof(uns32));
						ncsremrow_enc_inst_ids(&rra, &idx);
						m_REMROW_REQUEST(pwe_cb);
					} else {
						/* Individual row-delete events via SET are sent now. */
						NCSMIB_PARAM_ID status_param = tbl_info->ptbl_info->status_field;
						m_SINGLE_REM_REQUEST2(pwe_cb);
					}
				}

				if (tbl_info->capability == NCSMIB_CAPABILITY_SETALLROWS) {
					ncsrow_enc_init(&ra);
					ncsrow_enc_inst_ids(&ra, &idx);
				}

				lcl_params_max = pss_get_count_of_valid_params(elem->data, tbl_info);
				pa_inited = FALSE;
				for (j = 0; j < tbl_info->ptbl_info->num_objects; j++) {
					uns32 param_offset = tbl_info->pfields[j].offset;

					if (pss_get_bit_for_param(elem->data, (j + 1)) == 0)
						continue;
					if ((tbl_info->pfields[j].var_info.is_index_id) &&
					    (!((tbl_info->pfields[j].var_info.is_readonly_persistent == TRUE) ||
					       (tbl_info->pfields[j].var_info.access == NCSMIB_ACCESS_READ_WRITE) ||
					       (tbl_info->pfields[j].var_info.access == NCSMIB_ACCESS_READ_CREATE))))
						continue;

					if ((tbl_info->capability == NCSMIB_CAPABILITY_SETROW) && (pa_inited == FALSE)) {
						memset(&pa, '\0', sizeof(pa));
						ncsparm_enc_init(&pa);
						pa_inited = TRUE;
					}

					switch (tbl_info->pfields[j].var_info.fmat_id) {
					case NCSMIB_FMAT_INT:
						pv.i_fmat_id = NCSMIB_FMAT_INT;
						pv.i_param_id = tbl_info->pfields[j].var_info.param_id;
						switch (tbl_info->pfields[j].var_info.len) {
							uns16 data16;
						case sizeof(uns8):
							pv.info.i_int = (uns32)(*((uns8 *)(elem->data + param_offset)));
							break;
						case sizeof(uns16):
							memcpy(&data16, elem->data + param_offset, sizeof(uns16));
							pv.info.i_int = (uns32)data16;
							break;
						case sizeof(uns32):
							memcpy(&pv.info.i_int, elem->data + param_offset,
							       sizeof(uns32));
							break;
						default:
							return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
						}
						break;

					case NCSMIB_FMAT_BOOL:
						{
							NCS_BOOL data_bool;
							pv.i_fmat_id = NCSMIB_FMAT_INT;
							pv.i_param_id = tbl_info->pfields[j].var_info.param_id;
							memcpy(&data_bool, elem->data + param_offset, sizeof(NCS_BOOL));
							pv.info.i_int = (uns32)data_bool;
						}
						break;

					case NCSMIB_FMAT_OCT:
						{
							PSS_VAR_INFO *var_info = &(tbl_info->pfields[j]);
							if (var_info->var_length == TRUE) {
								memcpy(&pv.i_length, elem->data + param_offset,
								       sizeof(uns16));
								param_offset += sizeof(uns16);
							} else
								pv.i_length =
								    var_info->var_info.obj_spec.stream_spec.max_len;

							pv.i_fmat_id = NCSMIB_FMAT_OCT;
							pv.i_param_id = var_info->var_info.param_id;
							pv.info.i_oct = elem->data + param_offset;
						}
						break;

					default:
						retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
						goto cleanup;
					}

					if ((is_add == TRUE) &&
					    (tbl_info->ptbl_info->status_field ==
					     tbl_info->pfields[j].var_info.param_id)) {
						if (pv.info.i_int == NCSMIB_ROWSTATUS_ACTIVE)
							pv.info.i_int = NCSMIB_ROWSTATUS_CREATE_GO;
						else if (pv.info.i_int == NCSMIB_ROWSTATUS_NOTINSERVICE)
							pv.info.i_int = NCSMIB_ROWSTATUS_CREATE_WAIT;
					}

					if (tbl_info->capability == NCSMIB_CAPABILITY_SETALLROWS) {
						ncsrow_enc_param(&ra, &pv);
					} else if (tbl_info->capability == NCSMIB_CAPABILITY_SETROW) {
						ncsparm_enc_param(&pa, &pv);
#if (NCS_PSS_RED == 1)
					} else if (((pwe_cb->processing_pending_active_events == TRUE) &&
						  (((*i_continue_session) == TRUE) ||
						   (((pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len == 0) ||
						     ((tbl_info->ptbl_info->table_of_scalars) &&
						      (pwe_cb->curr_plbck_ssn_info.mib_obj_id == pv.i_param_id)) ||
						     ((idx.i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len)
						      &&
						      (memcmp
						       (idx.i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
							idx.i_inst_len) == 0))))))
						 || (pwe_cb->processing_pending_active_events == FALSE)) {
#else
					} else {
#endif
						/* Compose the SET request here. */
						NCSMIB_ARG mib_arg;
						NCSMEM_AID ma;
						uns8 space[2048];

#if (NCS_PSS_RED == 1)
						if ((pwe_cb->processing_pending_active_events == TRUE) &&
						    ((*i_continue_session) == FALSE)) {
							if (((tbl_info->ptbl_info->table_of_scalars) &&
							     (pwe_cb->curr_plbck_ssn_info.mib_obj_id == pv.i_param_id))
							    ||
							    ((idx.i_inst_len ==
							      pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len)
							     &&
							     (memcmp
							      (idx.i_inst_ids,
							       pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
							       idx.i_inst_len) == 0))) {
								/* Found the last sent MIB object */
								(*i_continue_session) = TRUE;
								--lcl_params_max;
								/* Skip to the next MIB object */
								if (is_add)
									--add_cnt;
								else
									--diff_cnt;
								continue;	/* in the MIB-object for-loop itself. */
							}
						}
#endif
						memset(&mib_arg, 0, sizeof(mib_arg));
						ncsmib_init(&mib_arg);
						ncsmem_aid_init(&ma, space, sizeof(space));
						mib_arg.i_idx.i_inst_len = idx.i_inst_len;
						mib_arg.i_idx.i_inst_ids = inst_ids;
						mib_arg.i_mib_key = pwe_cb->mac_key;
						mib_arg.i_op = NCSMIB_OP_REQ_SET;
						mib_arg.i_policy = NCSMIB_POLICY_PSS_BELIEVE_ME;
						if ((lcl_params_max == 1) &&
						    (((is_add == TRUE) && (add_cnt == 1) && (diff_cnt == 0)) ||
						     ((is_add == FALSE) && (diff_cnt == 1)))) {
							mib_arg.i_policy |= NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER;
							m_LOG_PSS_LAST_UPDT_INFO(NCSFL_SEV_DEBUG, PSS_LAST_UPDT_IS_SET,
										 " " /* Not utilized */ ,
										 pwe_cb->pwe_id, tbl);
						}
						mib_arg.i_rsp_fnc = NULL;
						mib_arg.i_tbl_id = tbl;
						mib_arg.i_usr_key = pwe_cb->p_pss_cb->hm_hdl;
						mib_arg.i_xch_id = pwe_cb->p_pss_cb->xch_id++;
						mib_arg.req.info.set_req.i_param_val = pv;

						m_PSS_LOG_NCSMIB_ARG(&mib_arg);

						retval =
						    pss_sync_mib_req(&mib_arg, pwe_cb->p_pss_cb->mib_req_func, 1000,
								     &ma);
#if (NCS_PSS_RED == 1)
						/* if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) */
						{
							/* Send Async updates to Standby */
							pwe_cb->curr_plbck_ssn_info.tbl_id = tbl;
							pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len = idx.i_inst_len;
							memcpy(pinst_re_ids, inst_ids,
							       tbl_info->num_inst_ids * sizeof(uns32));
							pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids = pinst_re_ids;
							pwe_cb->curr_plbck_ssn_info.mib_obj_id = pv.i_param_id;
							m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info);
						}
#endif
						if (retval != NCSCC_RC_SUCCESS) {
							retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
							m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_FAILED);
						} else if (mib_arg.i_op != NCSMIB_OP_RSP_SET) {
							retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
							m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL,
									   PSS_HDLN_MIB_REQ_RSP_NOT_SET);
						} else if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS) {
							retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
							m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE,
									 PSS_HDLN_MIB_REQ_RSP_IS_NOT_SUCCESS,
									 mib_arg.rsp.i_status);
						} else {
							m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_MIB_REQ_SUCCEEDED);
						}
						--lcl_params_max;
					}
#if (NCS_PSS_RED == 1)
					else
					{
						--lcl_params_max;
					}
#endif
				}	/* for (j = 0; j < tbl_info->ptbl_info->num_objects; j++) */

				if (tbl_info->capability == NCSMIB_CAPABILITY_SETALLROWS)
					ncsrow_enc_done(&ra);
#if (NCS_PSS_RED == 1)
				else if ((tbl_info->capability == NCSMIB_CAPABILITY_SETROW) &&
					 (((pwe_cb->processing_pending_active_events == TRUE) &&
					   (((*i_continue_session) == TRUE) ||
					    (((pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len == 0) ||
					      ((idx.i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len) &&
					       (memcmp(idx.i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						       idx.i_inst_len) == 0)))))) ||
					  (pwe_cb->processing_pending_active_events == FALSE)))
#else
				else if (tbl_info->capability == NCSMIB_CAPABILITY_SETROW)
#endif
				{
					NCSMIB_ARG mib_arg;
					NCSMEM_AID ma;
					uns8 space[2048];

#if (NCS_PSS_RED == 1)
					if ((pwe_cb->processing_pending_active_events == TRUE) &&
					    ((*i_continue_session) == FALSE)) {
						if ((idx.i_inst_len == pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len)
						    &&
						    (memcmp
						     (idx.i_inst_ids, pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids,
						      idx.i_inst_len) == 0)) {
							/* Found the last sent MIB object */
							(*i_continue_session) = TRUE;
							--lcl_params_max;
							/* Skip to the next MIB object */
							if (is_add)
								--add_cnt;
							else
								--diff_cnt;
							continue;	/* continue in the MIB-row for-loop itself. */
						}
					}
#endif

					memset(&mib_arg, 0, sizeof(mib_arg));

					ncsmib_init(&mib_arg);
					ncsmem_aid_init(&ma, space, sizeof(space));

					mib_arg.i_idx.i_inst_len = idx.i_inst_len;
					mib_arg.i_idx.i_inst_ids = inst_ids;
					mib_arg.i_mib_key = pwe_cb->mac_key;
					mib_arg.i_op = NCSMIB_OP_REQ_SETROW;
					mib_arg.i_policy = NCSMIB_POLICY_PSS_BELIEVE_ME;
					if (((is_add == TRUE) && (add_cnt == 1) && (diff_cnt == 0)) ||
					    ((is_add == FALSE) && (diff_cnt == 1))) {
						mib_arg.i_policy |= NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER;
						m_LOG_PSS_LAST_UPDT_INFO(NCSFL_SEV_DEBUG, PSS_LAST_UPDT_IS_SETROW,
									 " " /* Not utilized */ , pwe_cb->pwe_id, tbl);
					}
					mib_arg.i_rsp_fnc = NULL;
					mib_arg.i_tbl_id = tbl;
					mib_arg.i_usr_key = pwe_cb->p_pss_cb->hm_hdl;
					mib_arg.i_xch_id = pwe_cb->p_pss_cb->xch_id++;
					mib_arg.req.info.setrow_req.i_usrbuf = ncsparm_enc_done(&pa);
					if (mib_arg.req.info.setrow_req.i_usrbuf == NULL) {
						retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
						goto cleanup;
					}
					m_BUFR_STUFF_OWNER(mib_arg.req.info.setrow_req.i_usrbuf);

					m_PSS_LOG_NCSMIB_ARG(&mib_arg);

					retval = pss_sync_mib_req(&mib_arg, pwe_cb->p_pss_cb->mib_req_func, 1000, &ma);
#if (NCS_PSS_RED == 1)
					/* if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) */
					{
						/* Send Async updates to Standby */
						pwe_cb->curr_plbck_ssn_info.tbl_id = tbl;
						pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len = idx.i_inst_len;
						memcpy(pinst_re_ids, inst_ids, tbl_info->num_inst_ids * sizeof(uns32));
						pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids = pinst_re_ids;
						pwe_cb->curr_plbck_ssn_info.mib_obj_id = 0;
						m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info);
					}
#endif
					if (retval != NCSCC_RC_SUCCESS) {
						retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
						m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_FAILED);
					} else if (mib_arg.i_op != NCSMIB_OP_RSP_SETROW) {
						retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
						m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_RSP_NOT_SETROW);
					} else if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS) {
						retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
						m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_MIB_REQ_RSP_IS_NOT_SUCCESS,
								 mib_arg.rsp.i_status);
					} else {
						m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_MIB_REQ_SUCCEEDED);
						m_MMGR_FREE_BUFR_LIST(mib_arg.rsp.info.setallrows_rsp.i_usrbuf);
						if (mib_arg.req.info.setrow_req.i_usrbuf != NULL)
							m_MMGR_FREE_BUFR_LIST(mib_arg.rsp.info.setrow_rsp.i_usrbuf);
					}
				}

				m_MMGR_FREE_PSS_OCT(elem->data);
				m_MMGR_FREE_PSS_QELEM(elem);
				elem = (PSS_QELEM *)ncs_dequeue(queue);
				if (elem == NULL)
					break;
				if (is_add)
					--add_cnt;
				else
					--diff_cnt;

				if (elem->tbl_id != tbl)
					break;

				if (tbl_info->capability == NCSMIB_CAPABILITY_SETALLROWS) {
					if (ra.len >= NCS_PSS_MAX_CHUNK_SIZE)
						break;
				}
			}	/* while (elem != NULL) */

			if (is_add)
				--add_cnt;
			else
				--diff_cnt;

			if (elem != NULL) {
				tbl = elem->tbl_id;
			}
		}		/* while (elem != NULL) */
		if (is_add == TRUE) {
			/* Switch to the diff_queue now */
			queue = diff_queue;
			elem = (PSS_QELEM *)ncs_dequeue(queue);
			if (elem != NULL) {
				if (is_add)
					--add_cnt;
				else
					--diff_cnt;
			}
			is_add = FALSE;
		}
	}			/* while(elem != NULL) */

	if ((tbl_info->capability == NCSMIB_CAPABILITY_SETALLROWS) && (ra_inited == TRUE)) {
		USRBUF *ub = ncssetallrows_enc_done(&ra);
		ra_inited = FALSE;
		if (ub == NULL) {
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			goto cleanup;
		}
		m_BUFR_STUFF_OWNER(ub);

		/* Now send the mib request */
		{
			NCSMIB_ARG mib_arg;
			NCSMEM_AID ma;
			uns8 space[2048];

			memset(&mib_arg, 0, sizeof(mib_arg));
			ncsmib_init(&mib_arg);
			ncsmem_aid_init(&ma, space, sizeof(space));

			mib_arg.i_idx.i_inst_len = first_idx.i_inst_len;
			mib_arg.i_idx.i_inst_ids = first_inst_ids;
			mib_arg.i_mib_key = pwe_cb->mac_key;
			mib_arg.i_op = NCSMIB_OP_REQ_SETALLROWS;
			mib_arg.i_policy |= NCSMIB_POLICY_PSS_BELIEVE_ME;
			if (((is_add == TRUE) && (add_cnt == 1) && (diff_cnt == 0)) ||
			    ((is_add == FALSE) && (diff_cnt == 1))) {
				mib_arg.i_policy |= NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER;
				m_LOG_PSS_LAST_UPDT_INFO(NCSFL_SEV_DEBUG, PSS_LAST_UPDT_IS_SETALLROWS,
							 " " /* Not utilized */ , pwe_cb->pwe_id, tbl);
			}
			mib_arg.i_rsp_fnc = NULL;
			mib_arg.i_tbl_id = tbl;
			mib_arg.i_usr_key = pwe_cb->p_pss_cb->hm_hdl;
			mib_arg.i_xch_id = pwe_cb->p_pss_cb->xch_id++;
			mib_arg.req.info.setallrows_req.i_usrbuf = ub;

			m_PSS_LOG_NCSMIB_ARG(&mib_arg);
			retval = pss_sync_mib_req(&mib_arg, pwe_cb->p_pss_cb->mib_req_func, 1000, &ma);
#if (NCS_PSS_RED == 1)
			/* if(pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_ACTIVE) */
			{
				/* Send Async updates to Standby */
				pwe_cb->curr_plbck_ssn_info.tbl_id = tbl;
				pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_len = idx.i_inst_len;
				memcpy(pinst_re_ids, inst_ids, tbl_info->num_inst_ids * sizeof(uns32));
				pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids = pinst_re_ids;
				pwe_cb->curr_plbck_ssn_info.mib_obj_id = 0;
				m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info);
			}
#endif
			if (retval != NCSCC_RC_SUCCESS) {
				retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
				m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_FAILED);
			} else if (mib_arg.i_op != NCSMIB_OP_RSP_SETALLROWS) {
				retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
				m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_RSP_NOT_SETALLROWS);
			} else if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS) {
				retval = NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
				m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_MIB_REQ_RSP_IS_NOT_SUCCESS,
						 mib_arg.rsp.i_status);
			} else {
				m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_MIB_REQ_SUCCEEDED);
				m_MMGR_FREE_BUFR_LIST(mib_arg.rsp.info.setallrows_rsp.i_usrbuf);
			}
		}
	}
	/* if (ra_inited == TRUE) */
 cleanup:

	if (inst_ids != NULL)
		m_MMGR_FREE_MIB_INST_IDS(inst_ids);

#if (NCS_PSS_RED == 1)
	if (pinst_re_ids != NULL)
		m_MMGR_FREE_MIB_INST_IDS(pinst_re_ids);
	pwe_cb->curr_plbck_ssn_info.mib_idx.i_inst_ids = NULL;
#endif

	if (tbl_info->capability == NCSMIB_CAPABILITY_SETALLROWS) {
		if (first_inst_ids != NULL)
			m_MMGR_FREE_MIB_INST_IDS(first_inst_ids);

		if (ra_inited == TRUE) {
			USRBUF *ub = NULL;
			ncsrow_enc_done(&ra);
			ub = ncssetallrows_enc_done(&ra);
			if (ub != NULL)
				m_MMGR_FREE_BUFR_LIST(ub);
		}
	}

	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_PROCESS_DIFFQ_END);
	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_destroy_queue

  DESCRIPTION:       This function destroys the add and change queues. It
                     first empties the linked list and then destroys the
                     queue itself.

  RETURNS:           void

*****************************************************************************/
void pss_destroy_queue(NCS_QUEUE *diff_q)
{
	NCS_QELEM *elem = NULL;
	PSS_QELEM *pss_elem;

	if (diff_q == NULL)
		return;

	elem = ncs_dequeue(diff_q);
	while (elem != NCS_QELEM_NULL) {
		pss_elem = (PSS_QELEM *)elem;
		if (pss_elem->data != NULL)
			m_MMGR_FREE_PSS_OCT(pss_elem->data);

		m_MMGR_FREE_PSS_QELEM(pss_elem);
		elem = ncs_dequeue(diff_q);
	}

	ncs_destroy_queue(diff_q);
}

/*****************************************************************************

  PROCEDURE NAME:    pss_add_to_diff_q

  DESCRIPTION:       This function adds a new element to the change or add
                     queue.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                               for more details

*****************************************************************************/
uns32 pss_add_to_diff_q(NCS_QUEUE *diff_q, uns32 tbl, char *p_pcn, uns8 *data, uns32 len)
{
	PSS_QELEM *elem;

	elem = m_MMGR_ALLOC_PSS_QELEM;
	if (elem == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_PSS_QELEM_ALLOC_FAIL, "pss_add_to_diff_q()");
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}
	memset(elem, '\0', sizeof(PSS_QELEM));

	elem->tbl_id = tbl;
	elem->data = m_MMGR_ALLOC_PSS_OCT(len);
	if (elem->data == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_add_to_diff_q()");
		m_MMGR_FREE_PSS_QELEM(elem);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	memcpy(elem->data, data, len);
	ncs_enqueue_head(diff_q, (void *)elem);
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_send_remrow_request

  DESCRIPTION:       This function is a helper function to the on-demand playback
                     functions. It takes in a usrbuf and issues a removerow
                     request and waits for the response.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                               for more details

*****************************************************************************/
uns32 pss_send_remrow_request(PSS_PWE_CB *pwe_cb, USRBUF *ub, uns32 tbl, NCSMIB_IDX *first_idx)
{
	NCSMIB_ARG mib_arg;
	uns8 space[2048];
	NCSMEM_AID ma;
	uns32 retval = NCSCC_RC_SUCCESS;

	memset(&mib_arg, 0, sizeof(mib_arg));
	ncsmib_init(&mib_arg);
	mib_arg.i_idx = *first_idx;
	mib_arg.i_mib_key = pwe_cb->mac_key;
	mib_arg.i_op = NCSMIB_OP_REQ_REMOVEROWS;
	mib_arg.i_policy |= NCSMIB_POLICY_PSS_BELIEVE_ME;
	mib_arg.i_rsp_fnc = NULL;
	mib_arg.i_tbl_id = tbl;
	mib_arg.i_usr_key = pwe_cb->hm_hdl;
	mib_arg.i_xch_id = pwe_cb->p_pss_cb->xch_id++;
	mib_arg.req.info.removerows_req.i_usrbuf = ub;

	ncsmem_aid_init(&ma, space, sizeof(space));

	m_PSS_LOG_NCSMIB_ARG(&mib_arg);

	retval = pss_sync_mib_req(&mib_arg, pwe_cb->p_pss_cb->mib_req_func, 1000, &ma);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_FAILED);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (mib_arg.i_op != NCSMIB_OP_RSP_REMOVEROWS) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_RSP_NOT_REMOVEROWS);
		return NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
	}

	if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_MIB_REQ_RSP_IS_NOT_SUCCESS, mib_arg.rsp.i_status);
		return NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
	}

	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_MIB_REQ_SUCCEEDED);

	if (mib_arg.rsp.info.removerows_rsp.i_usrbuf != NULL)
		m_MMGR_FREE_BUFR_LIST(mib_arg.rsp.info.removerows_rsp.i_usrbuf);
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_send_remove_request

  DESCRIPTION:       This function is a helper function to the on-demand/warmboot 
                     playback functions. It takes in a "status" param-id and issues 
                     a remove request and waits for the response.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                               for more details

*****************************************************************************/
uns32 pss_send_remove_request(PSS_PWE_CB *pwe_cb, uns32 tbl, NCSMIB_IDX *idx, NCSMIB_PARAM_ID status_param_id)
{
	NCSMIB_ARG mib_arg;
	NCSMEM_AID ma;
	uns8 space[2048];
	uns32 retval = NCSCC_RC_SUCCESS;

	memset(&mib_arg, 0, sizeof(mib_arg));
	ncsmib_init(&mib_arg);
	mib_arg.i_idx = *idx;
	mib_arg.i_mib_key = pwe_cb->mac_key;
	mib_arg.i_op = NCSMIB_OP_REQ_SET;
	mib_arg.i_policy |= NCSMIB_POLICY_PSS_BELIEVE_ME;
	mib_arg.i_rsp_fnc = NULL;
	mib_arg.i_tbl_id = tbl;
	mib_arg.i_usr_key = pwe_cb->hm_hdl;
	mib_arg.i_xch_id = pwe_cb->p_pss_cb->xch_id++;
	mib_arg.req.info.set_req.i_param_val.i_param_id = status_param_id;
	mib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
	mib_arg.req.info.set_req.i_param_val.info.i_int = NCSMIB_ROWSTATUS_DESTROY;

	ncsmem_aid_init(&ma, space, sizeof(space));

	m_PSS_LOG_NCSMIB_ARG(&mib_arg);

	retval = pss_sync_mib_req(&mib_arg, pwe_cb->p_pss_cb->mib_req_func, 1000, &ma);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_FAILED);
		return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
	}

	if (mib_arg.i_op != NCSMIB_OP_RSP_SET) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_RSP_NOT_SET);
		return NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
	}

	if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_MIB_REQ_RSP_IS_NOT_SUCCESS, mib_arg.rsp.i_status);
		return NCSCC_RC_SUCCESS;	/* It is not PSS's plate to handle this error. */
	}

	m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_MIB_REQ_SUCCEEDED);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_process_tbl_cmd

  DESCRIPTION:       This function processes the MIB requests on the Cmd-MIB
                     table NCSMIB_TBL_PSR_CMD.

  RETURNS:           SUCCESS - all went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK
                     for details.

*****************************************************************************/
uns32 pss_process_tbl_cmd(PSS_CB *inst, NCSMIB_ARG *arg)
{
	uns32 cmd_id = arg->req.info.cli_req.i_cmnd_id;
	uns32 rc = NCSCC_RC_SUCCESS;

	switch (cmd_id) {
	case PSS_CMD_TBL_CMD_NUM_SET_XML_OPTION:
		rc = pss_process_set_plbck_option_for_spcn(inst, arg);
		m_PSS_RE_RELOAD_PSSVSPCNLIST(gl_pss_amf_attribs.csi_list->pwe_cb);
		arg->i_op = m_NCSMIB_REQ_TO_RSP(arg->i_op);
		arg->rsp.i_status = rc;
		arg->rsp.info.cli_rsp.i_cmnd_id = cmd_id;
		if (arg->i_rsp_fnc(arg) != NCSCC_RC_SUCCESS) {
			if (arg->rsp.info.cli_rsp.o_answer != NULL) {
				m_MMGR_FREE_BUFR_LIST(arg->rsp.info.cli_rsp.o_answer);
				arg->rsp.info.cli_rsp.o_answer = NULL;
			}
		}
		break;

	case PSS_CMD_TBL_CMD_NUM_DISPLAY_MIB:
		rc = pss_process_display_mib_entries(inst, arg);
		break;

	case PSS_CMD_TBL_CMD_NUM_DISPLAY_PROFILE:
		rc = pss_process_display_profile_entries(inst, arg);
		arg->i_op = m_NCSMIB_REQ_TO_RSP(arg->i_op);
		arg->rsp.i_status = rc;
		arg->rsp.info.cli_rsp.i_cmnd_id = cmd_id;
		if (arg->i_rsp_fnc(arg) != NCSCC_RC_SUCCESS) {
			if (arg->rsp.info.cli_rsp.o_answer != NULL) {
				m_MMGR_FREE_BUFR_LIST(arg->rsp.info.cli_rsp.o_answer);
				arg->rsp.info.cli_rsp.o_answer = NULL;
			}
		}
		break;

	default:
		if (arg->req.info.cli_req.i_usrbuf != NULL) {
			m_MMGR_FREE_BUFR_LIST(arg->req.info.cli_req.i_usrbuf);
			arg->req.info.cli_req.i_usrbuf = NULL;
		}
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_process_set_plbck_option_for_spcn

  DESCRIPTION:       This function is a helper function to execute a CLI 
                     command.

  RETURNS:           void.

*****************************************************************************/
uns32 pss_process_set_plbck_option_for_spcn(PSS_CB *inst, NCSMIB_ARG *arg)
{
	NCS_UBAID lcl_uba;
	USRBUF *buff = arg->req.info.cli_req.i_usrbuf;
	uns8 *buff_ptr = NULL;
	uns16 str_len = 0;
	uns16 local_len = 0;
	int8 pcn[NCSMIB_PCN_LENGTH_MAX];
	PSS_SPCN_LIST *spcn_entry = NULL;

	memset(&pcn, '\0', sizeof(pcn));
	memset(&lcl_uba, '\0', sizeof(lcl_uba));

	ncs_dec_init_space(&lcl_uba, buff);
	arg->req.info.cli_req.i_usrbuf = NULL;

	buff_ptr = ncs_dec_flatten_space(&lcl_uba, (uns8 *)&local_len, sizeof(uns16));
	if (buff_ptr == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MMGR_BUFFER_ALLOC_FAIL,
				  "pss_process_set_plbck_option_for_spcn()");
		return NCSCC_RC_FAILURE;
	}
	str_len = ncs_decode_16bit(&buff_ptr);
	ncs_dec_skip_space(&lcl_uba, sizeof(uns16));

	if (ncs_decode_n_octets_from_uba(&lcl_uba, (char *)&pcn, str_len) != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MMGR_BUFFER_ALLOC_FAIL,
				  "pss_process_set_plbck_option_for_spcn()");
		return NCSCC_RC_FAILURE;
	}
	if (lcl_uba.start != NULL) {
		m_MMGR_FREE_BUFR_LIST(lcl_uba.start);
	} else if (lcl_uba.ub != NULL) {
		m_MMGR_FREE_BUFR_LIST(lcl_uba.ub);
	}
	arg->req.info.cli_req.i_usrbuf = NULL;
	/* The character array "pcn" is already populated, when this macro returns. */

	/* Now, search the spcn_list, and update the boolean value. */
	if ((spcn_entry = pss_findadd_entry_frm_spcnlist(inst, (char *)&pcn, TRUE)) == NULL) {
		return NCSCC_RC_FAILURE;
	}
	spcn_entry->plbck_frm_bam = TRUE;	/* Playback to be done from BAM */

	/* Now, update the boolean value on the config file 
	   pwe_cb->p_pss_cb->spcn_list_file */
	if (pss_update_entry_in_spcn_conf_file(inst, spcn_entry) != NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}

	arg->rsp.info.cli_rsp.o_partial = FALSE;
	arg->rsp.info.cli_rsp.o_answer = NULL;

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_update_entry_in_spcn_conf_file

  DESCRIPTION:       This function is a helper function to update the boolean
                     of an SPCN client in the pssv_spcn.list config file.

  RETURNS:           NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.

*****************************************************************************/
uns32 pss_update_entry_in_spcn_conf_file(PSS_CB *inst, PSS_SPCN_LIST *entry)
{

	FILE *fh;
	PSS_SPCN_LIST *tmp = inst->spcn_list;

	/* Do not unlink, the file could be a symbolic link. Truncate file instead. */
	fh = sysf_fopen((char *)&inst->spcn_list_file, "w");
	if (fh == NULL) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_SPCN_LIST_FILE_OPEN_IN_APPEND_MODE_FAIL);
		return NCSCC_RC_FAILURE;
	}

	while (tmp != NULL) {

		if (tmp->plbck_frm_bam)
			fprintf(fh, "%s %s\n", tmp->pcn, m_PSS_SPCN_SOURCE_BAM);	/* Add entry */
		else
			fprintf(fh, "%s %s\n", tmp->pcn, m_PSS_SPCN_SOURCE_PSSV);	/* Add entry */
		tmp = tmp->next;
	}

	fclose(fh);		/* Close the file handle */

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_process_display_mib_entries

  DESCRIPTION:       This function is a helper function to execute a CLI
                     command.

  RETURNS:           void.

*****************************************************************************/
uns32 pss_process_display_mib_entries(PSS_CB *inst, NCSMIB_ARG *arg)
{
	NCS_UBAID lcl_uba, lcl_uba1;
	USRBUF *buff = arg->req.info.cli_req.i_usrbuf;
	uns8 *buff_ptr = NULL;
	uns16 str_len = 0, pwe_id = 0;
	char pcn_name[NCSMIB_PCN_LENGTH_MAX], file_name[512];
	char profile_name[128];
	uns32 retval = NCSCC_RC_SUCCESS, i = 0, tbl_cnt = 0, j = 0, tbl_id = 0;
	NCS_BOOL profile_exists = FALSE, partial = TRUE;
	FILE *fh = NULL;
	NCSMIB_ARG *temp_arg = NULL;
	uns16 pwe_cnt = 0;
	NCS_OS_FILE file;
	NCS_PSSTS_CB *ts_inst = NULL;
	char prof_path[NCS_PSSTS_MAX_PATH_LEN];
	PSS_MIB_TBL_INFO *tbl_info = NULL;
	uns32 rc = NCSCC_RC_SUCCESS;
	USRBUF *lcl_ubuf = NULL;
	char dest_file_path[NCS_PSSTS_MAX_PATH_LEN];
	long file_hdl = 0;
	PSS_TABLE_PATH_RECORD ps_file_record;
	PSS_TABLE_DETAILS_HEADER hdr;
	NCS_OS_FILE inst_file;

	memset(&pcn_name, '\0', sizeof(pcn_name));
	memset(&profile_name, '\0', sizeof(profile_name));
	memset(&file_name, '\0', sizeof(file_name));
	memset(&lcl_uba, '\0', sizeof(lcl_uba));
	memset(&lcl_uba1, '\0', sizeof(lcl_uba1));

	arg->rsp.info.cli_rsp.o_partial = FALSE;
	arg->rsp.info.cli_rsp.o_answer = NULL;

	ncs_dec_init_space(&lcl_uba, buff);
	arg->req.info.cli_req.i_usrbuf = NULL;

	buff_ptr = ncs_dec_flatten_space(&lcl_uba, (uns8 *)&str_len, sizeof(uns16));
	if (buff_ptr == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MMGR_BUFFER_ALLOC_FAIL,
				  "pss_process_display_mib_entries()");
		return NCSCC_RC_FAILURE;
	}
	str_len = ncs_decode_16bit(&buff_ptr);
	ncs_dec_skip_space(&lcl_uba, sizeof(uns16));

	if (ncs_decode_n_octets_from_uba(&lcl_uba, (char *)&profile_name, str_len) != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MMGR_BUFFER_ALLOC_FAIL,
				  "pss_process_display_mib_entries()");
		return NCSCC_RC_FAILURE;
	}
	/* The character array "profile_name" is already populated, when this macro returns. */

	buff_ptr = ncs_dec_flatten_space(&lcl_uba, (uns8 *)&str_len, sizeof(uns16));
	if (buff_ptr == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MMGR_BUFFER_ALLOC_FAIL,
				  "pss_process_display_mib_entries()");
		return NCSCC_RC_FAILURE;
	}
	str_len = ncs_decode_16bit(&buff_ptr);
	ncs_dec_skip_space(&lcl_uba, sizeof(uns16));

	if (ncs_decode_n_octets_from_uba(&lcl_uba, (char *)&file_name, str_len) != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MMGR_BUFFER_ALLOC_FAIL,
				  "pss_process_display_mib_entries()");
		return NCSCC_RC_FAILURE;
	}
	/* The character array "file_name" is already populated, when this macro returns. */

	fh = sysf_fopen((char *)&file_name, "a+");
	if (fh == NULL) {
		printf("\n\npss_cef_dump_profile(): Can't open file %s in append mode...\n\n", (char *)&file_name);
		return NCSCC_RC_FAILURE;
	}
	fprintf(fh, "***STARTING DUMP of PROFILE:%s for PCN:%s(ALL VALUES ARE SHOWN IN DECIMAL NOTATION)***\n",
		(char *)&profile_name, (char *)&pcn_name);

	buff_ptr = ncs_dec_flatten_space(&lcl_uba, (uns8 *)&str_len, sizeof(uns16));
	if (buff_ptr == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MMGR_BUFFER_ALLOC_FAIL,
				  "pss_process_display_mib_entries()");
		fclose(fh);
		return NCSCC_RC_FAILURE;
	}
	str_len = ncs_decode_16bit(&buff_ptr);
	ncs_dec_skip_space(&lcl_uba, sizeof(uns16));

	if (ncs_decode_n_octets_from_uba(&lcl_uba, (char *)&pcn_name, str_len) != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MMGR_BUFFER_ALLOC_FAIL,
				  "pss_process_display_mib_entries()");
		fclose(fh);
		return NCSCC_RC_FAILURE;
	}
	/* The character array "pcn_name" is already populated, when this macro returns. */

	/* Open the profile and read the PCNs */
	m_NCS_PSSTS_PROFILE_EXISTS(inst->pssts_api, inst->pssts_hdl, retval, (char *)&profile_name, profile_exists);
	if ((retval != NCSCC_RC_SUCCESS) || (profile_exists == FALSE)) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PROFILE_SPCFD);

		/* Get the copy of arg */
		temp_arg = ncsmib_memcopy(arg);
		if (temp_arg == NULL) {
			/* Not able to get the copy of NCSMIB_ARG */
			fprintf(fh, "***DUMP of PROFILE:%s for PCN:%s INCOMPLETE***\n", (char *)&profile_name,
				(char *)&pcn_name);
			ncshm_give_hdl(gl_pss_amf_attribs.handles.pssts_hdl);
			fclose(fh);
			return NCSCC_RC_FAILURE;
		}
		/* Send response back to the CLI engine */
		temp_arg->rsp.info.cli_rsp.o_partial = partial;
		temp_arg->rsp.i_status = NCSCC_RC_INV_VAL;
		temp_arg->i_op = m_NCSMIB_REQ_TO_RSP(arg->i_op);
		temp_arg->rsp.info.cli_rsp.i_cmnd_id = PSS_CMD_TBL_CMD_NUM_DISPLAY_MIB;
		if (temp_arg->i_rsp_fnc(temp_arg) != NCSCC_RC_SUCCESS) {
			;
		}
		if (temp_arg->rsp.info.cli_rsp.o_answer != NULL)
			m_MMGR_FREE_BUFR_LIST(temp_arg->rsp.info.cli_rsp.o_answer);
		temp_arg->rsp.info.cli_rsp.o_answer = NULL;
		m_MMGR_FREE_NCSMIB_ARG(temp_arg);
		temp_arg = NULL;
		fclose(fh);

		return NCSCC_RC_INV_VAL;
	}

	m_NCS_PSSTS_GET_MIB_LIST_PER_PCN(inst->pssts_api, inst->pssts_hdl,
					 retval, (char *)&profile_name, (char *)&pcn_name, &lcl_ubuf);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_GET_PROF_MIB_LIST_PER_PCN_OP_FAILED);

		/* Get the copy of arg */
		temp_arg = ncsmib_memcopy(arg);
		if (temp_arg == NULL) {
			/* Not able to get the copy of NCSMIB_ARG */
			fprintf(fh, "***DUMP of PROFILE:%s for PCN:%s INCOMPLETE***\n", (char *)&profile_name,
				(char *)&pcn_name);
			ncshm_give_hdl(gl_pss_amf_attribs.handles.pssts_hdl);
			fclose(fh);
			return NCSCC_RC_FAILURE;
		}
		/* Send response back to the CLI engine */
		temp_arg->rsp.info.cli_rsp.o_partial = partial;
		temp_arg->rsp.i_status = NCSCC_RC_FAILURE;
		temp_arg->i_op = m_NCSMIB_REQ_TO_RSP(arg->i_op);
		temp_arg->rsp.info.cli_rsp.i_cmnd_id = PSS_CMD_TBL_CMD_NUM_DISPLAY_MIB;
		if (temp_arg->i_rsp_fnc(temp_arg) != NCSCC_RC_SUCCESS) {
			;
		}
		if (temp_arg->rsp.info.cli_rsp.o_answer != NULL)
			m_MMGR_FREE_BUFR_LIST(temp_arg->rsp.info.cli_rsp.o_answer);
		temp_arg->rsp.info.cli_rsp.o_answer = NULL;
		m_MMGR_FREE_NCSMIB_ARG(temp_arg);
		temp_arg = NULL;
		fclose(fh);

		return NCSCC_RC_FAILURE;
	}

	/* Now, start looking at the buffer contents, and pick the MIB files for reading
	   the MIB rows. */
	ncs_dec_init_space(&lcl_uba1, lcl_ubuf);
	buff_ptr = ncs_dec_flatten_space(&lcl_uba1, (uns8 *)&pwe_cnt, sizeof(uns16));
	if (buff_ptr == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MMGR_BUFFER_ALLOC_FAIL,
				  "pss_process_display_mib_entries()");
		fclose(fh);
		return NCSCC_RC_FAILURE;
	}
	pwe_cnt = ncs_decode_16bit(&buff_ptr);
	ncs_dec_skip_space(&lcl_uba1, sizeof(uns16));
	if (pwe_cnt == 0) {
		/* No PCN information found in the profile. */
		/* Get the copy of arg */
		temp_arg = ncsmib_memcopy(arg);
		if (temp_arg == NULL) {
			/* Not able to get the copy of NCSMIB_ARG */
			fprintf(fh, "***DUMP of PROFILE:%s for PCN:%s INCOMPLETE***\n", (char *)&profile_name,
				(char *)&pcn_name);
			ncshm_give_hdl(gl_pss_amf_attribs.handles.pssts_hdl);
			fclose(fh);
			return NCSCC_RC_FAILURE;
		}
		/* Send response back to the CLI engine */
		temp_arg->rsp.info.cli_rsp.o_partial = partial;
		temp_arg->rsp.i_status = NCSCC_RC_NOSUCHNAME;
		temp_arg->i_op = m_NCSMIB_REQ_TO_RSP(arg->i_op);
		temp_arg->rsp.info.cli_rsp.i_cmnd_id = PSS_CMD_TBL_CMD_NUM_DISPLAY_MIB;
		if (temp_arg->i_rsp_fnc(temp_arg) != NCSCC_RC_SUCCESS) {
			;
		}
		if (temp_arg->rsp.info.cli_rsp.o_answer != NULL)
			m_MMGR_FREE_BUFR_LIST(temp_arg->rsp.info.cli_rsp.o_answer);
		temp_arg->rsp.info.cli_rsp.o_answer = NULL;
		m_MMGR_FREE_NCSMIB_ARG(temp_arg);
		temp_arg = NULL;
		fclose(fh);
		return NCSCC_RC_NO_INSTANCE;
	}

	ts_inst = ncshm_take_hdl(NCS_SERVICE_ID_PSSTS, gl_pss_amf_attribs.handles.pssts_hdl);
	for (i = 0; i < pwe_cnt; i++) {
		temp_arg = NULL;

		file.info.dir_path.i_main_dir = ts_inst->root_dir;
		file.info.dir_path.i_sub_dir = (char *)&profile_name;
		file.info.dir_path.i_buf_size = NCS_PSSTS_MAX_PATH_LEN;
		file.info.dir_path.io_buffer = (uns8 *)&prof_path;	/* Profile path */
		retval = m_NCS_FILE_OP(&file, NCS_OS_FILE_DIR_PATH);
		if (retval != NCSCC_RC_SUCCESS) {
			fprintf(fh, "***DUMP of PROFILE:%s for PCN:%s INCOMPLETE***\n", (char *)&profile_name,
				(char *)&pcn_name);
			ncshm_give_hdl(gl_pss_amf_attribs.handles.pssts_hdl);
			fclose(fh);
			return NCSCC_RC_FAILURE;
		}

		/* Decode pwe-id */
		buff_ptr = ncs_dec_flatten_space(&lcl_uba1, (uns8 *)&pwe_cnt, sizeof(uns16));
		if (buff_ptr == NULL) {
			m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MMGR_BUFFER_ALLOC_FAIL,
					  "pss_process_display_mib_entries()");
			fprintf(fh, "***DUMP of PROFILE:%s for PCN:%s INCOMPLETE***\n", (char *)&profile_name,
				(char *)&pcn_name);
			ncshm_give_hdl(gl_pss_amf_attribs.handles.pssts_hdl);
			fclose(fh);
			return NCSCC_RC_FAILURE;
		}
		pwe_id = ncs_decode_16bit(&buff_ptr);
		ncs_dec_skip_space(&lcl_uba1, sizeof(uns16));
		fprintf(fh, "PWE:%d:START\n", pwe_id);

		/* Decode tbl_cnt */
		buff_ptr = ncs_dec_flatten_space(&lcl_uba1, (uns8 *)&tbl_cnt, sizeof(uns16));
		if (buff_ptr == NULL) {
			m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MMGR_BUFFER_ALLOC_FAIL,
					  "pss_process_display_mib_entries()");
			fprintf(fh, "***DUMP of PROFILE:%s for PCN:%s INCOMPLETE***\n", (char *)&profile_name,
				(char *)&pcn_name);
			ncshm_give_hdl(gl_pss_amf_attribs.handles.pssts_hdl);
			fclose(fh);
			return NCSCC_RC_FAILURE;
		}
		tbl_cnt = ncs_decode_16bit(&buff_ptr);
		ncs_dec_skip_space(&lcl_uba1, sizeof(uns16));

		if (tbl_cnt != 0) {
			/* Get the copy of arg */
			temp_arg = ncsmib_memcopy(arg);
			if (temp_arg == NULL) {
				/* Not able to get the copy of NCSMIB_ARG */
				fprintf(fh, "***DUMP of PROFILE:%s for PCN:%s INCOMPLETE***\n", (char *)&profile_name,
					(char *)&pcn_name);
				ncshm_give_hdl(gl_pss_amf_attribs.handles.pssts_hdl);
				fclose(fh);
				return NCSCC_RC_FAILURE;
			}
		}

		for (j = 0; j < tbl_cnt; j++) {
			/* Decode tbl_id */
			buff_ptr = ncs_dec_flatten_space(&lcl_uba1, (uns8 *)&tbl_id, sizeof(uns32));
			if (buff_ptr == NULL) {
				m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MMGR_BUFFER_ALLOC_FAIL,
						  "pss_process_display_mib_entries()");
				fprintf(fh, "***DUMP of PROFILE:%s for PCN:%s INCOMPLETE***\n", (char *)&profile_name,
					(char *)&pcn_name);
				ncshm_give_hdl(gl_pss_amf_attribs.handles.pssts_hdl);
				ncsmib_memfree(temp_arg);
				fclose(fh);
				return NCSCC_RC_FAILURE;
			}
			tbl_id = ncs_decode_32bit(&buff_ptr);
			ncs_dec_skip_space(&lcl_uba1, sizeof(uns32));
			tbl_info = inst->mib_tbl_desc[tbl_id];
			if (tbl_info == NULL) {
				continue;
			}
			fprintf(fh, "\n\t  MIB-TBL:%d:%s:START:STATUS-PARAM_ID[%d]\n", tbl_id,
				(char *)tbl_info->ptbl_info->mib_tbl_name, tbl_info->ptbl_info->status_field);

			if ((i == (pwe_cnt - 1)) && (j == (tbl_cnt - 1))) {
				partial = FALSE;	/* This is the last table */
			}

			/* 3.0.a addition */
			snprintf(dest_file_path, sizeof(dest_file_path) - 1, "%s%d/%s/%d/%s/%d_tbl_details",
				 NCS_PSS_DEF_PSSV_ROOT_PATH, PSS_PS_FORMAT_VERSION, profile_name, pwe_id, pcn_name,
				 tbl_id);
			inst_file.info.file_exists.i_file_name = dest_file_path;
			m_NCS_OS_FILE(&inst_file, NCS_OS_FILE_EXISTS);

			if (inst_file.info.file_exists.o_file_exists == TRUE) {
				memset(&inst_file, '\0', sizeof(NCS_OS_FILE));
				/* Open the persistent file of the table for reading */
				inst_file.info.open.i_file_name = dest_file_path;
				inst_file.info.open.i_read_write_mask = NCS_OS_FILE_PERM_READ;

				if (NCSCC_RC_SUCCESS != m_NCS_FILE_OP(&inst_file, NCS_OS_FILE_OPEN)) {
					m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_OPEN_FAIL, pwe_id, pcn_name, tbl_id);
					return NCSCC_RC_FAILURE;
				}

				file_hdl = (long)inst_file.info.open.o_file_handle;

				/* Filling the current persistent format version, profile, pwe_id, pcn and table_id in ps_file_record */
				ps_file_record.ps_format_version = PSS_PS_FORMAT_VERSION;
				memcpy(&ps_file_record.profile, &profile_name, sizeof(profile_name));
				ps_file_record.pwe_id = pwe_id;
				memcpy(&ps_file_record.pcn, &pcn_name, sizeof(pcn_name));
				ps_file_record.tbl_id = tbl_id;

				/* Read the persistent file of the table for table details */
				retval =
				    pss_tbl_details_header_read(inst, inst->pssts_hdl, file_hdl, &ps_file_record, &hdr);

				if (NCSCC_RC_SUCCESS != retval) {
					if (file_hdl != 0)
						m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, inst->pssts_hdl, retval,
								       file_hdl);
					fprintf(fh, "\n\t  MIB-TBL:%d:%s:ERROR Reading Table details \n\n", tbl_id,
						(char *)tbl_info->ptbl_info->mib_tbl_name);
					return NCSCC_RC_FAILURE;
				}
				if (file_hdl != 0)
					m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, inst->pssts_hdl, retval, file_hdl);
				fprintf(fh,
					"\t TABLE DETAILS:\tHEADER LEN:%dPERSISTENT STORE VERSION:%d\n\tTABLE VERSION:%d\tMAX ROW LEN:%d\tMAX KEY LEN:%d\tBITMAP LEN:%d\n",
					hdr.header_len, hdr.ps_format_version, hdr.table_version, hdr.max_row_length,
					hdr.max_key_length, hdr.bitmap_length);
			}
			/* End of 3.0.a addition */

			/* Read the MIB file, and dump row entries into the log file */
			rc = pss_pop_mibrows_into_buffer(inst, (char *)&profile_name, pwe_id, (char *)&pcn_name, tbl_id,
							 temp_arg, fh);
			if (rc == NCSCC_RC_SUCCESS)
				fprintf(fh, "\n\t  MIB-TBL:%d:%s:END\n\n", tbl_id,
					(char *)tbl_info->ptbl_info->mib_tbl_name);
			else
				fprintf(fh, "\n\t  MIB-TBL:%d:%s:ERROR\n\n", tbl_id,
					(char *)tbl_info->ptbl_info->mib_tbl_name);

			/* Send response back to the CLI engine */
			temp_arg->rsp.info.cli_rsp.o_partial = partial;
			temp_arg->rsp.i_status = rc;
			temp_arg->i_op = m_NCSMIB_REQ_TO_RSP(arg->i_op);
			temp_arg->rsp.info.cli_rsp.i_cmnd_id = PSS_CMD_TBL_CMD_NUM_DISPLAY_MIB;
			if (temp_arg->i_rsp_fnc(temp_arg) != NCSCC_RC_SUCCESS) {
				if (temp_arg->rsp.info.cli_rsp.o_answer != NULL)
					m_MMGR_FREE_BUFR_LIST(temp_arg->rsp.info.cli_rsp.o_answer);
				temp_arg->rsp.info.cli_rsp.o_answer = NULL;
			}
			/* Assuming success return, would imply that temp_arg->rsp.info.cli_rsp.o_answer is freed already. */
		}
		fprintf(fh, "PWE:%d:END\n\n", pwe_id);
	}
	ncsmib_memfree(temp_arg);
	fprintf(fh, "***COMPLETED DUMP of PROFILE:%s for PCN:%s***\n", (char *)&profile_name, (char *)&pcn_name);
	ncshm_give_hdl(gl_pss_amf_attribs.handles.pssts_hdl);
	fclose(fh);
	return NCSCC_RC_SUCCESS;
}

uns32 pss_pop_mibrows_into_buffer(PSS_CB *inst, char *profile, uns16 pwe_id, char *pcn, uns32 tbl_id,
				  NCSMIB_ARG *arg, FILE *fh)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	if (inst->mib_tbl_desc[tbl_id]->ptbl_info->table_of_scalars)
		rc = pss_dump_sclr_tbl(inst, profile, pwe_id, pcn, tbl_id, arg, fh);
	else
		rc = pss_dump_tbl(inst, profile, pwe_id, pcn, tbl_id, arg, fh);

	return rc;
}

uns32 pss_dump_sclr_tbl(PSS_CB *inst, char *profile, uns16 pwe_id, char *pcn, uns32 tbl_id, NCSMIB_ARG *arg, FILE *fh)
{
	uns32 retval = NCSCC_RC_SUCCESS;
	uns32 bytes_read = 0, j = 0;
	uns8 *curr_data = NULL;
	PSS_MIB_TBL_INFO *tbl_info = inst->mib_tbl_desc[tbl_id];
	PSS_TABLE_PATH_RECORD ps_file_record;
	PSS_TABLE_DETAILS_HEADER hdr;
	long curr_file_hdl = 0;

	/* Allocate memory for the buffers and keys */
	curr_data = m_MMGR_ALLOC_PSS_OCT(tbl_info->max_row_length);
	if (curr_data == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_dump_sclr_tbl()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	memset(curr_data, '\0', tbl_info->max_row_length);

	/* Now open the files and read in data from them */
	m_NCS_PSSTS_FILE_OPEN(inst->pssts_api, inst->pssts_hdl, retval,
			      profile, pwe_id, pcn, tbl_id, NCS_PSSTS_FILE_PERM_READ, curr_file_hdl);
	if (retval != NCSCC_RC_SUCCESS) {
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	/* 3.0.b addition */

	/* Filling the current persistent format version, profile, pwe_id, pcn and table_id in ps_file_record */
	ps_file_record.ps_format_version = PSS_PS_FORMAT_VERSION;
	memcpy(&ps_file_record.profile, &profile, NCS_PSS_MAX_PROFILE_NAME);
	ps_file_record.pwe_id = pwe_id;
	memcpy(&ps_file_record.pcn, &pcn, NCSMIB_PCN_LENGTH_MAX);
	ps_file_record.tbl_id = tbl_id;

	memset(&hdr, '\0', sizeof(PSS_TABLE_DETAILS_HEADER));
	/* Read the persistent file of the table for table details */
	retval = pss_tbl_details_header_read(inst, inst->pssts_hdl, curr_file_hdl, &ps_file_record, &hdr);

	if (NCSCC_RC_SUCCESS != retval) {
		fprintf(fh, "\n\t  MIB-TBL:%d:%s:ERROR Reading Table details \n\n", tbl_id,
			(char *)tbl_info->ptbl_info->mib_tbl_name);
		goto cleanup;
	}
	fprintf(fh,
		"\t TABLE DETAILS:\tHEADER LEN:%dPERSISTENT STORE VERSION:%d\n\tTABLE VERSION:%d\tMAX ROW LEN:%d\tMAX KEY LEN:%d\tBITMAP LEN:%d",
		hdr.header_len, hdr.ps_format_version, hdr.table_version, hdr.max_row_length, hdr.max_key_length,
		hdr.bitmap_length);

	/* End of 3.0.b addition */

	m_NCS_PSSTS_FILE_READ(inst->pssts_api, inst->pssts_hdl, retval, curr_file_hdl,
			      tbl_info->max_row_length, hdr.header_len, curr_data, bytes_read);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_READ_FAIL, pwe_id, pcn, tbl_id);
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	if (bytes_read != tbl_info->max_row_length) {
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	fprintf(fh, "\t    ROW:START\n");
	/* lcl_params_max = pss_get_count_of_valid_params(curr_data, tbl_info); */
	/* Send MIB SET requests for each of the parameters of the scalar table. */
	for (j = 0; j < tbl_info->ptbl_info->num_objects; j++) {
		uns32 param_offset = tbl_info->pfields[j].offset;
		NCSMIB_PARAM_VAL pv;

		if (pss_get_bit_for_param(curr_data, (j + 1)) == 0)
			continue;

		memset(&pv, '\0', sizeof(pv));
		switch (tbl_info->pfields[j].var_info.fmat_id) {
		case NCSMIB_FMAT_INT:
			pv.i_fmat_id = NCSMIB_FMAT_INT;
			pv.i_param_id = tbl_info->pfields[j].var_info.param_id;
			switch (tbl_info->pfields[j].var_info.len) {
				uns16 data16;
			case sizeof(uns8):
				pv.info.i_int = (uns32)(*((uns8 *)(curr_data + param_offset)));
				break;
			case sizeof(uns16):
				memcpy(&data16, curr_data + param_offset, sizeof(uns16));
				pv.info.i_int = (uns32)data16;
				break;
			case sizeof(uns32):
				memcpy(&pv.info.i_int, curr_data + param_offset, sizeof(uns32));
				break;
			default:
				fprintf(fh, "\t    ROW:PARAM-ERROR - Invalid var_len for FMAT_ID\n");
				return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			}
			break;

		case NCSMIB_FMAT_BOOL:
			{
				NCS_BOOL data_bool;
				pv.i_fmat_id = NCSMIB_FMAT_INT;
				pv.i_param_id = tbl_info->pfields[j].var_info.param_id;
				memcpy(&data_bool, curr_data + param_offset, sizeof(NCS_BOOL));
				pv.info.i_int = (uns32)data_bool;
			}
			break;

		case NCSMIB_FMAT_OCT:
			{
				PSS_VAR_INFO *var_info = &(tbl_info->pfields[j]);
				if (var_info->var_length == TRUE) {
					memcpy(&pv.i_length, curr_data + param_offset, sizeof(uns16));
					param_offset += sizeof(uns16);
				} else
					pv.i_length = var_info->var_info.obj_spec.stream_spec.max_len;

				pv.i_fmat_id = NCSMIB_FMAT_OCT;
				pv.i_param_id = var_info->var_info.param_id;
				pv.info.i_oct = curr_data + param_offset;
			}
			break;

		default:
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			fprintf(fh, "\t    ROW:PARAM-ERROR - Invalid FMAT_ID\n");
			goto cleanup;
		}
		pss_dump_mib_var(fh, &pv, tbl_info, j);	/* Log the Variable info */
	}			/* for (j = 0; j < tbl_info->ptbl_info->num_objects; j++) */
	fprintf(fh, "\t    ROW:END\n\n");

 cleanup:
	if (curr_data != NULL)
		m_MMGR_FREE_PSS_OCT(curr_data);

	if (curr_file_hdl != 0)
		m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, inst->pssts_hdl, retval, curr_file_hdl);

	return retval;
}

void pss_dump_mib_var(FILE *fh, NCSMIB_PARAM_VAL *pv, PSS_MIB_TBL_INFO *tbl_info, uns32 param_index)
{
	uns8 *p8 = NULL;
	uns32 cnt = 0, j = 0;
	uns16 max_len = 0x0000;
	PSS_VAR_INFO *var_info = NULL;

	var_info = &tbl_info->pfields[param_index];

	switch (pv->i_fmat_id) {
	case NCSMIB_FMAT_INT:
		fprintf(fh, "\t      PARAM-ID(%d:%s), IS_INDEX[%s], FMAT-ID:INT, LEN:%d, VAL:%d\n", pv->i_param_id,
			var_info->var_name, (var_info->var_info.is_index_id ? "YES" : "NO"),
			var_info->var_info.len, pv->info.i_int);
		break;
	case NCSMIB_FMAT_OCT:
		if (var_info->var_length == TRUE) {
			if (var_info->var_info.obj_type == NCSMIB_OCT_DISCRETE_OBJ_TYPE)
				max_len = ncsmiblib_get_max_len_of_discrete_octet_string(&var_info->var_info);
			else
				max_len = var_info->var_info.obj_spec.stream_spec.max_len;
		} else
			max_len = pv->i_length;

		fprintf(fh, "\t      PARAM-ID(%d:%s), IS_INDEX[%s], FMAT-ID:OCT, LEN:%d, MAX:%d",
			pv->i_param_id, var_info->var_name, (var_info->var_info.is_index_id ? "YES" : "NO"),
			pv->i_length, max_len);
		for (cnt = 0, p8 = (uns8 *)pv->info.i_oct; cnt < pv->i_length; cnt++, ++p8) {
			/* Approximately 20 Octet values are being accomodated in one row dump of 80 columns each */
			if ((cnt % 20) == 0)
				fprintf(fh, "\n\t        OCT-VAL[%d] : %d ", cnt, (uns8)(*p8));
			else
				fprintf(fh, "%d ", (uns8)(*p8));
		}
		for (j = cnt; j < max_len; j++) {
			/* Approximately 20 Octet values are being accomodated in one row dump of 80 columns each */
			if ((j % 20) == 0)
				fprintf(fh, "\n\t        OCT-VAL[%d] : %d ", j, 0);
			else
				fprintf(fh, "%d ", 0);
		}
		fprintf(fh, "\n");
		break;
	case NCSMIB_FMAT_BOOL:
		fprintf(fh, "\t      PARAM-ID(%d:%s), IS_INDEX[%s], FMAT-ID:BOOL, LEN:%ld, VAL:%d\n",
			pv->i_param_id, var_info->var_name, (var_info->var_info.is_index_id ? "YES" : "NO"),
			(long)sizeof(NCS_BOOL), pv->info.i_int);
		break;
	default:
		fprintf(fh, "PARAM-INFO:ERROR - Invalid FMAT_ID\n");
		break;
	}

	return;
}

uns32 pss_dump_tbl(PSS_CB *inst, char *profile, uns16 pwe_id, char *pcn, uns32 tbl_id, NCSMIB_ARG *arg, FILE *fh)
{
	uns32 retval = NCSCC_RC_SUCCESS, num_items = 0, item_size = 0;
	uns32 bytes_read = 0, j = 0, row_size = 0;
	uns32 file_size = 0, rem_file_size = 0, max_num_rows = 0, buf_size = 0;
	uns32 read_offset = 0, i = 0;
	long file_hdl = 0;
	uns8 *in_buf = NULL, *ptr = NULL;
	PSS_MIB_TBL_INFO *tbl_info = inst->mib_tbl_desc[tbl_id];
	NCS_BOOL is_last_chunk = FALSE;
	PSS_TABLE_PATH_RECORD ps_file_record;
	PSS_TABLE_DETAILS_HEADER hdr;

	tbl_info = inst->mib_tbl_desc[tbl_id];
	row_size = tbl_info->max_row_length;
	max_num_rows = NCS_PSS_MAX_CHUNK_SIZE / row_size;
	if (max_num_rows == 0)	/* Or should we return an error here? */
		max_num_rows = 1;

	buf_size = max_num_rows * row_size;

	/* Allocate memory for the buffers */
	in_buf = m_MMGR_ALLOC_PSS_OCT(buf_size);
	if (in_buf == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_OCT_ALLOC_FAIL, "pss_dump_tbl()");
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}
	memset(in_buf, '\0', buf_size);

	/* Open the table file for reading */
	m_NCS_PSSTS_FILE_OPEN(inst->pssts_api, inst->pssts_hdl, retval,
			      profile, pwe_id, pcn, tbl_id, NCS_PSSTS_FILE_PERM_READ, file_hdl)
	    if (retval != NCSCC_RC_SUCCESS) {
		retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
		goto cleanup;
	}

	/* 3.0.b addition */

	/* Filling the current persistent format version, profile, pwe_id, pcn and table_id in ps_file_record */
	ps_file_record.ps_format_version = PSS_PS_FORMAT_VERSION;
	memcpy(&ps_file_record.profile, &profile, NCS_PSS_MAX_PROFILE_NAME);
	ps_file_record.pwe_id = pwe_id;
	memcpy(&ps_file_record.pcn, &pcn, NCSMIB_PCN_LENGTH_MAX);
	ps_file_record.tbl_id = tbl_id;

	memset(&hdr, '\0', sizeof(PSS_TABLE_DETAILS_HEADER));
	/* Read the persistent file of the table for table details */
	retval = pss_tbl_details_header_read(inst, inst->pssts_hdl, file_hdl, &ps_file_record, &hdr);

	if (NCSCC_RC_SUCCESS != retval) {
		fprintf(fh, "\n\t  MIB-TBL:%d:%s:ERROR Reading Table details \n\n", tbl_id,
			(char *)tbl_info->ptbl_info->mib_tbl_name);
		goto cleanup;
	}
	fprintf(fh,
		"\t TABLE DETAILS:\tHEADER LEN:%dPERSISTENT STORE VERSION:%d\n\tTABLE VERSION:%d\tMAX ROW LEN:%d\tMAX KEY LEN:%d\tBITMAP LEN:%d",
		hdr.header_len, hdr.ps_format_version, hdr.table_version, hdr.max_row_length, hdr.max_key_length,
		hdr.bitmap_length);
	/* End of 3.0.b addition */

	/* Get the total file size from the persistent store.
	 * Read in the data from this file in chunks
	 * and issue SETALLROWS requests from it,
	 * by calling into the MAC's ncsmib_timed_request function.
	 */
	m_NCS_PSSTS_FILE_SIZE(inst->pssts_api, inst->pssts_hdl, retval, profile, pwe_id, pcn, tbl_id, file_size);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_FILE_SIZE_OP_FAIL, pwe_id, pcn, tbl_id);
		goto cleanup;
	}
	rem_file_size = file_size - hdr.header_len;
	read_offset = hdr.header_len;
	item_size = tbl_info->max_row_length;
	max_num_rows = NCS_PSS_MAX_CHUNK_SIZE / item_size;
	if (max_num_rows == 0)	/* Should we return an error here? */
		max_num_rows = 1;

	while (rem_file_size != 0) {
		if (rem_file_size < tbl_info->max_row_length)
			break;

		if (rem_file_size < buf_size)
			buf_size = rem_file_size;

		memset(in_buf, '\0', buf_size);
		m_NCS_PSSTS_FILE_READ(inst->pssts_api, inst->pssts_hdl, retval, file_hdl,
				      buf_size, read_offset, in_buf, bytes_read);
		if (retval != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_STORE(NCSFL_SEV_ERROR, PSS_MIB_READ_FAIL, pwe_id, pcn, tbl_id);
			retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
			goto cleanup;
		}

		rem_file_size -= bytes_read;
		read_offset += bytes_read;
		if (rem_file_size < tbl_info->max_row_length) {
			is_last_chunk = TRUE;
		}

		num_items = bytes_read / item_size;
		if (num_items == 0)
			break;

		for (i = 0, ptr = in_buf; i < num_items; i++, ptr += item_size) {
			fprintf(fh, "\t    ROW:START\n");
			/* lcl_params_max = pss_get_count_of_valid_params(curr_data, tbl_info); */
			for (j = 0; j < tbl_info->ptbl_info->num_objects; j++) {
				uns32 param_offset = tbl_info->pfields[j].offset;
				NCSMIB_PARAM_VAL pv;

				if ((tbl_info->pfields[j].var_info.is_index_id == FALSE) &&
				    (pss_get_bit_for_param(ptr, (j + 1)) == 0))
					continue;

				memset(&pv, '\0', sizeof(pv));
				switch (tbl_info->pfields[j].var_info.fmat_id) {
				case NCSMIB_FMAT_INT:
					pv.i_fmat_id = NCSMIB_FMAT_INT;
					pv.i_param_id = tbl_info->pfields[j].var_info.param_id;
					switch (tbl_info->pfields[j].var_info.len) {
						uns16 data16;
					case sizeof(uns8):
						pv.info.i_int = (uns32)(*((uns8 *)(ptr + param_offset)));
						break;
					case sizeof(uns16):
						memcpy(&data16, ptr + param_offset, sizeof(uns16));
						pv.info.i_int = (uns32)data16;
						break;
					case sizeof(uns32):
						memcpy(&pv.info.i_int, ptr + param_offset, sizeof(uns32));
						break;
					default:
						fprintf(fh, "\t    ROW:PARAM-ERROR - Invalid var_len for FMAT_ID\n");
						return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					}
					break;

				case NCSMIB_FMAT_BOOL:
					{
						NCS_BOOL data_bool;
						pv.i_fmat_id = NCSMIB_FMAT_INT;
						pv.i_param_id = tbl_info->pfields[j].var_info.param_id;
						memcpy(&data_bool, ptr + param_offset, sizeof(NCS_BOOL));
						pv.info.i_int = (uns32)data_bool;
					}
					break;

				case NCSMIB_FMAT_OCT:
					{
						PSS_VAR_INFO *var_info = &(tbl_info->pfields[j]);
						if (var_info->var_length == TRUE) {
							memcpy(&pv.i_length, ptr + param_offset, sizeof(uns16));
							param_offset += sizeof(uns16);
						} else
							pv.i_length = var_info->var_info.obj_spec.stream_spec.max_len;

						pv.i_fmat_id = NCSMIB_FMAT_OCT;
						pv.i_param_id = var_info->var_info.param_id;
						pv.info.i_oct = ptr + param_offset;
					}
					break;

				default:
					retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
					fprintf(fh, "\t    ROW:PARAM-ERROR - Invalid FMAT_ID\n");
					goto cleanup;
				}
				pss_dump_mib_var(fh, &pv, tbl_info, j);	/* Log the Variable info */
			}	/* for (j = 0; j < tbl_info->ptbl_info->num_objects; j++) */
			fprintf(fh, "\t    ROW:END\n");
		}		/* for(i = 0, ptr = in_buf; i < num_items; i++, ptr += item_size) */
	}			/* while(rem_file_size != 0) */

 cleanup:
	if (in_buf != NULL)
		m_MMGR_FREE_PSS_OCT(in_buf);

	if (file_hdl != 0)
		m_NCS_PSSTS_FILE_CLOSE(inst->pssts_api, inst->pssts_hdl, retval, file_hdl);

	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_process_display_mib_entries

  DESCRIPTION:       This function is a helper function to execute a CLI 
                     command.

  RETURNS:           void.

*****************************************************************************/
uns32 pss_process_display_profile_entries(PSS_CB *inst, NCSMIB_ARG *arg)
{
	NCS_UBAID lcl_uba, lcl_uba1;
	USRBUF *buff = arg->req.info.cli_req.i_usrbuf;
	uns8 *buff_ptr = NULL;
	uns16 str_len = 0, local_len = 0;
	int8 pcn[NCSMIB_PCN_LENGTH_MAX];
	char profile_name[128];
	uns32 retval = NCSCC_RC_SUCCESS;
	NCS_BOOL profile_exists = FALSE;

	memset(&pcn, '\0', sizeof(pcn));
	memset(&profile_name, '\0', sizeof(profile_name));
	memset(&lcl_uba, '\0', sizeof(lcl_uba));
	memset(&lcl_uba1, '\0', sizeof(lcl_uba1));

	arg->rsp.info.cli_rsp.o_partial = FALSE;
	arg->rsp.info.cli_rsp.o_answer = NULL;

	ncs_dec_init_space(&lcl_uba, buff);
	arg->req.info.cli_req.i_usrbuf = NULL;

	buff_ptr = ncs_dec_flatten_space(&lcl_uba, (uns8 *)&local_len, sizeof(uns16));
	if (buff_ptr == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MMGR_BUFFER_ALLOC_FAIL,
				  "pss_process_display_profile_entries()");
		return NCSCC_RC_FAILURE;
	}
	str_len = ncs_decode_16bit(&buff_ptr);
	ncs_dec_skip_space(&lcl_uba, sizeof(uns16));

	if (ncs_decode_n_octets_from_uba(&lcl_uba, (char *)&profile_name, str_len) != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MMGR_BUFFER_ALLOC_FAIL,
				  "pss_process_display_profile_entries()");
		return NCSCC_RC_FAILURE;
	}
	/* The character array "profile_name" is already populated, when this macro returns. */

	/* Open the profile and read the PCNs */
	m_NCS_PSSTS_PROFILE_EXISTS(inst->pssts_api, inst->pssts_hdl, retval, profile_name, profile_exists);
	if ((retval != NCSCC_RC_SUCCESS) || (profile_exists == FALSE)) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_INVALID_PROFILE_SPCFD);
		return NCSCC_RC_FAILURE;
	}

	m_NCS_PSSTS_GET_CLIENTS(inst->pssts_api, inst->pssts_hdl,
				retval, profile_name, &arg->rsp.info.cli_rsp.o_answer);
	if (retval != NCSCC_RC_SUCCESS) {
		m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_GET_PROF_CLIENTS_OP_FAILED);
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_process_reload_pssvlibconf

  DESCRIPTION:       This function is a helper function to execute a CLI
                     command.

  RETURNS:           void.

*****************************************************************************/
void pss_process_reload_pssvlibconf(PSS_CB *inst, NCSMIB_ARG *arg)
{
	arg->rsp.info.cli_rsp.o_partial = FALSE;
	arg->rsp.info.cli_rsp.o_answer = NULL;

	(void)pss_read_lib_conf_info(inst, (char *)&inst->lib_conf_file);

	return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_process_reload_pssvspcnlist

  DESCRIPTION:       This function is a helper function to execute a CLI
                     command.

  RETURNS:           void.

*****************************************************************************/
void pss_process_reload_pssvspcnlist(PSS_CB *inst, NCSMIB_ARG *arg)
{
	arg->rsp.info.cli_rsp.o_partial = FALSE;
	arg->rsp.info.cli_rsp.o_answer = NULL;

	(void)pss_read_create_spcn_config_file(inst);

	return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_add_entry_to_spcn_wbreq_pend_list

  DESCRIPTION:       This function adds an entry to the spcn_wbreq_pend_list
                     of PSS_PWE_CB.

  RETURNS:           NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 pss_add_entry_to_spcn_wbreq_pend_list(PSS_PWE_CB *pwe_cb, char *pcn)
{
	PSS_SPCN_WBREQ_PEND_LIST *list = NULL, *prv_list = NULL;

	for (list = pwe_cb->spcn_wbreq_pend_list; list != NULL; prv_list = list, list = list->next) {
		if (strcmp(list->pcn, pcn) == 0) {
			/* Entry already existing. */
			m_LOG_PSS_HDLN_STR2(NCSFL_SEV_DEBUG, PSS_HDLN_SPCN_PEND_WBREQ_LIST_NODE_ALREADY_PRESENT, pcn);
			return NCSCC_RC_SUCCESS;
		}
	}

	/* Checkpoint to Standby's */
	m_PSS_RE_BAM_PEND_WBREQ_INFO(pwe_cb, pcn, NCS_MBCSV_ACT_ADD);

	/* Add new entry now */
	if ((list = m_MMGR_ALLOC_PSS_SPCN_WBREQ_PEND_LIST) == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_MMGR_BUFFER_ALLOC_FAIL,
				  "pss_add_entry_to_spcn_wbreq_pend_list()");
		m_LOG_PSS_HDLN_STR2(NCSFL_SEV_ERROR, PSS_HDLN_ADD_SPCN_PEND_WBREQ_LIST_NODE_ALLOC_FAIL, pcn);
		return NCSCC_RC_FAILURE;
	}
	memset(list, '\0', sizeof(PSS_SPCN_WBREQ_PEND_LIST));

	if ((list->pcn = m_MMGR_ALLOC_MAB_PCN_STRING(strlen(pcn) + 1)) == NULL) {
		m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_PCN_STRING_ALLOC_FAIL,
				  "pss_add_entry_to_spcn_wbreq_pend_list()");
		m_LOG_PSS_HDLN_STR2(NCSFL_SEV_ERROR, PSS_HDLN_ADD_SPCN_PEND_WBREQ_LIST_NODE_PCN_ALLOC_FAIL, pcn);
		m_MMGR_FREE_PSS_SPCN_WBREQ_PEND_LIST(list);
		return NCSCC_RC_FAILURE;
	}
	memset(list->pcn, '\0', strlen(pcn) + 1);
	strcpy(list->pcn, pcn);

	if (prv_list == NULL) {
		/* This node is the first entry in the list */
		pwe_cb->spcn_wbreq_pend_list = list;
	} else {
		prv_list->next = list;
	}
	m_LOG_PSS_HDLN_STR2(NCSFL_SEV_DEBUG, PSS_HDLN_ADD_SPCN_PEND_WBREQ_LIST_SUCCESS, pcn);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_del_entry_frm_spcn_wbreq_pend_list

  DESCRIPTION:       This function deletes an entry from the 
                     spcn_wbreq_pend_list of PSS_PWE_CB.

  RETURNS:           void

*****************************************************************************/
void pss_del_entry_frm_spcn_wbreq_pend_list(PSS_PWE_CB *pwe_cb, char *pcn)
{
	PSS_SPCN_WBREQ_PEND_LIST *list = NULL, *prv_list = NULL;

	for (list = pwe_cb->spcn_wbreq_pend_list; list != NULL; prv_list = list, list = list->next) {
		if (strcmp(list->pcn, pcn) == 0) {
			m_LOG_PSS_HDLN_STR2(NCSFL_SEV_DEBUG, PSS_HDLN_DEL_SPCN_PEND_WBREQ_LIST_NODE, pcn);
			/* Entry found. To be deleted. */
			if (prv_list == NULL)
				pwe_cb->spcn_wbreq_pend_list = list->next;
			else
				prv_list->next = list->next;

			m_MMGR_FREE_MAB_PCN_STRING(list->pcn);
			m_MMGR_FREE_PSS_SPCN_WBREQ_PEND_LIST(list);
			return;
		}
	}

	return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_send_pending_wbreqs_to_bam

  DESCRIPTION:       This function sends all the pending warmboot-requests of
                     the specific PWE to the BAM instance.

  RETURNS:           NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 pss_send_pending_wbreqs_to_bam(PSS_PWE_CB *pwe_cb)
{
	PSS_SPCN_WBREQ_PEND_LIST *list = NULL, *tmp_node = NULL;

	list = pwe_cb->spcn_wbreq_pend_list;
	while (list != NULL) {
		tmp_node = list;
		(void)pss_send_wbreq_to_bam(pwe_cb, tmp_node->pcn);

		/* Checkpoint to Standby's */
		m_PSS_RE_BAM_PEND_WBREQ_INFO(pwe_cb, tmp_node->pcn, NCS_MBCSV_ACT_RMV);

		list = list->next;
		m_MMGR_FREE_MAB_PCN_STRING(tmp_node->pcn);
		m_MMGR_FREE_PSS_SPCN_WBREQ_PEND_LIST(tmp_node);
	}
	pwe_cb->spcn_wbreq_pend_list = NULL;

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_send_ack_for_msg_to_oaa

  DESCRIPTION:       This function sends Ack to OAA.

  RETURNS:           NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 pss_send_ack_for_msg_to_oaa(PSS_PWE_CB *pwe_cb, MAB_MSG *msg)
{
	MAB_MSG pmsg;
	uns32 code = NCSCC_RC_SUCCESS;

#if (NCS_PSS_RED == 1)
	if (pwe_cb->p_pss_cb->ha_state == SA_AMF_HA_STANDBY)
		return NCSCC_RC_SUCCESS;
#endif

	memset(&pmsg, '\0', sizeof(pmsg));
	pmsg.op = MAB_OAC_PSS_ACK;
	pmsg.yr_hdl = pwe_cb;
	pmsg.pwe_hdl = pwe_cb->mds_pwe_handle;
	pmsg.fr_card = pwe_cb->p_pss_cb->my_vcard;
#if (NCS_PSS_RED == 1)
	if (pwe_cb->processing_pending_active_events) {
		/* pmsg.data.seq_num = pwe_cb->curr_plbck_ssn_info.info.info2.seq_num; */
		return NCSCC_RC_SUCCESS;
	}
#endif
	{
		pmsg.data.seq_num = msg->data.seq_num;
	}

	code = mab_mds_snd(pwe_cb->mds_pwe_handle, &pmsg, NCSMDS_SVC_ID_PSS, NCSMDS_SVC_ID_OAC, msg->fr_card);
	if (code == NCSCC_RC_SUCCESS) {
		m_LOG_PSS_ACK_EVT(NCSFL_SEV_DEBUG, PSS_OAA_ACK_SEND_SUCCESS, msg->data.seq_num);
	} else {
		m_LOG_PSS_ACK_EVT(NCSFL_SEV_ERROR, PSS_OAA_ACK_SEND_FAIL, msg->data.seq_num);
	}

	return code;
}

#if (NCS_PSS_RED == 1)
/*****************************************************************************

  PROCEDURE NAME:    pss_updt_in_wbreq_into_cb

  DESCRIPTION:       This function updates Warmboot-request info into the
                     PSS_PWE_CB. This is required for redundancy operations.

  RETURNS:           NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 pss_updt_in_wbreq_into_cb(PSS_PWE_CB *pwe_cb, MAB_PSS_WARMBOOT_REQ *req)
{
	uns32 rc = NCSCC_RC_SUCCESS, len = 0, cnt = 0;
	MAB_PSS_WARMBOOT_REQ *o_req = NULL, *in_req = NULL, **o_pp_req = NULL;
	MAB_PSS_TBL_LIST *p_tbl = NULL, **pp_tbl = NULL;

	/* If not initialized, do it now. */
	memset(&pwe_cb->curr_plbck_ssn_info, '\0', sizeof(pwe_cb->curr_plbck_ssn_info));

	/* Start updating the values now. */
	pwe_cb->curr_plbck_ssn_info.plbck_ssn_in_progress = TRUE;
	pwe_cb->curr_plbck_ssn_info.is_warmboot_ssn = TRUE;

	pwe_cb->curr_plbck_ssn_info.info.info2.wb_req = *req;

	for (o_req = &pwe_cb->curr_plbck_ssn_info.info.info2.wb_req, o_pp_req = &o_req, in_req = req;
	     in_req != NULL; in_req = in_req->next, o_pp_req = &(*o_pp_req)->next, cnt++) {
		if (*o_pp_req == NULL) {
			if ((*o_pp_req = m_MMGR_ALLOC_MAB_PSS_WARMBOOT_REQ) == NULL)
				return NCSCC_RC_FAILURE;
			memset(*o_pp_req, '\0', sizeof(MAB_PSS_WARMBOOT_REQ));
		}
		o_req = (*o_pp_req);
		*o_req = *in_req;
		o_req->next = NULL;	/* safety measure */
		if (in_req->pcn_list.pcn != NULL) {
			len = strlen(in_req->pcn_list.pcn);
			o_req->pcn_list.pcn = m_MMGR_ALLOC_MAB_PCN_STRING(len + 1);
			if (o_req->pcn_list.pcn == NULL)
				return NCSCC_RC_FAILURE;

			memset(o_req->pcn_list.pcn, '\0', len + 1);
			strcpy(o_req->pcn_list.pcn, in_req->pcn_list.pcn);

			o_req->pcn_list.tbl_list = NULL;
			pp_tbl = &o_req->pcn_list.tbl_list;
			for (p_tbl = in_req->pcn_list.tbl_list; p_tbl != NULL;
			     p_tbl = p_tbl->next, pp_tbl = &(*pp_tbl)->next) {
				if ((*pp_tbl = m_MMGR_ALLOC_MAB_PSS_TBL_LIST) == NULL) {
					m_MMGR_FREE_MAB_PCN_STRING(o_req->pcn_list.pcn);
					pss_free_tbl_list(o_req->pcn_list.tbl_list);
					memset(o_req, '\0', sizeof(MAB_PSS_WARMBOOT_REQ));
					/* Free all the earlier ones. */
					return NCSCC_RC_FAILURE;
				}
				memset((*pp_tbl), '\0', sizeof(MAB_PSS_TBL_LIST));
				(*pp_tbl)->tbl_id = p_tbl->tbl_id;
			}
		}
	}
	pwe_cb->curr_plbck_ssn_info.info.info2.wbreq_cnt = cnt;

	return rc;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_indicate_end_of_playback_to_standby

  DESCRIPTION:       This function sends end-of-playback indication to the
                     Standby PSS instances.

  RETURNS:           NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 pss_indicate_end_of_playback_to_standby(PSS_PWE_CB *pwe_cb, NCS_BOOL is_error_condition)
{
	if (is_error_condition)
		pss_flush_plbck_ssn_info(&pwe_cb->curr_plbck_ssn_info);
	else if (pwe_cb->curr_plbck_ssn_info.is_warmboot_ssn) {
		pwe_cb->curr_plbck_ssn_info.info.info2.wbreq_cnt--;
		if (pwe_cb->curr_plbck_ssn_info.info.info2.wbreq_cnt != 0)
			pwe_cb->curr_plbck_ssn_info.info.info2.partial_delete_done = TRUE;
		pss_partial_flush_warmboot_plbck_ssn_info(&pwe_cb->curr_plbck_ssn_info);
	} else
		pss_flush_plbck_ssn_info(&pwe_cb->curr_plbck_ssn_info);

	m_PSS_RE_PLBCK_SSN_INFO(pwe_cb, &pwe_cb->curr_plbck_ssn_info);

	if ((!is_error_condition) && (pwe_cb->curr_plbck_ssn_info.is_warmboot_ssn) &&
	    (pwe_cb->curr_plbck_ssn_info.info.info2.wbreq_cnt != 0)) {
		pwe_cb->curr_plbck_ssn_info.info.info2.partial_delete_done = FALSE;
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_dup_re_wbreq_info

  DESCRIPTION:       This function duplicates the MAB_PSS_WARMBOOT_REQ structure.

  RETURNS:           NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 pss_dup_re_wbreq_info(MAB_PSS_WARMBOOT_REQ *src, MAB_PSS_WARMBOOT_REQ *dst)
{
	uns32 len = 0;
	MAB_PSS_TBL_LIST *p_tbl = NULL, **pp_tbl = NULL;

	if ((!src) || (!dst))
		return NCSCC_RC_FAILURE;

	memset(dst, '\0', sizeof(MAB_PSS_WARMBOOT_REQ));
	if (memcmp(src, dst, sizeof(MAB_PSS_WARMBOOT_REQ)) == 0)
		return NCSCC_RC_SUCCESS;

	*dst = *src;
	if (src->pcn_list.pcn != NULL) {
		len = strlen(src->pcn_list.pcn);
		dst->pcn_list.pcn = m_MMGR_ALLOC_MAB_PCN_STRING(len + 1);
		if (dst->pcn_list.pcn == NULL)
			return NCSCC_RC_FAILURE;

		memset(dst->pcn_list.pcn, '\0', len + 1);
		strcpy(dst->pcn_list.pcn, src->pcn_list.pcn);

		dst->pcn_list.tbl_list = NULL;
		pp_tbl = &dst->pcn_list.tbl_list;
		for (p_tbl = src->pcn_list.tbl_list; p_tbl != NULL; p_tbl = p_tbl->next, pp_tbl = &(*pp_tbl)->next) {
			if ((*pp_tbl = m_MMGR_ALLOC_MAB_PSS_TBL_LIST) == NULL) {
				m_MMGR_FREE_MAB_PCN_STRING(dst->pcn_list.pcn);
				pss_free_tbl_list(dst->pcn_list.tbl_list);
				memset(dst, '\0', sizeof(MAB_PSS_WARMBOOT_REQ));
				return NCSCC_RC_FAILURE;
			}
			memset((*pp_tbl), '\0', sizeof(MAB_PSS_TBL_LIST));
			(*pp_tbl)->tbl_id = p_tbl->tbl_id;
		}
	}

	/* We are not duplicate multiple MAB_PSS_WARMBOOT_REQ instances 
	   purposefully. */

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_free_re_wbreq_info

  DESCRIPTION:       This function frees the structure MAB_PSS_WARMBOOT_REQ

  RETURNS:           void

*****************************************************************************/
void pss_free_re_wbreq_info(MAB_PSS_WARMBOOT_REQ *wbreq)
{
	MAB_PSS_WARMBOOT_REQ *next = NULL;

	/* Introduced a preventive check here. */
	if (wbreq->pcn_list.pcn != NULL) {
		m_MMGR_FREE_MAB_PCN_STRING(wbreq->pcn_list.pcn);
	}
	pss_free_tbl_list(wbreq->pcn_list.tbl_list);
	wbreq = wbreq->next;

	while (wbreq != NULL) {
		next = wbreq->next;
		m_MMGR_FREE_MAB_PCN_STRING(wbreq->pcn_list.pcn);
		pss_free_tbl_list(wbreq->pcn_list.tbl_list);
		m_MMGR_FREE_MAB_PSS_WARMBOOT_REQ(wbreq);
		wbreq = next;
	}

	return;
}
#endif

/*****************************************************************************

  PROCEDURE NAME:    pss_destroy_spcn_wbreq_pend_list

  DESCRIPTION:       This function frees the linked list PSS_SPCN_WBREQ_PEND_LIST

  RETURNS:           void

*****************************************************************************/
void pss_destroy_spcn_wbreq_pend_list(PSS_SPCN_WBREQ_PEND_LIST *pend_list)
{
	PSS_SPCN_WBREQ_PEND_LIST *tmp = pend_list;

	while (tmp != NULL) {
		pend_list = tmp->next;
		m_MMGR_FREE_MAB_PCN_STRING(tmp->pcn);
		m_MMGR_FREE_PSS_SPCN_WBREQ_PEND_LIST(tmp);
		tmp = pend_list;
	}
	return;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_sync_mib_req

  DESCRIPTION:       This function sends all the MIB requests to MASv.
                     In case the return value is no-such-tbl, the request
                     is resend 2 times.

  RETURNS:           NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 pss_sync_mib_req(NCSMIB_ARG *marg, NCSMIB_REQ_FNC mib_fnc, uns32 sleep_dur, NCSMEM_AID *mma)
{
	uns32 lcl_ret = NCSCC_RC_SUCCESS, retry_cnt = 2, m = 0;

	lcl_ret = ncsmib_sync_request(marg, mib_fnc, sleep_dur, mma);

	switch (lcl_ret) {
	case NCSCC_RC_NO_SUCH_TBL:
		m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_RETURN_NO_SUCH_TBL);
		break;
	case NCSCC_RC_NO_MAS:
		m_LOG_PSS_HEADLINE2(NCSFL_SEV_CRITICAL, PSS_HDLN2_MIB_REQ_RETURN_NO_MAS);
		break;
	default:
		m_LOG_PSS_HDLN2_I(NCSFL_SEV_DEBUG, PSS_HDLN2_MIB_REQ_RETURN_VALUE, lcl_ret);
		break;
	}

	if ((lcl_ret == NCSCC_RC_NO_SUCH_TBL) || (lcl_ret == NCSCC_RC_NO_MAS)) {
		for (m = 0; m < retry_cnt; m++) {
			m_LOG_PSS_HDLN2_I(NCSFL_SEV_DEBUG, PSS_HDLN2_MIB_REQ_RETRY_COUNT, (m + 1));

			/* Maybe a high-resolution timer need be invoked here. */
			lcl_ret = ncsmib_sync_request(marg, mib_fnc, sleep_dur, mma);
			switch (lcl_ret) {
			case NCSCC_RC_NO_SUCH_TBL:
				m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_MIB_REQ_RETURN_NO_SUCH_TBL);
				break;
			case NCSCC_RC_NO_MAS:
				m_LOG_PSS_HEADLINE2(NCSFL_SEV_CRITICAL, PSS_HDLN2_MIB_REQ_RETURN_NO_MAS);
				break;
			default:
				m_LOG_PSS_HDLN2_I(NCSFL_SEV_DEBUG, PSS_HDLN2_MIB_REQ_RETURN_VALUE, lcl_ret);
				break;
			}

			if ((lcl_ret != NCSCC_RC_NO_SUCH_TBL) && (lcl_ret != NCSCC_RC_NO_MAS))
				break;
		}
	}

	return lcl_ret;
}

/*****************************************************************************

  PROCEDURE NAME:    pss_send_eop_status_to_oaa

  DESCRIPTION:       This function sends the status of the warmboot playback
                     to the OAA. The OAA is inturn expected to deliver this
                     status update to the user who requested for this playback.

  RETURNS:           void

*****************************************************************************/
void pss_send_eop_status_to_oaa(MAB_MSG *mm, uns32 status, uns32 mib_key, uns32 wbreq_hdl, NCS_BOOL send_all)
{
	MAB_MSG lcl_msg;
	PSS_PWE_CB *pwe_cb = NULL;
	MAB_PSS_WARMBOOT_REQ *wbreq = NULL;
	uns32 rc = NCSCC_RC_SUCCESS;

	if ((pwe_cb = (PSS_PWE_CB *)mm->yr_hdl) == NULL)
		return;
	if (pwe_cb->p_pss_cb == NULL)
		return;

	memset(&lcl_msg, '\0', sizeof(lcl_msg));
	lcl_msg.op = MAB_OAC_PSS_EOP_EVT;
	lcl_msg.yr_hdl = pwe_cb;
	lcl_msg.pwe_hdl = pwe_cb->mds_pwe_handle;
	lcl_msg.fr_card = pwe_cb->p_pss_cb->my_vcard;
	if (send_all == FALSE) {
		lcl_msg.data.data.oac_pss_eop_ind.mib_key = mib_key;
		lcl_msg.data.data.oac_pss_eop_ind.wbreq_hdl = wbreq_hdl;
		lcl_msg.data.data.oac_pss_eop_ind.status = status;
		rc = mab_mds_snd(pwe_cb->mds_pwe_handle, &lcl_msg, NCSMDS_SVC_ID_PSS, NCSMDS_SVC_ID_OAC, mm->fr_card);
		if (rc != NCSCC_RC_SUCCESS) {
			m_LOG_PSS_HDLN2_I(NCSFL_SEV_ERROR, PSS_HDLN2_WBREQ_EOP_STATUS_SEND_FAIL, status);
			return;
		}
		m_LOG_PSS_HDLN2_I(NCSFL_SEV_NOTICE, PSS_HDLN2_WBREQ_EOP_STATUS_SEND_SUCCESS, status);
	} else {
		NCS_BOOL lcl_continue = FALSE;

		for (wbreq = &mm->data.data.oac_pss_warmboot_req; wbreq != NULL; wbreq = wbreq->next) {
			if (wbreq->wbreq_hdl == wbreq_hdl) {
				lcl_continue = TRUE;
			}
			if (lcl_continue == TRUE) {
				lcl_msg.data.data.oac_pss_eop_ind.mib_key = wbreq->mib_key;
				lcl_msg.data.data.oac_pss_eop_ind.wbreq_hdl = wbreq->wbreq_hdl;
				lcl_msg.data.data.oac_pss_eop_ind.status = status;
				rc = mab_mds_snd(pwe_cb->mds_pwe_handle, &lcl_msg, NCSMDS_SVC_ID_PSS,
						 NCSMDS_SVC_ID_OAC, mm->fr_card);
				if (rc != NCSCC_RC_SUCCESS) {
					m_LOG_PSS_HDLN2_I(NCSFL_SEV_ERROR, PSS_HDLN2_WBREQ_EOP_STATUS_SEND_FAIL,
							  status);
				} else {
					m_LOG_PSS_HDLN2_I(NCSFL_SEV_NOTICE, PSS_HDLN2_WBREQ_EOP_STATUS_SEND_SUCCESS,
							  status);
				}
			}
		}
	}

	return;
}
#endif   /* (NCS_PSR == 1) */

#endif   /* (NCS_MAB == 1) */
