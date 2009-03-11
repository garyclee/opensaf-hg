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
   This module deals with the creation, accessing and deletion of
   the SU database on the AVND. It also deals with all the MIB operations
   like get,getnext etc related to the SU table.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

#include "avnd.h"


/****************************************************************************
  Name          : avnd_sudb_init
 
  Description   : This routine initializes the SU database.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_sudb_init(AVND_CB *cb)
{
   NCS_PATRICIA_PARAMS params;
   uns32               rc = NCSCC_RC_SUCCESS;

   m_NCS_OS_MEMSET(&params, 0, sizeof(NCS_PATRICIA_PARAMS));

   params.key_size = sizeof(SaNameT);
   rc = ncs_patricia_tree_init(&cb->sudb, &params);
   if (NCSCC_RC_SUCCESS == rc)
      m_AVND_LOG_SU_DB(AVND_LOG_SU_DB_CREATE, AVND_LOG_SU_DB_SUCCESS, 
                       0, 0, NCSFL_SEV_INFO);
   else
      m_AVND_LOG_SU_DB(AVND_LOG_SU_DB_CREATE, AVND_LOG_SU_DB_FAILURE, 
                       0, 0, NCSFL_SEV_CRITICAL);

   return rc;
}


/****************************************************************************
  Name          : avnd_sudb_destroy
 
  Description   : This routine destroys the SU database. It deletes all the 
                  SU records in the database.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_sudb_destroy(AVND_CB *cb)
{
   AVND_SU *su = 0;
   uns32   rc = NCSCC_RC_SUCCESS;

   /* scan & delete each su */
   while ( 0 != (su = 
            (AVND_SU *)ncs_patricia_tree_getnext(&cb->sudb, (uns8 *)0)) )
   {
      /* unreg the row from mab */
      avnd_mab_unreg_tbl_rows(cb, NCSMIB_TBL_AVSV_NCS_SU_STAT, su->mab_hdl,
                     (su->su_is_external?cb->avnd_mbcsv_mab_hdl:cb->mab_hdl));

      /* delete the record 
      m_AVND_SEND_CKPT_UPDT_ASYNC_RMV(cb, su, AVND_CKPT_SU_CONFIG);
      AvND is going down, but don't send any async update even for 
      external components, otherwise external components will be deleted
      from ACT.*/
      rc = avnd_sudb_rec_del(cb, &su->name_net);
      if ( NCSCC_RC_SUCCESS != rc ) goto err;
   }

   /* finally destroy patricia tree */
   rc = ncs_patricia_tree_destroy(&cb->sudb);
   if ( NCSCC_RC_SUCCESS != rc ) goto err;

   m_AVND_LOG_SU_DB(AVND_LOG_SU_DB_DESTROY, AVND_LOG_SU_DB_SUCCESS, 
                    0, 0, NCSFL_SEV_INFO);
   return rc;

err:
   m_AVND_LOG_SU_DB(AVND_LOG_SU_DB_DESTROY, AVND_LOG_SU_DB_FAILURE, 
                    0, 0, NCSFL_SEV_CRITICAL);
   return rc;
}


/****************************************************************************
  Name          : avnd_sudb_rec_add
 
  Description   : This routine adds a SU record to the SU database. If a SU 
                  is already present, nothing is done.
 
  Arguments     : cb   - ptr to the AvND control block
                  info - ptr to the SU parameters
                  rc   - ptr to the operation result
 
  Return Values : ptr to the SU record, if success
                  0, otherwise
 
  Notes         : None.
******************************************************************************/
AVND_SU *avnd_sudb_rec_add(AVND_CB *cb, AVND_SU_PARAM *info, uns32 *rc)
{
   AVND_SU *su = 0;

   /* verify if this su is already present in the db */
   if ( 0 != m_AVND_SUDB_REC_GET(cb->sudb, info->name_net) )
   {
      *rc = AVND_ERR_DUP_SU;
      goto err;
   }

   /* a fresh su... */
   su = m_MMGR_ALLOC_AVND_SU;
   if (!su)
   {
      *rc = AVND_ERR_NO_MEMORY;
      goto err;
   }

   m_NCS_OS_MEMSET(su, 0, sizeof(AVND_SU));

   /*
    * Update the config parameters.
    */
   /* update the su-name (patricia key) */
   memcpy(&su->name_net, &info->name_net, sizeof(SaNameT));

   /* update error recovery escalation parameters */
   su->comp_restart_prob = info->comp_restart_prob;
   su->comp_restart_max = info->comp_restart_max;
   su->su_restart_prob = info->su_restart_prob;
   su->su_restart_max = info->su_restart_max;
   su->su_err_esc_level = AVND_ERR_ESC_LEVEL_0;

   /* update the NCS flag */
   su->is_ncs = info->is_ncs;
   su->su_is_external = info->su_is_external;

   /*
    * Update the rest of the parameters with default values.
    */
   m_AVND_SU_OPER_STATE_SET_AND_SEND_TRAP(cb, su, NCS_OPER_STATE_ENABLE);
   su->pres = NCS_PRES_UNINSTANTIATED;
   su->avd_updt_flag = FALSE;

   /* 
    * Initialize the comp-list.
    */
   su->comp_list.order = NCS_DBLIST_ASSCEND_ORDER;
   su->comp_list.cmp_cookie = avsv_dblist_uns32_cmp;
   su->comp_list.free_cookie = 0;

   /* 
    * Initialize the si-list.
    */
   su->si_list.order = NCS_DBLIST_ASSCEND_ORDER;
   su->si_list.cmp_cookie = avsv_dblist_saname_cmp;
   su->si_list.free_cookie = 0;

   /* 
    * Initialize the siq.
    */
   su->siq.order = NCS_DBLIST_ANY_ORDER;
   su->siq.cmp_cookie = 0;
   su->siq.free_cookie = 0;

   /* create the association with hdl-mngr */
   if ( (0 == (su->su_hdl = ncshm_create_hdl(cb->pool_id, NCS_SERVICE_ID_AVND,
                                             (NCSCONTEXT)su))) )
   {
      *rc = AVND_ERR_HDL;
      goto err;
   }

   /* 
    * Add to the patricia tree.
    */
   su->tree_node.bit = 0;
   su->tree_node.key_info = (uns8*)&su->name_net;
   *rc = ncs_patricia_tree_add(&cb->sudb, &su->tree_node);
   if ( NCSCC_RC_SUCCESS != *rc )
   {
      *rc = AVND_ERR_TREE;
      goto err;
   }

   m_AVND_LOG_SU_DB(AVND_LOG_SU_DB_REC_ADD, AVND_LOG_SU_DB_SUCCESS, 
                    &info->name_net, 0, NCSFL_SEV_INFO);
   return su;

err:
   if (su) 
   {
      if (su->su_hdl) ncshm_destroy_hdl(NCS_SERVICE_ID_AVND, su->su_hdl);
      m_MMGR_FREE_AVND_SU(su);
   }

   m_AVND_LOG_SU_DB(AVND_LOG_SU_DB_REC_ADD, AVND_LOG_SU_DB_FAILURE, 
                    &info->name_net, 0, NCSFL_SEV_CRITICAL);
   return 0;
}


/****************************************************************************
  Name          : avnd_sudb_rec_del
 
  Description   : This routine deletes a SU record from the SU database.
 
  Arguments     : cb       - ptr to the AvND control block
                  name_net - ptr to the su-name (in n/w order)
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : All the components belonging to this SU should have been 
                  deleted before deleting the SU record. Also no SIs should be
                  attached to this record.
******************************************************************************/
uns32 avnd_sudb_rec_del(AVND_CB *cb, SaNameT *name_net)
{
   AVND_SU *su = 0;
   uns32   rc = NCSCC_RC_SUCCESS;

   /* get the su record */
   su = m_AVND_SUDB_REC_GET(cb->sudb, *name_net);
   if (!su)
   {
      rc = AVND_ERR_NO_SU;
      goto err;
   }

   /* su should not have any comp or si attached to it */
   m_AVSV_ASSERT( (!su->comp_list.n_nodes) && (!su->si_list.n_nodes) );

   /* remove from the patricia tree */
   rc = ncs_patricia_tree_del(&cb->sudb, &su->tree_node);
   if ( NCSCC_RC_SUCCESS != rc )
   {
      rc = AVND_ERR_TREE;
      goto err;
   }

   /* remove the association with hdl mngr */
   ncshm_destroy_hdl(NCS_SERVICE_ID_AVND, su->su_hdl);

   m_AVND_LOG_SU_DB(AVND_LOG_SU_DB_REC_DEL, AVND_LOG_SU_DB_SUCCESS, 
                    name_net, 0, NCSFL_SEV_INFO);

   /* free the memory */
   m_MMGR_FREE_AVND_SU(su);

   return rc;

err:
   m_AVND_LOG_SU_DB(AVND_LOG_SU_DB_REC_DEL, AVND_LOG_SU_DB_FAILURE, 
                    name_net, 0, NCSFL_SEV_CRITICAL);
   return rc;
}


/*****************************************************************************
 * Function: ncsssutableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in NCS_S_S_U_TABLE_ENTRY_ID table. This is the status SU table. The
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
uns32 ncsssutableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT* data)
{
   AVND_CB *avnd_cb = (AVND_CB *)cb;
   AVND_SU *su = NULL;
   SaNameT su_name;
   uns32   i;


   if (avnd_cb->admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_INV_VAL;
   }

   m_NCS_OS_MEMSET(&su_name, 0, sizeof(SaNameT));
   
   su_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for (i = 0; i < su_name.length; i++)
   {
      su_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }
    
   /* Check if an SU with this name exists */
   su_name.length = m_NCS_OS_HTONS(su_name.length);
   su = m_AVND_SUDB_REC_GET(avnd_cb->sudb, su_name);
   su_name.length = m_NCS_OS_NTOHS(su_name.length);

   if (su == AVND_SU_NULL)
   {
      /* Row not exists */
      return NCSCC_RC_NO_INSTANCE;
   }
                      
   *data = (NCSCONTEXT)su;      
                                
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: ncsssutableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * NCS_S_S_U_TABLE_ENTRY_ID table. This is the status SU table. The
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
uns32 ncsssutableentry_extract(NCSMIB_PARAM_VAL *param, 
                               NCSMIB_VAR_INFO *var_info,
                               NCSCONTEXT data,
                               NCSCONTEXT buffer)
{
   AVND_SU *su = (AVND_SU *)data;

   if (su == AVND_SU_NULL)
   {
      /* Row doesn't exists */
      return NCSCC_RC_NO_INSTANCE;
   }

   switch(param->i_param_id)
   {
   default:
       /* call the MIBLIB utility routine for standard object types */
      if ((var_info != NULL) && (var_info->offset != 0))
         return ncsmiblib_get_obj_val(param, var_info, data, buffer);
      else
         return NCSCC_RC_NO_OBJECT;
       break;
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: ncsssutableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * NCS_S_S_U_TABLE_ENTRY_ID table. This is the status SU table. The
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
uns32 ncsssutableentry_set(NCSCONTEXT cb,
                           NCSMIB_ARG *arg, 
                           NCSMIB_VAR_INFO *var_info,
                           NCS_BOOL test_flag)
{
   /* This table has only read-only objects */ 
   return NCSCC_RC_INV_VAL;
}



/*****************************************************************************
 * Function: ncsssutableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * NCS_S_S_U_TABLE_ENTRY_ID table. This is the status SU table. The
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
uns32 ncsssutableentry_next(NCSCONTEXT cb,
                            NCSMIB_ARG *arg, 
                            NCSCONTEXT* data, 
                            uns32* next_inst_id,
                            uns32 *next_inst_id_len)
{
   AVND_CB *avnd_cb = (AVND_CB *)cb;
   AVND_SU *su = NULL;
   SaNameT su_name;
   uns32   i;

   if (avnd_cb->admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_INV_VAL;
   }

   m_NCS_OS_MEMSET(&su_name, 0, sizeof(SaNameT));
  
   /* Prepare the SU name from the instant ID */
   if (arg->i_idx.i_inst_len != 0)
   { 
       su_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
       for (i = 0; i < su_name.length; i++)
       {
          su_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
       }
   }
    
   /* Check if an SU with this name exists */
   su_name.length = m_NCS_OS_HTONS(su_name.length);
   su = m_AVND_SUDB_REC_GET_NEXT(avnd_cb->sudb, su_name);
   if (su == AVND_SU_NULL)
   {
      /* Row not exists */
      return NCSCC_RC_NO_INSTANCE;
   }
   
   /* Prepare the instant ID from the su name */
   su_name = su->name_net;
   su_name.length = m_NCS_OS_NTOHS(su_name.length);

   *next_inst_id_len = su_name.length + 1;

   next_inst_id[0] = su_name.length;
   for(i = 0; i < su_name.length; i++)
      next_inst_id[i + 1] = (uns32)(su_name.value[i]);

   *data = (NCSCONTEXT)su;      
                                
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: ncsssutableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * NCS_S_S_U_TABLE_ENTRY_ID table.This is the status SU table. The
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
uns32 ncsssutableentry_setrow(NCSCONTEXT cb,
                              NCSMIB_ARG* args,
                              NCSMIB_SETROW_PARAM_VAL* params,
                              struct ncsmib_obj_info* obj_info,
                              NCS_BOOL testrow_flag)
{
   /* This table has only read-only objects */ 
   return NCSCC_RC_INV_VAL;
}

/*****************************************************************************
 * Function: ncsssutableentry_rmvrow
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
uns32 ncsssutableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx)
{
   return NCSCC_RC_SUCCESS;
}

