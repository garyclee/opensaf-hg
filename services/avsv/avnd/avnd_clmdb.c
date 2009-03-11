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
   database on the AVND. It also deals with all the MIB operations
   like get,getnext etc related to the CLM Node Table.
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
   uns32       rc = NCSCC_RC_SUCCESS;

   /* timestamp this node */
   m_GET_TIME_STAMP(clmdb->node_info.bootTimestamp);

   /* initialize the clm dll list */
   clmdb->clm_list.order = NCS_DBLIST_ASSCEND_ORDER;
   clmdb->clm_list.cmp_cookie = avsv_dblist_uns32_cmp;
   clmdb->clm_list.free_cookie = avnd_clmdb_rec_free;

   m_AVND_LOG_CLM_DB(AVND_LOG_CLM_DB_CREATE, AVND_LOG_CLM_DB_SUCCESS, 
                     0, NCSFL_SEV_INFO);

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
   AVND_CLM_DB  *clmdb = &cb->clmdb;
   AVND_CLM_REC *rec = 0;
   uns32        rc = NCSCC_RC_SUCCESS;

   /* traverse & delete all the CLM records */
   while ( 0 != (rec = 
                (AVND_CLM_REC *)m_NCS_DBLIST_FIND_FIRST(&clmdb->clm_list)) )
   {
      rc = avnd_clmdb_rec_del(cb, rec->info.node_id);
      if ( NCSCC_RC_SUCCESS != rc ) goto err;
   }

   /* traverse & delete all the CLM track request records TBD */

   m_AVND_LOG_CLM_DB(AVND_LOG_CLM_DB_DESTROY, AVND_LOG_CLM_DB_SUCCESS, 
                     0, NCSFL_SEV_INFO);
   return rc;

err:
   m_AVND_LOG_CLM_DB(AVND_LOG_CLM_DB_DESTROY, AVND_LOG_CLM_DB_FAILURE, 
                     0, NCSFL_SEV_CRITICAL);
   return rc;
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
   AVND_CLM_DB  *clmdb = &cb->clmdb;
   AVND_CLM_REC *rec = 0;
   uns32        rc = NCSCC_RC_SUCCESS;

   /* get the record, if any */
   rec = avnd_clmdb_rec_get(cb, node_info->node_id);
   if (!rec) 
   {
      /* a new record.. alloc & link it to the dll */
      rec = m_MMGR_ALLOC_AVND_CLM_REC;
      if (rec)
      {
         memset(rec, 0, sizeof(AVND_CLM_REC));

         /* update the record key */
         rec->info.node_id = node_info->node_id;
         rec->clm_dll_node.key = (uns8 *)&rec->info.node_id;

         rc = ncs_db_link_list_add(&clmdb->clm_list, &rec->clm_dll_node);
         if (NCSCC_RC_SUCCESS != rc) goto done;
      }
      else 
      {
         rc = NCSCC_RC_FAILURE;
         goto done;
      }
   }

   /* update the params */
   rec->info.node_address = node_info->node_address;
   rec->info.node_name_net = node_info->node_name_net;
   rec->info.member = node_info->member;
   rec->info.boot_timestamp = node_info->boot_timestamp;
   rec->info.view_number = node_info->view_number;

   m_AVND_LOG_CLM_DB(AVND_LOG_CLM_DB_REC_ADD, AVND_LOG_CLM_DB_SUCCESS, 
                     node_info->node_id, NCSFL_SEV_INFO);

done:
   if (NCSCC_RC_SUCCESS != rc)
   {
      m_AVND_LOG_CLM_DB(AVND_LOG_CLM_DB_REC_ADD, AVND_LOG_CLM_DB_FAILURE, 
                        node_info->node_id, NCSFL_SEV_INFO);
      if (rec)
      {
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
   AVND_CLM_DB  *clmdb = &cb->clmdb;
   uns32        rc = NCSCC_RC_SUCCESS;

   rc = ncs_db_link_list_del(&clmdb->clm_list, (uns8 *)&node_id);

   if (NCSCC_RC_SUCCESS == rc)
      m_AVND_LOG_CLM_DB(AVND_LOG_CLM_DB_REC_DEL, AVND_LOG_CLM_DB_SUCCESS, 
                        node_id, NCSFL_SEV_INFO);
   else
      m_AVND_LOG_CLM_DB(AVND_LOG_CLM_DB_REC_DEL, AVND_LOG_CLM_DB_FAILURE, 
                        node_id, NCSFL_SEV_INFO);

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
   AVND_CLM_DB  *clmdb = &cb->clmdb;

   return (AVND_CLM_REC *)ncs_db_link_list_find(&clmdb->clm_list, 
                                               (uns8 *)&node_id);
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

   m_MMGR_FREE_AVND_CLM_REC(rec);

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
   AVND_CLM_DB       *db = &cb->clmdb;
   
   if(!trk_info)
   {
      /* LOG ERROR */
      return NCSCC_RC_FAILURE;  
   }

   if(db->clm_trk_info == NULL)
   {
      /* This is the first track info add it in the front */
      db->clm_trk_info = trk_info;
      trk_info->next = NULL;
   }
   else
   {
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

AVND_CLM_TRK_INFO * avnd_clm_trkinfo_list_del(AVND_CB *cb, SaClmHandleT hdl,
                                              MDS_DEST *dest)
{
   AVND_CLM_TRK_INFO *i_ptr = NULL;
   AVND_CLM_TRK_INFO **p_ptr;
   AVND_CLM_DB       *db = &cb->clmdb;
   
   if(!hdl)
   {
      /* LOG ERROR */
      return NULL;  
   }

   i_ptr = db->clm_trk_info;
   p_ptr = &(db->clm_trk_info);
   while(i_ptr != NULL)
   {
      if((i_ptr->req_hdl == hdl) &&
         (memcmp(&i_ptr->mds_dest, dest, sizeof(MDS_DEST)) == 0) )
      {
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

AVND_CLM_TRK_INFO * avnd_clm_trkinfo_list_find(AVND_CB *cb, SaClmHandleT hdl,
                                               MDS_DEST *dest)
{
   AVND_CLM_TRK_INFO *i_ptr = NULL;
   AVND_CLM_DB       *db = &cb->clmdb;
   
   if( (!db->clm_trk_info) || (!hdl) )
   {
      /* LOG ERROR */
      return NULL;  
   }

   i_ptr = db->clm_trk_info;
   while( (i_ptr != NULL) && 
          (i_ptr->req_hdl != hdl) && 
          (memcmp(&i_ptr->mds_dest, dest, sizeof(MDS_DEST)) != 0) )
   {
      i_ptr = i_ptr->next;
   }

   if((i_ptr != NULL) && (i_ptr->req_hdl == hdl) &&
          (memcmp(&i_ptr->mds_dest, dest, sizeof(MDS_DEST)) == 0) )
   {
      return i_ptr;
   }
   
   return NULL;
}


/*****************************************************************************
 * Function: ncssndtableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in NCS_S_N_D_TABLE_ENTRY_ID table. This is the status node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function finds the corresponding data structure for the given
 * instance and returns the pointer to the structure.
 *
 * Input:  cb        - AVND control block.
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
uns32 ncssndtableentry_get(NCSCONTEXT cb,
                           NCSMIB_ARG *arg, 
                           NCSCONTEXT* data)
{
   AVND_CB      *avnd_cb = (AVND_CB *)cb;
   SaClmNodeIdT node_id;

   if (avnd_cb->admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* get the node id from the instant ID */
   node_id = arg->i_idx.i_inst_ids[0];
  
   /* Check whether the Node Id matches with avnd nodeId */ 
   if (avnd_cb->clmdb.node_info.nodeId != node_id)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }
       
   *data = (NCSCONTEXT)cb;
     
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: ncssndtableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * NCS_S_N_D_TABLE_ENTRY_ID table. This is the status node table. The
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
uns32 ncssndtableentry_extract(NCSMIB_PARAM_VAL *param, 
                               NCSMIB_VAR_INFO *var_info,
                               NCSCONTEXT data,
                               NCSCONTEXT buffer)
{
   /* No special processing is required for the objects in this table,
    * call the MIBLIB utility routine for standard object types 
    */
   if ((var_info != NULL) && (var_info->offset != 0))
      return ncsmiblib_get_obj_val(param, var_info, data, buffer);
   else
      return NCSCC_RC_NO_OBJECT;
                                       
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: ncssndtableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * NCS_S_N_D_TABLE_ENTRY_ID table. This is the status node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for the objects that are settable. This same function can be used for test
 * operation also.
 *
 * Input:  cb        - AVND control block
 *         arg       - The pointer to the MIB arg that was provided by the caller.
 *         var_info  - The VAR INFO structure pointer generated by MIBLIB for
 *                     the objects in this table.
 *         test_flag - The flag that indicates if this is set or test.
 *
 * Returns: The status returned by the operation. MIB lib will use it
 *          to set the args->rsp.i_status field before returning the
 *          NCSMIB_ARG to the caller's context.
 *
 * NOTES: This table currently doesn't have any set objects so this function
 *        shouldn't be called. If called will return invalid error.
 *
 * 
 **************************************************************************/
uns32 ncssndtableentry_set(NCSCONTEXT cb, 
                           NCSMIB_ARG *arg, 
                           NCSMIB_VAR_INFO *var_info,
                           NCS_BOOL test_flag)
{
   /* This table has only read-only objects */ 
   return NCSCC_RC_INV_VAL;
}


/*****************************************************************************
 * Function: ncssndtableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * NCS_S_N_D_TABLE_ENTRY_ID table. This is the status node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function gets the next valid instance and its data structure
 * and it passes to the MIBLIB the values.
 *
 * Input: cb        - AVND control block.
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
uns32 ncssndtableentry_next(NCSCONTEXT cb,
                            NCSMIB_ARG *arg, 
                            NCSCONTEXT *data,
                            uns32 *next_inst_id,
                            uns32 *next_inst_id_len)
{
   AVND_CB      *avnd_cb = (AVND_CB *)cb;
   SaClmNodeIdT node_id = 0;

   if (avnd_cb->admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the node id from the instant ID */
   if (arg->i_idx.i_inst_len != 0)
   {
      /* get the node id from the instant ID */
      node_id = arg->i_idx.i_inst_ids[0];
   }
  
   /* Check whether the Node Id matches with avnd nodeId */ 
   if (avnd_cb->clmdb.node_info.nodeId <= node_id)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the instant ID from the nod id */

   *next_inst_id_len = 1;
   next_inst_id[0] = (uns32)(avnd_cb->clmdb.node_info.nodeId);

   *data = (NCSCONTEXT)avnd_cb;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: ncssndtableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * NCS_S_N_D_TABLE_ENTRY_ID table.This is the status node table. The
 * name of this function is generated by the MIBLIB tool. This function
 * will be called by MIBLIB after validating the arg information.
 * This function does the set of the object and the corresponding actions
 * for all the objects that are settable as part of the setrow operation. 
 * This same function can be used for test row
 * operation also.
 *
 * Input:  cb        - AVND control block
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
 * NOTES: This table currently doesn't have any set objects so this function
 *        shouldn't be called. If called will return invalid error.
 *
 * 
 **************************************************************************/
uns32 ncssndtableentry_setrow(NCSCONTEXT cb,
                              NCSMIB_ARG *args,
                              NCSMIB_SETROW_PARAM_VAL *params,
                              struct ncsmib_obj_info *obj_info,
                              NCS_BOOL testrow_flag)
{
   /* This table has only read-only objects */ 
   return NCSCC_RC_INV_VAL;
}

/*****************************************************************************
 * Function: ncssndtableentry_rmvrow
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
uns32 ncssndtableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx)
{
   return NCSCC_RC_SUCCESS;
}

