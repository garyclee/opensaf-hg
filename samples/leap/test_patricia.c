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
 */

#include "ncsgl_defs.h"
#include "ncs_osprm.h"

#include "ncssysf_mem.h"
#include "ncssysf_def.h"
#include "ncspatricia.h"
#include "leaptest.h"

/* Control block which houses the patricia tree */
typedef struct lt_test_pat_cb_tag
{
    NCS_PATRICIA_TREE   tree;
    NCS_BOOL            is_inited;
}LT_TEST_PAT_CB;

/* Key of the patricia tree */
typedef struct lt_elem_key_tag
{
    uns32       index;      /* Key is 32-bit */
}LT_ELEM_KEY;

/* Node element of the patricia tree */
typedef struct lt_pat_elem_tag
{
    NCS_PATRICIA_NODE   node;
    LT_ELEM_KEY         key;    /* Index of this particular entry goes here */

    uns32               data;   /* Node specific data goes here */
}LT_PAT_ELEM;

/* Malloc/free defines go here */
#define m_MMGR_ALLOC_LT_PAT_ELEM m_NCS_MEM_ALLOC(sizeof(LT_PAT_ELEM), \
                                               NCS_MEM_REGION_PERSISTENT,\
                                               UD_SERVICE_ID_STUB1,     \
                                               0)

#define m_MMGR_FREE_LT_PAT_ELEM(p) m_NCS_MEM_FREE(p, \
                                               NCS_MEM_REGION_PERSISTENT, \
                                               UD_SERVICE_ID_STUB1,     \
                                               0)


static uns32    lt_test_patricia(void);
static uns32    lt_add_node_entry(NCS_PATRICIA_TREE *pTree, LT_PAT_ELEM *elem, 
                  LT_ELEM_KEY *key);
static void     lt_destroy_pat_tree_nodes(LT_TEST_PAT_CB *cb);
static void     lt_pat_node_del(LT_TEST_PAT_CB *cb, LT_PAT_ELEM *elem);

LT_TEST_PAT_CB  gl_pat_cb;

int
lt_patricia_ops(int argc, char **argv)
{
    lt_test_patricia( );
    return NCSCC_RC_SUCCESS;
}

static uns32 lt_test_patricia(void)
{
    NCS_PATRICIA_PARAMS             params;
    NCS_PATRICIA_NODE               *pNode = NULL;
    LT_PAT_ELEM                     *elem = NULL;
    LT_ELEM_KEY                     key;

    /* Perform tree-init. */
    m_NCS_MEMSET(&params, '\0', sizeof(params));
    params.key_size = sizeof(LT_ELEM_KEY);
    if(ncs_patricia_tree_init(&gl_pat_cb.tree, &params) != NCSCC_RC_SUCCESS)
    {
        m_NCS_CONS_PRINTF("Patricia tree init failed...\n");
        return NCSCC_RC_FAILURE;
    }
    gl_pat_cb.is_inited = TRUE;
    m_NCS_CONS_PRINTF("Patricia tree init success...\n");

    m_NCS_CONS_PRINTF("Pls note that Index(32-bit key) to this tree is always in network-order...\n\n");

    /* Add first entry, with index value "0x00000001" */
    if((elem = m_MMGR_ALLOC_LT_PAT_ELEM) == NULL)
    {
        m_NCS_CONS_PRINTF("Malloc call failed...\n");
        ncs_patricia_tree_destroy(&gl_pat_cb.tree);
        return NCSCC_RC_FAILURE;
    }
    /* Populate "data in the entry", and "key" */
    m_NCS_MEMSET(elem, '\0', sizeof(LT_PAT_ELEM));
    m_NCS_MEMSET(&key, '\0', sizeof(LT_ELEM_KEY));
    key.index = m_NCS_OS_HTONL(0x00000001); /* Network order */
    elem->data = 0x0a0b0c0d;
    if(lt_add_node_entry(&gl_pat_cb.tree, elem, &key) != NCSCC_RC_SUCCESS)
    {
        m_NCS_CONS_PRINTF("Addition of node with index=0x00000001 to patricia tree failed...\n\n");
        m_MMGR_FREE_LT_PAT_ELEM(elem);
        ncs_patricia_tree_destroy(&gl_pat_cb.tree);
        return NCSCC_RC_FAILURE;
    }
    m_NCS_CONS_PRINTF("Addition of node with index=0x00000001 to patricia tree success...\n\n");


    /* Verify whether the node 0x00000001 can be retrieved */
    m_NCS_MEMSET(&key, 0, sizeof(key));
    key.index = m_NCS_OS_HTONL(0x00000001); /* Network order */
    pNode = ncs_patricia_tree_get(&gl_pat_cb.tree, (const uns8*)&key);
    if(pNode == NULL)
    {
        m_NCS_CONS_PRINTF("Get operation on node with index=0x00000001 failed...\n\n");
        lt_destroy_pat_tree_nodes(&gl_pat_cb);
        ncs_patricia_tree_destroy(&gl_pat_cb.tree);
        return NCSCC_RC_FAILURE;
    }
    m_NCS_CONS_PRINTF("Get operation on the node with index=0x00000001 success...\n");
    m_NCS_CONS_PRINTF("\tReturned node details : Index - %x, Data - %x...\n\n", 
    m_NCS_OS_NTOHL(((LT_PAT_ELEM*)pNode)->key.index),
    ((LT_PAT_ELEM*)pNode)->data);

    /* Add second entry, with index value "0x00000002" */
    if((elem = m_MMGR_ALLOC_LT_PAT_ELEM) == NULL)
    {
        m_NCS_CONS_PRINTF("Malloc call failed for index=0x00000002...\n");
        lt_destroy_pat_tree_nodes(&gl_pat_cb);
        ncs_patricia_tree_destroy(&gl_pat_cb.tree);
        return NCSCC_RC_FAILURE;
    }
    /* Populate "data in the entry", and "key" */
    m_NCS_MEMSET(elem, '\0', sizeof(LT_PAT_ELEM));
    m_NCS_MEMSET(&key, '\0', sizeof(LT_ELEM_KEY));
    key.index = m_NCS_OS_HTONL(0x00000002); /* Network order */
    elem->data = 0x0f0a0b0c;
    if(lt_add_node_entry(&gl_pat_cb.tree, elem, &key) != NCSCC_RC_SUCCESS)
    {
        m_NCS_CONS_PRINTF("Addition of node with index=0x00000002 to patricia tree failed...\n\n");
        m_MMGR_FREE_LT_PAT_ELEM(elem);
        lt_destroy_pat_tree_nodes(&gl_pat_cb);
        ncs_patricia_tree_destroy(&gl_pat_cb.tree);
        m_NCS_CONS_PRINTF("Destroyed the patricia tree...\n");
        return NCSCC_RC_FAILURE;
    }
    m_NCS_CONS_PRINTF("Addition of node with index=0x00000002 to patricia tree success...\n");

    /* Verify whether the node 0x00000002 can be retrieved */
    m_NCS_MEMSET(&key, 0, sizeof(key));
    key.index = m_NCS_OS_HTONL(0x00000002); /* Network order */
    pNode = ncs_patricia_tree_get(&gl_pat_cb.tree, (const uns8*)&key);
    if(pNode == NULL)
    {
        m_NCS_CONS_PRINTF("Get operation on node with index=0x00000002 failed...\n");
        lt_destroy_pat_tree_nodes(&gl_pat_cb);
        ncs_patricia_tree_destroy(&gl_pat_cb.tree);
        return NCSCC_RC_FAILURE;
    }
    m_NCS_CONS_PRINTF("Get operation on the node with index=0x00000002 success...\n");
    m_NCS_CONS_PRINTF("\tReturned node details : Index - %x, Data - %x...\n\n", 
    m_NCS_OS_NTOHL(((LT_PAT_ELEM*)pNode)->key.index),
    ((LT_PAT_ELEM*)pNode)->data);


    /* Do getnext on node 0x00000001. Verify whether node 0x00000002 is returned. */
    m_NCS_MEMSET(&key, 0, sizeof(key));
    key.index = m_NCS_OS_HTONL(0x00000001); /* Network order */
    pNode = ncs_patricia_tree_getnext(&gl_pat_cb.tree, (const uns8*)&key);
    if(pNode == NULL)
    {
        m_NCS_CONS_PRINTF("Get operation on node with index=0x00000001 failed...\n");
        lt_destroy_pat_tree_nodes(&gl_pat_cb);
        ncs_patricia_tree_destroy(&gl_pat_cb.tree);
        return NCSCC_RC_FAILURE;
    }
    if(((LT_PAT_ELEM*)pNode)->key.index != m_NCS_OS_HTONL(0x00000002))
    {
        m_NCS_CONS_PRINTF("Get operation on node with index=0x00000001 didn't return 0x00000002...\n");
        lt_destroy_pat_tree_nodes(&gl_pat_cb);
        ncs_patricia_tree_destroy(&gl_pat_cb.tree);
        return NCSCC_RC_FAILURE;
    }
    m_NCS_CONS_PRINTF("Getnext operation on index=0x00000001 returned node with index=0x00000002 ...\n");
    m_NCS_CONS_PRINTF("\tReturned node details : Index - %x, Data - %x...\n\n", 
    m_NCS_OS_NTOHL(((LT_PAT_ELEM*)pNode)->key.index),
    ((LT_PAT_ELEM*)pNode)->data);


    /* Cleaning up the tree now */
    m_NCS_CONS_PRINTF("Destroying Patricia tree and its nodes now...\n");
    lt_destroy_pat_tree_nodes(&gl_pat_cb);
    ncs_patricia_tree_destroy(&gl_pat_cb.tree);
    gl_pat_cb.is_inited = FALSE;
    m_NCS_CONS_PRINTF("Exiting from the test now...\n");

    return NCSCC_RC_SUCCESS;
}

static void lt_destroy_pat_tree_nodes(LT_TEST_PAT_CB *cb)
{
    NCS_PATRICIA_NODE   *pnode = NULL;
    LT_ELEM_KEY         *key = NULL, lclkey;
    
    if(cb->is_inited)
    {
    m_NCS_OS_MEMSET(&lclkey, '\0', sizeof(LT_ELEM_KEY));
    key = &lclkey;
        for (;;)
        {
        /* Here, we also go a getnext for zeroed-index */
            if ((pnode = ncs_patricia_tree_getnext(&cb->tree, 
                (const uns8 *)key)) != 0)
            {
                /* Detach and free the nh-node */
                lt_pat_node_del(cb, (LT_PAT_ELEM *)pnode);
            }
            else
            {
                break;
            }
        }
    }
    return;
}

static void lt_pat_node_del(LT_TEST_PAT_CB *cb, LT_PAT_ELEM *elem)
{
    /* Detach the node from tree */
    ncs_patricia_tree_del(&cb->tree, (NCS_PATRICIA_NODE *)&elem->node);

    m_NCS_CONS_PRINTF("Node with Index:%x and Data:%x is deleted...\n", 
    m_NCS_OS_NTOHL(elem->key.index), elem->data);

    /* If node has any alloc'ed memory, free it. */
    m_MMGR_FREE_LT_PAT_ELEM(elem);

    return;
}

static uns32 lt_add_node_entry(NCS_PATRICIA_TREE *pTree, LT_PAT_ELEM *elem, LT_ELEM_KEY *key)
{
    if((elem == NULL) || (key == NULL))
    return NCSCC_RC_FAILURE;

    elem->key = *key;   /* Copy the key here */
    elem->node.key_info = (uns8*) (&elem->key); /* This is mandatory */

    return ncs_patricia_tree_add(pTree, (NCS_PATRICIA_NODE*)elem);
}
