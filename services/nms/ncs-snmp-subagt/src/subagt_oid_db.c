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

  
  .....................................................................  
  
  DESCRIPTION: This file has the OID Database mgmt routines as well. 
  ***************************************************************************/ 
#include "subagt.h"
#include "subagt_oid_db.h"


/******************************************************************************
 *  Name:          snmpsubagt_table_oid_add
 *
 *  Description:   Routine to add the table-id and base-oid mapping 
 * 
 *  Arguments:     NCSMIB_TBL_ID   table-id - table id to be added in the tree
 *                 oid             *oid_base - Base OID to be added 
 *                 uns32           oid_len   - Base OID length
 *                 struct variable        *object_details - Properties of the 
 *                                                   objects in that table.
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
uns32 
snmpsubagt_table_oid_add(uns32              table_id, 
                        oid                 *oid_base, 
                        uns32               oid_len, 
                        struct variable     *object_details, 
                        uns32               number_of_objects)
{
    NCSSA_OID_DATABASE_NODE *db_node = NULL; 
    NCS_PATRICIA_TREE       *oid_db = NULL; 
    NCSSA_CB               *cb = NULL;  
    uns32                   status = NCSCC_RC_SUCCESS; 

    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_TABLE_OID_ADD); 

    if (oid_base == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_OID_BASE_NULL); 
        return NCSCC_RC_FAILURE; 
    }

    if  (oid_len == 0)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_OID_LEN_NULL); 
        return NCSCC_RC_FAILURE; 
    }
    
    /* get the NCS SubAgents Control Block, get the OID Databse */
    cb = m_SNMPSUBAGT_CB_GET;

    if (cb == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL); 
        return NCSCC_RC_FAILURE;
    }
    oid_db = &cb->oidDatabase;
    
    /* allocate the memory for the database node */ 
    db_node = m_MMGR_NCSSA_OID_DB_NODE_ALLOC;
    if (db_node == NULL)
    {
        /* log the error*/
        m_SNMPSUBAGT_MEM_FAIL_LOG(SNMPSUBAGT_OID_DB_NODE_MALLOC_FAIL);      
        return NCSCC_RC_OUT_OF_MEM; 
    }
    m_NCS_MEMSET(db_node, 0, sizeof(NCSSA_OID_DATABASE_NODE));

    /* no need to allocate the memory for the base OID */ 
    /* 'oid_base' is a static array in the generated code */
    db_node->base_oid = oid_base;

 
    /* compose the key for the patricia tree (table_id) */ 
    db_node->key.i_table_id = table_id; 
    
    /* get the base oid into our world */
    db_node->base_oid_len = oid_len;


    /* upload the object details */
    db_node->objects_details = object_details;

    /* upload the number of objects */
    db_node->number_of_objects = number_of_objects;
    
    /* add the node to the patricia tree */ 
    db_node->PatNode.key_info = (uns8*)&db_node->key;
    status = ncs_patricia_tree_add(oid_db, &db_node->PatNode); 
    if (status == NCSCC_RC_FAILURE)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_MEM_ALLOC_PAT_NODE_FAIL); 

        
        /* free the db node */
        m_MMGR_NCSSA_OID_DB_NODE_FREE(db_node);
        
        return NCSCC_RC_FAILURE; 
    }
    
    /* logging */
    m_SNMPSUBAGT_TABLE_ID_LOG(SNMPSUBAGT_NODE_ADDED_IN_OID_DB, 
                        db_node->key.i_table_id/*, db_node->base_oid --TBD */); 
  
    return NCSCC_RC_SUCCESS; 
}

/******************************************************************************
 *  Name:          snmpsubagt_table_oid_del
 *
 *  Description:   Routine to delete the table-id and base-oid mapping
 * 
 *  Arguments:     uns32   table_id
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
uns32 
snmpsubagt_table_oid_del(uns32   table_id)
{  
    uns32                       status = NCSCC_RC_SUCCESS;  
    NCSSA_OID_DATABASE_KEY      db_key; 
    NCSSA_OID_DATABASE_NODE     *db_node = NULL; 
    NCSSA_CB                    *cb = NULL;

    /* LOG Func Entry */ 
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_TABLE_OID_DEL);

    /* validate the inputs */
    /* get the Subagents control block */ 
    cb = m_SNMPSUBAGT_CB_GET;
    if (cb == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_CB_NULL); 
        return NCSCC_RC_FAILURE; 
    }
    
    /* Fill in the key to get the element from data base */
    m_NCS_MEMSET(&db_key, 0, sizeof(NCSSA_OID_DATABASE_KEY));
    db_key.i_table_id = table_id; 
    
    /* get the node pointer from the patricia tree */ 
    db_node = (NCSSA_OID_DATABASE_NODE *)ncs_patricia_tree_get(&cb->oidDatabase,
                                    (uns8*)&db_key);  
    if (db_node == NULL)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_OID_DB_NODE_NULL); 
  
        return NCSCC_RC_FAILURE; 
    }

    m_SNMPSUBAGT_TABLE_ID_LOG(SNMPSUBAGT_NODE_DEL_FROM_OID_DB, 
                        db_node->key.i_table_id/*, db_node->base_oidi TBD */); 

    /* delete the node from the tree */ 
    status = ncs_patricia_tree_del(&cb->oidDatabase, &db_node->PatNode);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_OID_DB_NODE_DEL_FAIL); 

        return status; 
    }
    
    /* free the databse node */
    m_MMGR_NCSSA_OID_DB_NODE_FREE(db_node);
    
    return status; 
}

/******************************************************************************
 *  Name:          snmpsubagt_table_oid_destroy
 *
 *  Description:   To free the contents of the OID database
 * 
 *  Arguments:     NCS_PATRICIA_TREE * - OID data base to be deleted 
 *
 *  Returns:       Nothing
 *  NOTE: 
 *****************************************************************************/
void
snmpsubagt_table_oid_destroy(NCS_PATRICIA_TREE   *oid_db)
{
    NCSSA_OID_DATABASE_NODE    *db_node = NULL;
  
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_TABLE_OID_DESTROY); 
  
    if (oid_db == NULL)
    {
        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_OID_DB_NULL); 
        return; 
    }

    /* destroy the rest of the nodes in the tree */
    while ((db_node= (NCSSA_OID_DATABASE_NODE*)ncs_patricia_tree_getnext(
                                            oid_db, 
                                            (uns8*)0)) != NULL)
    {
        /* delete the node from the tree */
        ncs_patricia_tree_del(oid_db, &db_node->PatNode); 


        /* free the node */
        m_MMGR_NCSSA_OID_DB_NODE_FREE(db_node); 

        db_node = NULL; 
    }

    return; 
}



