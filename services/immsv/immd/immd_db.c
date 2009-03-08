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
  
  This file contains the IMMD Database access Routines
*****************************************************************************/

#include "immd.h"

/****************************************************************************
  Name          : immd_immnd_info_tree_init
  Description   : This routine is used to initialize the IMMND info Tree
  Arguments     : cb - pointer to the IMMD Control Block
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : None
*****************************************************************************/
uns32 immd_immnd_info_tree_init (IMMD_CB  *cb)
{
    NCS_PATRICIA_PARAMS     param;
    memset(&param, 0, sizeof(NCS_PATRICIA_PARAMS));

    param.key_size = sizeof(NODE_ID);
    if (ncs_patricia_tree_init(&cb->immnd_tree, &param) != NCSCC_RC_SUCCESS)
    {
        return NCSCC_RC_FAILURE;
    }
    cb->is_immnd_tree_up = TRUE;
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : immd_immnd_info_node_get
  Description   : This routine finds the IMMND Info node.
  Arguments     : immnd_tree - IMMND Tree.
                  dest - MDS_DEST
  Return Values : immnd_info_node - IMMND Node
                  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
*****************************************************************************/
uns32 immd_immnd_info_node_get(NCS_PATRICIA_TREE *immnd_tree, 
                               MDS_DEST *dest,
                               IMMD_IMMND_INFO_NODE **immnd_info_node)
{
    NODE_ID  key;

    memset(&key, 0, sizeof(NODE_ID));
    /* Fill the Key */
    key = m_NCS_NODE_ID_FROM_MDS_DEST((*dest));

    *immnd_info_node = (IMMD_IMMND_INFO_NODE *)
                       ncs_patricia_tree_get(immnd_tree, (uns8*)&key);

    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : immd_immnd_info_node_getnext
  Description   : This routine finds the IMMND Info node.
  Arguments     : immnd_tree - IMMND Tree.
                  dest - MDS_DEST
  Return Values : immnd_info_node - IMMND Node
                  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : The caller takes the cb lock before calling this function
*****************************************************************************/
void immd_immnd_info_node_getnext(NCS_PATRICIA_TREE *immnd_tree, 
                                  MDS_DEST *dest,
                                  IMMD_IMMND_INFO_NODE **immnd_info_node)
{
    NODE_ID key;
    memset(&key, 0, sizeof(NODE_ID));
    /* Fill the Key */

    if (dest)
    {
        key = m_NCS_NODE_ID_FROM_MDS_DEST((*dest));

        *immnd_info_node = (IMMD_IMMND_INFO_NODE *)
                           ncs_patricia_tree_getnext(immnd_tree, (uns8*)&key);
    }
    else
        *immnd_info_node = (IMMD_IMMND_INFO_NODE *)
                           ncs_patricia_tree_getnext(immnd_tree,(uns8*)NULL);

    return;

}

/****************************************************************************
  Name          : immd_immnd_info_node_add
  Description   : This routine adds the new node to immnd_tree.
  Arguments     : immnd_tree - IMMND Tree.
                  immnd_node -  IMMND Node.
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  Notes         : The caller takes the cb lock before calling this function
*****************************************************************************/
uns32 immd_immnd_info_node_add(NCS_PATRICIA_TREE *immnd_tree,  
                               IMMD_IMMND_INFO_NODE *immnd_info_node)
{
    /* Store the client_info pointer as msghandle. */
    NODE_ID key;

    key = m_NCS_NODE_ID_FROM_MDS_DEST(immnd_info_node->immnd_dest);

    immnd_info_node->patnode.key_info = (uns8*)&key; 

    if (ncs_patricia_tree_add (immnd_tree, &immnd_info_node->patnode) != 
        NCSCC_RC_SUCCESS)
    {
        LOG_ER("IMMD - IMMND Info Node Add Failed");
        return NCSCC_RC_FAILURE;
    }

    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : immd_immnd_info_node_find_add
  Description   : This routine adds the new node to immnd_tree.
  Arguments     : immnd_tree - IMMND Tree.
                  immnd_info_node -  IMMND Node.
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
*****************************************************************************/
uns32 immd_immnd_info_node_find_add(NCS_PATRICIA_TREE *immnd_tree,  
                                    MDS_DEST *dest,
                                    IMMD_IMMND_INFO_NODE **immnd_info_node,  
                                    NCS_BOOL *add_flag)
{
    NODE_ID  key;

    memset(&key, 0, sizeof(NODE_ID)); 
    /* Fill the Key */
    key = m_NCS_NODE_ID_FROM_MDS_DEST(( *dest));  

    *immnd_info_node = (IMMD_IMMND_INFO_NODE *)
                       ncs_patricia_tree_get(immnd_tree, (uns8*)&key);
    if ((*immnd_info_node == NULL) && (*add_flag == TRUE))
    {
        *immnd_info_node = calloc(1, sizeof(IMMD_IMMND_INFO_NODE));
        if (*immnd_info_node == NULL)
        {
            LOG_ER("IMMD - Immnd_Dest_Info_Alloc_Failed");
            return NCSCC_RC_FAILURE;
        }


        (*immnd_info_node)->immnd_dest = *dest;
        (*immnd_info_node)->immnd_key  = m_NCS_NODE_ID_FROM_MDS_DEST(( *dest));
        (*immnd_info_node)->patnode.key_info = 
            (uns8*)&((*immnd_info_node)->immnd_key); 

        if (ncs_patricia_tree_add (immnd_tree, &(*immnd_info_node)->patnode) != 
            NCSCC_RC_SUCCESS)
        {
            LOG_ER("immd_immnd_info_node_find_add FAILED");  
            free(*immnd_info_node);
            *immnd_info_node = NULL;
            return NCSCC_RC_FAILURE;
        }
        *add_flag = FALSE;
    }

    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : immd_immnd_info_node_delete
  Description   : This routine deletes the immnd_info node from tree
  Arguments     : IMMD_CB *cb - IMMD Control Block.
                : IMMD_IMMND_INFO_NODE *immnd_info - IMMND Info Node.
  Return Values : None
*****************************************************************************/
uns32 immd_immnd_info_node_delete(IMMD_CB *cb, 
                                  IMMD_IMMND_INFO_NODE *immnd_info_node)
{
    uns32 rc = NCSCC_RC_SUCCESS;

    /* Remove the Node from the client tree */   
    if (ncs_patricia_tree_del(&cb->immnd_tree, &immnd_info_node->patnode)!=
        NCSCC_RC_SUCCESS)
    {
        LOG_ER("IMMD - IMMND INFO NODE DELETE FROM PAT TREE FAILED");
        rc =   NCSCC_RC_FAILURE;
    }

    /* Free the Client Node */
    if (immnd_info_node)
    {
        free(immnd_info_node);
    }

    return rc;
}

/****************************************************************************
  Name          : immd_immnd_info_tree_cleanup
  Description   : This routine Free all the nodes in immnd_tree.
  Arguments     : IMMD_CB *cb - IMMD Control Block.
  Return Values : None
****************************************************************************/
void immd_immnd_info_tree_cleanup(IMMD_CB *cb)
{
    IMMD_IMMND_INFO_NODE *immnd_info_node;
    NODE_ID  key;

    memset(&key, 0, sizeof(NODE_ID));

    /* Get the First Node */
    immnd_info_node = (IMMD_IMMND_INFO_NODE *)
                      ncs_patricia_tree_getnext(&cb->immnd_tree, (uns8*)&key);
    while (immnd_info_node)
    {
        key = m_NCS_NODE_ID_FROM_MDS_DEST(immnd_info_node->immnd_dest);

        immd_immnd_info_node_delete(cb, immnd_info_node);

        immnd_info_node = (IMMD_IMMND_INFO_NODE *)
            ncs_patricia_tree_getnext(&cb->immnd_tree, (uns8*)&key);
    }

    return;
}


/****************************************************************************
  Name          : immd_immnd_info_tree_destroy
  Description   : This routine destroys the IMMD lcl ckpt tree.
  Arguments     : IMMD_CB *cb - IMMD Control Block.
  Return Values : None
*****************************************************************************/
void immd_immnd_info_tree_destroy(IMMD_CB *cb)
{
    if (!cb->is_immnd_tree_up)
        return;

    /* cleanup the client tree */
    immd_immnd_info_tree_cleanup(cb);

    /* destroy the tree */
    ncs_patricia_tree_destroy(&cb->immnd_tree);

    return;
}

/****************************************************************************
 * Name          : immd_cb_db_init
 *
 * Description   : This is the function which initializes all the data 
 *                 structures and locks used belongs to IMMD.
 *
 * Arguments     : cb  - IMMD control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
*****************************************************************************/
uns32 immd_cb_db_init(IMMD_CB *cb)
{
    uns32 rc = NCSCC_RC_SUCCESS;

    rc=immd_immnd_info_tree_init (cb);
    if (rc != NCSCC_RC_SUCCESS)
    {
        LOG_ER("IMMD - IMMND INFO TREE INIT FAILED");
        return rc;
    }

    return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : immd_cb_db_destroy
 *
 * Description   : Destoroy the databases in CB
 *
 * Arguments     : cb  - Ifsv control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ****************************************************************************/
uns32 immd_cb_db_destroy (IMMD_CB *cb)
{
    immd_immnd_info_tree_destroy(cb);
    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 Name    :  immd_get_phy_slot_id

 Description :  To get the physical slot id from the node id

 Arguments   :
*****************************************************************************/
NCS_PHY_SLOT_ID  immd_get_phy_slot_id(MDS_DEST dest)
{
    NCS_PHY_SLOT_ID phy_slot; 

    m_NCS_GET_PHYINFO_FROM_NODE_ID(m_NCS_NODE_ID_FROM_MDS_DEST(dest),NULL,
                                   &phy_slot,NULL);
    return phy_slot;
}

