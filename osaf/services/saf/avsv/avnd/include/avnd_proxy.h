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
/****************************************************************************

  DESCRIPTION:

  This module is the include file for internode/ext proxy support.
  
****************************************************************************/
#ifndef AVND_PROXY_H
#define AVND_PROXY_H

/*************************************************************************
*                      Node Id to Mds Destination Mapping
**************************************************************************/

/* macro to get a component record from comp-db */
#define m_AVND_INT_EXT_COMPDB_REC_GET(compdb, name) \
   (AVND_COMP *)ncs_patricia_tree_get(&(compdb), (uint8_t *)&(name))

/* macro to get the next component record from comp-db */
#define m_AVND_INT_EXT_COMPDB_REC_GET_NEXT(compdb, name) \
   (AVND_COMP *)ncs_patricia_tree_getnext(&(compdb), (uint8_t *)&(name))

typedef struct avnd_node_id_to_mds_dest_map_tag {
	NCS_PATRICIA_NODE tree_node;
	NODE_ID node_id;
	MDS_DEST mds_dest;
} AVND_NODEID_TO_MDSDEST_MAP;

uns32 avnd_nodeid_mdsdest_rec_add(AVND_CB *cb, MDS_DEST mds_dest);
uns32 avnd_nodeid_mdsdest_rec_del(AVND_CB *cb, MDS_DEST mds_dest);
uns32 avnd_nodeid_to_mdsdest_map_db_init(AVND_CB *cb);
uns32 avnd_internode_avail_comp_db_init(AVND_CB *cb);
uns32 avnd_nodeid_to_mdsdest_map_db_destroy(AVND_CB *cb);
uns32 avnd_internode_avail_comp_db_destroy(AVND_CB *cb);
uns32 avnd_int_ext_comp_hdlr(AVND_CB *cb, AVSV_AMF_API_INFO *api_info,
			     MDS_SYNC_SND_CTXT *ctxt, SaAisErrorT *o_amf_rc, NCS_BOOL *int_ext_comp);
uns32 avnd_avnd_msg_send(AVND_CB *cb, uint8_t *msg_info, AVSV_AMF_API_TYPE type, MDS_SYNC_SND_CTXT *ctxt, NODE_ID node_id);
uns32 avnd_avnd_cbk_del_send(AVND_CB *cb, SaNameT *comp_name, uns32 *opq_hdl, NODE_ID *node_id);
MDS_DEST avnd_get_mds_dest_from_nodeid(AVND_CB *cb, NODE_ID node_id);
uns32 avnd_evt_ava_comp_val_req(AVND_CB *cb, AVND_EVT *evt);
AVND_COMP *avnd_internode_comp_add(NCS_PATRICIA_TREE *ptree, SaNameT *name, NODE_ID node_id, uns32 *rc, NCS_BOOL,
				   NCS_BOOL);
uns32 avnd_internode_comp_del(AVND_CB *cb, NCS_PATRICIA_TREE *ptree, SaNameT *name_net);
uns32 avnd_evt_mds_avnd_up_evh(AVND_CB *cb, AVND_EVT *evt);
uns32 avnd_evt_mds_avnd_dn_evh(AVND_CB *cb, AVND_EVT *evt);

#endif
