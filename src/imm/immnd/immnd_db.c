/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 * Copyright (C) 2017, Oracle and/or its affiliates. All rights reserved.
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
 * Author(s): Ericsson AB
 *
 */

/*****************************************************************************
  FILE NAME: immcn_evt.c

  DESCRIPTION: IMMND Routines to operate on client_info_db etc.
	       (Routines to add/delete nodes into database, routines to
		destroy the databases)

******************************************************************************/

#include "immnd.h"

/****************************************************************************
 * Name          : immnd_client_node_get
 *
 * Description   : Function to get the client node from client db Tree.
 *
 * Arguments     : IMMND_CB *cb, - IMMND Control Block
 *               : uint32_t imm_client_hdl - Client Handle.
 *
 * Return Values : IMMND_IMM_CLIENT_NODE** imm_client_node
 *
 * Notes         : None.
 *****************************************************************************/
void immnd_client_node_get(IMMND_CB *cb, SaImmHandleT imm_client_hdl,
			   IMMND_IMM_CLIENT_NODE **imm_client_node)
{
	*imm_client_node = (IMMND_IMM_CLIENT_NODE *)ncs_patricia_tree_get(
	    &cb->client_info_db, (uint8_t *)&imm_client_hdl);
	return;
}

/****************************************************************************
 * Name          : immnd_client_node_getnext
 *
 * Description   : Function to get next clinet node from client db Tree.
 *
 * Arguments     : IMMND_CB *cb, - IMMND Control Block
 *                 uint32_t imm_client_hdl - Client handle
 *
 * Return Values : IMMND_IMM_CLIENT_NODE** imm_client_node
 *
 * Notes         : None.
 *****************************************************************************/
void immnd_client_node_getnext(IMMND_CB *cb, SaImmHandleT imm_client_hdl,
			       IMMND_IMM_CLIENT_NODE **imm_client_node)
{
	if (imm_client_hdl)
		*imm_client_node =
		    (IMMND_IMM_CLIENT_NODE *)ncs_patricia_tree_getnext(
			&cb->client_info_db, (uint8_t *)&imm_client_hdl);
	else
		*imm_client_node =
		    (IMMND_IMM_CLIENT_NODE *)ncs_patricia_tree_getnext(
			&cb->client_info_db, (uint8_t *)NULL);
	return;
}

/****************************************************************************
 * Name          : immnd_client_node_add
 *
 * Description   : Function to Add the IMM Client node into client db Tree.
 *
 * Arguments     : IMMND_CB *cb, - IMMND Control Block
 *
 * Return Values :
 *
 * Notes         : None.
 *****************************************************************************/
uint32_t immnd_client_node_add(IMMND_CB *cb, IMMND_IMM_CLIENT_NODE *cl_node)
{
	cl_node->patnode.key_info = (uint8_t *)&cl_node->imm_app_hdl;

	uint32_t rc = ncs_patricia_tree_add(
	    &cb->client_info_db, (NCS_PATRICIA_NODE *)&cl_node->patnode);
	return rc;
}

/****************************************************************************
 * Name          : immnd_client_node_del
 *
 * Description   : Function to Delete the Client Node into Client Db Tree.
 *
 * Arguments     : IMMND_CB *cb, - IMMND Control Block.
 *               : IMMND_IMM_CLIENT_NODE* imm_client_node - IMM Client Node.
 *
 * Return Values : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS
 *
 * Notes         : None.
 *****************************************************************************/
uint32_t immnd_client_node_del(IMMND_CB *cb,
			       IMMND_IMM_CLIENT_NODE *imm_client_node)
{

	uint32_t rc = ncs_patricia_tree_del(
	    &cb->client_info_db,
	    (NCS_PATRICIA_NODE *)&imm_client_node->patnode);
	return rc;
}

/****************************************************************************
 * Name          :  immnd_client_node_tree_init
 *
 * Description   :  Function to Initialise client tree
 *
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
uint32_t immnd_client_node_tree_init(IMMND_CB *cb)
{
	NCS_PATRICIA_PARAMS param;
	memset(&param, 0, sizeof(NCS_PATRICIA_PARAMS));
	param.key_size = sizeof(SaImmHandleT);
	if (ncs_patricia_tree_init(&cb->client_info_db, &param) !=
	    NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : immnd_client_node_tree_cleanup
 * Description   : Function to cleanup client db info
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 * Return Values : NCSCC_RC_SUCCESS/Error.
 * Notes         : None.
 *****************************************************************************/
void immnd_client_node_tree_cleanup(IMMND_CB *cb)
{

	IMMND_IMM_CLIENT_NODE *cl_node;

	while ((cl_node = (IMMND_IMM_CLIENT_NODE *)ncs_patricia_tree_getnext(
		    &cb->client_info_db, (uint8_t *)0))) {
		ncs_patricia_tree_del(&cb->client_info_db,
				      (NCS_PATRICIA_NODE *)&cl_node->patnode);
		free(cl_node);
	}

	return;
}

/****************************************************************************
 * Name          : immnd_client_node_tree_destroy
 * Description   : Function to destroy client db tree
 * Arguments     : IMMND_CB *cb - IMMND CB pointer
 * Return Values : NCSCC_RC_SUCCESS/Error.
 * Notes         : None.
 *****************************************************************************/
void immnd_client_node_tree_destroy(IMMND_CB *cb)
{
	/* cleanup the client tree */
	immnd_client_node_tree_cleanup(cb);

	/* destroy the tree */
	ncs_patricia_tree_destroy(&cb->client_info_db);

	return;
}

/****************************************************************************
  Name          : immnd_clm_node_list_init
  Description   : This routine is used to initialize the IMMND clm node list
init Arguments     : cb - pointer to the IMMND Control Block Return Values :
NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE Notes         : None
*****************************************************************************/
uint32_t immnd_clm_node_list_init(IMMND_CB *cb)
{
	NCS_PATRICIA_PARAMS param;
	memset(&param, 0, sizeof(NCS_PATRICIA_PARAMS));

	param.key_size = sizeof(NODE_ID);
	if (ncs_patricia_tree_init(&cb->immnd_clm_list, &param) !=
	    NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : immnd_clm_node_get
 * Description   : Function to get the clm node from the clm list.
 * Arguments     : IMMND_CB *cb, - IMMND Control Block
 *               : NODE_ID  - CLM nodeid.
 * Return Values : IMMND_CLM_NODE_LIST ** immnd_clm_node_list
 * Notes         : None.
 *****************************************************************************/
void immnd_clm_node_get(IMMND_CB *cb, NODE_ID node,
			IMMND_CLM_NODE_LIST **imm_clm_node)
{
	*imm_clm_node = (IMMND_CLM_NODE_LIST *)ncs_patricia_tree_get(
	    &cb->immnd_clm_list, (uint8_t *)&node);
	return;
}

/****************************************************************************
  Name          : immnd_clm_node_add
  Description   : This routine adds the new node to immnd_clm_node_list
  Arguments     : immnd_tree - IMMND Tree.
		  NODE_ID -  CLM  Node.
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : The caller takes the cb lock before calling this function
*****************************************************************************/
uint32_t immnd_clm_node_add(IMMND_CB *cb, NODE_ID key)
{
	IMMND_CLM_NODE_LIST *immnd_clm_node =
	    calloc(1, sizeof(IMMND_CLM_NODE_LIST));
	immnd_clm_node->node_id = key;
	immnd_clm_node->patnode.key_info = (uint8_t *)&immnd_clm_node->node_id;

	if (ncs_patricia_tree_add(&cb->immnd_clm_list,
				  &immnd_clm_node->patnode) !=
	    NCSCC_RC_SUCCESS) {
		LOG_ER(
		    "IMMND - ncs_patricia_tree_add failed in immnd_clm_node_add");
		free(immnd_clm_node);
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : immnd_clm_node_delete
  Description   : This routine deletes the node from immnd_clm_node_list
  Arguments     : IMMD_CB *cb - IMMD Control Block.
		: NODE_ID -  CLM  Node.
  Return Values : None
*****************************************************************************/
uint32_t immnd_clm_node_delete(IMMND_CB *cb,
			       IMMND_CLM_NODE_LIST *immnd_clm_node)
{
	uint32_t rc = NCSCC_RC_SUCCESS;

	/* Remove the Node from the client tree */
	if (ncs_patricia_tree_del(
		&cb->immnd_clm_list,
		(NCS_PATRICIA_NODE *)&immnd_clm_node->patnode) !=
	    NCSCC_RC_SUCCESS) {
		LOG_WA("IMMND CLM NODE DELETE FROM PAT TREE FAILED");
		rc = NCSCC_RC_FAILURE;
	}

	/* Free the Client Node */
	if (immnd_clm_node) {
		free(immnd_clm_node);
	}
	return rc;
}

/****************************************************************************
  Name          : immnd_clm_node_cleanup
  Description   : This routine Free all the nodes in clm_node_list.
  Arguments     : IMMD_CB *cb - IMMD Control Block.
  Return Values : None
****************************************************************************/
void immnd_clm_node_cleanup(IMMND_CB *cb)
{
	IMMND_CLM_NODE_LIST *immnd_clm_node;
	NODE_ID key;
	memset(&key, 0, sizeof(NODE_ID));

	/* Get the First Node */
	immnd_clm_node = (IMMND_CLM_NODE_LIST *)ncs_patricia_tree_getnext(
	    &cb->immnd_clm_list, (uint8_t *)&key);
	while (immnd_clm_node) {
		key = immnd_clm_node->node_id;
		immnd_clm_node_delete(cb, immnd_clm_node);

		immnd_clm_node =
		    (IMMND_CLM_NODE_LIST *)ncs_patricia_tree_getnext(
			&cb->immnd_clm_list, (uint8_t *)&key);
	}

	return;
}

/****************************************************************************
  Name          : immnd_clm_node_destroy
  Description   : This routine destroys the IMMND clm node list.
  Arguments     : IMMD_CB *cb - IMMD Control Block.
  Return Values : None
*****************************************************************************/
void immnd_clm_node_destroy(IMMND_CB *cb)
{
	/* cleanup the clm list */
	immnd_clm_node_cleanup(cb);

	/* destroy the tree */
	ncs_patricia_tree_destroy(&cb->immnd_clm_list);

	return;
}

/* FEVS MESSAGE QUEUEING */

/*************************************************************************
 * Name            : immnd_enqueue_outgoing_fevs_msg
 *
 * Description     : Place a fevs message in backlog queue
 *
 *************************************************************************/
unsigned int immnd_enqueue_outgoing_fevs_msg(IMMND_CB *cb,
					     SaImmHandleT clnt_hdl,
					     IMMSV_OCTET_STRING *msg)
{
	IMMND_FEVS_MSG_NODE *new_node = malloc(sizeof(IMMND_FEVS_MSG_NODE));
	osafassert(new_node);
	new_node->msgNo = 0;
	new_node->clnt_hdl = clnt_hdl;
	new_node->reply_dest = 0LL;
	new_node->msg.size = msg->size;
	new_node->msg.buf = msg->buf;
	msg->buf = NULL; // Steal the message buffer instad of allocating again.
	msg->size = 0;
	new_node->next = NULL;

	if (cb->fevs_out_list == NULL) { /* First insert. */
		osafassert(cb->fevs_out_list_end == NULL);
		osafassert(cb->fevs_out_count == 0);
		cb->fevs_out_list = new_node;
		cb->fevs_out_count = 1;
	} else {
		cb->fevs_out_list_end->next = new_node; /* Insert at end. */
		++(cb->fevs_out_count);
	}

	cb->fevs_out_list_end = new_node;

	return cb->fevs_out_count;
}

/***************************************************************************
 * Name            : immnd_dequeue_outgoing_fevs_msg
 *
 * Description     : Removes a fevs_msg from backlog.
 *
 *************************************************************************/
unsigned int immnd_dequeue_outgoing_fevs_msg(IMMND_CB *cb,
					     IMMSV_OCTET_STRING *msg,
					     SaImmHandleT *clnt_hdl)
{
	osafassert(msg);
	osafassert(cb->fevs_out_list);
	osafassert(cb->fevs_out_list_end);
	osafassert(cb->fevs_out_count);
	IMMND_FEVS_MSG_NODE *tmp = cb->fevs_out_list;

	msg->buf = tmp->msg.buf;
	msg->size = tmp->msg.size;

	*clnt_hdl = tmp->clnt_hdl;
	tmp->msg.buf = NULL;
	tmp->msg.size = 0;

	/* remove the list lelement */
	cb->fevs_out_list = tmp->next;
	tmp->next = NULL;
	free(tmp);

	--(cb->fevs_out_count);

	if (cb->fevs_out_list == NULL) { /* Last remove */
		osafassert(cb->fevs_out_count == 0);
		cb->fevs_out_list_end = NULL;
	}

	return cb->fevs_out_count;
}
