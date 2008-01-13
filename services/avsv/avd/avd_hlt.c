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
  the health check database on the AVD. It also deals with all the MIB 
  operations like set,get,getnext etc related to the health check table.

  The key_size of patricia node is sizeof(AVSV_HLT_KEY) - sizeof(SaUint16T)
  In all the patricia operations however we fill the entire AVSV_HLT_KEY and 
  patricia functions will take of comparing to the appropriate length.


..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_hlt_struc_crt - creates and adds health check structure to database.
  avd_hlt_struc_find - Finds health check structure from the database.
  avd_hlt_struc_find_next - Finds the next health check structure from the database.  
  avd_hlt_struc_del - deletes and frees health check structure from database.
  saamfhealthchecktableentry_get - This function is one of the get processing
                        routines for objects in SA_AMF_HEALTH_CHECK_TABLE_ENTRY_ID table.
  saamfhealthchecktableentry_extract - This function is one of the get processing
                            function for objects in SA_AMF_HEALTH_CHECK_TABLE_ENTRY_ID
                            table.
  saamfhealthchecktableentry_set - This function is the set processing for objects in
                         SA_AMF_HEALTH_CHECK_TABLE_ENTRY_ID table.
  saamfhealthchecktableentry_next - This function is the next processing for objects in
                         SA_AMF_HEALTH_CHECK_TABLE_ENTRY_ID table.
  saamfhealthchecktableentry_setrow - This function is the setrow processing for
                           objects in SA_AMF_HEALTH_CHECK_TABLE_ENTRY_ID table.
  
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"


/*****************************************************************************
 * Function: avd_hlt_struc_crt
 *
 * Purpose:  This function will create and add a AVD_HLT structure to the
 * tree if an element with hlt_chk_name key value doesn't exist in the tree.
 *
 * Input: cb - the AVD control block
 *        hlt_chk_name - The healthcheck key name of the healthcheck that 
 *                    needs to be added.
 *
 * Returns: The pointer to AVD_HLT structure allocated and added.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_HLT * avd_hlt_struc_crt(AVD_CL_CB *cb,AVSV_HLT_KEY lhlt_chk,  NCS_BOOL ckpt)
{
   AVD_HLT *hlth = AVD_HLT_NULL;

   /* Allocate a new block structure now
    */
   if ((hlth = m_MMGR_ALLOC_AVD_HLT) == AVD_HLT_NULL)
   {
      /* log an error */
      m_AVD_LOG_MEM_FAIL(AVD_HLT_ALLOC_FAILED);
      return AVD_HLT_NULL;
   }

   m_NCS_MEMSET((char *)hlth, '\0', sizeof(AVD_HLT));
   
   hlth->key_name.comp_name_net.length = lhlt_chk.comp_name_net.length;
   m_NCS_MEMCPY(hlth->key_name.comp_name_net.value,lhlt_chk.comp_name_net.value,m_NCS_OS_NTOHS(lhlt_chk.comp_name_net.length));
   
   hlth->key_name.key_len_net = lhlt_chk.key_len_net;
   m_NCS_MEMCPY(hlth->key_name.name.key,lhlt_chk.name.key,lhlt_chk.name.keyLen);
   hlth->key_name.name.keyLen = lhlt_chk.name.keyLen;
 
   hlth->period = AVSV_DEFAULT_HEALTH_CHECK_PERIOD;
   hlth->max_duration = AVSV_DEFAULT_HEALTH_CHECK_MAX_DURATION; 
   hlth->tree_node.key_info = (uns8*)(&hlth->key_name);
   hlth->tree_node.bit   = 0;
   hlth->tree_node.left  = NCS_PATRICIA_NODE_NULL;
   hlth->tree_node.right = NCS_PATRICIA_NODE_NULL;

   if( ncs_patricia_tree_add(&cb->hlt_anchor,&hlth->tree_node) 
                      != NCSCC_RC_SUCCESS)
   {
      /* log an error */
      m_MMGR_FREE_AVD_HLT(hlth);
      return AVD_HLT_NULL;
   }


   return hlth;
   
}


/*****************************************************************************
 * Function: avd_hlt_struc_find
 *
 * Purpose:  This function will find a AVD_HLT structure in the
 * tree with hlt_chk_name value as key.
 *
 * Input: cb - the AVD control block
 *        hlt_chk_name - The healthcheck key name of the healthcheck.
 *
 * Returns: The pointer to AVD_HLT structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_HLT * avd_hlt_struc_find(AVD_CL_CB *cb, AVSV_HLT_KEY lhlt_key)
{
   AVD_HLT *hlth = AVD_HLT_NULL;
   hlth = (AVD_HLT *)ncs_patricia_tree_get(&cb->hlt_anchor, (uns8*)&lhlt_key);

   return hlth;
}


/*****************************************************************************
 * Function: avd_hlt_struc_find_next
 *
 * Purpose:  This function will find the next AVD_HLT structure in the
 * tree whose hlt_chk_name value is next of the given hlt_chk_name value.
 *
 * Input: cb - the AVD control block
 *        hlt_chk_name - The healthcheck key name of the healthcheck.
 *
 * Returns: The pointer to AVD_HLT structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_HLT * avd_hlt_struc_find_next(AVD_CL_CB *cb,AVSV_HLT_KEY lhlt_key)
{
   AVD_HLT *hlth = AVD_HLT_NULL;
   hlth = (AVD_HLT *)ncs_patricia_tree_getnext(&cb->hlt_anchor, 
                                                (uns8*)&lhlt_key);

   return hlth;
}


/*****************************************************************************
 * Function: avd_hlt_struc_del
 *
 * Purpose:  This function will delete and free AVD_HLT structure from 
 * the tree.
 *
 * Input: cb - the AVD control block
 *        hlt_chk - The health check structure that needs to be deleted.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE  
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_hlt_struc_del(AVD_CL_CB *cb,AVD_HLT *hlt_chk)
{
   if (hlt_chk == AVD_HLT_NULL)
      return NCSCC_RC_FAILURE;

   if(ncs_patricia_tree_del(&cb->hlt_anchor,&hlt_chk->tree_node) 
                      != NCSCC_RC_SUCCESS)
   {
      /* log error */
      return NCSCC_RC_FAILURE;
   }

   m_MMGR_FREE_AVD_HLT(hlt_chk);
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfhealthchecktableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_HEALTH_CHECK_TABLE_ENTRY_ID table. This is the health check table. The
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

uns32 saamfhealthchecktableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVSV_HLT_KEY  lhlt_chk;
   uns16 len;
   uns32 i;
   AVD_HLT     *hlt_chk = AVD_HLT_NULL;
   uns32 l_len;

   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   m_NCS_MEMSET(&lhlt_chk, '\0', sizeof(AVSV_HLT_KEY));

   /* Prepare the health check database key from the instant ID */
   len = (SaUint16T)arg->i_idx.i_inst_ids[0];
   lhlt_chk.comp_name_net.length  = m_NCS_OS_HTONS(len);
   for(i = 0; i < len; i++)
   {
      lhlt_chk.comp_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   lhlt_chk.name.keyLen = (SaUint16T)arg->i_idx.i_inst_ids[len+1];
   l_len = lhlt_chk.name.keyLen;
   lhlt_chk.key_len_net = m_NCS_OS_HTONL(l_len);
   for(i = 0; i < lhlt_chk.name.keyLen; i++)
   {
      lhlt_chk.name.key[i] = (uns8)(arg->i_idx.i_inst_ids[len+1+1+i]);
   }

   hlt_chk = avd_hlt_struc_find(avd_cb,lhlt_chk);

   if (hlt_chk == AVD_HLT_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)hlt_chk;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfhealthchecktableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_AMF_HEALTH_CHECK_TABLE_ENTRY_ID table. This is the health check table. The
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

uns32 saamfhealthchecktableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer)
{
   AVD_HLT     *hlt_chk = (AVD_HLT *)data;

   if (hlt_chk == AVD_HLT_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }
   switch(param->i_param_id)
   {
   case saAmfHealthCheckPeriod_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,hlt_chk->period);
      break;
   case saAmfHealthCheckMaxDuration_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,hlt_chk->max_duration);
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
 * Function: saamfhealthchecktableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_HEALTH_CHECK_TABLE_ENTRY_ID table. This is the health check table. The
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

uns32 saamfhealthchecktableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{

   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVSV_HLT_KEY               lhlt_chk;
   AVD_AVND                   *node_on_hlt = AVD_AVND_NULL;
   AVD_HLT                    *temp_hlt_chk = AVD_HLT_NULL;
   uns16 len;
   uns32 i;
   AVD_HLT     *hlt_chk = AVD_HLT_NULL;
   AVSV_PARAM_INFO l_param;
   SaTimeT     *time;
   SaTimeT      old_time;
   uns32 l_len;

   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_INV_VAL;  
   }

   m_NCS_MEMSET(&lhlt_chk, '\0', sizeof(AVSV_HLT_KEY));
   
   /* Prepare the health check database key from the instant ID */
   len = (SaUint16T)arg->i_idx.i_inst_ids[0];
   lhlt_chk.comp_name_net.length  = m_NCS_OS_HTONS(len);
   for(i = 0; i < len; i++)
   {
      lhlt_chk.comp_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   lhlt_chk.name.keyLen = (SaUint16T)arg->i_idx.i_inst_ids[len+1];
   l_len = lhlt_chk.name.keyLen;
   lhlt_chk.key_len_net = m_NCS_OS_HTONL(l_len);
   for(i = 0; i < lhlt_chk.name.keyLen; i++)
   {
      lhlt_chk.name.key[i] = (uns8)(arg->i_idx.i_inst_ids[len+1+1+i]);
   }
   
   hlt_chk = avd_hlt_struc_find(avd_cb,lhlt_chk);

   if (hlt_chk == AVD_HLT_NULL)
   {
      if((arg->req.info.set_req.i_param_val.i_param_id == saAmfHealthCheckRowStatus_ID)
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

      hlt_chk = avd_hlt_struc_crt(avd_cb,lhlt_chk, FALSE);

      m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

      if(hlt_chk == AVD_HLT_NULL)
      {
         /* Invalid instance object */
         return NCSCC_RC_NO_INSTANCE; 
      }

   }else /* if (hlt_chk == AVD_HLT_NULL) */
   {
      /* The record is already available */

      if(arg->req.info.set_req.i_param_val.i_param_id == saAmfHealthCheckRowStatus_ID)
      {
         /* This is a row status operation */
         if (arg->req.info.set_req.i_param_val.info.i_int == (uns32)hlt_chk->row_status)
         {
            /* row status object is same so nothing to be done. */
            return NCSCC_RC_SUCCESS;
         }

         switch(arg->req.info.set_req.i_param_val.info.i_int)
         {
         case NCS_ROW_ACTIVE:
            /* validate the structure to see if the row can be made active */

            
            if ((hlt_chk->max_duration == (SaTimeT)0) || 
               (hlt_chk->period == (SaTimeT)0))
            {
               /* both the period and max duration should be non zero for
                * the row to be made active.
                */
               return NCSCC_RC_INV_VAL;
            }


            /* set the value, checkpoint the entire record and send a message to
             * AVND to update it about this new record.
             */
            node_on_hlt =  avd_hlt_node_find(hlt_chk->key_name.comp_name_net, avd_cb);
            if(node_on_hlt == AVD_AVND_NULL)
            {
               return NCSCC_RC_INV_VAL;
            }
 
            if(test_flag == TRUE)
            {
               return NCSCC_RC_SUCCESS;
            }


            if( (avd_cb->init_state >= AVD_CFG_DONE) && 
                  ((node_on_hlt->node_state == AVD_AVND_STATE_PRESENT) || 
                  (node_on_hlt->node_state == AVD_AVND_STATE_NO_CONFIG) ||
                  (node_on_hlt->node_state == AVD_AVND_STATE_NCS_INIT) ))
            {
               if(avd_snd_hlt_msg(avd_cb,node_on_hlt,hlt_chk, FALSE) != NCSCC_RC_SUCCESS)
               {
                  /* log a fatal error that a row couldn't be set becaue of
                   * message not being sent
                   */
                  /* checkpoint to delete the structure fom the standby AVD */

                  hlt_chk->row_status = NCS_ROW_NOT_IN_SERVICE;
                  return NCSCC_RC_FAILURE;
               }
            }
            
            temp_hlt_chk = node_on_hlt->list_of_hlt;
            node_on_hlt->list_of_hlt = hlt_chk;
            hlt_chk->next_hlt_on_node = temp_hlt_chk;
            
            hlt_chk->row_status = NCS_ROW_ACTIVE;

            m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(avd_cb, hlt_chk, AVSV_CKPT_AVD_HLT_CONFIG);

            return NCSCC_RC_SUCCESS;
            break;

         case NCS_ROW_NOT_IN_SERVICE:
         case NCS_ROW_DESTROY:

            if(test_flag == TRUE)
            {
               return NCSCC_RC_SUCCESS;
            }
            /* check if it is active currently */

            if(hlt_chk->row_status == NCS_ROW_ACTIVE)
            {
               /* we need to delete this health check record from both the
                * standby AVD and the AVND.
                */

               /* check point to the standby AVD that this
                * record need to be deleted
                */
               m_NCS_MEMSET(((uns8 *)&l_param),'\0',sizeof(AVSV_PARAM_INFO));
               l_param.act = AVSV_OBJ_OPR_DEL;
              
               l_param.name_net.length = hlt_chk->key_name.comp_name_net.length;
               m_NCS_MEMCPY(l_param.name_net.value, hlt_chk->key_name.comp_name_net.value,
                            m_NCS_OS_NTOHS(hlt_chk->key_name.comp_name_net.length));

               l_param.name_sec_net.length = m_NCS_OS_HTONS(hlt_chk->key_name.name.keyLen);
               m_NCS_MEMCPY(l_param.name_sec_net.value, hlt_chk->key_name.name.key,
                            hlt_chk->key_name.name.keyLen);
               l_param.table_id = NCSMIB_TBL_AVSV_AMF_HLT_CHK;

               /* For delete request the information is enough. Now send
                * the message to the AVND
                */
               
               node_on_hlt =  avd_hlt_node_find(hlt_chk->key_name.comp_name_net, avd_cb);
               if(node_on_hlt == AVD_AVND_NULL)
               {
                  return NCSCC_RC_INV_VAL;
               }
               
               if((node_on_hlt->node_state == AVD_AVND_STATE_PRESENT) ||
               (node_on_hlt->node_state == AVD_AVND_STATE_NO_CONFIG) ||
               (node_on_hlt->node_state == AVD_AVND_STATE_NCS_INIT))
               {
                  if (NCSCC_RC_SUCCESS !=
                     avd_snd_op_req_msg(avd_cb,node_on_hlt,&l_param))
                  {
                     /* log information error */

                     /* treat the operation currently as failure */
                     return NCSCC_RC_INV_VAL;
                  }
               }

              avd_hlt_node_del_hlt_from_list(hlt_chk, node_on_hlt);
              m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(avd_cb, hlt_chk, AVSV_CKPT_AVD_HLT_CONFIG);
            }

            if(arg->req.info.set_req.i_param_val.info.i_int
                  == NCS_ROW_DESTROY)
            {
               /* delete and free the structure */
               m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);
             
               avd_hlt_struc_del(avd_cb,hlt_chk);

               m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

               return NCSCC_RC_SUCCESS;
            }

            hlt_chk->row_status = arg->req.info.set_req.i_param_val.info.i_int;
            return NCSCC_RC_SUCCESS;

            break;
         default:
            m_AVD_LOG_INVALID_VAL_ERROR(arg->req.info.set_req.i_param_val.info.i_int);
            /* Invalid row status object */
            return NCSCC_RC_INV_VAL;
            break;
         } /* switch(arg->req.info.set_req.i_param_val.info.i_int) */

      } /* if(arg->req.info.set_req.i_param_val.i_param_id == ncsHealthCheckRowStatus_ID) */

   } /* if (hlt_chk == AVD_HLT_NULL) */

   /* We now have the healthcheck block */
   
   if(hlt_chk->row_status == NCS_ROW_ACTIVE)
   {
      /* when row status is active we don't allow any MIB object to be
       * modified.
       */
      return NCSCC_RC_INV_VAL;
   }

   if(test_flag == TRUE)
   {
      return NCSCC_RC_SUCCESS;
   }

   if(arg->req.info.set_req.i_param_val.i_param_id == saAmfHealthCheckRowStatus_ID)
   {
      /* fill the row status value */
      if(arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT)
      {
         hlt_chk->row_status = arg->req.info.set_req.i_param_val.info.i_int;
      }
      
      
   }else
   {
      /* both period and max duration are 64 bit entities that need to be filled
       * from byte string because of MAB limitation.
       */
      time = (SaTimeT *)((uns8 *)hlt_chk + var_info->offset);
      old_time = *time;
      *time = 
     m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);

     if(*time < AVSV_DEFAULT_HEALTH_CHECK_MIN_TIME )
     {
       *time = old_time;
       return NCSCC_RC_INV_VAL;
     }
   }
   
   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: saamfhealthchecktableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_HEALTH_CHECK_TABLE_ENTRY_ID table. This is the health check table. The
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

uns32 saamfhealthchecktableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVSV_HLT_KEY  lhlt_chk;
   uns16 len;
   uns32 i;
   AVD_HLT     *hlt_chk = AVD_HLT_NULL;
   uns32 l_len;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   m_NCS_MEMSET(&lhlt_chk, '\0', sizeof(AVSV_HLT_KEY));
   
   /* Prepare the health check database key from the instant ID */
   if(arg->i_idx.i_inst_len != 0)
   {
      len = (SaUint16T)arg->i_idx.i_inst_ids[0];
      lhlt_chk.comp_name_net.length  = m_NCS_OS_HTONS(len);
      for(i = 0; i < len; i++)
      {
         lhlt_chk.comp_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
      
      if(arg->i_idx.i_inst_len  > len + 1 )
      { 
         lhlt_chk.name.keyLen = (SaUint16T)arg->i_idx.i_inst_ids[len+1];
         l_len = lhlt_chk.name.keyLen;
         lhlt_chk.key_len_net = m_NCS_OS_HTONL(l_len);
         for(i = 0; i < lhlt_chk.name.keyLen; i++)
         {
           lhlt_chk.name.key[i] = (uns8)(arg->i_idx.i_inst_ids[len+1+1+i]);
         }
      }
   }

   hlt_chk = avd_hlt_struc_find_next(avd_cb,lhlt_chk);

   if (hlt_chk == AVD_HLT_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the instant ID from the health key name */
   
   len = m_NCS_OS_NTOHS(hlt_chk->key_name.comp_name_net.length);

   *next_inst_id_len = len + 1 + hlt_chk->key_name.name.keyLen + 1;

   next_inst_id[0] = len;
   for(i = 0; i < len; i++)
   {
      next_inst_id[i + 1] = (uns32)(hlt_chk->key_name.comp_name_net.value[i]);
   }

   next_inst_id[len+1] = hlt_chk->key_name.name.keyLen;
   for(i = 0; i < hlt_chk->key_name.name.keyLen; i++)
   {
      next_inst_id[len+1+i+1] = (uns32)(hlt_chk->key_name.name.key[i]);
   }

   *data = (NCSCONTEXT)hlt_chk;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfhealthchecktableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_HEALTH_CHECK_TABLE_ENTRY_ID table. This is the health check table. The
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

uns32 saamfhealthchecktableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: avd_hlt_node_find
 *
 * Purpose:  This function will find AVD_AVND structure of node on which specified 
 * component resides.
 *
 * Input: avd_cb - the AVD control block
 *        comp_name_net - The component name
 *
 * Returns: The pointer to AVD_AVND structure found.
 * 
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_AVND *avd_hlt_node_find(SaNameT comp_name_net, AVD_CL_CB *avd_cb)
{
   SaNameT comp_name;
   SaNameT node_name;
   AVD_AVND *node_on_hlt = AVD_AVND_NULL;

   m_NCS_MEMSET(&comp_name,0, sizeof(comp_name));
   m_NCS_MEMSET(&node_name, 0, sizeof(node_name));
   comp_name.length = m_NCS_OS_NTOHS(comp_name_net.length);

   m_NCS_MEMCPY(comp_name.value,comp_name_net.value, comp_name.length);

  /* Copy Node DN from Component DN */
   if(avsv_cpy_node_DN_from_DN(&node_name,&comp_name) == NCSCC_RC_SUCCESS)
   {
      /* Find AVD_AVND structre */
      node_on_hlt = avd_avnd_struc_find(avd_cb,node_name);
      return node_on_hlt;
   }

  return AVD_AVND_NULL;
}



/*****************************************************************************
 * Function: avd_hlt_node_del_hlt_from_list
 *
 * Purpose:  This function will delete a health check table from the list at node.
 *
 * Input: hlt_chk - The Health Check table 
 *        node - Pointer to AVD_AVND structure 
 *
 * Returns: NCSCC_RC_SUCCESS    If health check table deleted successfully
 *          NCSCC_RC_FAILURE    If specified health check table not found
 * 
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_hlt_node_del_hlt_from_list(AVD_HLT *hlt_chk, AVD_AVND *node )
{
   AVD_HLT *temp_hlt_chk = node->list_of_hlt;
   AVD_HLT *prev_hlt_chk = AVD_HLT_NULL; 
 
    while(temp_hlt_chk != AVD_HLT_NULL)
    {
       /* Compare primary index (Component name) */
       if(m_NCS_MEMCMP(temp_hlt_chk->key_name.comp_name_net.value, hlt_chk->key_name.comp_name_net.value,
                       m_NCS_OS_NTOHS(hlt_chk->key_name.comp_name_net.length)) == 0)
       {
          /* Compare secondary index (Health check key) */          
          if(m_NCS_MEMCMP(temp_hlt_chk->key_name.name.key, hlt_chk->key_name.name.key,
                          hlt_chk->key_name.name.keyLen) == 0)
          {
             /* Update the list */
             if(temp_hlt_chk == node->list_of_hlt)
             {
                node->list_of_hlt = temp_hlt_chk->next_hlt_on_node;
                return NCSCC_RC_SUCCESS;
             }
             prev_hlt_chk->next_hlt_on_node = temp_hlt_chk->next_hlt_on_node;
             return NCSCC_RC_SUCCESS;
          }
       }
       prev_hlt_chk = temp_hlt_chk;
       temp_hlt_chk = temp_hlt_chk->next_hlt_on_node;
              
    }
   return NCSCC_RC_FAILURE;
}

/*****************************************************************************
 * Function: avd_hlt_ack_msg
 *
 * Purpose:  This function processes the acknowledgment received for
 *          a HLT related message from AVND for operator related changes.
 *          If the message acknowledges success change the row status of the
 *          HLT to active, if failure change row status to not in service.
 *
 * Input: cb - the AVD control block
 *        ack_msg - The acknowledgement message received from the AVND.
 *
 * Returns: none.
 *
 * NOTES: none.
 *
 *
 **************************************************************************/

void avd_hlt_ack_msg(AVD_CL_CB *cb,AVD_DND_MSG *ack_msg)
{
   AVD_HLT          *hlt=AVD_HLT_NULL;
   AVSV_PARAM_INFO  param;
   AVD_AVND       *avnd = AVD_AVND_NULL;

   /* check the ack message for errors. If error find the HLT that
    * has error.
    */

   if (ack_msg->msg_info.n2d_reg_hlt.error != NCSCC_RC_SUCCESS)
   {
      /* Find the HLT */
      hlt = avd_hlt_struc_find(cb,ack_msg->msg_info.n2d_reg_hlt.hltchk_name);
      if (hlt == AVD_HLT_NULL)
      {
         /* The HLT has already been deleted. there is nothing
          * that can be done.
          */

         /* Log an information error that the HLT is
          * deleted.
          */
         return;
      }

      /* log an fatal error as normally this shouldnt happen.
       */

      m_AVD_LOG_INVALID_VAL_FATAL(ack_msg->msg_info.n2d_reg_hlt.error);
      avnd =  avd_hlt_node_find(ack_msg->msg_info.n2d_reg_hlt.hltchk_name.comp_name_net, cb);
      avd_hlt_node_del_hlt_from_list(hlt, avnd);
      hlt->row_status = NCS_ROW_NOT_IN_SERVICE;
      m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, hlt, AVSV_CKPT_AVD_HLT_CONFIG);
      return;
   }

   /* Log an information that the node was updated with the
    * information of the HLT.
    */
   /* Find the HLT */
   hlt = avd_hlt_struc_find(cb,ack_msg->msg_info.n2d_reg_hlt.hltchk_name);
   if (hlt == AVD_HLT_NULL)
   {
      /* The HLT has already been deleted. there is nothing
       * that can be done.
       */

      /* Log an information error that the HLT is
       * deleted.
       */
      /* send a delete message to the AvND for the HLT. */
      m_NCS_MEMSET(((uns8 *)&param),'\0',sizeof(AVSV_PARAM_INFO));
      param.act = AVSV_OBJ_OPR_DEL;

      param.name_net.length = ack_msg->msg_info.n2d_reg_hlt.hltchk_name.comp_name_net.length;
      m_NCS_MEMCPY(param.name_net.value, ack_msg->msg_info.n2d_reg_hlt.hltchk_name.comp_name_net.value,
                   m_NCS_OS_NTOHS(param.name_net.length));

      param.name_sec_net.length = m_NCS_OS_HTONS(ack_msg->msg_info.n2d_reg_hlt.hltchk_name.name.keyLen);
      m_NCS_MEMCPY(param.name_sec_net.value, ack_msg->msg_info.n2d_reg_hlt.hltchk_name.name.key,
                   ack_msg->msg_info.n2d_reg_hlt.hltchk_name.name.keyLen);
      param.table_id = NCSMIB_TBL_AVSV_AMF_HLT_CHK;
      avnd = avd_avnd_struc_find_nodeid(cb,ack_msg->msg_info.n2d_reg_hlt.node_id);

      avd_snd_op_req_msg(cb,avnd,&param);
      return;
   }
   return;
}
               
