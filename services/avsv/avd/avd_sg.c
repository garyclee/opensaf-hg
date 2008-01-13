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
  the SG database on the AVD. It also deals with all the MIB operations
  like set,get,getnext etc related to the SG table.


..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_sg_struc_crt - creates and adds SG structure to database.
  avd_sg_struc_find - Finds SG structure from the database.
  avd_sg_struc_find_next - Finds the next SG structure from the database.  
  avd_sg_struc_del - deletes and frees SG structure from database.
  ncssgtableentry_get - This function is one of the get processing
                        routines for objects in NCS_S_G_TABLE_ENTRY_ID table.
  ncssgtableentry_extract - This function is one of the get processing
                            function for objects in NCS_S_G_TABLE_ENTRY_ID
                            table.
  ncssgtableentry_set - This function is the set processing for objects in
                         NCS_S_G_TABLE_ENTRY_ID table.
  ncssgtableentry_next - This function is the next processing for objects in
                         NCS_S_G_TABLE_ENTRY_ID table.
  ncssgtableentry_setrow - This function is the setrow processing for
                           objects in NCS_S_G_TABLE_ENTRY_ID table.
  ncssgtableentry_rmvrow - This function is the remove row processing for
                           objects in NCS_S_G_TABLE_ENTRY_ID table.
  saamfsgtableentry_get - This function is one of the get processing
                          routines for objects in NCSMIB_TBL_AVSV_AMF_SG table.
  saamfsgtableentry_extract - This function is one of the get processing
                          routines for objects in NCSMIB_TBL_AVSV_AMF_SG table.
  saamfsgtableentry_set - This function is the set processing for objects in
                          NCSMIB_TBL_AVSV_AMF_SG table.
  saamfsgtableentry_next - This function is the next processing for objects in
                          NCSMIB_TBL_AVSV_AMF_SG table.
  saamfsgtableentry_setrow - This function is the setrow processing for
                           objects in NCSMIB_TBL_AVSV_AMF_SG table.

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"


/*****************************************************************************
 * Function: avd_sg_struc_crt
 *
 * Purpose:  This function will create and add a AVD_SG structure to the
 * tree if an element with sg_name key value doesn't exist in the tree.
 *
 * Input: cb - the AVD control block
 *        sg_name - The SG name of the service group that needs to be
 *                    added.
 *        ckpt - TRUE - Create is called from ckpt update.
 *               FALSE - Create is called from AVD mib set.
 *
 * Returns: The pointer to AVD_SG structure allocated and added. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SG * avd_sg_struc_crt(AVD_CL_CB *cb,SaNameT sg_name, NCS_BOOL ckpt)
{
   AVD_SG *sg;

   if( (sg_name.length <= 6) || (m_NCS_OS_STRNCMP(sg_name.value,"safSg=",6) != 0) )
   {
      return AVD_SG_NULL;
   }

   /* Allocate a new block structure now
    */
   if ((sg = m_MMGR_ALLOC_AVD_SG) == AVD_SG_NULL)
   {
      /* log an error */
      m_AVD_LOG_MEM_FAIL(AVD_SG_ALLOC_FAILED);
      return AVD_SG_NULL;
   }

   m_NCS_MEMSET((char *)sg, '\0', sizeof(AVD_SG));

   if (ckpt)
   {
      m_NCS_MEMCPY(sg->name_net.value,sg_name.value,m_NCS_OS_NTOHS(sg_name.length));
      sg->name_net.length = sg_name.length;
   }
   else
   {
      m_NCS_MEMCPY(sg->name_net.value,sg_name.value,sg_name.length);
      sg->name_net.length = m_HTON_SANAMET_LEN(sg_name.length);
      
      sg->su_failback = FALSE;
      sg->sg_ncs_spec = SA_FALSE;
      sg->sg_fsm_state = AVD_SG_FSM_STABLE;
      
      sg->su_redundancy_model = AVSV_SG_RED_MODL_NORED;
      sg->admin_state = NCS_ADMIN_STATE_UNLOCK;
      sg->row_status = NCS_ROW_NOT_READY;
   }
   
   /* Init pointers */
   sg->list_of_si = AVD_SI_NULL;
   sg->list_of_su = AVD_SU_NULL;
   sg->admin_si = AVD_SI_NULL;
   sg->su_oper_list.su = AVD_SU_NULL;
   sg->su_oper_list.next = AVD_SG_OPER_NULL;
   

   sg->tree_node.key_info = (uns8*)&(sg->name_net);
   sg->tree_node.bit   = 0;
   sg->tree_node.left  = NCS_PATRICIA_NODE_NULL;
   sg->tree_node.right = NCS_PATRICIA_NODE_NULL;

   if( ncs_patricia_tree_add(&cb->sg_anchor,&sg->tree_node) 
                      != NCSCC_RC_SUCCESS)
   {
      /* log an error */
      m_MMGR_FREE_AVD_SG(sg);
      return AVD_SG_NULL;
   }


   return sg;
}


/*****************************************************************************
 * Function: avd_sg_struc_find
 *
 * Purpose:  This function will find a AVD_SG structure in the
 * tree with sg_name value as key.
 *
 * Input: cb - the AVD control block
 *        sg_name - The name of the service group that needs to be
 *                    found.
 *
 *        host_order - Flag indicating if the length is host order
 * Returns: The pointer to AVD_SG structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SG * avd_sg_struc_find(AVD_CL_CB *cb,SaNameT sg_name,NCS_BOOL host_order)
{
   AVD_SG *sg;
   SaNameT  lsg_name;

   m_NCS_MEMSET((char *)&lsg_name, '\0', sizeof(SaNameT));
   if(host_order)
   {
      lsg_name.length = m_HTON_SANAMET_LEN(sg_name.length);
      m_NCS_MEMCPY(lsg_name.value,sg_name.value,sg_name.length);
   }
   else
   {
      lsg_name.length = sg_name.length;
      m_NCS_MEMCPY(lsg_name.value,sg_name.value,m_NCS_OS_NTOHS(lsg_name.length));
   }

   sg = (AVD_SG *)ncs_patricia_tree_get(&cb->sg_anchor, (uns8*)&lsg_name);

   return sg;
}

/*****************************************************************************
 * Function: avd_sg_struc_find_next
 *
 * Purpose:  This function will find the next AVD_SG structure in the
 * tree whose sg_name value is next of the given sg_name value.
 *
 * Input: cb - the AVD control block
 *        sg_name - The name of the service group that needs to be
 *                    found.
 *        host_order - Flag indicating if the length is host order
 *
 * Returns: The pointer to AVD_SG structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SG * avd_sg_struc_find_next(AVD_CL_CB *cb,SaNameT sg_name,NCS_BOOL host_order)
{
   AVD_SG *sg;
   SaNameT  lsg_name;

   m_NCS_MEMSET((char *)&lsg_name, '\0', sizeof(SaNameT));
   if(host_order)
   {
      lsg_name.length = m_HTON_SANAMET_LEN(sg_name.length);
      m_NCS_MEMCPY(lsg_name.value,sg_name.value,sg_name.length);
   }
   else
   {
      lsg_name.length = sg_name.length;
      m_NCS_MEMCPY(lsg_name.value,sg_name.value,m_NCS_OS_NTOHS(lsg_name.length));
   }

   sg = (AVD_SG *)ncs_patricia_tree_getnext(&cb->sg_anchor, (uns8*)&lsg_name);

   return sg;
}

/*****************************************************************************
 * Function: avd_sg_struc_del
 *
 * Purpose:  This function will delete and free AVD_SG structure from 
 * the tree.
 *
 * Input: cb - the AVD control block
 *        sg - The SG structure that needs to be deleted.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE  
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_sg_struc_del(AVD_CL_CB *cb,AVD_SG *sg)
{
   if (sg == AVD_SG_NULL)
      return NCSCC_RC_FAILURE;

   if(ncs_patricia_tree_del(&cb->sg_anchor,&sg->tree_node) 
                      != NCSCC_RC_SUCCESS)
   {
      /* log error */
      return NCSCC_RC_FAILURE;
   }

   m_MMGR_FREE_AVD_SG(sg);
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: ncssgtableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in NCS_S_G_TABLE_ENTRY_ID table. This is the properitary SG table. The
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

uns32 ncssgtableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SG        *sg;
   SaNameT       sg_name;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   m_NCS_MEMSET(&sg_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service group database key from the instant ID */
   sg_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < sg_name.length; i++)
   {
      sg_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   sg = avd_sg_struc_find(avd_cb,sg_name,TRUE);

   if (sg == AVD_SG_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)sg;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: ncssgtableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * NCS_S_G_TABLE_ENTRY_ID table. This is the properitary SG table. The
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

uns32 ncssgtableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer)
{
   AVD_SG        *sg = (AVD_SG *)data;

   if (sg == AVD_SG_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* call the MIBLIB utility routine for standfard object types */
   if ((var_info != NULL) && (var_info->offset != 0))
      return ncsmiblib_get_obj_val(param, var_info, data, buffer);
   else
      return NCSCC_RC_NO_OBJECT;
}


/*****************************************************************************
 * Function: ncssgtableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * NCS_S_G_TABLE_ENTRY_ID table. This is the properitary SG table. The
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

uns32 ncssgtableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SG        *sg;
   SaNameT       sg_name;
   uns32         i;
   NCSMIBLIB_REQ_INFO temp_mib_req;
   NCS_BOOL       val_same_flag = FALSE;
   SaAdjustState  backup_val;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_INV_VAL;  
   }

   m_NCS_MEMSET(&sg_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service group database key from the instant ID */
   sg_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < sg_name.length; i++)
   {
      sg_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   sg = avd_sg_struc_find(avd_cb,sg_name,TRUE);

   if(sg == AVD_SG_NULL)
   {
      if(avd_cb->init_state == AVD_CFG_READY)
      {
         m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

         sg = avd_sg_struc_crt(avd_cb,sg_name, FALSE);

         m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);
      }

      /* Invalid instance object */
      if(sg == AVD_SG_NULL)
         return NCSCC_RC_NO_INSTANCE; 
   }
  
   /* We now have the SG block */
 
   if(test_flag == TRUE)
   {
      return NCSCC_RC_SUCCESS;
   }
   
   if(arg->req.info.set_req.i_param_val.i_param_id == ncsSGIsNcs_ID)
   {
      if(sg->row_status == NCS_ROW_ACTIVE)
         return NCSCC_RC_INV_VAL; 
   }
   else if(arg->req.info.set_req.i_param_val.i_param_id == ncsSGAdjustState_ID)
   {
      if((sg->row_status != NCS_ROW_ACTIVE) || 
         (arg->req.info.set_req.i_param_val.info.i_int != AVSV_SG_ADJUST) ||
         (sg->sg_ncs_spec == SA_TRUE) ||
         (avd_cb->init_state  != AVD_APP_STATE))
         return NCSCC_RC_INV_VAL; 
      else
      {                  
         backup_val = sg->adjust_state;
         m_AVD_SET_SG_ADJUST(cb,sg,AVSV_SG_ADJUST);
         switch(sg->su_redundancy_model)
         {
         case AVSV_SG_RED_MODL_2N:
            if(avd_sg_2n_realign_func(cb, sg) != NCSCC_RC_SUCCESS)
            {
               m_AVD_SET_SG_ADJUST(cb,sg,backup_val);
               return NCSCC_RC_INV_VAL;
            }
            break;

         case AVSV_SG_RED_MODL_NWAY:
            if(avd_sg_nway_realign_func(cb, sg) != NCSCC_RC_SUCCESS)
            {
               sg->adjust_state = backup_val;
               return NCSCC_RC_INV_VAL;
            }
            break;

         case AVSV_SG_RED_MODL_NWAYACTV:
            if(avd_sg_nacvred_realign_func(cb, sg) != NCSCC_RC_SUCCESS)
            {
               m_AVD_SET_SG_ADJUST(cb,sg,backup_val);
               return NCSCC_RC_INV_VAL;
            }
            break;
            
         case AVSV_SG_RED_MODL_NPM:
            if(avd_sg_npm_realign_func(cb, sg) != NCSCC_RC_SUCCESS)
            {
               sg->adjust_state = backup_val;
               return NCSCC_RC_INV_VAL;
            }
            break;

         case AVSV_SG_RED_MODL_NORED:
         default:
            if(avd_sg_nored_realign_func(cb, sg) != NCSCC_RC_SUCCESS)
            {
               m_AVD_SET_SG_ADJUST(cb,sg,backup_val);
               return NCSCC_RC_INV_VAL;
            }
            break;
         }

         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, sg, AVSV_CKPT_SG_ADJUST_STATE);
         return NCSCC_RC_SUCCESS;
      }
   } /* ncsSGAdjustState_ID */


   m_NCS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

   temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
   temp_mib_req.info.i_set_util_info.param = &(arg->req.info.set_req.i_param_val);
   temp_mib_req.info.i_set_util_info.var_info = var_info;
   temp_mib_req.info.i_set_util_info.data = sg;
   temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

   /* call the mib routine handler */
   if (ncsmiblib_process_req(&temp_mib_req) != NCSCC_RC_SUCCESS)   
      return NCSCC_RC_FAILURE;
   
   if (sg->row_status == NCS_ROW_ACTIVE)
   {
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, sg, AVSV_CKPT_AVD_SG_CONFIG);
   }
   
   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: ncssgtableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * NCS_S_G_TABLE_ENTRY_ID table. This is the properitary SG table. The
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

uns32 ncssgtableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SG        *sg;
   SaNameT       sg_name;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   m_NCS_MEMSET(&sg_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service group database key from the instant ID */
   if (arg->i_idx.i_inst_len != 0)
   {
      sg_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
      for(i = 0; i < sg_name.length; i++)
      {
         sg_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
   }

   sg = avd_sg_struc_find_next(avd_cb,sg_name,TRUE);

   if (sg == AVD_SG_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the instant ID from the SG name */

   *next_inst_id_len = m_NCS_OS_NTOHS(sg->name_net.length) + 1; 

   next_inst_id[0] = *next_inst_id_len -1;
   for(i = 0; i <  next_inst_id[0]; i++)
   {
      next_inst_id[i + 1] = (uns32)(sg->name_net.value[i]);
   }

   *data = (NCSCONTEXT)sg;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: ncssgtableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * NCS_S_G_TABLE_ENTRY_ID table. This is the properitary SG table. The
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

uns32 ncssgtableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}

/*****************************************************************************
 * Function: ncssgtableentry_rmvrow
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
uns32 ncssgtableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx) 
{
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfsgtableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_S_G_TABLE_ENTRY_ID table. This is the AMF specific SG table. The
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

uns32 saamfsgtableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SG        *sg;
   SaNameT       sg_name;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   m_NCS_MEMSET(&sg_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service group database key from the instant ID */
   sg_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < sg_name.length; i++)
   {
      sg_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   sg = avd_sg_struc_find(avd_cb,sg_name,TRUE);

   if (sg == AVD_SG_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)sg;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfsgtableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_AMF_S_G_TABLE_ENTRY_ID table. This is the AMF specific SG table. The
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

uns32 saamfsgtableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer)
{
   AVD_SG        *sg = (AVD_SG *)data;
   AVD_SU        *temp_su = AVD_SU_NULL;

   if (sg == AVD_SG_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   switch(param->i_param_id)
   {
   case saAmfSGFailbackOption_ID:
      param->i_fmat_id = NCSMIB_FMAT_INT; 
      param->i_length = 1; 
      param->info.i_int = (sg->su_failback == FALSE) ? NCS_SNMP_FALSE:NCS_SNMP_TRUE;
      break;
   case saAmfSGCompRestartProb_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,sg->comp_restart_prob);
      break;
   case saAmfSGSuRestartProb_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,sg->su_restart_prob);
      break;
   case saAmfSGNumCurrNonInstantiatedSpareSU_ID:
      param->i_fmat_id = NCSMIB_FMAT_INT;
      param->i_length = 1;
      param->info.i_int = 0;
      
      temp_su = sg->list_of_su;
      while(temp_su != AVD_SU_NULL)
      {
         if( (temp_su->row_status == NCS_ROW_ACTIVE) && 
             (temp_su->pres_state == NCS_PRES_UNINSTANTIATED) &&
             (temp_su->num_of_comp == temp_su->curr_num_comp) )
         {
            param->info.i_int = param->info.i_int + 1;
         }

         temp_su = temp_su->sg_list_su_next;
      }
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
 * Function: saamfsgtableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_S_G_TABLE_ENTRY_ID table. This is the AMF specific SG table. The
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

uns32 saamfsgtableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SG        *sg;
   SaNameT       sg_name;
   uns32         back_val;
   AVSV_PARAM_INFO param;
   NCS_ADMIN_STATE adm_state;
   AVD_SU        *ncs_su;
   NCSMIBLIB_REQ_INFO temp_mib_req;
   NCS_BOOL      val_same_flag = FALSE;
   uns32         param_id, i, rc = NCSCC_RC_SUCCESS;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_INV_VAL;  
   }

   m_NCS_MEMSET(&sg_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service group database key from the instant ID */
   sg_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < sg_name.length; i++)
   {
      sg_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   sg = avd_sg_struc_find(avd_cb,sg_name,TRUE);

   if (sg == AVD_SG_NULL)
   {
      if((arg->req.info.set_req.i_param_val.i_param_id == saAmfSGRowStatus_ID)
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

      sg = avd_sg_struc_crt(avd_cb,sg_name, FALSE);

      m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

      if(sg == AVD_SG_NULL)
      {
         /* Invalid instance object */
         return NCSCC_RC_NO_INSTANCE; 
      }

   }else /* if (sg == AVD_SG_NULL) */
   {
      /* The record is already available */

      if(arg->req.info.set_req.i_param_val.i_param_id == saAmfSGRowStatus_ID)
      {
         /* This is a row status operation */
         if (arg->req.info.set_req.i_param_val.info.i_int == (uns32)sg->row_status)
         {
            /* row status object is same so nothing to be done. */
            return NCSCC_RC_SUCCESS;
         }

         switch(arg->req.info.set_req.i_param_val.info.i_int)
         {
         case NCS_ROW_ACTIVE:

            /* validate the structure to see if the row can be made active */

            if ((sg->comp_restart_max == 0) ||
               (sg->comp_restart_prob == 0) ||
               (sg->su_restart_max == 0) ||
               (sg->su_restart_prob == 0) || 
               (sg->pref_num_insvc_su == 0) || 
               (sg->su_redundancy_model < AVSV_SG_RED_MODL_2N) ||
               (sg->su_redundancy_model > AVSV_SG_RED_MODL_NORED))
            {
               /* All the mandatory fields are not filled
                */
               /* log information error */
               return NCSCC_RC_INV_VAL;
            }
            if((sg->su_redundancy_model < AVSV_SG_RED_MODL_NWAYACTV) && 
               (sg->pref_num_insvc_su < SG_2N_PREF_INSVC_SU_MIN))
            {
               /* minimum preferred num of su should be 2 */ 
               /* log information error */
               return NCSCC_RC_INV_VAL;
            }
            
            /* Check that NCS SGs are only 2N or No redundancy model
             * SGs.
             */
            if ((sg->sg_ncs_spec == SA_TRUE) &&
               (sg->su_redundancy_model != AVSV_SG_RED_MODL_2N) &&
               (sg->su_redundancy_model != AVSV_SG_RED_MODL_NORED))
            {
               return NCSCC_RC_INV_VAL;
            }

            if(test_flag == TRUE)
            {
               return NCSCC_RC_SUCCESS;
            }

            /* set the value, checkpoint the entire record.
             */
            sg->row_status = NCS_ROW_ACTIVE;
            
            m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(cb, sg, AVSV_CKPT_AVD_SG_CONFIG);

            return NCSCC_RC_SUCCESS;
            break;

         case NCS_ROW_NOT_IN_SERVICE:
         case NCS_ROW_DESTROY:
            
            /* check if it is active currently */

            if(sg->row_status == NCS_ROW_ACTIVE)
            {

               /* Check to see that the SG is in admin locked state before
                * making the row status as not in service or delete 
                */

               if(sg->admin_state != NCS_ADMIN_STATE_LOCK)
               {
                  /* log information error */
                  return NCSCC_RC_INV_VAL;
               }

               /* Check to see that no SUs or SIs exists as part
                * of this node.
                */
               if((sg->list_of_su != AVD_SU_NULL) ||
                  (sg->list_of_si != AVD_SI_NULL)) 
               {
                  /* log information error */
                  return NCSCC_RC_INV_VAL;
               }

               if(test_flag == TRUE)
               {
                  return NCSCC_RC_SUCCESS;
               }

               /* we need to delete this SG structure on the
                * standby AVD.
                */

               /* check point to the standby AVD that this
                * record need to be deleted
                */
               m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(avd_cb, sg, AVSV_CKPT_AVD_SG_CONFIG);
               
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
               
               avd_sg_struc_del(avd_cb,sg);

               m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

               return NCSCC_RC_SUCCESS;
            }

            sg->row_status = arg->req.info.set_req.i_param_val.info.i_int;
            return NCSCC_RC_SUCCESS;

            break;
         default:
            m_AVD_LOG_INVALID_VAL_ERROR(arg->req.info.set_req.i_param_val.info.i_int);
            /* Invalid row status object */
            return NCSCC_RC_INV_VAL;
            break;
         } /* switch(arg->req.info.set_req.i_param_val.info.i_int) */

      } /* if(arg->req.info.set_req.i_param_val.i_param_id == ncsSGRowStatus_ID) */

   } /* if (sg == AVD_SG_NULL) */

   /* We now have the SG block */
   

   if(test_flag == TRUE)
   {
      return NCSCC_RC_SUCCESS;
   }

   /* This Block deals with the values to be modified while
    * the ROW_STATUS is ACTIVE.
    * 
    * Modifying Some values may require to get each SU in the SG and 
    * making the changes internally in the SU. 
    * 
    * If the madifications fails for some SUs its not practical to 
    * revert such values for all the Sus which was set and hence we continue.
    */
   if(sg->row_status == NCS_ROW_ACTIVE)
   {
      /* Dont set anything if the AVD state is not APP state */
      if(avd_cb->init_state  != AVD_APP_STATE)
      {
         return NCSCC_RC_INV_VAL;
      }

      param_id = arg->req.info.set_req.i_param_val.i_param_id;
      switch(arg->req.info.set_req.i_param_val.i_param_id)
      {
         case saAmfSGAdminState_ID:
            /* No modification is allowed for NCS SGs. */
            if (arg->req.info.set_req.i_param_val.info.i_int == sg->admin_state)
               return NCSCC_RC_SUCCESS;
         
            if (sg->sg_ncs_spec == SA_TRUE)
               return NCSCC_RC_INV_VAL;
            
            if ((sg->admin_state == NCS_ADMIN_STATE_LOCK) && 
               (arg->req.info.set_req.i_param_val.info.i_int == NCS_ADMIN_STATE_SHUTDOWN))
               return NCSCC_RC_INV_VAL;
            
            adm_state = sg->admin_state;
            m_AVD_SET_SG_ADMIN(cb,sg,(arg->req.info.set_req.i_param_val.info.i_int));
            if(avd_sg_app_sg_admin_func(cb, sg) != NCSCC_RC_SUCCESS)
            {
               m_AVD_SET_SG_ADMIN(cb,sg,adm_state);
               return NCSCC_RC_INV_VAL;
            }
            break;
         case saAmfSGSuRestartProb_ID:
            m_NCS_MEMSET(((uns8 *)&param),'\0',sizeof(AVSV_PARAM_INFO));
            param.table_id = NCSMIB_TBL_AVSV_AMF_SG;
            param.obj_id = param_id;     
            param.act = AVSV_OBJ_OPR_MOD;

            param.value_len = sizeof(SaTimeT);
            m_NCS_MEMCPY(&param.value[0], 
                         arg->req.info.set_req.i_param_val.info.i_oct, 
                         param.value_len);

            sg->su_restart_prob = m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct); 

            /* This value has to be updated on each SU on this SG */
            ncs_su = sg->list_of_su;
            while(ncs_su)
            {
               if( (ncs_su->su_on_node) && 
                  (ncs_su->su_on_node->node_state == AVD_AVND_STATE_PRESENT) )
               {
                  param.name_net = ncs_su->name_net;

                  if(avd_snd_op_req_msg(avd_cb, ncs_su->su_on_node, &param) 
                                                          != NCSCC_RC_SUCCESS)
                  {
                     /* Revert the value and return ?? */
                     /*return NCSCC_RC_INV_VAL; */
                  }
               }
               ncs_su = ncs_su->sg_list_su_next;
            }
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, sg, AVSV_CKPT_AVD_SG_CONFIG);
            break;
         case saAmfSGSuRestartMax_ID:
            m_NCS_MEMSET(((uns8 *)&param),'\0',sizeof(AVSV_PARAM_INFO));
            param.table_id = NCSMIB_TBL_AVSV_AMF_SG;
            param.obj_id = param_id;     
            param.act = AVSV_OBJ_OPR_MOD;

            param.value_len = sizeof(uns32);
            m_NCS_OS_HTONL_P(&param.value[0], arg->req.info.set_req.i_param_val.info.i_int);
            sg->su_restart_max = arg->req.info.set_req.i_param_val.info.i_int;
            /* This value has to be updated on each SU on this SG */
            ncs_su = sg->list_of_su;
            while(ncs_su)
            {
               if( (ncs_su->su_on_node) && 
                  (ncs_su->su_on_node->node_state == AVD_AVND_STATE_PRESENT) )
               {
                  param.name_net = ncs_su->name_net;

                  if(avd_snd_op_req_msg(avd_cb, ncs_su->su_on_node, &param) 
                                                          != NCSCC_RC_SUCCESS)
                  {
                     /* Revert the value and return ?? */
                     /*return NCSCC_RC_INV_VAL; */
                  }
               }
               ncs_su = ncs_su->sg_list_su_next;
            }
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, sg, AVSV_CKPT_AVD_SG_CONFIG);
            break;
         case saAmfSGCompRestartProb_ID:
            m_NCS_MEMSET(((uns8 *)&param),'\0',sizeof(AVSV_PARAM_INFO));
            param.table_id = NCSMIB_TBL_AVSV_AMF_SG;
            param.obj_id = param_id;     
            param.act = AVSV_OBJ_OPR_MOD;

            param.value_len = sizeof(SaTimeT);
            m_NCS_MEMCPY(&param.value[0], 
                         arg->req.info.set_req.i_param_val.info.i_oct, 
                         param.value_len);

            sg->comp_restart_prob = m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);

            /* This value has to be updated on each SU on this SG */
            ncs_su = sg->list_of_su;
            while(ncs_su)
            {
               if( (ncs_su->su_on_node) && 
                  (ncs_su->su_on_node->node_state == AVD_AVND_STATE_PRESENT) )
               {
                  param.name_net = ncs_su->name_net;

                  if(avd_snd_op_req_msg(avd_cb, ncs_su->su_on_node, &param) 
                                                          != NCSCC_RC_SUCCESS)
                  {
                     /* Revert the value and return ?? */
                     /*return NCSCC_RC_INV_VAL; */
                  }
               }
               ncs_su = ncs_su->sg_list_su_next;
            }
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, sg, AVSV_CKPT_AVD_SG_CONFIG);
            break;
         case saAmfSGCompRestartMax_ID:
            m_NCS_MEMSET(((uns8 *)&param),'\0',sizeof(AVSV_PARAM_INFO));
            param.table_id = NCSMIB_TBL_AVSV_AMF_SG;
            param.obj_id = param_id;     
            param.act = AVSV_OBJ_OPR_MOD;

            param.value_len = sizeof(uns32);
            m_NCS_OS_HTONL_P(&param.value[0], arg->req.info.set_req.i_param_val.info.i_int);
            sg->comp_restart_max = arg->req.info.set_req.i_param_val.info.i_int; 
            /* This value has to be updated on each SU on this SG */
            ncs_su = sg->list_of_su;
            while(ncs_su)
            {
               if( (ncs_su->su_on_node) && 
                  (ncs_su->su_on_node->node_state == AVD_AVND_STATE_PRESENT) )
               {
                  param.name_net = ncs_su->name_net;

                  if(avd_snd_op_req_msg(avd_cb, ncs_su->su_on_node, &param) 
                                                          != NCSCC_RC_SUCCESS)
                  {
                     /* Revert the value and return ?? */
                     /*return NCSCC_RC_INV_VAL; */
                  }
               }
               ncs_su = ncs_su->sg_list_su_next;
            }
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, sg, AVSV_CKPT_AVD_SG_CONFIG);
            break;
         case saAmfSGNumPrefInserviceSUs_ID:
            if((arg->req.info.set_req.i_param_val.info.i_int == 0) || 
               ((sg->su_redundancy_model < AVSV_SG_RED_MODL_NWAYACTV) &&
                (arg->req.info.set_req.i_param_val.info.i_int < SG_2N_PREF_INSVC_SU_MIN)))
            {
               /* minimum preferred num of su should be 2 in 2N, N+M and NWay red models*/
               /* log information error */
               return NCSCC_RC_INV_VAL;
            }

            back_val = sg->pref_num_insvc_su;
            sg->pref_num_insvc_su = arg->req.info.set_req.i_param_val.info.i_int;
            if(avd_sg_app_su_inst_func(avd_cb, sg) != NCSCC_RC_SUCCESS)
            {
               sg->pref_num_insvc_su = back_val;
               return NCSCC_RC_INV_VAL;
            }
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, sg, AVSV_CKPT_AVD_SG_CONFIG);
            break;
         default:
            return NCSCC_RC_INV_VAL;
      } /* switch(arg->req.info.set_req.i_param_val.i_param_id) */

      return NCSCC_RC_SUCCESS;
   } /* if(sg->row_status == NCS_ROW_ACTIVE */



   switch(arg->req.info.set_req.i_param_val.i_param_id)
   {

   case saAmfSGRowStatus_ID:
      /* fill the row status value */
      if(arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT)
      {
         sg->row_status = arg->req.info.set_req.i_param_val.info.i_int;
      }
      break;
   case saAmfSGRedModel_ID:
      if((arg->req.info.set_req.i_param_val.info.i_int < AVSV_SG_RED_MODL_2N) ||
        (arg->req.info.set_req.i_param_val.info.i_int > AVSV_SG_RED_MODL_NORED))
      {
         return NCSCC_RC_INV_VAL;
      }
      else
      {
         sg->su_redundancy_model = arg->req.info.set_req.i_param_val.info.i_int;
      }
      break;
   case saAmfSGFailbackOption_ID:
      /* Truth value 1 for true */ 
      if(arg->req.info.set_req.i_param_val.info.i_int == NCS_SNMP_TRUE) 
      {
         return NCSCC_RC_INV_VAL;
      }
      else
      {
         sg->su_failback = FALSE;
      }
      break;
   case saAmfSGNumPrefInserviceSUs_ID:
         sg->pref_num_insvc_su =  arg->req.info.set_req.i_param_val.info.i_int;
      break;
   case saAmfSGCompRestartProb_ID:
      /* fill the probation period value */
      sg->comp_restart_prob = m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
      break;
   case saAmfSGSuRestartProb_ID:
      /* fill the probation period value */
      sg->su_restart_prob = m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
      break;
   default:
      m_NCS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

      temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
      temp_mib_req.info.i_set_util_info.param = &(arg->req.info.set_req.i_param_val);
      temp_mib_req.info.i_set_util_info.var_info = var_info;
      temp_mib_req.info.i_set_util_info.data = sg;
      temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

      /* call the mib routine handler */ 
      if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
      {
         return rc;
      }
      /* check to verify that the admin object is not shutdown
      * if shutdown change to LOCK.
      */
      if (sg->admin_state == NCS_ADMIN_STATE_SHUTDOWN)
      {
         sg->admin_state = NCS_ADMIN_STATE_LOCK;
      }
      break;
   } /* switch(arg->req.info.set_req.i_param_val.i_param_id) */
   
   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: saamfsgtableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_S_G_TABLE_ENTRY_ID table. This is the AMF specific SG table. The
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

uns32 saamfsgtableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SG        *sg;
   SaNameT       sg_name;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   m_NCS_MEMSET(&sg_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service group database key from the instant ID */
   if (arg->i_idx.i_inst_len != 0)
   {
      sg_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
      for(i = 0; i < sg_name.length; i++)
      {
         sg_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
   }

   sg = avd_sg_struc_find_next(avd_cb,sg_name,TRUE);

   if (sg == AVD_SG_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the instant ID from the SG name */

   *next_inst_id_len = m_NCS_OS_NTOHS(sg->name_net.length) + 1; 

   next_inst_id[0] = *next_inst_id_len -1;
   for(i = 0; i < next_inst_id[0]; i++)
   {
      next_inst_id[i + 1] = (uns32)(sg->name_net.value[i]);
   }

   *data = (NCSCONTEXT)sg;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfsgtableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_S_G_TABLE_ENTRY_ID table. This is the AMF specific SG table. The
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

uns32 saamfsgtableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}
