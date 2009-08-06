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
 * Author(s): Ericsson AB
 *
 */

/*****************************************************************************

  DESCRIPTION:
  
  This file contains the IMMA processing routines (Routines to add, 
  delete nodes from IMM Agent CB repository, callback processing routines etc.)
*****************************************************************************/

#include "imma.h"
/****************************************************************************
  Name          : imma_client_tree_init
  Description   : This routine is used to initialize the client tree
  Arguments     : cb - pointer to the IMMA Control Block
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : None
******************************************************************************/
uns32 imma_client_tree_init(IMMA_CB *cb)
{
	NCS_PATRICIA_PARAMS param;
	memset(&param, 0, sizeof(NCS_PATRICIA_PARAMS));
	param.key_size = sizeof(SaImmHandleT);
	if (ncs_patricia_tree_init(&cb->client_tree, &param) != NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : imma_client_node_get
  Description   : This routine finds the client node.
  Arguments     : client_tree - Client Tree.
                  cl_hdl - Client Handle
  Return Values : cl_node - Client Node
                  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : The caller takes the cb lock before calling this function                 
******************************************************************************/
uns32 imma_client_node_get(NCS_PATRICIA_TREE *client_tree, SaImmHandleT *cl_hdl, IMMA_CLIENT_NODE **cl_node)
{
	*cl_node = (IMMA_CLIENT_NODE *)
	    ncs_patricia_tree_get(client_tree, (uns8 *)cl_hdl);

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : imma_client_node_add
  Description   : This routine adds the new client to the client tree.
  Arguments     :client_tree - Client Tree.
                 cl_node - Client Node
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : The caller takes the cb lock before calling this function                  
******************************************************************************/
uns32 imma_client_node_add(NCS_PATRICIA_TREE *client_tree, IMMA_CLIENT_NODE *cl_node)
{
	uns32 rc = NCSCC_RC_FAILURE;
	/* Store the client_info pointer as msghandle. */
	cl_node->patnode.key_info = (uns8 *)&cl_node->handle;

	if ((rc = ncs_patricia_tree_add(client_tree, &cl_node->patnode)) != NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : imma_client_node_delete
  Description   : This routine deletes the client from the client tree
  Arguments     : IMMA_CB *cb - IMMA Control Block.
                : IMMA_CLIENT_NODE *cl_node - Client Node.
  Return Values : None
  Notes         : None
******************************************************************************/
uns32 imma_client_node_delete(IMMA_CB *cb, IMMA_CLIENT_NODE *cl_node)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	if (cl_node == NULL)
		return NCSCC_RC_FAILURE;

	/* Remove the Node from the client tree */
	if (ncs_patricia_tree_del(&cb->client_tree, &cl_node->patnode) != NCSCC_RC_SUCCESS) {
		rc = NCSCC_RC_FAILURE;
	}

	/* Free the Client Node */
	if (cl_node)
		free(cl_node);

	return rc;
}

/****************************************************************************
  Name          : imma_client_tree_destroy
  Description   : This routine destroys the IMMA client tree.
  Arguments     : IMMA_CB *cb - IMMA Control Block.
  Return Values : None
  Notes         : None
******************************************************************************/
void imma_client_tree_destroy(IMMA_CB *cb)
{
	TRACE_ENTER();
	/* cleanup the client tree */
	imma_client_tree_cleanup(cb);

	/* destroy the tree */
	ncs_patricia_tree_destroy(&cb->client_tree);

	return;
}

/****************************************************************************
  Name          : imma_client_tree_cleanup
  Description   : This routine cleansup the IMMA Client tree
  Arguments     : IMMA_CB *cb - IMMA Control Block.
  Return Values : None
  Notes         : None
******************************************************************************/
void imma_client_tree_cleanup(IMMA_CB *cb)
{
	IMMA_CLIENT_NODE *clnode;
	SaImmHandleT *temp_ptr = 0;
	SaImmHandleT temp_hdl = 0;

	TRACE_ENTER();

	/* scan the entire handle db & delete each record */
	while ((clnode = (IMMA_CLIENT_NODE *)
		ncs_patricia_tree_getnext(&cb->client_tree, (uns8 *)temp_ptr))) {
		/* delete the client info */
		temp_hdl = clnode->handle;
		temp_ptr = &temp_hdl;

		/* Destroy the IPC attached to this client */
		imma_callback_ipc_destroy(clnode);

		/* Delete the Client Node */
		imma_client_node_delete(cb, clnode);
	}
	return;
}

/****************************************************************************
  Name          : imma_mark_clients_stale
  Description   : When we get IMMND DOWN over MDS, then mark all handles stale.
  Arguments     : IMMA_CB *cb - IMMA Control Block.
******************************************************************************/
void imma_mark_clients_stale(IMMA_CB *cb)
{
	/* We are locked already */
	IMMA_CLIENT_NODE *clnode;
	SaImmHandleT *temp_ptr = 0;
	SaImmHandleT temp_hdl = 0;

	TRACE_ENTER();

	/* scan the entire handle db & mark each record */
	while ((clnode = (IMMA_CLIENT_NODE *)
		ncs_patricia_tree_getnext(&cb->client_tree, (uns8 *)temp_ptr))) {
		temp_hdl = clnode->handle;
		temp_ptr = &temp_hdl;
		clnode->stale = TRUE;
		imma_proc_stale_dispatch(cb, clnode);
	}
	return;
}

/****************************************************************************
  Name          : imma_admin_owner_tree_init
  Description   : This routine is used to initialize the Admin Owner Tree.
  Arguments     : cb - pointer to the IMMA Control Block
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : None
******************************************************************************/
uns32 imma_admin_owner_tree_init(IMMA_CB *cb)
{
	NCS_PATRICIA_PARAMS param;
	memset(&param, 0, sizeof(NCS_PATRICIA_PARAMS));
	param.key_size = sizeof(SaImmAdminOwnerHandleT);
	if (ncs_patricia_tree_init(&cb->admin_owner_tree, &param) != NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : imma_admin_owner_node_get
  Description   : This routine finds the Admin Owner node.
  Arguments     : admin_owner_tree - Admin Owner Tree.
                  adm_hdl - Admin Owner Handle
  Return Values : adm_node - Admin Owner Node
                  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : The caller takes the cb lock before calling this function                 
******************************************************************************/
uns32 imma_admin_owner_node_get(NCS_PATRICIA_TREE *admin_owner_tree,
				SaImmAdminOwnerHandleT *adm_hdl, IMMA_ADMIN_OWNER_NODE **adm_node)
{
	*adm_node = (IMMA_ADMIN_OWNER_NODE *)
	    ncs_patricia_tree_get(admin_owner_tree, (uns8 *)adm_hdl);

	return NCSCC_RC_SUCCESS;
}

/******************************************************************************
  Name          : imma_admin_owner_node_getnext
  Description   : This routine will find the next Admin Owner Node
  Arguments     : admin_owner_tree - Admin Owner Tree.
                  adm_hdl - Admin Owner Handle
  Return Values :
 
  Notes
******************************************************************************/
void imma_admin_owner_node_getnext(IMMA_CB *cb, SaImmAdminOwnerHandleT *adm_hdl, IMMA_ADMIN_OWNER_NODE **adm_node)
{
	if (adm_hdl) {
		*adm_node = (IMMA_ADMIN_OWNER_NODE *)
		    ncs_patricia_tree_getnext(&cb->admin_owner_tree, (uns8 *)adm_hdl);
	} else {
		*adm_node = (IMMA_ADMIN_OWNER_NODE *)
		    ncs_patricia_tree_getnext(&cb->admin_owner_tree, (uns8 *)NULL);
	}

	return;
}

/****************************************************************************
  Name          : imma_admin_owner_node_add
  Description   : This routine adds the new node to admin_owner_tree.
  Arguments     : admin_owner_tree - Admin Owner Tree.
                  adm_node - Admin Owner Node.
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : The caller takes the cb lock before calling this function                  
******************************************************************************/
uns32 imma_admin_owner_node_add(NCS_PATRICIA_TREE *admin_owner_tree, IMMA_ADMIN_OWNER_NODE *adm_node)
{
	adm_node->patnode.key_info = (uns8 *)&adm_node->admin_owner_hdl;

	if (ncs_patricia_tree_add(admin_owner_tree, &adm_node->patnode) != NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : imma_admin_owner_node_delete
  Description   : This routine deletes the Admin Owner node from tree
  Arguments     : IMMA_CB *cb - IMMA Control Block.
                : IMMA_ADMIN_OWNERNODE *adm_node - Admin Owner Node.
  Notes         : None
******************************************************************************/
uns32 imma_admin_owner_node_delete(IMMA_CB *cb, IMMA_ADMIN_OWNER_NODE *adm_node)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	if (adm_node == NULL)
		return NCSCC_RC_FAILURE;

	/* Remove the Node from the tree */
	if (ncs_patricia_tree_del(&cb->admin_owner_tree, &adm_node->patnode) != NCSCC_RC_SUCCESS) {
		rc = NCSCC_RC_FAILURE;
	}

	/* Free the Node */
	if (adm_node) {
		free(adm_node);
	}

	return rc;
}

/****************************************************************************
  Name          : imma_admin_owner_tree_cleanup
  Description   : This routine destroys the nodes in the IMMA admin owner tree.
  Arguments     : IMMA_CB *cb - IMMA Control Block.
  Return Values : None
  Notes         : None
******************************************************************************/
void imma_admin_owner_tree_cleanup(IMMA_CB *cb)
{
	SaImmAdminOwnerHandleT prev_adm_owner_id = 0;
	IMMA_ADMIN_OWNER_NODE *adm_node;

	/* Get the First Node */
	adm_node = (IMMA_ADMIN_OWNER_NODE *)
	    ncs_patricia_tree_getnext(&cb->admin_owner_tree, (uns8 *)&prev_adm_owner_id);
	while (adm_node) {
		prev_adm_owner_id = adm_node->admin_owner_hdl;
		imma_admin_owner_node_delete(cb, adm_node);
		adm_node = (IMMA_ADMIN_OWNER_NODE *)
		    ncs_patricia_tree_getnext(&cb->admin_owner_tree, (uns8 *)&prev_adm_owner_id);
	}
	return;
}

/****************************************************************************
  Name          : imma_admin_owner_tree_destroy
  Description   : This routine destroys the IMMA admin owner tree.
  Arguments     : IMMA_CB *cb - IMMA Control Block.
  Return Values : None
  Notes         : None
******************************************************************************/
void imma_admin_owner_tree_destroy(IMMA_CB *cb)
{
	/* cleanup the admin owner tree */
	imma_admin_owner_tree_cleanup(cb);

	/* destroy the tree */
	ncs_patricia_tree_destroy(&cb->admin_owner_tree);

	return;
}

/****************************************************************************
  Name          : imma_ccb_tree_init
  Description   : This routine is used to initialize the Ccb Tree.
  Arguments     : cb - pointer to the IMMA Control Block
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : None
******************************************************************************/
uns32 imma_ccb_tree_init(IMMA_CB *cb)
{
	NCS_PATRICIA_PARAMS param;
	memset(&param, 0, sizeof(NCS_PATRICIA_PARAMS));
	param.key_size = sizeof(SaImmCcbHandleT);
	if (ncs_patricia_tree_init(&cb->ccb_tree, &param) != NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : imma_ccb_node_get
  Description   : This routine finds the Ccb node.
  Arguments     : ccb_tree - Ccb Tree.
                  adm_hdl - Ccb Handle
  Return Values : ccb_node - Ccb Node
                  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : The caller takes the cb lock before calling this function                 
******************************************************************************/
uns32 imma_ccb_node_get(NCS_PATRICIA_TREE *ccb_tree, SaImmCcbHandleT *ccb_hdl, IMMA_CCB_NODE **ccb_node)
{
	*ccb_node = (IMMA_CCB_NODE *)
	    ncs_patricia_tree_get(ccb_tree, (uns8 *)ccb_hdl);

	return NCSCC_RC_SUCCESS;
}

/******************************************************************************
  Name          : imma_ccb_node_getnext
  Description   : This routine will find the next Ccb Node
  Arguments     : ccb_tree - Ccb Tree.
                  ccb_hdl - Ccb Handle
  Return Values :
 
  Notes
******************************************************************************/
void imma_ccb_node_getnext(IMMA_CB *cb, SaImmCcbHandleT *ccb_hdl, IMMA_CCB_NODE **ccb_node)
{
	if (ccb_hdl) {
		*ccb_node = (IMMA_CCB_NODE *)
		    ncs_patricia_tree_getnext(&cb->ccb_tree, (uns8 *)ccb_hdl);
	} else {
		*ccb_node = (IMMA_CCB_NODE *)
		    ncs_patricia_tree_getnext(&cb->ccb_tree, (uns8 *)NULL);
	}

	return;
}

/****************************************************************************
  Name          : imma_ccb_node_add
  Description   : This routine adds the new node to ccb_tree.
  Arguments     : ccb_tree - Ccb Tree.
                  ccb_node - Ccb Node.
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : The caller takes the cb lock before calling this function
******************************************************************************/
uns32 imma_ccb_node_add(NCS_PATRICIA_TREE *ccb_tree, IMMA_CCB_NODE *ccb_node)
{
	ccb_node->patnode.key_info = (uns8 *)&ccb_node->ccb_hdl;

	if (ncs_patricia_tree_add(ccb_tree, &ccb_node->patnode) != NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : imma_ccb_node_delete
  Description   : This routine deletes the CCb node from tree
  Arguments     : IMMA_CB *cb - IMMA Control Block.
                : IMMA_CC_BNODE *ccb_node - Ccb Node.
  Notes         : None
******************************************************************************/
uns32 imma_ccb_node_delete(IMMA_CB *cb, IMMA_CCB_NODE *ccb_node)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	if (ccb_node == NULL)
		return NCSCC_RC_FAILURE;

	/* Remove the Node from the tree */
	if (ncs_patricia_tree_del(&cb->ccb_tree, &ccb_node->patnode) != NCSCC_RC_SUCCESS) {
		rc = NCSCC_RC_FAILURE;
	}

	/* Free the Node */
	if (ccb_node) {
		free(ccb_node);
	}

	return rc;
}

/****************************************************************************
  Name          : imma_ccb_tree_cleanup
  Description   : This routine destroys the nodes in the IMMA ccb tree.
  Arguments     : IMMA_CB *cb - IMMA Control Block.
  Return Values : None
  Notes         : None
******************************************************************************/
void imma_ccb_tree_cleanup(IMMA_CB *cb)
{
	SaImmCcbHandleT prev_ccb_id = 0;
	IMMA_CCB_NODE *ccb_node;

	/* Get the First Node */
	ccb_node = (IMMA_CCB_NODE *)
	    ncs_patricia_tree_getnext(&cb->ccb_tree, (uns8 *)&prev_ccb_id);
	while (ccb_node) {
		prev_ccb_id = ccb_node->ccb_hdl;
		imma_ccb_node_delete(cb, ccb_node);
		ccb_node = (IMMA_CCB_NODE *)
		    ncs_patricia_tree_getnext(&cb->ccb_tree, (uns8 *)&prev_ccb_id);
	}
	return;
}

/****************************************************************************
  Name          : imma_ccb_tree_destroy
  Description   : This routine destroys the IMMA ccb tree.
  Arguments     : IMMA_CB *cb - IMMA Control Block.
  Return Values : None
  Notes         : None
******************************************************************************/
void imma_ccb_tree_destroy(IMMA_CB *cb)
{
	/* cleanup the ccb tree */
	imma_ccb_tree_cleanup(cb);

	/* destroy the tree */
	ncs_patricia_tree_destroy(&cb->ccb_tree);

	return;
}

/****************************************************************************
  Name          : imma_search_tree_init
  Description   : This routine is used to initialize the Search Tree.
  Arguments     : cb - pointer to the IMMA Control Block
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : None
******************************************************************************/
uns32 imma_search_tree_init(IMMA_CB *cb)
{
	NCS_PATRICIA_PARAMS param;
	memset(&param, 0, sizeof(NCS_PATRICIA_PARAMS));
	param.key_size = sizeof(SaImmSearchHandleT);
	if (ncs_patricia_tree_init(&cb->search_tree, &param) != NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : imma_search_node_get
  Description   : This routine finds the Search node.
  Arguments     : search_tree - Search Tree.
                  search_hdl - Search Handle
  Return Values : search_node - Search Node
                  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : The caller takes the cb lock before calling this function 
******************************************************************************/
uns32 imma_search_node_get(NCS_PATRICIA_TREE *search_tree,
			   SaImmSearchHandleT *search_hdl, IMMA_SEARCH_NODE **search_node)
{
	*search_node = (IMMA_SEARCH_NODE *)
	    ncs_patricia_tree_get(search_tree, (uns8 *)search_hdl);

	return NCSCC_RC_SUCCESS;
}

/******************************************************************************
  Name          : imma_search_node_getnext
  Description   : This routine will find the next Search Node
  Arguments     : search_tree - Search Tree.
                  search_hdl - Search Handle
  Return Values :
 
  Notes
******************************************************************************/
void imma_search_node_getnext(IMMA_CB *cb, SaImmSearchHandleT *search_hdl, IMMA_SEARCH_NODE **search_node)
{
	if (search_hdl) {
		*search_node = (IMMA_SEARCH_NODE *)
		    ncs_patricia_tree_getnext(&cb->search_tree, (uns8 *)search_hdl);
	} else {
		*search_node = (IMMA_SEARCH_NODE *)
		    ncs_patricia_tree_getnext(&cb->search_tree, (uns8 *)NULL);
	}

	return;
}

/****************************************************************************
  Name          : imma_search_node_add
  Description   : This routine adds the new node to search_tree.
  Arguments     : search_tree - Search Tree.
                  search_node - Search Node.
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : The caller takes the cb lock before calling this function
******************************************************************************/
uns32 imma_search_node_add(NCS_PATRICIA_TREE *search_tree, IMMA_SEARCH_NODE *search_node)
{
	search_node->patnode.key_info = (uns8 *)&search_node->search_hdl;

	if (ncs_patricia_tree_add(search_tree, &search_node->patnode) != NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : imma_search_node_delete
  Description   : This routine deletes the Search node from tree
  Arguments     : IMMA_CB *cb - IMMA Control Block.
                : IMMA_SEARCH_NODE *search_node - Search Node.
  Notes         : None
******************************************************************************/
uns32 imma_search_node_delete(IMMA_CB *cb, IMMA_SEARCH_NODE *search_node)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	if (search_node == NULL)
		return NCSCC_RC_FAILURE;

	/* Remove the Node from the tree */
	if (ncs_patricia_tree_del(&cb->search_tree, &search_node->patnode) != NCSCC_RC_SUCCESS) {
		rc = NCSCC_RC_FAILURE;
	}

	/* Free the Node */
	if (search_node) {
		free(search_node);
	}

	return rc;
}

/****************************************************************************
  Name          : imma_search_tree_cleanup
  Description   : This routine destroys the nodes in the IMMA search tree.
  Arguments     : IMMA_CB *cb - IMMA Control Block.
  Return Values : None
  Notes         : None
******************************************************************************/
void imma_search_tree_cleanup(IMMA_CB *cb)
{
	SaImmSearchHandleT prev_search_id = 0;
	IMMA_SEARCH_NODE *search_node;

	/* Get the First Node */
	search_node = (IMMA_SEARCH_NODE *)
	    ncs_patricia_tree_getnext(&cb->search_tree, (uns8 *)&prev_search_id);

	while (search_node) {
		prev_search_id = search_node->search_hdl;
		imma_search_node_delete(cb, search_node);
		search_node = (IMMA_SEARCH_NODE *)
		    ncs_patricia_tree_getnext(&cb->search_tree, (uns8 *)&prev_search_id);
	}
	return;
}

/****************************************************************************
  Name          : imma_search_tree_destroy
  Description   : This routine destroys the IMMA search tree.
  Arguments     : IMMA_CB *cb - IMMA Control Block.
  Return Values : None
  Notes         : None
******************************************************************************/
void imma_search_tree_destroy(IMMA_CB *cb)
{
	/* cleanup the search tree */
	imma_search_tree_cleanup(cb);

	/* destroy the tree */
	ncs_patricia_tree_destroy(&cb->search_tree);

	return;
}

/****************************************************************************
  Name          : imma_db_init 
  Description   : This routine initializes the IMMA CB database.
  Arguments     : IMMA_CB *cb - IMMA Control Block.
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_error
  Notes         : None
******************************************************************************/
uns32 imma_db_init(IMMA_CB *cb)
{
	uns32 rc;

	rc = imma_client_tree_init(cb);
	if (rc != NCSCC_RC_SUCCESS) {
		TRACE_1("imma_client_tree_init failed");
		return rc;
	}

	rc = imma_admin_owner_tree_init(cb);
	if (rc != NCSCC_RC_SUCCESS) {
		TRACE_1("imma_admin_owner_tree_init failed");
		return rc;
	}

	rc = imma_ccb_tree_init(cb);
	if (rc != NCSCC_RC_SUCCESS) {
		TRACE_1("imma_ccb_tree_init failed");
		return rc;
	}

	rc = imma_search_tree_init(cb);
	if (rc != NCSCC_RC_SUCCESS) {
		TRACE_1("imma_search_tree_init failed");
		return rc;
	}

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : imma_db_destroy 
  Description   : This routine will destroy the IMMA CB database.
  Arguments     : IMMA_CB *cb - IMMA Control Block.
  Return Values : NCSCC_RC_SUCCESS
  Notes         : None
******************************************************************************/
uns32 imma_db_destroy(IMMA_CB *cb)
{
	TRACE_ENTER();

	imma_client_tree_destroy(cb);

	imma_admin_owner_tree_destroy(cb);

	imma_ccb_tree_destroy(cb);

	imma_search_tree_destroy(cb);

	return NCSCC_RC_SUCCESS;
}
