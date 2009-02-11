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

  DESCRIPTION:This module deals with the creation, accessing and deletion of
  the AVND database on the AVD. It also deals with all the MIB operations
  like set,get,getnext etc related to the Node tables.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_avnd_struc_crt - creates and adds avnd structure to database.
  avd_avnd_struc_find - Finds avnd structure from the database.
  avd_avnd_struc_find_next - Finds the next avnd structure from the database.
  avd_avnd_struc_find_next_addr - Finds the next avnd structure from
                                  the database based on node address.
  avd_avnd_struc_del - deletes and frees avnd structure from database.
  ncsndtableentry_get - This function is one of the get processing
                        routines for objects in NCS_N_D_TABLE_ENTRY_ID table.
  ncsndtableentry_extract - This function is one of the get processing
                            function for objects in NCS_N_D_TABLE_ENTRY_ID
                            table.
  ncsndtableentry_set - This function is the set processing for objects in
                         NCS_N_D_TABLE_ENTRY_ID table.
  ncsndtableentry_next - This function is the next processing for objects in
                         NCS_N_D_TABLE_ENTRY_ID table.
  ncsndtableentry_setrow - This function is the setrow processing for
                           objects in NCS_N_D_TABLE_ENTRY_ID table.
  ncsndtableentry_rmvrow - This function is for handling RMVROW event from MIBLIB.

  AMF NODE related functions

  saamfnodetableentry_get - This function is one of the get processing
                            routines for objects in SA_AMF_NODE_TABLE_ENTRY_ID
                            table.
  saamfnodetableentry_extract - This function is one of the get processing
                                routines for objects in 
                                SA_AMF_NODE_TABLE_ENTRY_ID table.
  saamfnodetableentry_set - This function is the set processing for objects in
                            SA_AMF_NODE_TABLE_ENTRY_ID table.
  saamfnodetableentry_next - This function is the next processing for objects in
                             SA_AMF_NODE_TABLE_ENTRY_ID table.
  saamfnodetableentry_setrow - This function is the setrow processing for
                               SA_AMF_NODE_TABLE_ENTRY_ID table.


  CLM NODE related functions.

  saclmnodetableentry_get - This function is one of the get processing
                            routines for objects in SA_CLM_NODE_TABLE_ENTRY_ID
                            table.
  saclmnodetableentry_extract - This function is one of the get processing
                                routines for objects in 
                                SA_CLM_NODE_TABLE_ENTRY_ID table.
  saclmnodetableentry_set - This function is the set processing for objects in
                            SA_CLM_NODE_TABLE_ENTRY_ID table.
  saclmnodetableentry_next -  This function is the next processing for object in
                              SA_CLM_NODE_TABLE_ENTRY_ID table
  saclmnodetableentry_setrow - This function is the setrow processing for
                               SA_CLM_NODE_TABLE_ENTRY_ID table.


******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"



/*****************************************************************************
 * Function: avd_avnd_struc_crt
 *
 * Purpose:  This function will create and add a AVD_AVND structure to the
 * tree if an element with node_name key value doesn't exist.
 *
 * Input: cb - the AVD control block
 *        node_name - The node name on which the AVND is running.
 *        ckpt  - TRUE - Create is called from ckpt update.
 *                FALSE - Create is called from AVD mib set.
 *
 * Returns: The pointer to AVD_AVND structure allocated and added.
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

AVD_AVND * avd_avnd_struc_crt(AVD_CL_CB *cb,SaNameT node_name, NCS_BOOL ckpt)
{
   AVD_AVND *avnd;

   if( (node_name.length <= 8) || (m_NCS_OS_STRNCMP(node_name.value,"safNode=",8) != 0) )
   {
      return AVD_AVND_NULL;
   }

   /* Allocate a new block structure now
    */
   if ((avnd = m_MMGR_ALLOC_AVD_AVND) == AVD_AVND_NULL)
   {
      /* log an error */
      m_AVD_LOG_MEM_FAIL(AVD_AVND_ALLOC_FAILED);
      return AVD_AVND_NULL;
   }

   m_NCS_MEMSET((char *)avnd, '\0', sizeof(AVD_AVND));

   if (ckpt)
   {
      /* Fill the node name into the field and also into the key area. */ 
      m_NCS_MEMCPY(avnd->node_info.nodeName.value, 
         node_name.value, m_NCS_OS_NTOHS(node_name.length));
      avnd->node_info.nodeName.length = node_name.length;
   }
   else
   {
      /* Fill the node name into the field and also into the key area. */ 
      m_NCS_MEMCPY(avnd->node_info.nodeName.value, 
         node_name.value, node_name.length);

      avnd->node_info.nodeName.length = m_HTON_SANAMET_LEN(node_name.length);
      
      

      avnd->su_admin_state = NCS_ADMIN_STATE_UNLOCK;
      avnd->oper_state = NCS_OPER_STATE_DISABLE;
      avnd->row_status = NCS_ROW_NOT_READY;
      avnd->node_state = AVD_AVND_STATE_ABSENT;
      avnd->node_info.member = SA_FALSE;

      /* if the node is sitting on the same node as the AVD mark
       * the type value as indicating that it is system controller AvND.
       */
      avnd->type = AVSV_AVND_CARD_PAYLOAD;
   }
   avnd->avm_oper_state = NCS_OPER_STATE_ENABLE;

   /* init pointers */
   avnd->list_of_su = AVD_SU_NULL;
   avnd->list_of_ncs_su = AVD_SU_NULL;

   /* initialize the pg csi-list */
   avnd->pg_csi_list.order = NCS_DBLIST_ANY_ORDER;
   avnd->pg_csi_list.cmp_cookie = avsv_dblist_uns32_cmp;
   avnd->pg_csi_list.free_cookie = 0;

   
   avnd->tree_node_name_node.key_info = (uns8*)&(avnd->node_info.nodeName);
   avnd->tree_node_name_node.bit   = 0;
   avnd->tree_node_name_node.left  = NCS_PATRICIA_NODE_NULL;
   avnd->tree_node_name_node.right = NCS_PATRICIA_NODE_NULL;

   m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);

   if( ncs_patricia_tree_add(&cb->avnd_anchor_name,&avnd->tree_node_name_node) 
                      != NCSCC_RC_SUCCESS)
   {
      /* log an error */
      m_MMGR_FREE_AVD_AVND(avnd);
      return AVD_AVND_NULL;
   }

   m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);

   return avnd;
}

/*****************************************************************************
 * Function: avd_avnd_struc_add_nodeid
 *
 * Purpose:  This function will add a AVD_AVND structure to the
 * tree with node_id value as key.
 *
 * Input: cb - the AVD control block
 *        avnd - The avnd structure that needs to be added.
 *
 * Returns: The pointer to AVD_AVND structure found in the tree with node id.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 avd_avnd_struc_add_nodeid(AVD_CL_CB *cb,AVD_AVND *avnd)
{
   if(avnd && avnd->node_info.nodeId)
   {
      avnd->tree_node_id_node.key_info = (uns8*)&(avnd->node_info.nodeId);
      avnd->tree_node_id_node.bit   = 0;
      avnd->tree_node_id_node.left  = NCS_PATRICIA_NODE_NULL;
      avnd->tree_node_id_node.right = NCS_PATRICIA_NODE_NULL;

      m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);

      if( ncs_patricia_tree_add(&cb->avnd_anchor,&avnd->tree_node_id_node)
                      != NCSCC_RC_SUCCESS)
      {
         /* log an error */
         return NCSCC_RC_FAILURE;
      }
      
      m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);
   }
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_avnd_struc_find
 *
 * Purpose:  This function will find a AVD_AVND structure in the
 * tree with node_name value as key. The function internaly converts the length
 * to network order.
 *
 * Input: cb - the AVD control block
 *        node_name - The node name on which the AVND is running.
 *
 * Returns: The pointer to AVD_AVND structure found in the tree.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

AVD_AVND *avd_avnd_struc_find(AVD_CL_CB *cb,SaNameT node_name)
{
   AVD_AVND *avnd;
   SaNameT lnode_name_net;

   m_NCS_MEMSET((char *)&lnode_name_net, '\0', sizeof(SaNameT));
   m_NCS_MEMCPY(lnode_name_net.value,node_name.value,node_name.length);
   lnode_name_net.length = m_HTON_SANAMET_LEN(node_name.length);

   avnd = (AVD_AVND *)ncs_patricia_tree_get(&cb->avnd_anchor_name,
                                          (uns8*)&lnode_name_net);

   if (avnd != AVD_AVND_NULL)
   {
       /* Adjust the pointer
        */
       avnd = (AVD_AVND *)(((char *)avnd)
                      - (((char *)&(AVD_AVND_NULL->tree_node_name_node))
                       - ((char *)AVD_AVND_NULL)));
   }

   return avnd;
}

/*****************************************************************************
 * Function: avd_avnd_struc_find_nodeid
 *
 * Purpose:  This function will find a AVD_AVND structure in the
 * tree with node_id value as key.
 *
 * Input: cb - the AVD control block
 *        node_id - The node id on which the AVND is running.
 *
 * Returns: The pointer to AVD_AVND structure found in the tree.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

AVD_AVND *avd_avnd_struc_find_nodeid(AVD_CL_CB *cb,SaClmNodeIdT node_id)
{
   AVD_AVND *avnd;

   avnd = (AVD_AVND *)ncs_patricia_tree_get(&cb->avnd_anchor,(uns8*)&node_id);

   return avnd;
}


/*****************************************************************************
 * Function: avd_avnd_struc_find_next
 *
 * Purpose:  This function will find the next AVD_AVND structure in the
 * tree whose node_name value is next of the given node_name value. The function 
 * internaly converts the length to network order.
 *
 * Input: cb - the AVD control block
 *        node_name - The node name on which the AVND is running.
 *
 * Returns: The pointer to AVD_AVND structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_AVND * avd_avnd_struc_find_next(AVD_CL_CB *cb,SaNameT node_name)
{
   AVD_AVND *avnd;
   SaNameT lnode_name_net;

   m_NCS_MEMSET((char *)&lnode_name_net, '\0', sizeof(SaNameT));
   m_NCS_MEMCPY(lnode_name_net.value,node_name.value,node_name.length);
   lnode_name_net.length = m_HTON_SANAMET_LEN(node_name.length);

   avnd = (AVD_AVND *)ncs_patricia_tree_getnext(&cb->avnd_anchor_name,
                                          (uns8*)&lnode_name_net);

   if (avnd != AVD_AVND_NULL)
   {
       /* Adjust the pointer
        */
       avnd = (AVD_AVND *)(((char *)avnd)
                      - (((char *)&(AVD_AVND_NULL->tree_node_name_node))
                       - ((char *)AVD_AVND_NULL)));
   }

   return avnd;
}

/*****************************************************************************
 * Function: avd_avnd_struc_find_next_cl_member
 *
 * Purpose:  This function will find the next AVD_AVND structure in the
 * tree whose node_name value is next of the given node_name value 
 * and is the member of the cluster. The function 
 * internaly converts the length to network order.
 *
 * Input: cb - the AVD control block
 *        node_name - The node name on which the AVND is running.
 *
 * Returns: The pointer to AVD_AVND structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

static AVD_AVND * avd_avnd_struc_find_next_cl_member(AVD_CL_CB *cb,SaNameT node_name)
{
   AVD_AVND *avnd = AVD_AVND_NULL;
   SaNameT lnode_name_net;

   m_NCS_MEMSET((char *)&lnode_name_net, '\0', sizeof(SaNameT));
   m_NCS_MEMCPY(lnode_name_net.value,node_name.value,node_name.length);
   lnode_name_net.length = m_HTON_SANAMET_LEN(node_name.length);

   do
   {
      avnd = (AVD_AVND *)ncs_patricia_tree_getnext(&cb->avnd_anchor_name,
                                                   (uns8*)&lnode_name_net);

      if (avnd != AVD_AVND_NULL)
      {
          /* Adjust the pointer
           */
          avnd = (AVD_AVND *)(((char *)avnd)
                      - (((char *)&(AVD_AVND_NULL->tree_node_name_node))
                       - ((char *)AVD_AVND_NULL)));

          lnode_name_net = avnd->node_info.nodeName;
      }

   }while( (avnd != AVD_AVND_NULL) && (avnd->node_info.member == FALSE) );

   return avnd;
}

/*****************************************************************************
 * Function: avd_avnd_struc_find_next_nodeid
 *
 * Purpose:  This function will find the next AVD_AVND structure in the
 * tree whose node_id value is next of the given node_id value.
 *
 * Input: cb - the AVD control block
 *        node_id - The node id on which the AVND is running.
 *
 * Returns: The pointer to AVD_AVND structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_AVND * avd_avnd_struc_find_next_nodeid(AVD_CL_CB *cb,SaClmNodeIdT node_id)
{
   AVD_AVND *avnd;

   avnd = (AVD_AVND *)ncs_patricia_tree_getnext(&cb->avnd_anchor,(uns8*)&node_id);

   return avnd;
}


/*****************************************************************************
 * Function: avd_add_avnd_anchor_nodeid
 *
 * Purpose:  This function will add a AVD_AVND structure to the
 * tree if an element with node_id key value doesn't exist.
 *
 * Input: cb - the AVD control block
 *        node_name - The node name on which the AVND is running.
 *        node_id - The node id which is the index to the tree.
 *
 * Returns: The pointer to AVD_AVND structure allocated and added.
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

uns32 avd_add_avnd_anchor_nodeid(AVD_CL_CB *cb, SaNameT node_name,
                                 SaClmNodeIdT node_id)
{
   AVD_AVND *avnd;
   avnd = avd_avnd_struc_find(cb, node_name);

   if(!avnd)
   {
      /* log an error */
      return NCSCC_RC_FAILURE;
   }

   avnd->node_info.nodeId = node_id;

   avnd->tree_node_id_node.key_info = (uns8*)&(avnd->node_info.nodeId);
   avnd->tree_node_id_node.bit   = 0;
   avnd->tree_node_id_node.left  = NCS_PATRICIA_NODE_NULL;
   avnd->tree_node_id_node.right = NCS_PATRICIA_NODE_NULL;


   if( ncs_patricia_tree_add(&cb->avnd_anchor,&avnd->tree_node_id_node)
                      != NCSCC_RC_SUCCESS)
   {
      /* log an error */
      m_MMGR_FREE_AVD_AVND(avnd);
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: avd_avnd_struc_del
 *
 * Purpose:  This function will delete and free AVD_AVND structure from 
 * the tree with node name as key.
 *
 * Input: cb - the AVD control block
 *        avnd - The avnd structure that needs to be deleted.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES: Make sure that the node is removed from the node_id tree of the CB.
 * avd_avnd_struc_rmv_nodeid() should be called prior to calling this func.
 *
 * 
 **************************************************************************/

uns32 avd_avnd_struc_del(AVD_CL_CB *cb, AVD_AVND *avnd)
{
   if (avnd == AVD_AVND_NULL)
      return NCSCC_RC_FAILURE;

   /* the pg-csi list should be null */
   m_AVSV_ASSERT(!avnd->pg_csi_list.n_nodes);

   m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);

   if(ncs_patricia_tree_del(&cb->avnd_anchor_name,&avnd->tree_node_name_node) 
                      != NCSCC_RC_SUCCESS)
   {
      /* log error */
      return NCSCC_RC_FAILURE;
   }

   m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);

   m_MMGR_FREE_AVD_AVND(avnd);
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_avnd_struc_rmv_nodeid
 *
 * Purpose:  This function will delete AVD_AVND structure from 
 * the tree with node id as key.
 *
 * Input: cb - the AVD control block
 *        avnd - The avnd structure that needs to be deleted.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_avnd_struc_rmv_nodeid(AVD_CL_CB *cb, AVD_AVND *avnd)
{
   if (avnd == AVD_AVND_NULL)
      return NCSCC_RC_FAILURE;

   m_AVD_CB_AVND_TBL_LOCK(cb, NCS_LOCK_WRITE);

   if(ncs_patricia_tree_del(&cb->avnd_anchor,&avnd->tree_node_id_node) 
                      != NCSCC_RC_SUCCESS)
   {
      /* log error */
      return NCSCC_RC_FAILURE;
   }
   
   m_AVD_CB_AVND_TBL_UNLOCK(cb, NCS_LOCK_WRITE);

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: ncsndtableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in NCS_N_D_TABLE_ENTRY_ID table. This is the properitory Node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function finds the corresponding data structure for the given
 * instance and returns the pointer to the structure.
 *
 * Input:  cb        - AVD control block.
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 * NOTES: This function works in conjunction with extract function to provide the
 * get functionality.
 *
 * 
 **************************************************************************/

uns32 ncsndtableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   SaNameT  node_name;
   AVD_AVND      *avnd;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }
   
   m_NCS_MEMSET(&node_name, '\0', sizeof(SaNameT));
   
   /* Prepare the nodename database key from the instant ID */
   node_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0] ;
   for(i = 0; i < node_name.length; i++)
   {
      node_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   avnd = avd_avnd_struc_find(avd_cb,node_name);

   if (avnd == AVD_AVND_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   
   *data = (NCSCONTEXT)avnd;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: ncsndtableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * NCS_N_D_TABLE_ENTRY_ID table. This is the  properitory node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after calling the get call to get data structure.
 * This function fills the value information in the param filed structure. For
 * octate information the buffer field will be used for filling the information.
 * MIBLIB will provide the memory and pointer to the buffer. For only objects that
 * have a direct value(i.e their offset is not 0 in VAR INFO) in the structure
 * the data field is filled using the VAR INFO provided by MIBLIB, for others based
 * on the OID the value is filled accordingly.
 *
 * Input:  param     -  param->i_param_id indicates the parameter to extract
 *                      The remaining elements of the param need to be filled
 *                      by the subystem's extract function
 *         var_info  - Pointer to the var_info structure for the param.
 *         data      - The pointer to the data-structure containing the object
 *                     value which we have already provided to MIBLIB from get call.
 *         buffer    - The buffer pointer provided by MIBLIB for filling the octate
 *                     type data.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 *
 * NOTES:  This function works in conjunction with other functions to provide the
 * get,getnext and getrow functionality.
 *
 * 
 **************************************************************************/

uns32 ncsndtableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer)
{
   AVD_AVND      *avnd = (AVD_AVND *)data;

   if (avnd == AVD_AVND_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* This table have the standard object types so just call the MIBLIB 
    * utility routine for standard object types */

   if ((var_info != NULL) && (var_info->offset != 0))
      return ncsmiblib_get_obj_val(param, var_info, data, buffer);
   else
      return NCSCC_RC_NO_OBJECT;
}


/*****************************************************************************
 * Function: ncsndtableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * NCS_N_D_TABLE_ENTRY_ID table. This is the  properitory node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for the objects that are settable. This same function can be used for test
 * operation also.
 *
 * Input:  cb        - AVD control block
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         var_info  - The VAR INFO structure pointer generated by MIBLIB for
 *                     the objects in this table.
 *         test_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 ncsndtableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{

   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   SaNameT       node_name;
   AVD_AVND      *avnd;
   NCSMIBLIB_REQ_INFO temp_mib_req;
   NCS_BOOL      val_same_flag = FALSE;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_INV_VAL;  
   }
   
   m_NCS_MEMSET(&node_name, '\0', sizeof(SaNameT));
   
   /* Prepare the nodename database key from the instant ID */
   node_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0] ;
   for(i = 0; i < node_name.length; i++)
   {
      node_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   avnd = avd_avnd_struc_find(avd_cb,node_name);
   if (avnd == AVD_AVND_NULL)
   {
      /* RowStatus doesnt exist for this table. A row is created automatically
      ** when a row in AMF node table is created. CREATE request doesnt
      ** happen for this table and hence return if the struc is not available
      */

      /* however, when PSS does the playback we want to relax this restriction
      ** so check the AVD state and create the structure
      */

      if(avd_cb->init_state == AVD_CFG_READY)
      {
         m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

         avnd = avd_avnd_struc_crt(avd_cb, node_name, FALSE);

         m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);
      }

      if(avnd == AVD_AVND_NULL)
         return NCSCC_RC_NO_INSTANCE; 
   }

   /* We now have the avnd block */
   
   if(avnd->row_status == NCS_ROW_ACTIVE)
   {
      /* when row status is active we don't allow any other MIB object to be
       * modified.
       */
      return NCSCC_RC_INV_VAL;
   }

   if(test_flag == TRUE)
   {
      return NCSCC_RC_SUCCESS;
   }

   /* All the fields are standard mib objects */
   m_NCS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

   temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
   temp_mib_req.info.i_set_util_info.param = &(arg->req.info.set_req.i_param_val);
   temp_mib_req.info.i_set_util_info.var_info = var_info;
   temp_mib_req.info.i_set_util_info.data = avnd;
   temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

   /* call the mib routine handler */ 
   return ncsmiblib_process_req(&temp_mib_req);
}



/*****************************************************************************
 * Function: ncsndtableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * NCS_N_D_TABLE_ENTRY_ID table. This is the properitory node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function gets the next valid instance and its data structure
 * and it passes to the MIBLIB the values.
 *
 * Input: cb        - AVD control block.
 *        arg       - The pointer to the MIB arg that was provided by the caller.
 *        data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 *     next_inst_id - The next instance id will be filled in this buffer
 *                     and returned by reference.
 * next_inst_id_len - The next instance id length.
 *
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context
 *
 * NOTES: This function works in conjunction with extract function to provide the
 * getnext functionality.
 *
 * 
 **************************************************************************/

uns32 ncsndtableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_AVND      *avnd;
   SaNameT       node_name;
   uns32         i;

   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }
   
   m_NCS_MEMSET(&node_name, '\0', sizeof(SaNameT));
   
   /* Prepare the nodename database key from the instant ID */
   
    if (arg->i_idx.i_inst_len != 0)
   {   
   
      node_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
      for(i = 0; i < node_name.length; i++)
      {
         node_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
   }

   avnd = avd_avnd_struc_find_next(avd_cb,node_name);

   if (avnd == AVD_AVND_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the instant ID from the node_name */

   *next_inst_id_len = m_NCS_OS_NTOHS(avnd->node_info.nodeName.length) + 1;
   next_inst_id[0] = *next_inst_id_len -1;

   for(i = 0; i < next_inst_id[0]; i++)
   {
      next_inst_id[i + 1] = (uns32)(avnd->node_info.nodeName.value[i]);
   }

   *data = (NCSCONTEXT)avnd;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: ncsndtableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * NCS_N_D_TABLE_ENTRY_ID table. This is the properitory node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for all the objects that are settable as part of the setrow operation. 
 * This same function can be used for test row
 * operation also.
 *
 * Input:  cb        - AVD control block
 *         args      - The pointer to the MIB arg that was provided by the caller.
 *         params    - The List of object ids and their values.
 *         obj_info  - The VAR INFO structure array pointer generated by MIBLIB for
 *                     the objects in this table.
 *      testrow_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 ncsndtableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}


/*****************************************************************************
 * Function: ncsndtableentry_rmvrow
 *
 * Purpose:  This function is one of the RMVROW processing routines for objects
 * in NCS_N_D_TABLE_ENTRY_ID table. 
 *
 * Input:  cb        - AVD control block.
 *         idx       - pointer to NCSMIB_IDX 
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 **************************************************************************/
uns32 ncsndtableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx) 
{
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfnodetableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_NODE_TABLE_ENTRY_ID table. This is the AMF specific Node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function finds the corresponding data structure for the given
 * instance and returns the pointer to the structure.
 *
 * Input:  cb        - AVD control block.
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 * NOTES: This function works in conjunction with extract function to provide the
 * get functionality.
 *
 * 
 **************************************************************************/

uns32 saamfnodetableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   SaNameT       node_name;
   AVD_AVND      *avnd;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }
   
   m_NCS_MEMSET(&node_name, '\0', sizeof(SaNameT));
   
   /* Prepare the nodename database key from the instant ID */
   node_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < node_name.length; i++)
   {
      node_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   avnd = avd_avnd_struc_find(avd_cb,node_name);

   if (avnd == AVD_AVND_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   
   *data = (NCSCONTEXT)avnd;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfnodetableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_AMF_NODE_TABLE_ENTRY_ID table. This is the  AMF specific node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after calling the get call to get data structure.
 * This function fills the value information in the param filed structure. For
 * octate information the buffer field will be used for filling the information.
 * MIBLIB will provide the memory and pointer to the buffer. For only objects that
 * have a direct value(i.e their offset is not 0 in VAR INFO) in the structure
 * the data field is filled using the VAR INFO provided by MIBLIB, for others based
 * on the OID the value is filled accordingly.
 *
 * Input:  param     -  param->i_param_id indicates the parameter to extract
 *                      The remaining elements of the param need to be filled
 *                      by the subystem's extract function
 *         var_info  - Pointer to the var_info structure for the param.
 *         data      - The pointer to the data-structure containing the object
 *                     value which we have already provided to MIBLIB from get call.
 *         buffer    - The buffer pointer provided by MIBLIB for filling the octate
 *                     type data.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 *
 * NOTES:  This function works in conjunction with other functions to provide the
 * get,getnext and getrow functionality.
 *
 * 
 **************************************************************************/

uns32 saamfnodetableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer)
{
   AVD_AVND      *avnd = (AVD_AVND *)data;

   if (avnd == AVD_AVND_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   switch(param->i_param_id)
   {
   case saAmfNodeSuFailoverProb_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,avnd->su_failover_prob);
      break;
   default:
      /* call the MIBLIB utility routine for standfard object types */
      if ((var_info != NULL) && (var_info->offset != 0))
         return ncsmiblib_get_obj_val(param, var_info, data, buffer);
      else
         return NCSCC_RC_NO_OBJECT;
      break;
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfnodetableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_NODE_TABLE_ENTRY_ID table. This is the  AMF specific node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for the objects that are settable. This same function can be used for test
 * operation also.
 *
 * Input:  cb        - AVD control block
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         var_info  - The VAR INFO structure pointer generated by MIBLIB for
 *                     the objects in this table.
 *         test_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 saamfnodetableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   SaNameT       node_name;
   AVD_AVND      *avnd;
   NCSMIBLIB_REQ_INFO temp_mib_req;
   NCS_BOOL      val_same_flag = FALSE;
   uns32         i;
   uns32         rc;
   SaTimeT       backup_time;
   uns32         back_val;
   AVSV_PARAM_INFO param;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_INV_VAL;  
   }
   
   m_NCS_MEMSET(&node_name, '\0', sizeof(SaNameT));
   
   /* Prepare the nodename database key from the instant ID */
   node_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < node_name.length; i++)
   {
      node_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   avnd = avd_avnd_struc_find(avd_cb,node_name);
   if (avnd == AVD_AVND_NULL)
   {
      if((arg->req.info.set_req.i_param_val.i_param_id == saAmfNodeRowStatus_ID)
         && (arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT))
      {
         /* Invalid row status object */
         return NCSCC_RC_INV_VAL;      
      }

      if(test_flag == TRUE)
      {
         return NCSCC_RC_SUCCESS;
      }

      m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

      avnd = avd_avnd_struc_crt(avd_cb, node_name, FALSE);

      m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

      if(avnd == AVD_AVND_NULL)
      {
         /* Invalid instance object */
         return NCSCC_RC_NO_INSTANCE; 
      }

   }else /* if (avnd == AVD_AVND_NULL) */
   {
      /* The record is already available */

      if(arg->req.info.set_req.i_param_val.i_param_id == saAmfNodeRowStatus_ID)
      {
         /* This is a row status operation */
         if (arg->req.info.set_req.i_param_val.info.i_int == (uns32)avnd->row_status)
         {
            /* row status object already set nothing to do. */
            return NCSCC_RC_SUCCESS;
         }

         switch(arg->req.info.set_req.i_param_val.info.i_int)
         {
         case NCS_ROW_ACTIVE:
            /* validate the structure to see if the row can be made active */
            
            if ((avnd->node_info.nodeId == 0) ||
               (avnd->su_failover_max == 0) ||
               (avnd->su_failover_prob == 0))
            {
               /* All the mandatory fields are not filled
                */
               /* log information error */
               return NCSCC_RC_INV_VAL;
            }

            if(test_flag == TRUE)
            {
               return NCSCC_RC_SUCCESS;
            }

            /* add the structure with nodeId as the index in the CB */
            m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

            rc = avd_avnd_struc_add_nodeid(avd_cb, avnd);

            m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

            /* set the value, checkpoint the entire record.
             */
            if( rc == NCSCC_RC_SUCCESS)
            {
               avnd->row_status = NCS_ROW_ACTIVE;
               m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(cb, avnd, AVSV_CKPT_AVD_AVND_CONFIG);

            }

            return rc;

         case NCS_ROW_NOT_IN_SERVICE:
         case NCS_ROW_DESTROY:
            
            /* check if it is active currently */

            if(avnd->row_status == NCS_ROW_ACTIVE)
            {

               /* Check to see that the node is in admin locked state before
                * making the row status as not in service or delete 
                */
               if((avnd->node_info.member) || 
                  (avnd->type == AVSV_AVND_CARD_SYS_CON) )
               {
                  return NCSCC_RC_INV_VAL;
               }

               if((avnd->node_info.member != SA_FALSE) ||
                  (avnd->su_admin_state != NCS_ADMIN_STATE_LOCK))
               {
                  /* log information error */
                  return NCSCC_RC_INV_VAL;
               }

               /* Check to see that no SUs exists on this node */
               if(avnd->list_of_su != AVD_SU_NULL)
               {
                  /* log information error */
                  return NCSCC_RC_INV_VAL;
               }

               if(test_flag == TRUE)
               {
                  return NCSCC_RC_SUCCESS;
               }

               /* Remove the node from the node_id tree of the CB. 
                * This is done for both NOT_IN_SERVICE and DESTROY
                * requests.
                */
               m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);
               
               avd_avnd_struc_rmv_nodeid(avd_cb, avnd);

               m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

               /* we need to delete this avnd structure on the
                * standby AVD.
                */

               /* check point to the standby AVD that this
                * record need to be deleted
                */
               m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(avd_cb, avnd, AVSV_CKPT_AVD_AVND_CONFIG);
            }

            if(test_flag == TRUE)
            {
               return NCSCC_RC_SUCCESS;
            }

            if(arg->req.info.set_req.i_param_val.info.i_int
                  == NCS_ROW_DESTROY)
            {
               /* delete and free the structure */
               m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);
               
               /* Remove the node from the node_id tree of the CB. */
               avd_avnd_struc_del(avd_cb,avnd);

               m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

               return NCSCC_RC_SUCCESS;
            }

            avnd->row_status = arg->req.info.set_req.i_param_val.info.i_int;
            return NCSCC_RC_SUCCESS;

            break;
         default:
            /* Invalid row status object */
            m_AVD_LOG_INVALID_VAL_ERROR(arg->req.info.set_req.i_param_val.info.i_int);
            return NCSCC_RC_INV_VAL;
            break;
         } /* switch(arg->req.info.set_req.i_param_val.info.i_int) */

      } /* if(arg->req.info.set_req.i_param_val.i_param_id == ncsNDRowStatus_ID) */

   } /* if (avnd == AVD_AVND_NULL) */

   if(test_flag == TRUE)
   {
      return NCSCC_RC_SUCCESS;
   }

   /* We now have the avnd block */
   
   if(avnd->row_status == NCS_ROW_ACTIVE)
   {
      if(avd_cb->init_state  != AVD_APP_STATE)
      {
         return NCSCC_RC_INV_VAL;
      }

      if(arg->req.info.set_req.i_param_val.i_param_id == saAmfNodeAdminState_ID)
      {
         if (arg->req.info.set_req.i_param_val.info.i_int == avnd->su_admin_state)
            return NCSCC_RC_SUCCESS;
         
         if ((avnd->su_admin_state == NCS_ADMIN_STATE_LOCK) && 
           (arg->req.info.set_req.i_param_val.info.i_int == NCS_ADMIN_STATE_SHUTDOWN))
            return NCSCC_RC_INV_VAL;
         
         if(avd_sg_app_node_admin_func(cb, avnd,arg->req.info.set_req.i_param_val.info.i_int)
            != NCSCC_RC_SUCCESS )
         {
            return NCSCC_RC_INV_VAL;
         }
         return NCSCC_RC_SUCCESS;
      }
      else if((arg->req.info.set_req.i_param_val.i_param_id == saAmfNodeSuFailoverProb_ID) ||
              (arg->req.info.set_req.i_param_val.i_param_id == saAmfNodeSuFailoverMax_ID))
      {  
         m_NCS_MEMSET(((uns8 *)&param),'\0',sizeof(AVSV_PARAM_INFO));
         param.table_id = NCSMIB_TBL_AVSV_AMF_NODE;
         param.obj_id = arg->req.info.set_req.i_param_val.i_param_id;   
         param.act = AVSV_OBJ_OPR_MOD;
         param.name_net = avnd->node_info.nodeName;
        
         if(param.obj_id == saAmfNodeSuFailoverProb_ID)
         {
            if(avnd->node_state != AVD_AVND_STATE_ABSENT)
            {
               backup_time = avnd->su_failover_prob;
               param.value_len = sizeof(SaTimeT);
               m_NCS_MEMCPY(&param.value[0], 
                            arg->req.info.set_req.i_param_val.info.i_oct, 
                            param.value_len);

               avnd->su_failover_prob = m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct); 

               if(avd_snd_op_req_msg(avd_cb,avnd,&param)!= NCSCC_RC_SUCCESS)
               {
                  avnd->su_failover_prob = backup_time;
                  /* don't do the operation now */
                  return NCSCC_RC_INV_VAL;
               }
            }
            else
            {
               avnd->su_failover_prob = m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct); 
            }
            
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVD_AVND_CONFIG);
            return NCSCC_RC_SUCCESS;
         }
         else
         {   
            if(avnd->node_state != AVD_AVND_STATE_ABSENT)
            {
               back_val = avnd->su_failover_max;
               param.value_len = sizeof(uns32);
               m_NCS_OS_HTONL_P(&param.value[0], arg->req.info.set_req.i_param_val.info.i_int);
               avnd->su_failover_max = arg->req.info.set_req.i_param_val.info.i_int;

               if(avd_snd_op_req_msg(avd_cb,avnd,&param)!= NCSCC_RC_SUCCESS)
               {
                  avnd->su_failover_max = back_val;
                  /* don't do the operation now */
                  return NCSCC_RC_INV_VAL;
               }
            }
            else
            {
               avnd->su_failover_max = arg->req.info.set_req.i_param_val.info.i_int;
            }

            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, avnd, AVSV_CKPT_AVD_AVND_CONFIG);
            return NCSCC_RC_SUCCESS;
         }
      }
      /* when row status is active we don't allow any other MIB object to be
       * modified.
       */
      return NCSCC_RC_INV_VAL;
   }  /* if(avnd->row_status == NCS_ROW_ACTIVE) */


   switch(arg->req.info.set_req.i_param_val.i_param_id)
   {
   case saAmfNodeRowStatus_ID:
      /* fill the row status value */
      if(arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT)
      {
         avnd->row_status = arg->req.info.set_req.i_param_val.info.i_int;
      }
      break;
   case saAmfNodeSuFailoverProb_ID:
      /* fill the probation period value */
      avnd->su_failover_prob = m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
      break;
   default:
      m_NCS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

      temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
      temp_mib_req.info.i_set_util_info.param = &(arg->req.info.set_req.i_param_val);
      temp_mib_req.info.i_set_util_info.var_info = var_info;
      temp_mib_req.info.i_set_util_info.data = avnd;
      temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

      /* call the mib routine handler */ 
      if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
      {
         return rc;
      }

     /* check to verify that the admin object is not shutdown
      * if shutdown change to LOCK.
      */
      if (avnd->su_admin_state == NCS_ADMIN_STATE_SHUTDOWN)
      {
         avnd->su_admin_state = NCS_ADMIN_STATE_LOCK;
      }
      break;
   } /* switch(arg->req.info.set_req.i_param_val.i_param_id) */
   
   return NCSCC_RC_SUCCESS;
}
  

/*****************************************************************************
 * Function: saamfnodetableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_NODE_TABLE_ENTRY_ID table. This is the AMF specific node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function gets the next valid instance and its data structure
 * and it passes to the MIBLIB the values.
 *
 * Input: cb        - AVD control block.
 *        arg       - The pointer to the MIB arg that was provided by the caller.
 *        data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 *     next_inst_id - The next instance id will be filled in this buffer
 *                     and returned by reference.
 * next_inst_id_len - The next instance id length.
 *
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context
 *
 * NOTES: This function works in conjunction with extract function to provide the
 * getnext functionality.
 *
 * 
 **************************************************************************/

uns32 saamfnodetableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_AVND      *avnd;
   SaNameT       node_name;
   uns32         i;

   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }
   
   m_NCS_MEMSET(&node_name, '\0', sizeof(SaNameT));

   if (arg->i_idx.i_inst_len != 0)
   {
      /* Prepare the nodename database key from the instant ID */
      node_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
      for(i = 0; i < node_name.length; i++)
      {
         node_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
   }
   avnd = avd_avnd_struc_find_next(avd_cb,node_name);

   if (avnd == AVD_AVND_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the instant ID from the node_name */

   *next_inst_id_len = m_NCS_OS_NTOHS(avnd->node_info.nodeName.length) + 1;
   next_inst_id[0] = *next_inst_id_len -1;

   for(i = 0; i < next_inst_id[0]; i++)
   {
      next_inst_id[i + 1] = (uns32)(avnd->node_info.nodeName.value[i]);
   }

   *data = (NCSCONTEXT)avnd;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfnodetableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_NODE_TABLE_ENTRY_ID table. This is the AMF specific node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for all the objects that are settable as part of the setrow operation. 
 * This same function can be used for test row
 * operation also.
 *
 * Input:  cb        - AVD control block
 *         args      - The pointer to the MIB arg that was provided by the caller.
 *         params    - The List of object ids and their values.
 *         obj_info  - The VAR INFO structure array pointer generated by MIBLIB for
 *                     the objects in this table.
 *      testrow_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 saamfnodetableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}



/*****************************************************************************
 * Function: saclmnodetableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_CLM_NODE_TABLE_ENTRY_ID table. This is the CLM specific Node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function finds the corresponding data structure for the given
 * instance and returns the pointer to the structure.
 *
 * Input:  cb        - AVD control block.
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 * NOTES: This function works in conjunction with extract function to provide the
 * get functionality.
 *
 * 
 **************************************************************************/

uns32 saclmnodetableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   SaNameT       node_name;
   AVD_AVND      *avnd;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }
   
   m_NCS_MEMSET(&node_name, '\0', sizeof(SaNameT));
   
   /* Prepare the nodename database key from the instant ID */
   node_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < node_name.length; i++)
   {
      node_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   avnd = avd_avnd_struc_find(avd_cb,node_name);

   if (avnd == AVD_AVND_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   
   *data = (NCSCONTEXT)avnd;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saclmnodetableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_CLM_NODE_TABLE_ENTRY_ID table. This is the  CLM specific node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after calling the get call to get data structure.
 * This function fills the value information in the param filed structure. For
 * octate information the buffer field will be used for filling the information.
 * MIBLIB will provide the memory and pointer to the buffer. For only objects that
 * have a direct value(i.e their offset is not 0 in VAR INFO) in the structure
 * the data field is filled using the VAR INFO provided by MIBLIB, for others based
 * on the OID the value is filled accordingly.
 *
 * Input:  param     -  param->i_param_id indicates the parameter to extract
 *                      The remaining elements of the param need to be filled
 *                      by the subystem's extract function
 *         var_info  - Pointer to the var_info structure for the param.
 *         data      - The pointer to the data-structure containing the object
 *                     value which we have already provided to MIBLIB from get call.
 *         buffer    - The buffer pointer provided by MIBLIB for filling the octate
 *                     type data.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 *
 * NOTES:  This function works in conjunction with other functions to provide the
 * get,getnext and getrow functionality.
 *
 * 
 **************************************************************************/

uns32 saclmnodetableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer)
{
   AVD_AVND      *avnd = (AVD_AVND *)data;

   if (avnd == AVD_AVND_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   switch(param->i_param_id)
   {
   case saClmNodeHPIEntityPath_ID:
      m_AVSV_OCTVAL_TO_PARAM(param, buffer, avnd->hpi_entity_path.length,
                             avnd->hpi_entity_path.value);
      break;
   case saClmNodeAddress_ID:
      m_AVSV_LENVAL_TO_PARAM(param,buffer,avnd->node_info.nodeAddress);
      break;
   case saClmNodeBootTimeStamp_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,avnd->node_info.bootTimestamp);
      break;
   case saClmNodeInitialViewNumber_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,avnd->node_info.initialViewNumber);
      break;
   case saClmNodeIsMember_ID:
      param->i_fmat_id = NCSMIB_FMAT_INT; 
      param->i_length = 1; 
      param->info.i_int = (avnd->node_info.member == 0) ? NCS_SNMP_FALSE:NCS_SNMP_TRUE;
      break;
   default:
      /* call the MIBLIB utility routine for standfard object types */
      if ((var_info != NULL) && (var_info->offset != 0))
         return ncsmiblib_get_obj_val(param, var_info, data, buffer);
      else
         return NCSCC_RC_NO_OBJECT;
      break;
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saclmnodetableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_CLM_NODE_TABLE_ENTRY_ID table. This is the  CLM specific node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for the objects that are settable. This same function can be used for test
 * operation also.
 *
 * Input:  cb        - AVD control block
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         var_info  - The VAR INFO structure pointer generated by MIBLIB for
 *                     the objects in this table.
 *         test_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 saclmnodetableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{
  
   return NCSCC_RC_FAILURE;
}



/*****************************************************************************
 * Function: saclmnodetableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_CLM_NODE_TABLE_ENTRY_ID table. This is the CLM specific node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function gets the next valid instance and its data structure
 * and it passes to the MIBLIB the values.
 *
 * Input: cb        - AVD control block.
 *        arg       - The pointer to the MIB arg that was provided by the caller.
 *        data      - The pointer to the data-structure containing the object
 *                     value is returned by reference.
 *     next_inst_id - The next instance id will be filled in this buffer
 *                     and returned by reference.
 * next_inst_id_len - The next instance id length.
 *
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context
 *
 * NOTES: This function works in conjunction with extract function to provide the
 * getnext functionality.
 *
 * 
 **************************************************************************/

uns32 saclmnodetableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_AVND      *avnd = AVD_AVND_NULL;
   SaNameT       node_name;
   uns32         i;

   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }
   
   m_NCS_MEMSET(&node_name, '\0', sizeof(SaNameT));
   
   if (arg->i_idx.i_inst_len != 0)
   {
      /* Prepare the nodename database key from the instant ID */
      node_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
      for(i = 0; i < node_name.length; i++)
      {
         node_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
   }

   avnd = avd_avnd_struc_find_next_cl_member(avd_cb,node_name);

   if (avnd == AVD_AVND_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the instant ID from the node_name */

   *next_inst_id_len = m_NCS_OS_NTOHS(avnd->node_info.nodeName.length) + 1;
   next_inst_id[0] = *next_inst_id_len -1;

   for(i = 0; i < next_inst_id[0]; i++)
   {
      next_inst_id[i + 1] = (uns32)(avnd->node_info.nodeName.value[i]);
   }

   *data = (NCSCONTEXT)avnd;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saclmnodetableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_CLM_NODE_TABLE_ENTRY_ID table. This is the CLM specific node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for all the objects that are settable as part of the setrow operation. 
 * This same function can be used for test row
 * operation also.
 *
 * Input:  cb        - AVD control block
 *         args      - The pointer to the MIB arg that was provided by the caller.
 *         params    - The List of object ids and their values.
 *         obj_info  - The VAR INFO structure array pointer generated by MIBLIB for
 *                     the objects in this table.
 *      testrow_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

uns32 saclmnodetableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}

/*****************************************************************************
 * Function: saclmnodetableentry_rmvrow
 *
 * Purpose:  This function is one of the RMVROW processing routines for objects
 * in NCS_CLM_TABLE_ENTRY_ID table. 
 *
 * Input:  cb        - AVD control block.
 *         idx       - pointer to NCSMIB_IDX 
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *                   to set the args->rsp.i_status field before returning the
 *                   NCSMIB_ARG to the caller's context
 **************************************************************************/
uns32 saclmnodetableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx) 
{
   return NCSCC_RC_SUCCESS;
}
