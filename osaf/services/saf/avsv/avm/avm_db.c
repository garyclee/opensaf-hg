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
 
  DESCRIPTION:This module deals with creating an AVM_ENT_INFO_T with a key,
  SaHpiEntityPathT and  also adding entries into it and also finding 
  AVM_ENT_INFO_T with SaHpiEntityPathT and SaNameT.
  ..............................................................................

  Function Included in this Module:

  avm_add_ent_info           -  
  avm_find_ent_info          - 
  avm_find_next_ent_info     - 
  avm_add_node_name_info     -
  avm_populate_ent_info      - 
  avm_populate_node_info     - 
  avm_insert_fault_domain    -
 
******************************************************************************
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "avm.h"

/**********************************************************************
 ******
 * Name          : avm_add_ent_info
 *
 * Description   : This function initializes an entity and also does the 
 *                 job of inserting it in patricia tree. 
 *
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 SAHpiEntityPathT* - Pointer to entity path.  
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 *****/

extern AVM_ENT_INFO_T* 
avm_add_ent_info(
                  AVM_CB_T          *avm_cb, 
                  SaHpiEntityPathT  *entity_path
                )
{
   AVM_ENT_INFO_T *ent_info;
   uns32           i;
   
   if((ent_info = m_MMGR_ALLOC_AVM_ENT_INFO) == AVM_ENT_INFO_NULL)
   {
      m_AVM_LOG_MEM(AVM_LOG_ENT_INFO_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      return AVM_ENT_INFO_NULL;
   }   
   
   memset((uns8 *)ent_info, '\0', sizeof(AVM_ENT_INFO_T));
   memcpy(ent_info->entity_path.Entry, entity_path->Entry, sizeof(SaHpiEntityPathT));

   ent_info->tree_node.key_info = (uns8 *)(ent_info->entity_path.Entry);

   ent_info->tree_node.bit      = 0;
   ent_info->tree_node.left     = NCS_PATRICIA_NODE_NULL;   
   ent_info->tree_node.right    = NCS_PATRICIA_NODE_NULL;

   if(ncs_patricia_tree_add(&avm_cb->db.ent_info_anchor, &ent_info->tree_node) != NCSCC_RC_SUCCESS)
   {
      m_AVM_LOG_PATRICIA_OP(AVM_LOG_PAT_ADD, entity_path->Entry, sizeof(SaHpiEntityPathT), AVM_LOG_PAT_FAILURE, NCSFL_SEV_CRITICAL);
      m_MMGR_FREE_AVM_ENT_INFO(ent_info);
      return AVM_ENT_INFO_NULL;
   }

   if ((ent_info->ent_hdl = ncshm_create_hdl(NCS_HM_POOL_ID_COMMON, 
        NCS_SERVICE_ID_AVM, ent_info)) == 0)
   {
        m_AVM_LOG_CB(AVM_LOG_CB_HDL_ASS_CREATE, AVM_LOG_SEAPI_FAILURE, NCSFL_SEV_CRITICAL);
        m_MMGR_FREE_AVM_ENT_INFO(ent_info);
        return AVM_ENT_INFO_NULL;
   }

   /* vivek_avm */
   ent_info->cb_hdl                  = avm_cb->cb_hdl;
   ent_info->fault_domain            = AVM_FAULT_DOMAIN_NULL;
   ent_info->child                   = AVM_ENT_INFO_LIST_NULL;
   ent_info->parent                  = AVM_ENT_INFO_NULL;

   ent_info->sensor_count           = 0;
   ent_info->sensor_assert          = NCSCC_RC_FAILURE;

   ent_info->fault_domain           = AVM_FAULT_DOMAIN_NULL;
   ent_info->depends_on_valid       = NCSCC_RC_FAILURE;
   
   memset(ent_info->node_name.value, '\0', SA_MAX_NAME_LENGTH);
   ent_info->node_name.length       = 0;

   ent_info->current_state          = AVM_ENT_NOT_PRESENT;
   ent_info->previous_state         = AVM_ENT_INVALID;
   ent_info->previous_diff_state    = AVM_ENT_INVALID;
   
   /* Initialization of all DHCP configuration fields */
   ent_info->dhcp_serv_conf.net_boot = NCS_SNMP_FALSE;

   ent_info->dhcp_serv_conf.label1.other_label = 
      &ent_info->dhcp_serv_conf.label2;

   ent_info->dhcp_serv_conf.label2.other_label = 
      &ent_info->dhcp_serv_conf.label1;

   /* Create host name entries using entity path */
   m_AVM_CREATE_HOST_ENTRY(entity_path, ent_info->dhcp_serv_conf);

   ent_info->parent      = NULL;
   ent_info->child       = NULL;    
   ent_info->act_policy  = AVM_SELF_VALIDATION_ACTIVATION;
   ent_info->row_status       = NCS_ROW_NOT_READY;

   for(i = 0; i < AVM_MAX_SENSOR_COUNT; i++)
   {
      avm_sensor_event_initialize(ent_info, i);
   }

    m_AVM_LOG_PATRICIA_OP(AVM_LOG_PAT_ADD, entity_path->Entry, sizeof(SaHpiEntityPathT), AVM_LOG_PAT_SUCCESS, NCSFL_SEV_INFO);

   ent_info->desc_name.length = 0;
   memset(ent_info->desc_name.name, '\0', NCS_MAX_INDEX_LEN);

   ent_info->parent_desc_name.length = 0;
   memset(ent_info->parent_desc_name.name, '\0', NCS_MAX_INDEX_LEN);   

   return ent_info;
}

/***********************************************************************
 ******
 * Name          : avm_find_ent_info
 *
 * Description   : This function finds an entity corresponding to an 
 *                 entity path in Patricia tree.
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 SAHpiEntityPathT* - Pointer to entity path.
 *
 * Return Values : AVM_ENT_INFO* - Pointer to the traced entity.
 *                 Null is returned if not found. 
 *
 * Notes         : None.
 ***********************************************************************
 ******/

extern AVM_ENT_INFO_T* 
avm_find_ent_info(
                     AVM_CB_T          *avm_cb, 
                     SaHpiEntityPathT  *entity_path
                 )
{
   AVM_ENT_INFO_T *ent_info;
   
   ent_info = (AVM_ENT_INFO_T*) ncs_patricia_tree_get(&avm_cb->db.ent_info_anchor, (uns8*)entity_path->Entry);

   return ent_info;
}

extern AVM_ENT_INFO_T*
avm_find_first_ent_info(AVM_CB_T   *avm_cb)
{  
   AVM_ENT_INFO_T *ent_info = AVM_ENT_INFO_NULL;
   
   ent_info = (AVM_ENT_INFO_T*) 
              ncs_patricia_tree_getnext(&avm_cb->db.ent_info_anchor, 
                                 (uns8*)AVM_ENT_INFO_NULL);

   return ent_info;
}


/**********************************************************************
 ******
 * Name          : avm_find_next_ent_info
 *
 * Description   : This function finds the next entity corresponding
 *                 to an entity path in Patricia tree.
 *
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 SAHpiEntityPathT* - Pointer to entity path.
 *
 * Return Values : AVM_ENT_INFO* - Pointer to the traced entity.
 *                 Null is returned if not found.
 *
 * Notes         : None.
 **********************************************************************
 ******/

extern AVM_ENT_INFO_T* 
avm_find_next_ent_info(
                        AVM_CB_T          *avm_cb, 
                        SaHpiEntityPathT  *entity_path
                      )
{
   AVM_ENT_INFO_T *ent_info;
   
   
   ent_info = (AVM_ENT_INFO_T*) ncs_patricia_tree_getnext(&avm_cb->db.ent_info_anchor, (uns8*)entity_path->Entry);

   return ent_info;
}

/**********************************************************************
 ******
 * Name          : avm_delete_ent_info
 *
 * Description   : This function deletes the entity corresponding
 *                 to an entity path in Patricia tree.
 *
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 SAHpiEntityPathT* - Pointer to entity path.
 *
 * Return Values : NCSCC_RS_SUCCESS /  NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 **********************************************************************
 ******/

extern uns32 
avm_delete_ent_info(
                     AVM_CB_T          *avm_cb, 
                     AVM_ENT_INFO_T    *ent_info
                   )
{
   if(AVM_ENT_INFO_NULL == ent_info)
   {
      return NCSCC_RC_FAILURE;
   }
   if(NCSCC_RC_SUCCESS != ncs_patricia_tree_del(&avm_cb->db.ent_info_anchor, &ent_info->tree_node))

   {
      m_NCS_DBG_PRINTF("\n Failed to delete the entity..");
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}


/**********************************************************************
 ******
 * Name          : avm_add_node_name_info
 *
 * Description   : This function initializes node iinfo and also does 
 *                 the job of inserting it in patricia tree.
 *
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 SaNameT   - Name of the node.
 * Return Values : AVM_NODE_INFO_T* - Pointer to the Node info inserted
 *                 in the patricia tree.   
 *
 * Notes         : None.
 ***********************************************************************
 *****/

extern AVM_NODE_INFO_T* 
avm_add_node_name_info(
                        AVM_CB_T    *avm_cb, 
                        SaNameT      node_name
                      )
{
   AVM_NODE_INFO_T *node_info;
   
   if((node_info = m_MMGR_ALLOC_AVM_NODE_INFO) == AVM_NODE_INFO_NULL)
   {
      m_AVM_LOG_MEM(AVM_LOG_NODE_INFO_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      return AVM_NODE_INFO_NULL;
   }   

   memset((uns8 *)node_info, '\0', sizeof(AVM_NODE_INFO_T));
   
   memset(node_info->node_name.value, '\0', SA_MAX_NAME_LENGTH);

   memcpy(node_info->node_name.value, node_name.value, node_name.length);
   node_info->node_name.length = node_name.length;

   node_info->tree_node_name.key_info = (uns8 *)(node_info->node_name.value);

   node_info->tree_node_name.bit      = 0;
   node_info->tree_node_name.left     = NCS_PATRICIA_NODE_NULL;   
   node_info->tree_node_name.right    = NCS_PATRICIA_NODE_NULL;

   if(ncs_patricia_tree_add(&avm_cb->db.node_name_anchor, &node_info->tree_node_name) != NCSCC_RC_SUCCESS)
   {
      m_AVM_LOG_PATRICIA_OP(AVM_LOG_PAT_ADD, node_name.value, SA_MAX_NAME_LENGTH, AVM_LOG_PAT_FAILURE, NCSFL_SEV_CRITICAL);
      m_MMGR_FREE_AVM_NODE_INFO(node_info);
      return AVM_NODE_INFO_NULL;
   }

   m_AVM_LOG_PATRICIA_OP(AVM_LOG_PAT_ADD, node_name.value, SA_MAX_NAME_LENGTH, AVM_LOG_PAT_SUCCESS, NCSFL_SEV_INFO);
   return node_info;
         
}

/***********************************************************************
 ******
 * Name          : avm_find_node_info
 *
 * Description   : This function finds a node corresponding to a 
 *                 Node Name in Patricia tree.

 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 SaNameT   - Name of the node
 *
 * Return Values : AVM_ENT_INFO* - Pointer to the traced node.
 *                 Null is returned if not found. 
 *
 * Notes         : None.
 ***********************************************************************
 ******/

extern AVM_NODE_INFO_T* 
avm_find_node_name_info(
                         AVM_CB_T *avm_cb, 
                         SaNameT node_name
                       )
{
   AVM_NODE_INFO_T *node_info;
  
   node_info = (AVM_NODE_INFO_T*) ncs_patricia_tree_get(&avm_cb->db.node_name_anchor, (uns8*)node_name.value);

   return node_info;
}

/***********************************************************************
 ******
 * Name          : avm_find_next_node_name_info
 *
 * Description   : This function finds a next node corresponding to a 
 *                 Node Name in Patricia tree.

 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 SaNameT   - Name of the node
 *
 * Return Values : AVM_ENT_INFO* - Pointer to the traced node.
 *                 Null is returned if not found. 
 *
 * Notes         : None.
 ***********************************************************************
 ******/

extern AVM_NODE_INFO_T* 
avm_find_next_node_name_info(
                              AVM_CB_T *avm_cb, 
                              SaNameT node_name
                            )
{
   AVM_NODE_INFO_T *node_info;
   
   node_info = (AVM_NODE_INFO_T*) ncs_patricia_tree_getnext(&avm_cb->db.node_name_anchor, (uns8*)node_name.value);

   return node_info;
}


/***********************************************************************
 ******
 * Name          : avm_delete_node_name_info
 *
 * Description   : This function deletes a node corresponding to a 
 *                 Node Name in Patricia tree.

 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 SaNameT   - Name of the node
 *
 * Return Values : AVM_ENT_INFO* - Pointer to the traced node.
 *                 Null is returned if not found. 
 *
 * Notes         : None.
 ***********************************************************************
 ******/

extern uns32 
avm_delete_node_name_info(
                           AVM_CB_T *avm_cb, 
                           AVM_NODE_INFO_T *node_info
                         )
{
   if(AVM_NODE_INFO_NULL == node_info)
   {
      return NCSCC_RC_FAILURE;
   }

   if(NCSCC_RC_SUCCESS != ncs_patricia_tree_del(&avm_cb->db.node_name_anchor, &node_info->tree_node_name))
   {
      m_NCS_DBG_PRINTF("\n Failed to delete the entity..");
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}      

/***********************************************************************
 ******
 * Name          : avm_populate_ent_info
 *
 * Description   : This function populates ent info's parent.

 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 AVM_ENT_INFO_T* - Pointer to Entity info.
 *                 SaHpiEntityPathT - Pointer to the Entity Path
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/
extern uns32 
avm_populate_ent_info(
                        AVM_CB_T            *cb, 
                        AVM_ENT_INFO_T      *ent_info
                     )
{
   AVM_ENT_INFO_T        *parent;
   AVM_ENT_INFO_LIST_T   *head;
   AVM_ENT_INFO_LIST_T   *child;   
   SaHpiEntityPathT       parent_entity_path;

   m_AVM_LOG_FUNC_ENTRY("avm_populate_ent_info");

   m_AVM_LOG_GEN_EP_STR("Entity Path:", ent_info->ep_str.name, NCSFL_SEV_INFO);

   if(SAHPI_ENT_ROOT == ent_info->entity_type)
   {
      ent_info->parent = AVM_ENT_INFO_NULL;
      return NCSCC_RC_SUCCESS;
   }

   find_parent_entity_path(&ent_info->entity_path, &parent_entity_path);

   parent = avm_find_ent_info(cb, &parent_entity_path);

   if(AVM_ENT_INFO_NULL == parent)
   {
      m_AVM_LOG_INVALID_VAL_ERROR(0);
      ent_info->parent = AVM_ENT_INFO_NULL;
      return NCSCC_RC_FAILURE;
   }

   ent_info->parent = parent;
   
   head = parent->child;

   child = m_MMGR_ALLOC_AVM_ENT_INFO_LIST;

   if(AVM_ENT_INFO_LIST_NULL == child)
   {
      m_NCS_DBG_PRINTF("\n Malloc failed for AVM_ENT_INFO_LIST_NODE");
      return NCSCC_RC_FAILURE;
   }
   
   child->ent_info = ent_info;
   child->next     = head;
   parent->child   = child;

   return NCSCC_RC_SUCCESS;
}



/***********************************************************************
 ******
 * Name          : avm_populate_node_info
 *
 * Description   : This function populates node info.

 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 SaHpiEntityPathT - Pointer to the Entity Path
 *                 AVM_NODE_INFO_T* - Pointer to Node info.
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/

extern uns32 
avm_populate_node_info(
                        AVM_CB_T           *cb, 
                        SaHpiEntityPathT   *entity_path, 
                        AVM_NODE_INFO_T    *node_info
                      )
{
   AVM_ENT_INFO_T  *ent_info;
   
   ent_info = avm_find_ent_info(cb, entity_path);
   if(AVM_ENT_INFO_NULL == ent_info)
   {                                                                         
      m_AVM_LOG_GEN("No entity with Entity Path:",  entity_path->Entry, sizeof(SaHpiEntityPathT), NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;
   }
                                                                         
   m_AVM_LOG_GEN("Entity with Entity Path:",  entity_path->Entry, sizeof(SaHpiEntityPathT), NCSFL_SEV_INFO);
   node_info->ent_info = ent_info;
   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
 * Name          : find_parent_entity_path
 *
 * Description   : This function gives parent of an entity path.

 * Arguments     : SaHpiEntityPathT - Pointer to the Entity Path
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/

extern uns32
find_parent_entity_path(
          SaHpiEntityPathT *child_entity_path, 
          SaHpiEntityPathT *parent_entity_path
                       )
{
   uns32 i;

   for(i = 0; i < SAHPI_MAX_ENTITY_PATH - 1; i++)
   {    
      parent_entity_path->Entry[i].EntityType        = child_entity_path->Entry[i+1].EntityType;
#ifdef HAVE_HPI_A01
        parent_entity_path->Entry[i].EntityInstance    = child_entity_path->Entry[i+1].EntityInstance;   
#else
        parent_entity_path->Entry[i].EntityLocation    = child_entity_path->Entry[i+1].EntityLocation;   
#endif
   }

   parent_entity_path->Entry[SAHPI_MAX_ENTITY_PATH - 1].EntityType     = 0;
#ifdef HAVE_HPI_A01
   parent_entity_path->Entry[SAHPI_MAX_ENTITY_PATH - 1].EntityInstance = 0;
#else
   parent_entity_path->Entry[SAHPI_MAX_ENTITY_PATH - 1].EntityLocation = 0;
#endif
   
   return NCSCC_RC_SUCCESS;

}
 
/***********************************************************************
 ******
 * Name          : avm_insert_fault_domain
 *
 * Description   : This function inserts a fd node in ent info

 * Arguments     : AVM_ENT_INFO_T*  - Pointer to the entity info. 
 *                 SaHpiEntityPathT - Pointer to the Entity Path
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/

extern uns32 
avm_insert_fault_domain(
                           AVM_ENT_INFO_T    *ent_info,
                           SaHpiEntityPathT  *entity_path
                       )
{
   AVM_FAULT_DOMAIN_T *node;
   AVM_FAULT_DOMAIN_T *head;

   uns32 rc = NCSCC_RC_SUCCESS;   

   head = ent_info->fault_domain;

   node = m_MMGR_ALLOC_AVM_FAULT_DOMAIN;
   
   memcpy(node->ent_path.Entry, entity_path->Entry, sizeof(SaHpiEntityPathT));

   node->next             = head;
   ent_info->fault_domain = node;

   return rc;   

}

/***********************************************************************
 ******
 * Name          : avm_add_valid_str_info
 *
 * Description   : This function adds AVM_VALID_INFO_T strut based on 
 *                 AVM_ENT_DESC_NAME_T.   
 *                 
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 AVM_ENT_DESC_NAME_T* - Pointer to Entity Path string.
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/
extern AVM_VALID_INFO_T* 
avm_add_valid_info(
                    AVM_CB_T            *avm_cb, 
                    AVM_ENT_DESC_NAME_T *desc_name
                  )
{
   AVM_VALID_INFO_T *valid_info;
   
   if((valid_info = m_MMGR_ALLOC_AVM_VALID_INFO) == AVM_VALID_INFO_NULL)
   {
      m_AVM_LOG_MEM(AVM_LOG_VALID_INFO_ALLOC, AVM_LOG_MEM_ALLOC_FAILURE, NCSFL_SEV_EMERGENCY);
      return AVM_VALID_INFO_NULL;
   }   
   
   memset((uns8 *)valid_info, '\0', sizeof(AVM_VALID_INFO_T));
   memcpy(valid_info->desc_name.name, desc_name->name, desc_name->length);
   valid_info->desc_name.length = desc_name->length;
  

   valid_info->tree_node.key_info = (uns8 *)(valid_info->desc_name.name);

   valid_info->tree_node.bit      = 0;
   valid_info->tree_node.left     = NCS_PATRICIA_NODE_NULL;   
   valid_info->tree_node.right    = NCS_PATRICIA_NODE_NULL;

   if(ncs_patricia_tree_add(&avm_cb->db.valid_info_anchor, &valid_info->tree_node) != NCSCC_RC_SUCCESS)
   {
      m_AVM_LOG_PATRICIA_OP_STR(AVM_LOG_PAT_ADD, desc_name->name, AVM_LOG_PAT_FAILURE, NCSFL_SEV_ERROR);
      m_MMGR_FREE_AVM_VALID_INFO(valid_info);
      return AVM_VALID_INFO_NULL;
   }

   valid_info->parent_cnt = 0;

   valid_info->inv_data.product_name.DataLength = 0;
   valid_info->inv_data.product_name.Language   = SAHPI_LANG_UNDEF;
   memset(valid_info->inv_data.product_name.Data, '\0', SAHPI_MAX_TEXT_BUFFER_LENGTH);

   valid_info->inv_data.product_version.DataLength = 0;
   valid_info->inv_data.product_version.Language   = SAHPI_LANG_UNDEF;
   memset(valid_info->inv_data.product_version.Data, '\0', SAHPI_MAX_TEXT_BUFFER_LENGTH);
   
   m_AVM_LOG_PATRICIA_OP_STR(AVM_LOG_PAT_ADD, desc_name->name, AVM_LOG_PAT_SUCCESS, NCSFL_SEV_INFO);

   return valid_info;
}

/***********************************************************************
 ******
 * Name          : avm_find_valid_info
 *
 * Description   : This function adds AVM_VALID_INFO_T strut based on 
 *                 AVM_ENT_DESC_NAME_T.   
 *                 
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 AVM_ENT_DESC_NAME_T* - Pointer to Entity Path string.
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/
extern AVM_VALID_INFO_T* 
avm_find_valid_info(
                    AVM_CB_T            *avm_cb, 
                    AVM_ENT_DESC_NAME_T *desc_name
                   )
{
   AVM_VALID_INFO_T *valid_info;
   
   valid_info = (AVM_VALID_INFO_T*) ncs_patricia_tree_get(&avm_cb->db.valid_info_anchor, (uns8*)desc_name->name);

   return valid_info;
}

/***********************************************************************
 ******
 * Name          : avm_find_next_valid_info
 *
 * Description   : This function finds next AVM_VALID_INFO_T strut base on 
 *                 desc name string.   
 *                 
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 AVM_DESC_NAME_T* - Pointer to Entity desc name.
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/
extern AVM_VALID_INFO_T* 
avm_find_next_valid_info(
                          AVM_CB_T                *avm_cb, 
                          AVM_ENT_DESC_NAME_T     *desc_name
                        )
{
   AVM_VALID_INFO_T *valid_info = AVM_VALID_INFO_NULL;
   
   valid_info = (AVM_VALID_INFO_T*) ncs_patricia_tree_getnext(&avm_cb->db.valid_info_anchor, (uns8*)desc_name->name);

   return valid_info;
}

/***********************************************************************
 ******
 * Name          : avm_add_ent_str_info
 *
 * Description   : This function adds AVM_ENT_INFO_T strut based on 
 *                 entity path string.   
 *                 
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 AVM_ENT_PATH_STR_T* - Pointer to Entity Path string.
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/
extern uns32 
avm_add_ent_str_info(
                     AVM_CB_T             *avm_cb,  
                     AVM_ENT_INFO_T       *ent_info, 
                     AVM_ENT_PATH_STR_T   *ep
                    )
{
   ent_info->ep_str.length = m_NCS_OS_HTONS(ep->length);
   memcpy(ent_info->ep_str.name, ep->name, ep->length);    

   ent_info->tree_node_str.key_info = (uns8 *)(&ent_info->ep_str);
   ent_info->tree_node_str.bit      = 0;
   ent_info->tree_node_str.left     = NCS_PATRICIA_NODE_NULL;   
   ent_info->tree_node_str.right    = NCS_PATRICIA_NODE_NULL;

   if(ncs_patricia_tree_add(&avm_cb->db.ent_info_str_anchor, &ent_info->tree_node_str) != NCSCC_RC_SUCCESS)
   {
      m_AVM_LOG_PATRICIA_OP_STR(AVM_LOG_PAT_ADD, ep->name, AVM_LOG_PAT_FAILURE, NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE; 
   }

    m_AVM_LOG_PATRICIA_OP_STR(AVM_LOG_PAT_ADD, ep->name, AVM_LOG_PAT_SUCCESS, NCSFL_SEV_INFO);

   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
 * Name          : avm_finds_ent_str_info
 *
 * Description   : This function finds AVM_ENT_INFO_T strut based on 
 *                 entity path string.   
 *                 
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 AVM_ENT_PATH_STR_T* - Pointer to Entity Path string.
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/
extern AVM_ENT_INFO_T* 
avm_find_ent_str_info(
                      AVM_CB_T            *avm_cb, 
                      AVM_ENT_PATH_STR_T  *ep,
                      NCS_BOOL            is_host_order      
                     )
{
   AVM_ENT_INFO_T *ent_info;
   AVM_ENT_PATH_STR_T ep_net;
   
   memset(ep_net.name, '\0', AVM_MAX_INDEX_LEN);
   
   memcpy(ep_net.name, ep->name, ep->length);
   if(TRUE == is_host_order )
      ep_net.length = m_NCS_OS_HTONS(ep->length);      
   else
      ep_net.length = ep->length; /*Already in Network order  */     
      
   
   ent_info = (AVM_ENT_INFO_T*) ncs_patricia_tree_get(&avm_cb->db.ent_info_str_anchor, (uns8*)&ep_net);

   if(AVM_ENT_INFO_NULL != ent_info)
   {
      ent_info = (AVM_ENT_INFO_T*)(((char*)ent_info)
                                   -(((char*)&(AVM_ENT_INFO_NULL->tree_node_str)
               -(char*)AVM_ENT_INFO_NULL))); 
   }   

   return ent_info;
}


/***********************************************************************
 ******
 * Name          : avm_finds_next_ent_str_info
 *
 * Description   : This function finds next AVM_ENT_INFO_T strut base on 
 *                 entity path string.   
 *                 
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 AVM_ENT_PATH_STR_T* - Pointer to Entity Path string.
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/
extern AVM_ENT_INFO_T* 
avm_find_next_ent_str_info(
                           AVM_CB_T            *avm_cb, 
                           AVM_ENT_PATH_STR_T  *ep
                          )
{
   AVM_ENT_INFO_T *ent_info = AVM_ENT_INFO_NULL;
   AVM_ENT_PATH_STR_T ep_net;
   
   memset(ep_net.name, '\0', AVM_MAX_INDEX_LEN);
   
   memcpy(ep_net.name, ep->name, ep->length);
   ep_net.length = m_NCS_OS_HTONS(ep->length);      
   
   ent_info = (AVM_ENT_INFO_T*) ncs_patricia_tree_getnext(&avm_cb->db.ent_info_str_anchor, (uns8*)&ep_net);

   if(AVM_ENT_INFO_NULL != ent_info)
   {
      ent_info = (AVM_ENT_INFO_T*)(((char*)ent_info)
                                   -(((char*)&(AVM_ENT_INFO_NULL->tree_node_str)
               -(char*)AVM_ENT_INFO_NULL))); 
   }   

   return ent_info;
}

/***********************************************************************
 ******
 * Name          : avm_delete_ent_str_info
 *
 * Description   : This function deletes a node in Patricia Tree
 *                 
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 AVM_ENT_INFO_T* - Pointer to Entity info.
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/
extern uns32 
avm_delete_ent_str_info(
                        AVM_CB_T          *avm_cb, 
                        AVM_ENT_INFO_T    *ent_info
                      )
{
   if(AVM_ENT_INFO_NULL == ent_info)
   {
      return NCSCC_RC_FAILURE;
   }
   if(NCSCC_RC_SUCCESS != ncs_patricia_tree_del(&avm_cb->db.ent_info_str_anchor, &ent_info->tree_node_str))

   {
      m_NCS_DBG_PRINTF("\n Failed to delete the entity..");
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}

/***********************************************************************
 ******
 * Name          : avm_delete_valid_info
 *
 * Description   : This function deletes a valid info in Patricia Tree
 *                 
 * Arguments     : AVM_CB_T* - Pointer to AvM Control Block
 *                 AVM_VALID_INFO_T* - Pointer to Validation info.
 *
 * Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ***********************************************************************
 ******/
extern uns32 
avm_delete_valid_info(
                       AVM_CB_T            *avm_cb, 
                       AVM_VALID_INFO_T    *valid_info
                     )
{
   if(AVM_VALID_INFO_NULL == valid_info)
   {
      return NCSCC_RC_FAILURE;
   }

   if(NCSCC_RC_SUCCESS != ncs_patricia_tree_del(&avm_cb->db.valid_info_anchor, &valid_info->tree_node))

   {
      m_NCS_DBG_PRINTF("\n Failed to delete the validation Info..");
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}
