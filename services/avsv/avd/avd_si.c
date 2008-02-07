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
  the SI database and SU SI relationship list on the AVD. It also deals with
  all the MIB operations like set,get,getnext etc related to the 
  Service instance table and service unit service instance relationship table.


..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_si_struc_crt - creates and adds SI structure to database.
  avd_si_struc_find - Finds SI structure from the database.
  avd_si_struc_find_next - Finds the next SI structure from the database.  
  avd_si_struc_del - deletes and frees SI structure from database.
  avd_si_del_sg_list - deletes SI structure from SG list of SIs.
  saamfsitableentry_get - This function is one of the get processing
                        routines for objects in SA_AMF_S_I_TABLE_ENTRY_ID table.
  saamfsitableentry_extract - This function is one of the get processing
                            function for objects in SA_AMF_S_I_TABLE_ENTRY_ID
                            table.
  saamfsitableentry_set - This function is the set processing for objects in
                         SA_AMF_S_I_TABLE_ENTRY_ID table.
  saamfsitableentry_next - This function is the next processing for objects in
                         SA_AMF_S_I_TABLE_ENTRY_ID table.
  saamfsitableentry_setrow - This function is the setrow processing for
                           objects in SA_AMF_S_I_TABLE_ENTRY_ID table.
  ncssitableentry_rmvrow- This function is the setrow processing for
                           objects in SA_AMF_S_I_TABLE_ENTRY_ID table.


  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"


/*****************************************************************************
 * Function: avd_si_struc_crt
 *
 * Purpose:  This function will create and add a AVD_SI structure to the
 * tree if an element with si_name key value doesn't exist in the tree.
 *
 * Input: cb - the AVD control block
 *        si_name - The SI name of the service instance that needs to be
 *                    added.
 *        ckpt - TRUE - Create is called from ckpt update.
 *               FALSE - Create is called from AVD mib set.
 *
 * Returns: The pointer to AVD_SI structure allocated and added. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SI * avd_si_struc_crt(AVD_CL_CB *cb,SaNameT si_name, NCS_BOOL ckpt)
{
   AVD_SI *si;
 
   if( (si_name.length <= 6) || (m_NCS_OS_STRNCMP(si_name.value,"safSi=",6) != 0) )
   {
      return AVD_SI_NULL;
   }

   /* Allocate a new block structure now
    */
   if ((si = m_MMGR_ALLOC_AVD_SI) == AVD_SI_NULL)
   {
      /* log an error */
      m_AVD_LOG_MEM_FAIL(AVD_SI_ALLOC_FAILED);
      return AVD_SI_NULL;
   }

   m_NCS_MEMSET((char *)si, '\0', sizeof(AVD_SI));

   if (ckpt)
   {
      m_NCS_MEMCPY(si->name_net.value,si_name.value,m_NCS_OS_NTOHS(si_name.length));
      si->name_net.length = si_name.length;
   }
   else
   {
      m_NCS_MEMCPY(si->name_net.value,si_name.value,si_name.length);
      si->name_net.length = m_HTON_SANAMET_LEN(si_name.length);
      
      si->row_status = NCS_ROW_NOT_READY;
      si->si_switch = AVSV_SI_TOGGLE_STABLE;
      si->admin_state = NCS_ADMIN_STATE_UNLOCK;
      si->su_config_per_si = 1;
   }

   /* init pointers */
   si->list_of_csi = AVD_CSI_NULL;
   si->list_of_sisu = AVD_SU_SI_REL_NULL;
   si->sg_list_of_si_next = AVD_SI_NULL;
   si->sg_of_si = AVD_SG_NULL;


   si->tree_node.key_info = (uns8*)&(si->name_net);
   si->tree_node.bit   = 0;
   si->tree_node.left  = NCS_PATRICIA_NODE_NULL;
   si->tree_node.right = NCS_PATRICIA_NODE_NULL;

   if( ncs_patricia_tree_add(&cb->si_anchor,&si->tree_node) 
                      != NCSCC_RC_SUCCESS)
   {
      /* log an error */
      m_MMGR_FREE_AVD_SI(si);
      return AVD_SI_NULL;
   }


   return si;
   
}


/*****************************************************************************
 * Function: avd_si_struc_find
 *
 * Purpose:  This function will find a AVD_SI structure in the
 * tree with si_name value as key.
 *
 * Input: cb - the AVD control block
 *        si_name - The name of the service instance.
 *        host_order - Flag indicating if the length is host order
 *
 * Returns: The pointer to AVD_SI structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SI * avd_si_struc_find(AVD_CL_CB *cb,SaNameT si_name,NCS_BOOL host_order)
{
   AVD_SI *si;
   SaNameT  lsi_name;

   m_NCS_MEMSET((char *)&lsi_name, '\0', sizeof(SaNameT));
   lsi_name.length = (host_order == FALSE) ? si_name.length :  
                                            m_HTON_SANAMET_LEN(si_name.length);

   m_NCS_MEMCPY(lsi_name.value,si_name.value,m_NCS_OS_NTOHS(lsi_name.length));

   si = (AVD_SI *)ncs_patricia_tree_get(&cb->si_anchor, (uns8*)&lsi_name);

   return si;
}


/*****************************************************************************
 * Function: avd_si_struc_find_next
 *
 * Purpose:  This function will find the next AVD_SI structure in the
 * tree whose si_name value is next of the given si_name value.
 *
 * Input: cb - the AVD control block
 *        si_name - The name of the service instance.
 *        host_order - Flag indicating if the length is host order
 *
 * Returns: The pointer to AVD_SI structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SI * avd_si_struc_find_next(AVD_CL_CB *cb,SaNameT si_name,NCS_BOOL host_order)
{
   AVD_SI *si;
   SaNameT  lsi_name;

   m_NCS_MEMSET((char *)&lsi_name, '\0', sizeof(SaNameT));
   lsi_name.length = (host_order == FALSE) ? si_name.length :  
                                            m_HTON_SANAMET_LEN(si_name.length);

   m_NCS_MEMCPY(lsi_name.value,si_name.value,m_NCS_OS_NTOHS(lsi_name.length));

   si = (AVD_SI *)ncs_patricia_tree_getnext(&cb->si_anchor, (uns8*)&lsi_name);

   return si;
}


/*****************************************************************************
 * Function: avd_si_struc_del
 *
 * Purpose:  This function will delete and free AVD_SI structure from 
 * the tree.
 *
 * Input: cb - the AVD control block
 *        si - The SI structure that needs to be deleted.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_si_struc_del(AVD_CL_CB *cb,AVD_SI *si)
{
   if (si == AVD_SI_NULL)
      return NCSCC_RC_FAILURE;

   if(ncs_patricia_tree_del(&cb->si_anchor,&si->tree_node) 
                      != NCSCC_RC_SUCCESS)
   {
      /* log error */
      return NCSCC_RC_FAILURE;
   }

   m_MMGR_FREE_AVD_SI(si);
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: avd_si_del_sg_list
 *
 * Purpose:  This function will del the given SI from the SG list, and fill
 * the SIs pointer with NULL
 *
 * Input: cb - the AVD control block
 *        si - The SI pointer
 *
 * Returns: None. 
 *
 * NOTES: None
 *
 * 
 **************************************************************************/

void avd_si_del_sg_list(AVD_CL_CB *cb,AVD_SI *si)
{
   AVD_SI *i_si = AVD_SI_NULL;
   AVD_SI *prev_si = AVD_SI_NULL;

   if (si->sg_of_si != AVD_SG_NULL)
   {
      /* remove SI from SG */
      i_si = si->sg_of_si->list_of_si;

      while ((i_si != AVD_SI_NULL) && (i_si != si))
      {
         prev_si = i_si;
         i_si = i_si->sg_list_of_si_next;
      }

      if(i_si != si)
      {
         /* Log a fatal error */
      }else
      {
         if (prev_si == AVD_SI_NULL)
         {
            si->sg_of_si->list_of_si = i_si->sg_list_of_si_next;
         }else
         {
            prev_si->sg_list_of_si_next = si->sg_list_of_si_next;
         }                     
      }

      si->sg_list_of_si_next = AVD_SI_NULL;
      si->sg_of_si = AVD_SG_NULL;
   } /* if (si->sg_of_si != AVD_SG_NULL) */  
   
   return;
}

/*****************************************************************************
 * Function: avd_si_add_sg_list
 *
 * Purpose:  This function will add the given SI to the SG list, and fill
 * the SIs pointers. 
 *
 * Input: cb - the AVD control block
 *        si - The SI pointer
 *
 * Returns: None. 
 *
 * NOTES: None
 *
 * 
 **************************************************************************/

void avd_si_add_sg_list(AVD_CL_CB *cb,AVD_SI *si)
{
   AVD_SI *i_si = AVD_SI_NULL;
   AVD_SI *prev_si = AVD_SI_NULL;

   if ( (si == AVD_SI_NULL) || (si->sg_of_si == AVD_SG_NULL) )
      return;

   i_si = si->sg_of_si->list_of_si;

   while( (i_si != AVD_SI_NULL) && (i_si->rank < si->rank) )
   {
      prev_si = i_si;
      i_si = i_si->sg_list_of_si_next;
   }
   
   if(prev_si == AVD_SI_NULL)
   {
      si->sg_list_of_si_next = si->sg_of_si->list_of_si;
      si->sg_of_si->list_of_si = si;
   }
   else
   {
      prev_si->sg_list_of_si_next = si;
      si->sg_list_of_si_next = i_si;
   }
   return;
}
/*****************************************************************************
 * Function: saamfsitableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_S_I_TABLE_ENTRY_ID table. This is the SI table. The
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

uns32 saamfsitableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SI        *si;
   SaNameT       si_name;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   m_NCS_MEMSET(&si_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service instance database key from the instant ID */
   si_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < si_name.length; i++)
   {
      si_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   si = avd_si_struc_find(avd_cb,si_name,TRUE);

   if (si == AVD_SI_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)si;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfsitableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_AMF_S_I_TABLE_ENTRY_ID table. This is the SI table. The
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

uns32 saamfsitableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer)
{
   AVD_SI        *si = (AVD_SI *)data;

   if (si == AVD_SI_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   switch(param->i_param_id)
   {
   case saAmfSIParentSGName_ID:
       m_AVSV_OCTVAL_TO_PARAM(param, buffer,si->sg_name.length,
                              si->sg_name.value); 
       break;
   case saAmfSIOperState_ID:

      param->i_fmat_id = NCSMIB_FMAT_INT; 
      param->i_length = 1; 

      if(si->list_of_sisu != AVD_SU_SI_REL_NULL)
         param->info.i_int = NCS_OPER_STATE_ENABLE;
      else
         param->info.i_int = NCS_OPER_STATE_DISABLE;
         
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
 * Function: saamfsitableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_S_I_TABLE_ENTRY_ID table. This is the SI table. The
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

uns32 saamfsitableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SI        *si;
   SaNameT       si_name;
   SaNameT       temp_name;
   uns32 i;
   NCSMIBLIB_REQ_INFO temp_mib_req;
   NCS_BOOL       val_same_flag = FALSE;
   uns32 rc;
   NCS_ADMIN_STATE back_val;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_INV_VAL;  
   }

   m_NCS_MEMSET(&si_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service instance database key from the instant ID */
   si_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < si_name.length; i++)
   {
      si_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   si = avd_si_struc_find(avd_cb,si_name,TRUE);

   if (si == AVD_SI_NULL)
   {
      if((arg->req.info.set_req.i_param_val.i_param_id == saAmfSIRowStatus_ID)
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

      si = avd_si_struc_crt(avd_cb,si_name, FALSE);

      m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

      if(si == AVD_SI_NULL)
      {
         /* Invalid instance object */
         return NCSCC_RC_NO_INSTANCE; 
      }

   }else /* if (si == AVD_SI_NULL) */
   {
      /* The record is already available */

      if(arg->req.info.set_req.i_param_val.i_param_id == saAmfSIRowStatus_ID)
      {
         /* This is a row status operation */
         if (arg->req.info.set_req.i_param_val.info.i_int == (uns32)si->row_status)
         {
            /* row status object is same so nothing to be done. */
            return NCSCC_RC_SUCCESS;
         }

         switch(arg->req.info.set_req.i_param_val.info.i_int)
         {
         case NCS_ROW_ACTIVE:
            /* validate the structure to see if the row can be made active */

            if ((si->su_config_per_si == 0) || 
               (si->max_num_csi ==0) ||
               (si->rank == 0) )
            {
               /* log information error */
               return NCSCC_RC_INV_VAL;
            }
            
            /* check that the SG is present and is active.
             */

            if(si->sg_name.length == 0)
            {
               /* log information error */
               return NCSCC_RC_INV_VAL;
            }
            if ((si->sg_of_si = avd_sg_struc_find(avd_cb,si->sg_name,TRUE)) == AVD_SG_NULL)
            {
               return NCSCC_RC_INV_VAL;
            }
      
            if (si->sg_of_si->row_status != NCS_ROW_ACTIVE)
            {
               si->sg_of_si = AVD_SG_NULL;
               return NCSCC_RC_INV_VAL;
            }

            if(test_flag == TRUE)
            {
               return NCSCC_RC_SUCCESS;
            }

            /* add to SG-SI Rank Table */
            if(avd_sg_si_rank_add_row(avd_cb, si) == AVD_SG_SI_RANK_NULL)
            {
               return NCSCC_RC_INV_VAL;
            }

            /* add to the list of SG  */
            avd_si_add_sg_list(avd_cb, si);

            /* set the value, checkpoint the entire record.
             */
            si->row_status = NCS_ROW_ACTIVE;
            
            m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(avd_cb, si, AVSV_CKPT_AVD_SI_CONFIG);

            return NCSCC_RC_SUCCESS;
            break;

         case NCS_ROW_NOT_IN_SERVICE:
         case NCS_ROW_DESTROY:
            
            /* check if it is active currently */

            if(si->row_status == NCS_ROW_ACTIVE)
            {

               /* Check to see that the SI is in admin locked state before
                * making the row status as not in service or delete 
                */

               if(si->admin_state != NCS_ADMIN_STATE_LOCK)
               {
                  /* log information error */
                  return NCSCC_RC_INV_VAL;
               }

               /* Check to see that no CSI or SUSI exists on this SI */
               if((si->list_of_csi != AVD_CSI_NULL) ||
                  (si->list_of_sisu != AVD_SU_SI_REL_NULL))
               {
                  /* log information error */
                  return NCSCC_RC_INV_VAL;
               }

               if(test_flag == TRUE)
               {
                  return NCSCC_RC_SUCCESS;
               }              

               m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

               /* remove the SI from the SG list if present.
                */

               avd_si_del_sg_list(avd_cb,si);

               avd_sg_si_rank_del_row(avd_cb, si);
               
               m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);
               /* we need to delete this avnd structure on the
                * standby AVD.
                */

               /* check point to the standby AVD that this
                * record need to be deleted
                */
               m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(avd_cb, si, AVSV_CKPT_AVD_SI_CONFIG);
               
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

               /* remove the SI from the SG list if present.
                */

               avd_si_del_sg_list(avd_cb,si);
               
               avd_si_struc_del(avd_cb,si);

               m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

               return NCSCC_RC_SUCCESS;

            } /* if(arg->req.info.set_req.i_param_val.info.i_int
                  == NCS_ROW_DESTROY) */

            si->row_status = arg->req.info.set_req.i_param_val.info.i_int;
            return NCSCC_RC_SUCCESS;

            break;
         default:
            m_AVD_LOG_INVALID_VAL_ERROR(arg->req.info.set_req.i_param_val.info.i_int);
            /* Invalid row status object */
            return NCSCC_RC_INV_VAL;
            break;
         } /* switch(arg->req.info.set_req.i_param_val.info.i_int) */

      } /* if(arg->req.info.set_req.i_param_val.i_param_id == ncsSISiRowStatus_ID) */

   } /* if (si == AVD_SI_NULL) */

   /* We now have the si block */
   if(test_flag == TRUE)
   {
      return NCSCC_RC_SUCCESS;
   }

   
   if(si->row_status == NCS_ROW_ACTIVE)
   {
      if(avd_cb->init_state  != AVD_APP_STATE)
      {
         return NCSCC_RC_INV_VAL;
      }
      
      switch(arg->req.info.set_req.i_param_val.i_param_id)
      {
         case saAmfSINumCSIs_ID:
            if(si->admin_state == NCS_ADMIN_STATE_LOCK)
            {
               si->max_num_csi = arg->req.info.set_req.i_param_val.info.i_int;
               m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, si, AVSV_CKPT_AVD_SI_CONFIG);
            }
            break;
         case saAmfSIPrefNumAssignments_ID:
            si->su_config_per_si = arg->req.info.set_req.i_param_val.info.i_int;
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, si, AVSV_CKPT_AVD_SI_CONFIG);
            break;
         case saAmfSIAdminState_ID:
            if (arg->req.info.set_req.i_param_val.info.i_int == si->admin_state)
               return NCSCC_RC_SUCCESS;
            
            if (si->sg_of_si->sg_ncs_spec == SA_TRUE)
               return NCSCC_RC_INV_VAL;
            
            if(arg->req.info.set_req.i_param_val.info.i_int == NCS_ADMIN_STATE_UNLOCK)
            {
               if (si->sg_of_si->sg_fsm_state != AVD_SG_FSM_STABLE)
               {
                  return NCSCC_RC_INV_VAL;
               }
               
               m_AVD_SET_SI_ADMIN(cb,si,NCS_ADMIN_STATE_UNLOCK);
               if(si->max_num_csi == si->num_csi)
               {
                  switch(si->sg_of_si->su_redundancy_model)
                  {
                  case AVSV_SG_RED_MODL_2N:
                     if(avd_sg_2n_si_func(avd_cb, si) != NCSCC_RC_SUCCESS)
                     { 
                        m_AVD_SET_SI_ADMIN(cb,si,NCS_ADMIN_STATE_LOCK);
                        return NCSCC_RC_INV_VAL;
                     }
                     break;

                  case AVSV_SG_RED_MODL_NWAYACTV:
                     if(avd_sg_nacvred_si_func(avd_cb, si) != NCSCC_RC_SUCCESS)
                     { 
                        m_AVD_SET_SI_ADMIN(cb,si,NCS_ADMIN_STATE_LOCK); 
                        return NCSCC_RC_INV_VAL;
                     }
                     break;

                  case AVSV_SG_RED_MODL_NWAY:
                     if(avd_sg_nway_si_func(avd_cb, si) != NCSCC_RC_SUCCESS)
                     { 
                        m_AVD_SET_SI_ADMIN(cb,si,NCS_ADMIN_STATE_LOCK); 
                        return NCSCC_RC_INV_VAL;
                     }
                     break;
                     
                  case AVSV_SG_RED_MODL_NPM:
                     if(avd_sg_npm_si_func(avd_cb, si) != NCSCC_RC_SUCCESS)
                     { 
                        m_AVD_SET_SI_ADMIN(cb,si,NCS_ADMIN_STATE_LOCK); 
                        return NCSCC_RC_INV_VAL;
                     }
                     break;

                  case AVSV_SG_RED_MODL_NORED:
                  default:
                     if(avd_sg_nored_si_func(avd_cb, si) != NCSCC_RC_SUCCESS)
                     { 
                        m_AVD_SET_SI_ADMIN(cb,si,NCS_ADMIN_STATE_LOCK);
                        return NCSCC_RC_INV_VAL;
                     }
                     break;
                  }            
                  
               }
               
               return NCSCC_RC_SUCCESS;
            }
            else
            {
               if ((si->admin_state == NCS_ADMIN_STATE_LOCK) && 
                  (arg->req.info.set_req.i_param_val.info.i_int == NCS_ADMIN_STATE_SHUTDOWN))
                  return NCSCC_RC_INV_VAL;
               
               if (si->list_of_sisu == AVD_SU_SI_REL_NULL)
               {
                  m_AVD_SET_SI_ADMIN(cb,si,NCS_ADMIN_STATE_LOCK);
                  return NCSCC_RC_SUCCESS;
               }            
               
               /* Check if other semantics are happening for other SUs. If yes
                * return an error.
                */
               if (si->sg_of_si->sg_fsm_state != AVD_SG_FSM_STABLE)
               {
                  if((si->sg_of_si->sg_fsm_state != AVD_SG_FSM_SI_OPER) ||
                     (si->admin_state != NCS_ADMIN_STATE_SHUTDOWN) || 
                     (arg->req.info.set_req.i_param_val.info.i_int != NCS_ADMIN_STATE_LOCK))
                  {
                     return NCSCC_RC_INV_VAL;
                  }
               }/*if (si->sg_of_si->sg_fsm_state != AVD_SG_FSM_STABLE)*/
               
               
               back_val = si->admin_state;
               m_AVD_SET_SI_ADMIN(cb,si,(arg->req.info.set_req.i_param_val.info.i_int));
               switch(si->sg_of_si->su_redundancy_model)
               {
               case AVSV_SG_RED_MODL_2N:
                  if(avd_sg_2n_si_admin_down(avd_cb, si) != NCSCC_RC_SUCCESS)
                  {
                     m_AVD_SET_SI_ADMIN(cb,si,back_val);
                     return NCSCC_RC_INV_VAL;
                  }
                  break;

               case AVSV_SG_RED_MODL_NWAY:
                  if(avd_sg_nway_si_admin_down(avd_cb, si) != NCSCC_RC_SUCCESS)
                  {
                     m_AVD_SET_SI_ADMIN(cb,si,back_val);
                     return NCSCC_RC_INV_VAL;
                  }
                  break;

               case AVSV_SG_RED_MODL_NWAYACTV:
                  if(avd_sg_nacvred_si_admin_down(avd_cb, si) != NCSCC_RC_SUCCESS)
                  {
                     m_AVD_SET_SI_ADMIN(cb,si,back_val);
                     return NCSCC_RC_INV_VAL;
                  }
                  break;
                  
               case AVSV_SG_RED_MODL_NPM:
                  if(avd_sg_npm_si_admin_down(avd_cb, si) != NCSCC_RC_SUCCESS)
                  {
                     m_AVD_SET_SI_ADMIN(cb,si,back_val);
                     return NCSCC_RC_INV_VAL;
                  }
                  break;

               case AVSV_SG_RED_MODL_NORED:
               default:
                  if(avd_sg_nored_si_admin_down(avd_cb, si) != NCSCC_RC_SUCCESS)
                  {
                     m_AVD_SET_SI_ADMIN(cb,si,back_val);
                     return NCSCC_RC_INV_VAL;
                  }
                  break;
               } 
               
               return NCSCC_RC_SUCCESS;
            }            
            break;
         default:
            
            /* when row status is active we don't allow any other MIB object to be
             * modified.
             */
 
            return NCSCC_RC_INV_VAL;
      }
      return NCSCC_RC_SUCCESS;
   }


   switch(arg->req.info.set_req.i_param_val.i_param_id)
   {
   case saAmfSIRowStatus_ID:
      /* fill the row status value */
      if(arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT)
      {
         si->row_status = arg->req.info.set_req.i_param_val.info.i_int;
      }
      break;
   case saAmfSIParentSGName_ID:
      
      /* fill the SG name value */
      temp_name.length = arg->req.info.set_req.i_param_val.i_length;
      m_NCS_MEMCPY(temp_name.value,
                   arg->req.info.set_req.i_param_val.info.i_oct,
                   temp_name.length);

      /* check if already This SI is part of a SG */
      if(si->sg_of_si != AVD_SG_NULL)
      {
         if (temp_name.length == si->sg_of_si->name_net.length)
         {
            if(m_NCS_MEMCMP(si->sg_of_si->name_net.value,
                            temp_name.value,temp_name.length) == 0)
            {
               return NCSCC_RC_SUCCESS;
            }
         }
         
         /* delete the SI from the prev SG list */
         avd_si_del_sg_list(avd_cb,si);
      }

      si->sg_name.length = arg->req.info.set_req.i_param_val.i_length;
      m_NCS_MEMCPY(si->sg_name.value, 
                   arg->req.info.set_req.i_param_val.info.i_oct, 
                   si->sg_name.length);
      break;

   default:
      m_NCS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

      temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
      temp_mib_req.info.i_set_util_info.param = &(arg->req.info.set_req.i_param_val);
      temp_mib_req.info.i_set_util_info.var_info = var_info;
      temp_mib_req.info.i_set_util_info.data = si;
      temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

      /* call the mib routine handler */ 
      if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
      {
         return rc;
      }
      break;
   } /* switch(arg->req.info.set_req.i_param_val.i_param_id) */
   
   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: saamfsitableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_S_I_TABLE_ENTRY_ID table. This is the SI table. The
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

uns32 saamfsitableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SI        *si;
   SaNameT       si_name;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   m_NCS_MEMSET(&si_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service instance database key from the instant ID */
   if (arg->i_idx.i_inst_len != 0)
   {
      si_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
      for(i = 0; i < si_name.length; i++)
      {
         si_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
   }

   si = avd_si_struc_find_next(avd_cb,si_name,TRUE);

   if (si == AVD_SI_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the instant ID from the SI name */

   *next_inst_id_len = m_NCS_OS_NTOHS(si->name_net.length) + 1;

   next_inst_id[0] = *next_inst_id_len -1;
   for(i = 0; i < next_inst_id[0]; i++)
   {
      next_inst_id[i + 1] = (uns32)(si->name_net.value[i]);
   }

   *data = (NCSCONTEXT)si;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfsitableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_S_I_TABLE_ENTRY_ID table. This is the SI table. The
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

uns32 saamfsitableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: ncssitableentry_rmvrow
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
uns32 ncssitableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx) 
{
   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: ncssitableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in NCS_S_I_TABLE_ENTRY_ID table. This is the properitary SI table. The
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

uns32 ncssitableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SI        *si;
   SaNameT       si_name;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   m_NCS_MEMSET(&si_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service instance database key from the instant ID */
   si_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < si_name.length; i++)
   {
      si_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   si = avd_si_struc_find(avd_cb,si_name,TRUE);

   if (si == AVD_SI_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)si;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: ncssitableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * NCS_S_I_TABLE_ENTRY_ID table. This is the properitary SI table. The
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

uns32 ncssitableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer)
{
   AVD_SI        *si = (AVD_SI *)data;

   if (si == AVD_SI_NULL)
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
 * Function: ncssitableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * NCS_S_I_TABLE_ENTRY_ID table. This is the properitary SI table. The
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

uns32 ncssitableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SI        *si;
   SaNameT       si_name;
   uns32         i;
   NCSMIBLIB_REQ_INFO temp_mib_req;
   NCS_BOOL      val_same_flag = FALSE;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_INV_VAL;  
   }

   m_NCS_MEMSET(&si_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service instance database key from the instant ID */
   si_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < si_name.length; i++)
   {
      si_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   si = avd_si_struc_find(avd_cb,si_name,TRUE);

   if(si == AVD_SI_NULL)
   {
      /* Invalid instance object */
      return NCSCC_RC_NO_INSTANCE; 
   }
   
   if(si->row_status != NCS_ROW_ACTIVE)
      return NCSCC_RC_INV_VAL;  

   if(si->sg_of_si->sg_ncs_spec)
      return NCSCC_RC_INV_VAL;
   
   if(avd_cb->init_state  != AVD_APP_STATE)
   {
      return NCSCC_RC_INV_VAL;
   }
   
   if(test_flag == TRUE)
   {
      return NCSCC_RC_SUCCESS;
   }

   /* We now have the si block */

   if(arg->req.info.set_req.i_param_val.i_param_id == ncsSISwitchState_ID)
   {
      if((arg->req.info.set_req.i_param_val.info.i_int != AVSV_SI_TOGGLE_SWITCH)
         || si->si_switch == AVSV_SI_TOGGLE_SWITCH)
      {
         return NCSCC_RC_INV_VAL;
      }
      
      m_AVD_SET_SI_SWITCH(cb,si,AVSV_SI_TOGGLE_SWITCH);
      
      switch(si->sg_of_si->su_redundancy_model)
      {
      case AVSV_SG_RED_MODL_2N:
         if (avd_sg_2n_siswitch_func(avd_cb, si) != NCSCC_RC_SUCCESS)
         {
            m_AVD_SET_SI_SWITCH(cb,si,AVSV_SI_TOGGLE_STABLE);
            return NCSCC_RC_INV_VAL;
         }
         break;

      case AVSV_SG_RED_MODL_NWAY:
         if (avd_sg_nway_siswitch_func(avd_cb, si) != NCSCC_RC_SUCCESS)
         {
            si->si_switch = AVSV_SI_TOGGLE_STABLE;
            return NCSCC_RC_INV_VAL;
         }
         break;
         
      case AVSV_SG_RED_MODL_NPM:
         if (avd_sg_npm_siswitch_func(avd_cb, si) != NCSCC_RC_SUCCESS)
         {
            si->si_switch = AVSV_SI_TOGGLE_STABLE;
            return NCSCC_RC_INV_VAL;
         }
         break;

      case AVSV_SG_RED_MODL_NWAYACTV:
      case AVSV_SG_RED_MODL_NORED:
      default:
         m_AVD_SET_SI_SWITCH(cb,si,AVSV_SI_TOGGLE_STABLE);
         return NCSCC_RC_INV_VAL;
         break;
      }
      
      return NCSCC_RC_SUCCESS;
      
   }


   m_NCS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

   temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
   temp_mib_req.info.i_set_util_info.param = &(arg->req.info.set_req.i_param_val);
   temp_mib_req.info.i_set_util_info.var_info = var_info;
   temp_mib_req.info.i_set_util_info.data = si;
   temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

   /* call the mib routine handler */ 
   return ncsmiblib_process_req(&temp_mib_req);

}



/*****************************************************************************
 * Function: ncssitableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * NCS_S_I_TABLE_ENTRY_ID table. This is the properitary SI table. The
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

uns32 ncssitableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SI        *si;
   SaNameT       si_name;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   m_NCS_MEMSET(&si_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service instance database key from the instant ID */
   if (arg->i_idx.i_inst_len != 0)
   {
      si_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
      for(i = 0; i < si_name.length; i++)
      {
         si_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
   }

   si = avd_si_struc_find_next(avd_cb,si_name,TRUE);

   if (si == AVD_SI_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the instant ID from the SI name */

   *next_inst_id_len = m_NCS_OS_NTOHS(si->name_net.length) + 1;

   next_inst_id[0] = *next_inst_id_len -1;
   for(i = 0; i < next_inst_id[0]; i++)
   {
      next_inst_id[i + 1] = (uns32)(si->name_net.value[i]);
   }

   *data = (NCSCONTEXT)si;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: ncssitableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * NCS_S_I_TABLE_ENTRY_ID table. This is the properitary SI table. The
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

uns32 ncssitableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: saamfsisideptableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_S_I_S_I_DEP_TABLE_ENTRY_ID table. This is the SI SI dependency table. The
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

uns32 saamfsisideptableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data)
{
   return NCSCC_RC_NO_INSTANCE;
}

/*****************************************************************************
 * Function: saamfsisideptableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_AMF_S_I_S_I_DEP_TABLE_ENTRY_ID table. This is the SI SI dependency table. The
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

uns32 saamfsisideptableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer)
{
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfsisideptableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_S_I_S_I_DEP_TABLE_ENTRY_ID table. This is the SI SI dependency table. The
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

uns32 saamfsisideptableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{
   return NCSCC_RC_FAILURE;
}



/*****************************************************************************
 * Function: saamfsisideptableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_S_I_S_I_DEP_TABLE_ENTRY_ID table. This is the SI SI dependency table. The
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

uns32 saamfsisideptableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
   return NCSCC_RC_NO_INSTANCE;
}


/*****************************************************************************
 * Function: saamfsisideptableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_S_I_S_I_DEP_TABLE_ENTRY_ID table. This is the SI SI dependency table. The
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

uns32 saamfsisideptableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}


/*****************************************************************************
 * Function: avd_sg_si_rank_add_row
 *
 * Purpose:  This function will create and add a AVD_SG_SI_RANK structure to the
 * tree if an element with given index value doesn't exist in the tree.
 *
 * Input: cb - the AVD control block
 *        si - Pointer to service instance row 
 *
 * Returns: The pointer to AVD_SG_SI_RANK structure allocated and added.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SG_SI_RANK * avd_sg_si_rank_add_row(AVD_CL_CB *cb, AVD_SI *si)
{  
   AVD_SG_SI_RANK *rank_elt = AVD_SG_SI_RANK_NULL;

   /* Allocate a new block structure now
    */
   if ((rank_elt = m_MMGR_ALLOC_AVD_SG_SI_RANK) == AVD_SG_SI_RANK_NULL)
   {
      /* log an error */
      m_AVD_LOG_MEM_FAIL(AVD_SG_SI_RANK_ALLOC_FAILED);
      return AVD_SG_SI_RANK_NULL;
   }

   m_NCS_MEMSET((char *)rank_elt, '\0', sizeof(AVD_SG_SI_RANK));

   rank_elt->indx.sg_name_net.length = m_NCS_OS_HTONS(si->sg_name.length);

   m_NCS_MEMCPY(rank_elt->indx.sg_name_net.value,si->sg_name.value,si->sg_name.length);

   rank_elt->indx.si_rank_net = m_NCS_OS_HTONL(si->rank);
 
   rank_elt->tree_node.key_info = (uns8*)(&rank_elt->indx);
   rank_elt->tree_node.bit   = 0;
   rank_elt->tree_node.left  = NCS_PATRICIA_NODE_NULL;
   rank_elt->tree_node.right = NCS_PATRICIA_NODE_NULL;

   if( ncs_patricia_tree_add(&cb->sg_si_rank_anchor,&rank_elt->tree_node)
                      != NCSCC_RC_SUCCESS)
   {
      /* log an error */
      m_MMGR_FREE_AVD_SG_SI_RANK(rank_elt);
      return AVD_SG_SI_RANK_NULL;
   }

   rank_elt->si_name.length = m_NCS_OS_NTOHS(si->name_net.length);
   m_NCS_MEMCPY(rank_elt->si_name.value, si->name_net.value, rank_elt->si_name.length);


   return rank_elt;

}


/*****************************************************************************
 * Function: avd_sg_si_rank_struc_find
 *
 * Purpose:  This function will find a AVD_SG_SI_RANK structure in the
 * tree with indx value as key.
 *
 * Input: cb - the AVD control block
 *        indx - The key.
 *
 * Returns: The pointer to AVD_SG_SI_RANK structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SG_SI_RANK * avd_sg_si_rank_struc_find(AVD_CL_CB *cb, AVD_SG_SI_RANK_INDX indx)
{
   AVD_SG_SI_RANK *rank_elt = AVD_SG_SI_RANK_NULL;

   rank_elt = (AVD_SG_SI_RANK *)ncs_patricia_tree_get(&cb->sg_si_rank_anchor, (uns8*)&indx);

   return rank_elt;
}


/*****************************************************************************
 * Function: avd_sg_si_rank_struc_find_next
 *
 * Purpose:  This function will find the next AVD_SG_SI_RANK structure in the
 * tree whose key value is next of the given key value.
 *
 * Input: cb - the AVD control block
 *        indx - The key value.
 *
 * Returns: The pointer to AVD_SG_SI_RANK structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SG_SI_RANK * avd_sg_si_rank_struc_find_next(AVD_CL_CB *cb, AVD_SG_SI_RANK_INDX indx)
{
   AVD_SG_SI_RANK *rank_elt = AVD_SG_SI_RANK_NULL;

   rank_elt = (AVD_SG_SI_RANK *)ncs_patricia_tree_getnext(&cb->sg_si_rank_anchor, (uns8*)&indx);

   return rank_elt;
}


/*****************************************************************************
 * Function: avd_sg_si_rank_del_row
 *
 * Purpose:  This function will delete and free AVD_SG_SI_RANK structure from 
 * the tree.
 *
 * Input: cb - The AVD control block
 *        si - Pointer to service instance row 
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE  
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_sg_si_rank_del_row(AVD_CL_CB *cb, AVD_SI *si)
{
   AVD_SG_SI_RANK_INDX    indx;
   AVD_SG_SI_RANK *       rank_elt = AVD_SG_SI_RANK_NULL;

   if (si == AVD_SI_NULL)
      return NCSCC_RC_FAILURE;

   m_NCS_MEMSET((char *)&indx, '\0', sizeof(AVD_SG_SI_RANK_INDX));

   indx.sg_name_net.length = m_NCS_OS_HTONS(si->sg_name.length);

   m_NCS_MEMCPY(indx.sg_name_net.value,si->sg_name.value,si->sg_name.length);

   indx.si_rank_net = m_NCS_OS_HTONL(si->rank);

   rank_elt = avd_sg_si_rank_struc_find(cb, indx);

   /* Row not found */
   if(rank_elt == AVD_SG_SI_RANK_NULL)
   {
      return NCSCC_RC_FAILURE;
   }       

   if(ncs_patricia_tree_del(&cb->sg_si_rank_anchor,&rank_elt->tree_node)
                      != NCSCC_RC_SUCCESS)
   {
      /* log error */
      return NCSCC_RC_FAILURE;
   }

   m_MMGR_FREE_AVD_SG_SI_RANK(rank_elt);

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfsgsirankentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_S_G_S_I_RANK_ENTRY_ID table. This is the SG-SI Rank table. The
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


uns32 saamfsgsirankentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg,
                             NCSCONTEXT* data)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SG_SI_RANK_INDX   indx;
   uns16 len;
   uns32 i;
   AVD_SG_SI_RANK *   rank_elt = AVD_SG_SI_RANK_NULL;

   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   m_NCS_MEMSET(&indx, '\0', sizeof(AVD_SG_SI_RANK_INDX));

   /* Prepare the SuperSiRank database key from the instant ID */
   len = (SaUint16T)arg->i_idx.i_inst_ids[0];

   indx.sg_name_net.length  = m_NCS_OS_HTONS(len);

   for(i = 0; i < len; i++)
   {
      indx.sg_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }
   if (arg->i_idx.i_inst_len > len + 1 )
   {
      indx.si_rank_net = m_NCS_OS_HTONL((SaUint32T)arg->i_idx.i_inst_ids[len + 1]);
   }

   rank_elt = avd_sg_si_rank_struc_find(avd_cb,indx);

   if (rank_elt == AVD_SG_SI_RANK_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)rank_elt;

   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: saamfsgsirankentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_AMF_S_G_S_I_RANK_ENTRY_ID table. This is the SG-SI Rank table. The
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


uns32 saamfsgsirankentry_extract(NCSMIB_PARAM_VAL* param,
                                 NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                                 NCSCONTEXT buffer)
{
   AVD_SG_SI_RANK *   rank_elt = (AVD_SG_SI_RANK *)data;

   if (rank_elt == AVD_SG_SI_RANK_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   switch(param->i_param_id)
   {

   case saAmfSGSIRankSIName_ID:
      m_AVSV_OCTVAL_TO_PARAM(param, buffer, rank_elt->si_name.length,
                             rank_elt->si_name.value);
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
 * Function: saamfsgsirankentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_S_G_S_I_RANK_ENTRY_ID table. This is the SG-SIRank table. The
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


uns32 saamfsgsirankentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
                              NCSCONTEXT* data, uns32* next_inst_id,
                              uns32 *next_inst_id_len)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SG_SI_RANK_INDX   indx;
   uns16 len;
   uns32 i;
   AVD_SG_SI_RANK *   rank_elt = AVD_SG_SI_RANK_NULL;

   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   m_NCS_MEMSET(&indx, '\0', sizeof(AVD_SG_SI_RANK_INDX));

   if (arg->i_idx.i_inst_len != 0)
   {

      /* Prepare the SuperSiRank database key from the instant ID */
      len = (SaUint16T)arg->i_idx.i_inst_ids[0];

      indx.sg_name_net.length  = m_NCS_OS_HTONS(len);

      for(i = 0; i < len; i++)
      {
         indx.sg_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
      if (arg->i_idx.i_inst_len > len + 1)
      {
         indx.si_rank_net = m_NCS_OS_HTONL((SaUint32T)arg->i_idx.i_inst_ids[len + 1]);
      }
   }

   rank_elt = avd_sg_si_rank_struc_find_next(avd_cb,indx);

   if (rank_elt == AVD_SG_SI_RANK_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)rank_elt;

   /* Prepare the instant ID from the SI name and SU rank */
   len = m_NCS_OS_NTOHS(rank_elt->indx.sg_name_net.length);

   *next_inst_id_len = len + 1 + 1;

   next_inst_id[0] = len;

   for(i = 0; i < len; i++)
   {
      next_inst_id[i + 1] = (uns32)(rank_elt->indx.sg_name_net.value[i]);
   }

   next_inst_id[len+1] = m_NCS_OS_NTOHL(rank_elt->indx.si_rank_net);

   *data = (NCSCONTEXT)rank_elt;

   return NCSCC_RC_SUCCESS;
}




/*****************************************************************************
 * Function: saamfsgsirankentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_S_G_S_I_RANK_ENTRY_ID table. This is the SG-SIRank table. The
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


uns32 saamfsgsirankentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                             NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{
   return NCSCC_RC_FAILURE;
}


/*****************************************************************************
 * Function: saamfsgsirankentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_S_G_S_I_RANK_ENTRY_ID table. This is the SG-SIRank table. The
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


uns32 saamfsgsirankentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: saamfsgsirankentry_rmvrow 
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
uns32 saamfsgsirankentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx)
{
   return NCSCC_RC_SUCCESS;
}
