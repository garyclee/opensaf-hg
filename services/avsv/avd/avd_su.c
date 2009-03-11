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
  the SU database on the AVD. It also deals with all the MIB operations
  like set,get,getnext etc related to the SU table.


..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_su_struc_crt - creates and adds SU structure to database.
  avd_su_struc_find - Finds SU structure from the database.
  avd_su_struc_find_next - Finds the next SU structure from the database.  
  avd_su_struc_del - deletes and frees SU structure from database.
  avd_su_del_avnd_list - deletes SU structure from avnd list of SUs.
  avd_su_del_sg_list - deletes SU structure from SG list of SUs.
  saamfsutableentry_get - This function is one of the get processing
                        routines for objects in SA_AMF_S_U_TABLE_ENTRY_ID table.
  saamfsutableentry_extract - This function is one of the get processing
                            function for objects in SA_AMF_S_U_TABLE_ENTRY_ID
                            table.
  saamfsutableentry_set - This function is the set processing for objects in
                         SA_AMF_S_U_TABLE_ENTRY_ID table.
  saamfsutableentry_next - This function is the next processing for objects in
                         SA_AMF_S_U_TABLE_ENTRY_ID table.
  saamfsutableentry_setrow - This function is the setrow processing for
                           objects in SA_AMF_S_U_TABLE_ENTRY_ID table.
  avd_su_ack_msg - This function processes the acknowledgment received for
                   a SU related message from AVND for operator related
                   addition.


  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"


/*****************************************************************************
 * Function: avd_su_struc_crt
 *
 * Purpose:  This function will create and add a AVD_SU structure to the
 * tree if an element with su_name key value doesn't exist in the tree.
 *
 * Input: cb - the AVD control block
 *        su_name - The SU name of the SU that needs to be
 *                    added.
 *
 * Returns: The pointer to AVD_SU structure allocated and added. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SU * avd_su_struc_crt(AVD_CL_CB *cb,SaNameT su_name, NCS_BOOL ckpt)
{
   AVD_SU *su;
   NCS_BOOL su_is_external = FALSE;

   if( (su_name.length <= 6) || (m_NCS_OS_STRNCMP(su_name.value,"safSu=",6) != 0) )
   {
      /* Check wether it is an external component. */
      if(m_NCS_OS_STRNCMP(su_name.value,"safEsu=",7) != 0)
        return AVD_SU_NULL;
      else
        su_is_external = TRUE;
   }

   /* Allocate a new block structure now
    */
   if ((su = m_MMGR_ALLOC_AVD_SU) == AVD_SU_NULL)
   {
      /* log an error */
      m_AVD_LOG_MEM_FAIL(AVD_SU_ALLOC_FAILED);
      return AVD_SU_NULL;
   }

   memset((char *)su, '\0', sizeof(AVD_SU));

   if (ckpt)
   {
      memcpy(su->name_net.value,su_name.value,m_NCS_OS_NTOHS(su_name.length));
      su->name_net.length = su_name.length;
   }
   else
   {
      memcpy(su->name_net.value,su_name.value,su_name.length);
      su->name_net.length = m_HTON_SANAMET_LEN(su_name.length);
      
      su->is_su_failover = FALSE; 
      su->term_state = FALSE;
      su->su_switch = AVSV_SI_TOGGLE_STABLE;
      su->su_preinstan = TRUE;
      
      su->admin_state = NCS_ADMIN_STATE_UNLOCK;
      su->oper_state = NCS_OPER_STATE_DISABLE;
      su->pres_state = NCS_PRES_UNINSTANTIATED;
      su->readiness_state = NCS_OUT_OF_SERVICE;
      su->su_act_state = AVD_SU_NO_STATE;
      su->su_is_external = su_is_external;
      su->row_status = NCS_ROW_NOT_READY;
   }

   /* Init pointers */
   su->sg_of_su = AVD_SG_NULL;
   su->su_on_node = AVD_AVND_NULL;
   su->list_of_susi = AVD_SU_SI_REL_NULL;
   su->list_of_comp = AVD_COMP_NULL;
   su->sg_list_su_next = AVD_SU_NULL;
   su->avnd_list_su_next = AVD_SU_NULL;

   su->tree_node.key_info = (uns8*)&(su->name_net);
   su->tree_node.bit   = 0;
   su->tree_node.left  = NCS_PATRICIA_NODE_NULL;
   su->tree_node.right = NCS_PATRICIA_NODE_NULL;

   if( ncs_patricia_tree_add(&cb->su_anchor,&su->tree_node) 
                      != NCSCC_RC_SUCCESS)
   {
      /* log an error */
      m_MMGR_FREE_AVD_SU(su);
      return AVD_SU_NULL;
   }


   return su;
   
}


/*****************************************************************************
 * Function: avd_su_struc_find
 *
 * Purpose:  This function will find a AVD_SU structure in the
 * tree with su_name value as key.
 *
 * Input: cb - the AVD control block
 *        su_name - The name of the SU.
 *        host_order - Flag indicating if the length is host order
 *
 * Returns: The pointer to AVD_SU structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SU * avd_su_struc_find(AVD_CL_CB *cb,SaNameT su_name,NCS_BOOL host_order)
{
   AVD_SU *su;
   SaNameT  lsu_name;

   memset((char *)&lsu_name, '\0', sizeof(SaNameT));
   lsu_name.length = (host_order == FALSE) ? su_name.length :  
                                            m_HTON_SANAMET_LEN(su_name.length);

   memcpy(lsu_name.value,su_name.value,m_NCS_OS_NTOHS(lsu_name.length));

   su = (AVD_SU *)ncs_patricia_tree_get(&cb->su_anchor, (uns8*)&lsu_name);

   return su;
}



/*****************************************************************************
 * Function: avd_su_struc_find_next
 *
 * Purpose:  This function will find the next AVD_SU structure in the
 * tree whose su_name value is next of the given su_name value.
 *
 * Input: cb - the AVD control block
 *        su_name - The name of the SU.
 *        host_order - Flag indicating if the length is host order
 *
 * Returns: The pointer to AVD_SU structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SU * avd_su_struc_find_next(AVD_CL_CB *cb,SaNameT su_name,NCS_BOOL host_order)
{
   AVD_SU *su;
   SaNameT  lsu_name;

   memset((char *)&lsu_name, '\0', sizeof(SaNameT));
   lsu_name.length = (host_order == FALSE) ? su_name.length :  
                                            m_HTON_SANAMET_LEN(su_name.length);

   memcpy(lsu_name.value,su_name.value,m_NCS_OS_NTOHS(lsu_name.length));

   su = (AVD_SU *)ncs_patricia_tree_getnext(&cb->su_anchor, (uns8*)&lsu_name);

   return su;
}


/*****************************************************************************
 * Function: avd_su_struc_del
 *
 * Purpose:  This function will delete and free AVD_SU structure from 
 * the tree.
 *
 * Input: cb - the AVD control block
 *        su - The SU structure that needs to be deleted.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_su_struc_del(AVD_CL_CB *cb,AVD_SU *su)
{
   if (su == AVD_SU_NULL)
      return NCSCC_RC_FAILURE;

   if(ncs_patricia_tree_del(&cb->su_anchor,&su->tree_node) 
                      != NCSCC_RC_SUCCESS)
   {
      /* log error */
      return NCSCC_RC_FAILURE;
   }

   m_MMGR_FREE_AVD_SU(su);
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: avd_su_del_avnd_list
 *
 * Purpose:  This function will del the given SU from the AVND list, and fill
 * the SUs pointer with NULL
 *
 * Input: cb - the AVD control block
 *        su - The SU pointer
 *
 * Returns: None. 
 *
 * NOTES: None
 *
 * 
 **************************************************************************/
void avd_su_del_avnd_list(AVD_CL_CB *cb,AVD_SU *su)
{
   AVD_SU *i_su = AVD_SU_NULL;
   AVD_SU *prev_su = AVD_SU_NULL;
   NCS_BOOL isNcs;

   if( (su->sg_of_su) && (su->sg_of_su->sg_ncs_spec == SA_TRUE))
      isNcs = TRUE;
   else
      isNcs = FALSE;

   /* For external component, there is no AvND attached, so let it return. */
   if (su->su_on_node != AVD_AVND_NULL)
   {
      /* remove SU from node */
      i_su= (isNcs) ? su->su_on_node->list_of_ncs_su : su->su_on_node->list_of_su;

      while ((i_su != AVD_SU_NULL) && (i_su != su))
      {
         prev_su = i_su;
         i_su = i_su->avnd_list_su_next;
      }

      if(i_su != su)
      {
         /* Log a fatal error */
      }else
      {
         if (prev_su == AVD_SU_NULL)
         {
            if(isNcs)
               su->su_on_node->list_of_ncs_su = su->avnd_list_su_next;
            else 
               su->su_on_node->list_of_su = su->avnd_list_su_next;
         }else
         {
               prev_su->avnd_list_su_next = su->avnd_list_su_next;
         }                     
      }

      su->avnd_list_su_next = AVD_SU_NULL;
      su->su_on_node = AVD_AVND_NULL;
   } /* if (su->su_on_node != AVD_AVND_NULL) */ 

   return;            
}


/*****************************************************************************
 * Function: avd_su_del_sg_list
 *
 * Purpose:  This function will del the given SU from the SG list, and fill
 * the SUs pointer with NULL
 *
 * Input: cb - the AVD control block
 *        su - The SU pointer
 *
 * Returns: None. 
 *
 * NOTES: None
 *
 * 
 **************************************************************************/

void avd_su_del_sg_list(AVD_CL_CB *cb,AVD_SU *su)
{
   AVD_SU *i_su = AVD_SU_NULL;
   AVD_SU *prev_su = AVD_SU_NULL;

   if (su->sg_of_su != AVD_SG_NULL)
   {
      /* remove SU from SG */
      i_su = su->sg_of_su->list_of_su;

      while ((i_su != AVD_SU_NULL) && (i_su != su))
      {
         prev_su = i_su;
         i_su = i_su->sg_list_su_next;
      }

      if(i_su != su)
      {
         /* Log a fatal error */
      }else
      {
         if (prev_su == AVD_SU_NULL)
         {
            su->sg_of_su->list_of_su = su->sg_list_su_next;
         }else
         {
            prev_su->sg_list_su_next = su->sg_list_su_next;
         }                     
      }

      su->sg_list_su_next = AVD_SU_NULL;
      su->sg_of_su = AVD_SG_NULL;
   } /* if (su->sg_of_su != AVD_SG_NULL) */  
   
   return;
}

/*****************************************************************************
 * Function: avd_su_add_sg_list
 *
 * Purpose:  This function will add the given SU to the SG list, and fill
 * the SUs pointers. 
 *
 * Input: cb - the AVD control block
 *        su - The SU pointer
 *
 * Returns: None. 
 *
 * NOTES: None
 *
 * 
 **************************************************************************/

void avd_su_add_sg_list(AVD_CL_CB *cb, AVD_SU *su)
{
   AVD_SU *i_su = AVD_SU_NULL;
   AVD_SU *prev_su = AVD_SU_NULL;

   if ( (su == AVD_SU_NULL) || (su->sg_of_su == AVD_SG_NULL) )
      return;

   i_su = su->sg_of_su->list_of_su;

   while( (i_su != AVD_SU_NULL) && (i_su->rank < su->rank) )
   {
      prev_su = i_su;
      i_su = i_su->sg_list_su_next;
   }
   
   if(prev_su == AVD_SU_NULL)
   {
      su->sg_list_su_next = su->sg_of_su->list_of_su;
      su->sg_of_su->list_of_su = su;
   }
   else
   {
      prev_su->sg_list_su_next = su;
      su->sg_list_su_next = i_su;
   }
   return;
}


/*****************************************************************************
 * Function: saamfsutableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_S_U_TABLE_ENTRY_ID table. This is the Service Unit table. The
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

uns32 saamfsutableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SU        *su;
   SaNameT       su_name;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   memset(&su_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service unit database key from the instant ID */
   su_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < su_name.length; i++)
   {
      su_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   su = avd_su_struc_find(avd_cb,su_name,TRUE);

   if (su == AVD_SU_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)su;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfsutableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_AMF_S_U_TABLE_ENTRY_ID table. This is the Service Unit table. The
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

uns32 saamfsutableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer)
{
   AVD_SU        *su = (AVD_SU *)data;

   if (su == AVD_SU_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   switch(param->i_param_id)
   {
   case saAmfSUFailOver_ID:
      param->i_fmat_id = NCSMIB_FMAT_INT; 
      param->i_length = 1; 
      param->info.i_int = (su->is_su_failover == FALSE) ? NCS_SNMP_FALSE:NCS_SNMP_TRUE;
      break;
   case saAmfSUPreInstantiable_ID:
      param->i_fmat_id = NCSMIB_FMAT_INT; 
      param->i_length = 1; 
      param->info.i_int = (su->su_preinstan == FALSE) ? NCS_SNMP_FALSE:NCS_SNMP_TRUE;
      break;
   case saAmfSUIsExternal_ID:
      param->i_fmat_id = NCSMIB_FMAT_INT;
      param->i_length = 1; 
      param->info.i_int = (su->su_is_external == FALSE) ? NCS_SNMP_FALSE:NCS_SNMP_TRUE;
      break;

   case saAmfSUParentSGName_ID:
      m_AVSV_OCTVAL_TO_PARAM(param, buffer, su->sg_name.length,
                             su->sg_name.value); 
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
 * Function: saamfsutableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_S_U_TABLE_ENTRY_ID table. This is the Service Unit table. The
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

uns32 saamfsutableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SU        *su;
   SaNameT       su_name;
   SaNameT       temp_name;
   uns32         i;
   NCS_BOOL      val_same_flag = FALSE, temp_bool;
   uns32         rc;
   NCSMIBLIB_REQ_INFO temp_mib_req;
   NCS_READINESS_STATE back_red_state;
   NCS_ADMIN_STATE back_admin_state;
   AVSV_PARAM_INFO param;
   AVD_AVND *su_node_ptr = NULL;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_INV_VAL;  
   }

   memset(&su_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service unit database key from the instant ID */
   su_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < su_name.length; i++)
   {
      su_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }
   
   if((arg->req.info.set_req.i_param_val.i_param_id == saAmfSUIsExternal_ID)
         && (arg->req.info.set_req.i_param_val.info.i_int != NCS_SNMP_FALSE))
   {
     return NCSCC_RC_INV_VAL;
   }

   su = avd_su_struc_find(avd_cb,su_name,TRUE);

   if (su == AVD_SU_NULL)
   {
      if((arg->req.info.set_req.i_param_val.i_param_id == saAmfSURowStatus_ID)
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

      su = avd_su_struc_crt(avd_cb,su_name, FALSE);

      m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

      if(su == AVD_SU_NULL)
      {
         /* Invalid instance object */
         return NCSCC_RC_NO_INSTANCE; 
      }

   }
   else /* if (su == AVD_SU_NULL) */
   {
      /* The record is already available */

      if(arg->req.info.set_req.i_param_val.i_param_id == saAmfSURowStatus_ID)
      {
         /* This is a row status operation */
         if (arg->req.info.set_req.i_param_val.info.i_int == (uns32)su->row_status)
         {
            /* row status object is same so nothing to be done. */
            return NCSCC_RC_SUCCESS;
         }

         switch(arg->req.info.set_req.i_param_val.info.i_int)
         {
         case NCS_ROW_ACTIVE:
            /* validate the structure to see if the row can be made active */

            /* check that the SG is present and is active.
             */
            
            if ((su->num_of_comp == 0) ||  (su->rank == 0))
            {
               /* All the mandatory fields are not filled
                */
               /* log information error */
               return NCSCC_RC_INV_VAL;
            }

            if(su->sg_name.length == 0)
            {
               /* log information error */
               return NCSCC_RC_INV_VAL;
            }
            if ((su->sg_of_su = avd_sg_struc_find(avd_cb,su->sg_name,TRUE)) == AVD_SG_NULL)
            {
               return NCSCC_RC_INV_VAL;
            }
      
            if (su->sg_of_su->row_status != NCS_ROW_ACTIVE)
            {
               su->sg_of_su = AVD_SG_NULL;
               return NCSCC_RC_INV_VAL;
            }
            /* Found the SG check if its NCS SG if yes make the 
             * SU state UNLOCKED 
             */
            if(su->sg_of_su->sg_ncs_spec == TRUE)
               su->admin_state = NCS_ADMIN_STATE_UNLOCK;

            /* add to the list of SG  */
            avd_su_add_sg_list(avd_cb, su);
      
            /* check that the NODE is present and is active.
             */
           if(FALSE == su->su_is_external)
           {
            /* First get the node name */
            avsv_cpy_node_DN_from_DN(&temp_name, &su_name); 

            if(temp_name.length == 0)
            {
               avd_su_del_sg_list(avd_cb,su);
               /* log information error */
               return NCSCC_RC_INV_VAL;
            }

            /* Find the AVND node and link it to the structure.
             */

            if ((su->su_on_node = avd_avnd_struc_find(avd_cb, temp_name)) 
                                                             == AVD_AVND_NULL)
            {
               avd_su_del_sg_list(avd_cb,su);
               return NCSCC_RC_INV_VAL;
            }
      
            if (su->su_on_node->row_status != NCS_ROW_ACTIVE)
            {
               su->su_on_node = AVD_AVND_NULL;  
               avd_su_del_sg_list(avd_cb,su);
         
               return NCSCC_RC_INV_VAL;
            }
            /* add to the list of AVND */ 
            if(su->sg_of_su->sg_ncs_spec == TRUE)
            {
               su->avnd_list_su_next = su->su_on_node->list_of_ncs_su;
               su->su_on_node->list_of_ncs_su = su;
            }
            else
            {
               su->avnd_list_su_next = su->su_on_node->list_of_su;
               su->su_on_node->list_of_su = su;
            }
            su_node_ptr = su->su_on_node;
           } /* if(FALSE == su->su_is_external)  */
           else
           {
                if(NULL == avd_cb->ext_comp_info.ext_comp_hlt_check)
                {
                  /* This is an external SU and we need to create the 
                     supporting info.*/
                  avd_cb->ext_comp_info.ext_comp_hlt_check = 
                                           m_MMGR_ALLOC_AVD_AVND;
                  if(NULL == avd_cb->ext_comp_info.ext_comp_hlt_check)
                  {
                    avd_su_del_sg_list(avd_cb,su);
                    return NCSCC_RC_INV_VAL;
                  }
                  memset(avd_cb->ext_comp_info.ext_comp_hlt_check, 0,
                               sizeof(AVD_AVND));
                  avd_cb->ext_comp_info.local_avnd_node = 
                     avd_avnd_struc_find_nodeid(avd_cb, avd_cb->node_id_avd);

                  if(NULL == avd_cb->ext_comp_info.local_avnd_node)
                  {
                    m_AVD_PXY_PXD_ERR_LOG(
                  "sutableentry_set:Local node unavailable:SU Name,node id are",
                    &su->name_net,avd_cb->node_id_avd,0,0,0);
                    avd_su_del_sg_list(avd_cb,su);
                    return NCSCC_RC_INV_VAL;
                  }

                } /* if(NULL == avd_cb->ext_comp_info.ext_comp_hlt_check) */

                su_node_ptr = avd_cb->ext_comp_info.local_avnd_node;

                if (su_node_ptr->row_status != NCS_ROW_ACTIVE)
                {
                  su->su_on_node = AVD_AVND_NULL;  
                  avd_su_del_sg_list(avd_cb,su);
         
                  return NCSCC_RC_INV_VAL;
                }
           } /* else of if(FALSE == su->su_is_external)  */


            if(test_flag == TRUE)
            {
               avd_su_del_avnd_list(avd_cb,su);
               avd_su_del_sg_list(avd_cb,su);

               return NCSCC_RC_SUCCESS;
            }

            /* add a row to the SG-SU Rank table */
            if(avd_sg_su_rank_add_row(avd_cb, su) == AVD_SG_SU_RANK_NULL )
            {
               avd_su_del_avnd_list(avd_cb,su);
               avd_su_del_sg_list(avd_cb,su);

               /* Row status can not be make active */
               return NCSCC_RC_INV_VAL;
            }

            /* set the value, checkpoint the entire record.
             */
            su->row_status = NCS_ROW_ACTIVE;

            if((su_node_ptr->node_state == AVD_AVND_STATE_PRESENT) ||
               (su_node_ptr->node_state == AVD_AVND_STATE_NO_CONFIG) ||
               (su_node_ptr->node_state == AVD_AVND_STATE_NCS_INIT))
            {
               if (avd_snd_su_msg(avd_cb, su) != NCSCC_RC_SUCCESS)
               {
                  avd_su_del_avnd_list(avd_cb,su);
                  avd_su_del_sg_list(avd_cb,su);

                  /* remove the SU from SG-SU Rank table*/
                  avd_sg_su_rank_del_row(avd_cb, su);
                  su->row_status = NCS_ROW_NOT_READY;
                  return NCSCC_RC_INV_VAL;
               }
            }
            
            m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(avd_cb, su, AVSV_CKPT_AVD_SU_CONFIG);
            return NCSCC_RC_SUCCESS;
            break;

         case NCS_ROW_NOT_IN_SERVICE:
         case NCS_ROW_DESTROY:

            /* check if it is active currently */

            if(su->row_status == NCS_ROW_ACTIVE)
            {

               if(su->sg_of_su->sg_ncs_spec == TRUE)
                  return NCSCC_RC_INV_VAL;
               else if((su->pres_state != NCS_PRES_UNINSTANTIATED) &&
                       (su->pres_state != NCS_PRES_INSTANTIATIONFAILED))
                  return NCSCC_RC_INV_VAL;
                 

               /* Check to see that the node is in admin locked state before
                * making the row status as not in service or delete 
                */

               if(su->admin_state != NCS_ADMIN_STATE_LOCK)
               {
                  /* log information error */
                  return NCSCC_RC_INV_VAL;
               }

               /* Check to see that no components or SUSI exists on this component */
               if((su->list_of_comp != AVD_COMP_NULL) ||
                  (su->list_of_susi != AVD_SU_SI_REL_NULL))  
               {
                  /* log information error */
                  return NCSCC_RC_INV_VAL;
               }

               if(test_flag == TRUE)
               {
                  return NCSCC_RC_SUCCESS;
               }

               m_AVD_GET_SU_NODE_PTR(avd_cb,su,su_node_ptr);

               if((su_node_ptr->node_state == AVD_AVND_STATE_PRESENT) ||
                  (su_node_ptr->node_state == AVD_AVND_STATE_NO_CONFIG) ||
                  (su_node_ptr->node_state == AVD_AVND_STATE_NCS_INIT))
               {
                  memset(((uns8 *)&param),'\0',sizeof(AVSV_PARAM_INFO));
                  param.table_id = NCSMIB_TBL_AVSV_AMF_SU;
                  param.act = AVSV_OBJ_OPR_DEL;
                  param.name_net = su->name_net;  
                  avd_snd_op_req_msg(avd_cb, su_node_ptr, &param);
               }

               /* remove the SU from both the SG list and the
                * AVND list if present.
                */
               m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

               avd_su_del_sg_list(avd_cb,su);
               avd_su_del_avnd_list(avd_cb,su);

               /* remove the SU from SG-SU Rank table*/
               avd_sg_su_rank_del_row(avd_cb, su);
               
               m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

               /* we need to delete this avnd structure on the
                * standby AVD.
                */

               /* check point to the standby AVD that this
                * record need to be deleted
                */
               m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(avd_cb, su, AVSV_CKPT_AVD_SU_CONFIG);
               
            } /* if(su->row_status == NCS_ROW_ACTIVE) */

            if(test_flag == TRUE)
            {
               return NCSCC_RC_SUCCESS;
            }

            if(arg->req.info.set_req.i_param_val.info.i_int
                  == NCS_ROW_DESTROY)
            {
               /* delete and free the structure */
               m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

               /* remove the SU from both the SG list and the
                * AVND list if present.
                */

               avd_su_del_sg_list(avd_cb,su);
               avd_su_del_avnd_list(avd_cb,su);

               avd_su_struc_del(avd_cb,su);

               m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

               return NCSCC_RC_SUCCESS;

            } /* if(arg->req.info.set_req.i_param_val.info.i_int
                  == NCS_ROW_DESTROY) */

            su->row_status = arg->req.info.set_req.i_param_val.info.i_int;
            return NCSCC_RC_SUCCESS;
            break;
         default:
            m_AVD_LOG_INVALID_VAL_ERROR(arg->req.info.set_req.i_param_val.info.i_int);
            /* Invalid row status object */
            return NCSCC_RC_INV_VAL;
            break;
         } /* switch(arg->req.info.set_req.i_param_val.info.i_int) */

      } /* if(arg->req.info.set_req.i_param_val.i_param_id == ncsSURowStatus_ID) */

   } /* else of if (su == AVD_SU_NULL) */

   /* We now have the su block */
   
   if(test_flag == TRUE)
   {
      return NCSCC_RC_SUCCESS;
   }

   if(su->row_status == NCS_ROW_ACTIVE) 
   {
      if(avd_cb->init_state  != AVD_APP_STATE)
      {
         return NCSCC_RC_INV_VAL;
      }

      m_AVD_GET_SU_NODE_PTR(avd_cb,su,su_node_ptr);

      switch(arg->req.info.set_req.i_param_val.i_param_id)
      {
      case saAmfSUAdminState_ID:
         if (arg->req.info.set_req.i_param_val.info.i_int == su->admin_state)
            return NCSCC_RC_SUCCESS;
         
         if (su->sg_of_su->sg_ncs_spec == SA_TRUE)
            return NCSCC_RC_INV_VAL;
         
         if(arg->req.info.set_req.i_param_val.info.i_int == NCS_ADMIN_STATE_UNLOCK)
         {
            if (su->sg_of_su->sg_fsm_state != AVD_SG_FSM_STABLE)
            {
               return NCSCC_RC_INV_VAL;
            }
            
            m_AVD_SET_SU_ADMIN(cb,su,NCS_ADMIN_STATE_UNLOCK);
            if(su->num_of_comp == su->curr_num_comp)
            {               
               m_AVD_GET_SU_NODE_PTR(avd_cb,su,su_node_ptr);
         
               if (m_AVD_APP_SU_IS_INSVC(su,su_node_ptr))
               {
                  m_AVD_SET_SU_REDINESS(cb,su,NCS_IN_SERVICE);
                  switch(su->sg_of_su->su_redundancy_model)
                  {
                  case AVSV_SG_RED_MODL_2N:
                     if(avd_sg_2n_su_insvc_func(avd_cb, su) != NCSCC_RC_SUCCESS)
                     {
                        m_AVD_SET_SU_REDINESS(cb,su,NCS_OUT_OF_SERVICE);
                        m_AVD_SET_SU_ADMIN(cb,su,NCS_ADMIN_STATE_LOCK);
                        return NCSCC_RC_INV_VAL;
                     }
                     break;

                  case AVSV_SG_RED_MODL_NWAY:
                     if(avd_sg_nway_su_insvc_func(avd_cb, su) != NCSCC_RC_SUCCESS)
                     {
                        m_AVD_SET_SU_REDINESS(cb,su,NCS_OUT_OF_SERVICE);
                        m_AVD_SET_SU_ADMIN(cb,su,NCS_ADMIN_STATE_LOCK);
                        return NCSCC_RC_INV_VAL;
                     }
                     break;

                  case AVSV_SG_RED_MODL_NWAYACTV:
                     if(avd_sg_nacvred_su_insvc_func(avd_cb, su) != NCSCC_RC_SUCCESS)
                     {
                        m_AVD_SET_SU_REDINESS(cb,su,NCS_OUT_OF_SERVICE);
                        m_AVD_SET_SU_ADMIN(cb,su,NCS_ADMIN_STATE_LOCK);
                        return NCSCC_RC_INV_VAL;
                     }
                     break;
                     
                  case AVSV_SG_RED_MODL_NPM:
                     if(avd_sg_npm_su_insvc_func(avd_cb, su) != NCSCC_RC_SUCCESS)
                     {
                        m_AVD_SET_SU_REDINESS(cb,su,NCS_OUT_OF_SERVICE);
                        m_AVD_SET_SU_ADMIN(cb,su,NCS_ADMIN_STATE_LOCK);
                        return NCSCC_RC_INV_VAL;
                     }
                     break;

                  case AVSV_SG_RED_MODL_NORED:
                  default:
                     if(avd_sg_nored_su_insvc_func(avd_cb, su) != NCSCC_RC_SUCCESS)
                     {
                        m_AVD_SET_SU_REDINESS(cb,su,NCS_OUT_OF_SERVICE);
                        m_AVD_SET_SU_ADMIN(cb,su,NCS_ADMIN_STATE_LOCK);
                        return NCSCC_RC_INV_VAL;
                     }
                     break;
                  }                  
                  
                  avd_sg_app_su_inst_func(avd_cb, su->sg_of_su);
               }
            }
            
            return NCSCC_RC_SUCCESS;
         }
         else
         {
            if ((su->admin_state == NCS_ADMIN_STATE_LOCK) && 
               (arg->req.info.set_req.i_param_val.info.i_int == NCS_ADMIN_STATE_SHUTDOWN))
               return NCSCC_RC_INV_VAL;
            
            if (su->list_of_susi == NULL)
            {
               m_AVD_SET_SU_REDINESS(cb,su,NCS_OUT_OF_SERVICE);
               m_AVD_SET_SU_ADMIN(cb,su,NCS_ADMIN_STATE_LOCK);
               avd_sg_app_su_inst_func(avd_cb, su->sg_of_su);
               return NCSCC_RC_SUCCESS;
            }            
            
            /* Check if other semantics are happening for other SUs. If yes
             * return an error.
             */
            if (su->sg_of_su->sg_fsm_state != AVD_SG_FSM_STABLE)
            {
               if((su->sg_of_su->sg_fsm_state != AVD_SG_FSM_SU_OPER) ||
                  (su->admin_state != NCS_ADMIN_STATE_SHUTDOWN) || 
                  (arg->req.info.set_req.i_param_val.info.i_int != NCS_ADMIN_STATE_LOCK))
               {
                  return NCSCC_RC_INV_VAL;
               }
            }/*if (su->sg_of_su->sg_fsm_state != AVD_SG_FSM_STABLE)*/   
            back_red_state = su->readiness_state;
            back_admin_state = su->admin_state;
            m_AVD_SET_SU_REDINESS(cb,su,NCS_OUT_OF_SERVICE);
            m_AVD_SET_SU_ADMIN(cb,su,(arg->req.info.set_req.i_param_val.info.i_int));
            switch(su->sg_of_su->su_redundancy_model)
            {
            case AVSV_SG_RED_MODL_2N:
               if(avd_sg_2n_su_admin_fail(avd_cb, su, su_node_ptr) != NCSCC_RC_SUCCESS)
               {
                  m_AVD_SET_SU_REDINESS(cb,su,back_red_state);
                  m_AVD_SET_SU_ADMIN(cb,su,back_admin_state);
                  return NCSCC_RC_INV_VAL;
               }
               break;

            case AVSV_SG_RED_MODL_NWAY:
               if(avd_sg_nway_su_admin_fail(avd_cb, su, su_node_ptr) != NCSCC_RC_SUCCESS)
               {
                  m_AVD_SET_SU_REDINESS(cb,su,back_red_state);
                  m_AVD_SET_SU_ADMIN(cb,su,back_admin_state);
                  return NCSCC_RC_INV_VAL;
               }
               break;
               
            case AVSV_SG_RED_MODL_NPM:
               if(avd_sg_npm_su_admin_fail(avd_cb, su, su_node_ptr) != NCSCC_RC_SUCCESS)
               {
                  m_AVD_SET_SU_REDINESS(cb,su,back_red_state);
                  m_AVD_SET_SU_ADMIN(cb,su,back_admin_state);
                  return NCSCC_RC_INV_VAL;
               }
               break;

            case AVSV_SG_RED_MODL_NWAYACTV:
               if(avd_sg_nacvred_su_admin_fail(avd_cb, su, su_node_ptr) != NCSCC_RC_SUCCESS)
               {
                  m_AVD_SET_SU_REDINESS(cb,su,back_red_state);
                  m_AVD_SET_SU_ADMIN(cb,su,back_admin_state);
                  return NCSCC_RC_INV_VAL;
               }
               break;
            case AVSV_SG_RED_MODL_NORED:
            default:
               if(avd_sg_nored_su_admin_fail(avd_cb, su, su_node_ptr) != NCSCC_RC_SUCCESS)
               {
                  m_AVD_SET_SU_REDINESS(cb,su,back_red_state);
                  m_AVD_SET_SU_ADMIN(cb,su,back_admin_state);
                  return NCSCC_RC_INV_VAL;
               }
               break;
            }  
            
            avd_sg_app_su_inst_func(avd_cb, su->sg_of_su);
            return NCSCC_RC_SUCCESS;
         }
         break;
      case saAmfSUFailOver_ID:
         if((su_node_ptr->node_state == AVD_AVND_STATE_PRESENT) ||
            (su_node_ptr->node_state == AVD_AVND_STATE_NO_CONFIG) ||
            (su_node_ptr->node_state == AVD_AVND_STATE_NCS_INIT))
         {
            memset(((uns8 *)&param),'\0',sizeof(AVSV_PARAM_INFO));
            param.table_id = NCSMIB_TBL_AVSV_AMF_SU;
            param.obj_id = arg->req.info.set_req.i_param_val.i_param_id;   
            param.act = AVSV_OBJ_OPR_MOD;
            param.name_net = su->name_net;
            temp_bool = su->is_su_failover;
         
            su->is_su_failover = TRUE;

            if(avd_snd_op_req_msg(avd_cb,su_node_ptr,&param)!= NCSCC_RC_SUCCESS)
            {
               su->is_su_failover = temp_bool;
               return NCSCC_RC_INV_VAL;
            }
            
         }else
         {
            su->is_su_failover = TRUE;
         }
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(avd_cb, su, AVSV_CKPT_AVD_SU_CONFIG);
         return NCSCC_RC_SUCCESS;
         break;
      default:
         return NCSCC_RC_INV_VAL;
 
      } /* switch(arg->req.info.set_req.i_param_val.i_param_id) */
   } /* if(su->row_status == NCS_ROW_ACTIVE) */ 


   switch(arg->req.info.set_req.i_param_val.i_param_id)
   {
   case saAmfSURowStatus_ID:
      /* fill the row status value */
      if(arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT)
      {
         su->row_status = arg->req.info.set_req.i_param_val.info.i_int;
      }
      break;
   case saAmfSUParentSGName_ID:
      
      /* fill the SG name value */
      memset(&temp_name, '\0', sizeof(SaNameT));
      temp_name.length = arg->req.info.set_req.i_param_val.i_length;
      memcpy(temp_name.value, arg->req.info.set_req.i_param_val.info.i_oct,
                   temp_name.length);

      /* check if already This SU is part of a SG */
      if(su->sg_of_su != AVD_SG_NULL)
      {
         if (temp_name.length == su->sg_of_su->name_net.length)
         {
            if(m_NCS_MEMCMP(su->sg_of_su->name_net.value,temp_name.value,temp_name.length)
               == 0)
            {
               return NCSCC_RC_SUCCESS;
            }
         }
      }

      su->sg_name.length = arg->req.info.set_req.i_param_val.i_length;
      memcpy(su->sg_name.value, 
                   arg->req.info.set_req.i_param_val.info.i_oct, 
                   su->sg_name.length);
      break;
   case saAmfSUFailOver_ID:
      su->is_su_failover = (arg->req.info.set_req.i_param_val.info.i_int == NCS_SNMP_TRUE) ? TRUE:FALSE;
      break;
   default:
      memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

      temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
      temp_mib_req.info.i_set_util_info.param = &(arg->req.info.set_req.i_param_val);
      temp_mib_req.info.i_set_util_info.var_info = var_info;
      temp_mib_req.info.i_set_util_info.data = su;
      temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

      /* call the mib routine handler */ 
      if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
      {
         return rc;
      }
      /* check to verify that the admin object is not shutdown
      * if shutdown change to LOCK.
      */
      if (su->admin_state == NCS_ADMIN_STATE_SHUTDOWN)
      {
         su->admin_state = NCS_ADMIN_STATE_LOCK;
      }
      break;
   } /* switch(arg->req.info.set_req.i_param_val.i_param_id) */

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfsutableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_S_U_TABLE_ENTRY_ID table. This is the Service Unit table. The
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

uns32 saamfsutableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SU        *su;
   SaNameT       su_name;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   memset(&su_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service unit database key from the instant ID */
   if (arg->i_idx.i_inst_len != 0)
   {
      su_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
      for(i = 0; i < su_name.length; i++)
      {
         su_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
   }

   su = avd_su_struc_find_next(avd_cb,su_name,TRUE);

   if (su == AVD_SU_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the instant ID from the SU name */

   *next_inst_id_len = m_NCS_OS_NTOHS(su->name_net.length) + 1;

   next_inst_id[0] = *next_inst_id_len -1;
   for(i = 0; i < next_inst_id[0]; i++)
   {
      next_inst_id[i + 1] = (uns32)(su->name_net.value[i]);
   }

   *data = (NCSCONTEXT)su;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfsutableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_S_U_TABLE_ENTRY_ID table. This is the Service Unit table. The
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

uns32 saamfsutableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}


/*****************************************************************************
 * Function: ncssutableentry_rmvrow
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
uns32 ncssutableentry_rmvrow(NCSCONTEXT cb, NCSMIB_IDX *idx)
{
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: avd_su_ack_msg
 *
 * Purpose:  This function processes the acknowledgment received for
 *          a SU related message from AVND for operator related changes.
 *          If the message acknowledges success change the row status of the
 *          SU to active, if failure change row status to not in service.
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

void avd_su_ack_msg(AVD_CL_CB *cb,AVD_DND_MSG *ack_msg)
{
   AVD_SU          *su;   
   AVSV_PARAM_INFO param;
   AVD_AVND       *avnd;

   /* check the ack message for errors. If error find the SU that
    * has error.
    */

   if (ack_msg->msg_info.n2d_reg_su.error != NCSCC_RC_SUCCESS)
   {
      /* Find the SU */
      su = avd_su_struc_find(cb,ack_msg->msg_info.n2d_reg_su.su_name_net,FALSE);
      if (su == AVD_SU_NULL)
      {
         /* The SU has already been deleted. there is nothing
          * that can be done.
          */

         /* Log an information error that the SU is
          * deleted.
          */
         return;
      }

      /* log an fatal error as normally this shouldnt happen.
       */

      m_AVD_LOG_INVALID_VAL_FATAL(ack_msg->msg_info.n2d_reg_su.error);

      m_AVD_GET_SU_NODE_PTR(cb,su,avnd);

      /* remove the SU from both the SG list and the
       * AVND list if present.
       */

      m_AVD_CB_LOCK(cb, NCS_LOCK_WRITE);

      avd_su_del_sg_list(cb,su);

      avd_su_del_avnd_list(cb,su);

      /* remove the SU from SG-SU Rank table*/
      avd_sg_su_rank_del_row(cb, su);
      
      m_AVD_CB_UNLOCK(cb, NCS_LOCK_WRITE);
      
      if (su->list_of_comp != AVD_COMP_NULL)
      {
         /* Mark All the components as Not in service. Send
          * the node delete request for all the components.
          */
         
         while (su->list_of_comp != AVD_COMP_NULL)
         {
            su->list_of_comp->row_status = NCS_ROW_NOT_IN_SERVICE;
            m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, (su->list_of_comp), AVSV_CKPT_AVD_COMP_CONFIG);
            /* send a delete message to the AvND for the comp. */
            memset(((uns8 *)&param),'\0',sizeof(AVSV_PARAM_INFO));
            param.act = AVSV_OBJ_OPR_DEL;
            param.name_net = su->list_of_comp->comp_info.name_net;
            param.table_id = NCSMIB_TBL_AVSV_AMF_COMP;      
            avd_snd_op_req_msg(cb,avnd,&param);
            /* delete from the SU list */
            avd_comp_del_su_list(cb,su->list_of_comp);
         }
         
         su->si_max_active = 0;
         su->si_max_standby = 0;
         su->su_preinstan = TRUE;
         su->curr_num_comp = 0;
      }

      su->row_status = NCS_ROW_NOT_IN_SERVICE;
      m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, su, AVSV_CKPT_AVD_SU_CONFIG);
      return;
   }

   /* Log an information error that the node was updated with the
    * information of the SU.
    */
   /* Find the SU */
   su = avd_su_struc_find(cb,ack_msg->msg_info.n2d_reg_su.su_name_net,FALSE);
   if (su == AVD_SU_NULL)
   {
      /* The SU has already been deleted. there is nothing
       * that can be done.
       */

      /* Log an information error that the SU is
       * deleted.
       */
      /* send a delete message to the AvND for the SU. */
      memset(((uns8 *)&param),'\0',sizeof(AVSV_PARAM_INFO));
      param.act = AVSV_OBJ_OPR_DEL;
      param.name_net = ack_msg->msg_info.n2d_reg_su.su_name_net;
      param.table_id = NCSMIB_TBL_AVSV_AMF_SU;
      avnd = avd_avnd_struc_find_nodeid(cb,ack_msg->msg_info.n2d_reg_su.node_id);      
      avd_snd_op_req_msg(cb,avnd,&param);
      return;
   }
   su->row_status = NCS_ROW_ACTIVE;
   /* checkpoint the SU */
   return;
}




/*****************************************************************************
 * Function: ncssutableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in NCS_S_U_TABLE_ENTRY_ID table. This is the properitary SU table. The
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

uns32 ncssutableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SU        *su;
   SaNameT       su_name;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   memset(&su_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service unit database key from the instant ID */
   su_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < su_name.length; i++)
   {
      su_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   su = avd_su_struc_find(avd_cb,su_name,TRUE);

   if (su == AVD_SU_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)su;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: ncssutableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * NCS_S_U_TABLE_ENTRY_ID table. This is the properitary SU table. The
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

uns32 ncssutableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer)
{
   AVD_SU        *su = (AVD_SU *)data;

   if (su == AVD_SU_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }
   if(param->i_param_id == ncsSUTermState_ID)
   {
      param->i_fmat_id = NCSMIB_FMAT_INT; 
      param->i_length = 1; 
      param->info.i_int = (su->term_state == FALSE) ? NCS_SNMP_FALSE:NCS_SNMP_TRUE;
      return NCSCC_RC_SUCCESS; 
   }
   
   /* call the MIBLIB utility routine for standfard object types */
   if ((var_info != NULL) && (var_info->offset != 0))
      return ncsmiblib_get_obj_val(param, var_info, data, buffer);
   else
      return NCSCC_RC_NO_OBJECT;
}


/*****************************************************************************
 * Function: ncssutableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * NCS_S_U_TABLE_ENTRY_ID table. This is the properitary SU table. The
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

uns32 ncssutableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SU        *su;
   SaNameT       su_name;
   uns32         i;
   NCS_BOOL      val_same_flag = FALSE;
   NCSMIBLIB_REQ_INFO temp_mib_req;
   AVD_AVND *su_node_ptr = NULL;
 
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_INV_VAL;  
   }

   memset(&su_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service unit database key from the instant ID */
   su_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < su_name.length; i++)
   {
      su_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   su = avd_su_struc_find(avd_cb,su_name,TRUE);

   if(su == AVD_SU_NULL)
   {
      /* Invalid instance object */
      return NCSCC_RC_NO_INSTANCE; 
   }

   if(su->row_status != NCS_ROW_ACTIVE)
      return NCSCC_RC_INV_VAL;  

   if(su->sg_of_su->sg_ncs_spec)
      return NCSCC_RC_INV_VAL;
   
   if(avd_cb->init_state  != AVD_APP_STATE)
   {
      return NCSCC_RC_INV_VAL;
   }
   
   if(test_flag == TRUE)
   {
      return NCSCC_RC_SUCCESS;
   }

   if(arg->req.info.set_req.i_param_val.i_param_id == ncsSUTermState_ID)
   {
      if(arg->req.info.set_req.i_param_val.info.i_int == su->term_state)
         return NCSCC_RC_SUCCESS;

      if ((su->su_preinstan == FALSE) || (su->curr_num_comp == 0))
         return NCSCC_RC_INV_VAL;
      
      if(arg->req.info.set_req.i_param_val.info.i_int == NCS_SNMP_TRUE) 
      {
         if( (su->admin_state != NCS_ADMIN_STATE_LOCK) || (su->list_of_susi != NULL) )
         {
            return NCSCC_RC_INV_VAL;  
         }

         m_AVD_GET_SU_NODE_PTR(avd_cb,su,su_node_ptr);

         if((su_node_ptr->node_state == AVD_AVND_STATE_PRESENT) ||
            (su_node_ptr->node_state == AVD_AVND_STATE_NO_CONFIG) ||
            (su_node_ptr->node_state == AVD_AVND_STATE_NCS_INIT))
         {
            if(avd_snd_presence_msg(avd_cb, su, TRUE) == NCSCC_RC_SUCCESS)
            {
                m_AVD_SET_SU_TERM(cb,su,TRUE);
                su->oper_state = NCS_OPER_STATE_DISABLE;
                return NCSCC_RC_SUCCESS;
            }
            return NCSCC_RC_INV_VAL;
         }
         else
         {
            m_AVD_SET_SU_TERM(cb,su,TRUE);
            return NCSCC_RC_SUCCESS;
         }
      }
      else if(arg->req.info.set_req.i_param_val.info.i_int == NCS_SNMP_FALSE) 
      {
         m_AVD_GET_SU_NODE_PTR(avd_cb,su,su_node_ptr);

         if((su_node_ptr->node_state == AVD_AVND_STATE_PRESENT) ||
            (su_node_ptr->node_state == AVD_AVND_STATE_NO_CONFIG) ||
            (su_node_ptr->node_state == AVD_AVND_STATE_NCS_INIT))
         {
            if(avd_snd_presence_msg(avd_cb, su, FALSE) == NCSCC_RC_SUCCESS)
            {
               m_AVD_SET_SU_TERM(cb,su,FALSE);
               return NCSCC_RC_SUCCESS;
            }
            return NCSCC_RC_INV_VAL;
         }
         else
         {
            m_AVD_SET_SU_TERM(cb,su,FALSE);
            return NCSCC_RC_SUCCESS;
         }
      }
      return NCSCC_RC_INV_VAL;
   }
   if(arg->req.info.set_req.i_param_val.i_param_id == ncsSUSwitchState_ID)
   {
      if((arg->req.info.set_req.i_param_val.info.i_int != AVSV_SI_TOGGLE_SWITCH)
         || su->su_switch == AVSV_SI_TOGGLE_SWITCH)
      {
         return NCSCC_RC_INV_VAL;
      }
     
      switch(su->sg_of_su->su_redundancy_model)
      {
      case AVSV_SG_RED_MODL_2N:
         m_AVD_SET_SU_SWITCH(cb,su,AVSV_SI_TOGGLE_SWITCH);
         if (avd_sg_2n_suswitch_func(avd_cb, su) != NCSCC_RC_SUCCESS)
         {
            m_AVD_SET_SU_SWITCH(cb,su,AVSV_SI_TOGGLE_STABLE);
            return NCSCC_RC_INV_VAL;
         }
      
         break;
      default:
         m_AVD_SET_SU_SWITCH(cb,su,AVSV_SI_TOGGLE_STABLE);
         return NCSCC_RC_INV_VAL;
      }  
            
      
      return NCSCC_RC_SUCCESS;
   }
   /* We now have the su block */
   

   memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

   temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
   temp_mib_req.info.i_set_util_info.param = &(arg->req.info.set_req.i_param_val);
   temp_mib_req.info.i_set_util_info.var_info = var_info;
   temp_mib_req.info.i_set_util_info.data = su;
   temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

   /* call the mib routine handler */ 
   return ncsmiblib_process_req(&temp_mib_req);
}



/*****************************************************************************
 * Function: ncssutableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * NCS_S_U_TABLE_ENTRY_ID table. This is the properitary SU table. The
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

uns32 ncssutableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SU        *su;
   SaNameT       su_name;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   memset(&su_name, '\0', sizeof(SaNameT));
   
   /* Prepare the service unit database key from the instant ID */
   if (arg->i_idx.i_inst_len != 0)
   {
      su_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
      for(i = 0; i < su_name.length; i++)
      {
         su_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
   }

   su = avd_su_struc_find_next(avd_cb,su_name,TRUE);

   if (su == AVD_SU_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the instant ID from the SU name */

   *next_inst_id_len = m_NCS_OS_NTOHS(su->name_net.length) + 1;

   next_inst_id[0] = *next_inst_id_len -1;
   for(i = 0; i < next_inst_id[0]; i++)
   {
      next_inst_id[i + 1] = (uns32)(su->name_net.value[i]);
   }

   *data = (NCSCONTEXT)su;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: ncssutableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * NCS_S_U_TABLE_ENTRY_ID table. This is the properitary SU table. The
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

uns32 ncssutableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}




/*****************************************************************************
 * Function: avd_sg_su_rank_add_row
 *
 * Purpose:  This function will create and add a AVD_SG_SU_RANK structure to the
 * tree if an element with given index value doesn't exist in the tree.
 *
 * Input: cb - the AVD control block
 *        su - Pointer to service unit row 
 *
 * Returns: The pointer to AVD_SG_SU_RANK structure allocated and added.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SG_SU_RANK * avd_sg_su_rank_add_row(AVD_CL_CB *cb, AVD_SU *su)
{
   AVD_SG_SU_RANK *rank_elt = AVD_SG_SU_RANK_NULL;

   /* Allocate a new block structure now
    */
   if ((rank_elt = m_MMGR_ALLOC_AVD_SG_SU_RANK) == AVD_SG_SU_RANK_NULL)
   {
      /* log an error */
      m_AVD_LOG_MEM_FAIL(AVD_SG_SU_RANK_ALLOC_FAILED);
      return AVD_SG_SU_RANK_NULL;
   }

   memset((char *)rank_elt, '\0', sizeof(AVD_SG_SU_RANK));

   rank_elt->indx.sg_name_net.length = m_NCS_OS_HTONS(su->sg_name.length);

   memcpy(rank_elt->indx.sg_name_net.value,su->sg_name.value,su->sg_name.length);

   rank_elt->indx.su_rank_net = m_NCS_OS_HTONL(su->rank);

   rank_elt->tree_node.key_info = (uns8*)(&rank_elt->indx);
   rank_elt->tree_node.bit   = 0;
   rank_elt->tree_node.left  = NCS_PATRICIA_NODE_NULL;
   rank_elt->tree_node.right = NCS_PATRICIA_NODE_NULL;

   if( ncs_patricia_tree_add(&cb->sg_su_rank_anchor,&rank_elt->tree_node)
                      != NCSCC_RC_SUCCESS)
   {
      /* log an error */
      m_MMGR_FREE_AVD_SG_SU_RANK(rank_elt);
      return AVD_SG_SU_RANK_NULL;
   }

   rank_elt->su_name.length = m_NCS_OS_NTOHS(su->name_net.length);
   memcpy(rank_elt->su_name.value, su->name_net.value, rank_elt->su_name.length);

   return rank_elt;
}



/*****************************************************************************
 * Function: avd_sg_su_rank_struc_find
 *
 * Purpose:  This function will find a AVD_SG_SU_RANK structure in the
 * tree with indx value as key.
 *
 * Input: cb - the AVD control block
 *        indx - The key.
 *
 * Returns: The pointer to AVD_SG_SU_RANK structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SG_SU_RANK * avd_sg_su_rank_struc_find(AVD_CL_CB *cb, AVD_SG_SU_RANK_INDX indx)
{
   AVD_SG_SU_RANK *rank_elt = AVD_SG_SU_RANK_NULL;

   rank_elt = (AVD_SG_SU_RANK *)ncs_patricia_tree_get(&cb->sg_su_rank_anchor, (uns8*)&indx);

   return rank_elt;
}


/*****************************************************************************
 * Function: avd_sg_su_rank_struc_find_next
 *
 * Purpose:  This function will find the next AVD_SG_SU_RANK structure in the
 * tree whose key value is next of the given key value.
 *
 * Input: cb - the AVD control block
 *        indx - The key value.
 *
 * Returns: The pointer to AVD_SG_SU_RANK structure found in the tree. 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_SG_SU_RANK * avd_sg_su_rank_struc_find_next(AVD_CL_CB *cb, AVD_SG_SU_RANK_INDX indx)
{
   AVD_SG_SU_RANK *rank_elt = AVD_SG_SU_RANK_NULL;

   rank_elt = (AVD_SG_SU_RANK *)ncs_patricia_tree_getnext(&cb->sg_su_rank_anchor, (uns8*)&indx);

   return rank_elt;
}



/*****************************************************************************
 * Function: avd_sg_su_rank_del_row
 *
 * Purpose:  This function will delete and free AVD_SG_SU_RANK structure from 
 * the tree.
 *
 * Input: cb - The AVD control block
 *        su - Pointer to service instance row 
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE  
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_sg_su_rank_del_row(AVD_CL_CB *cb, AVD_SU *su)
{
   AVD_SG_SU_RANK_INDX    indx;
   AVD_SG_SU_RANK *       rank_elt = AVD_SG_SU_RANK_NULL;

   if (su == AVD_SU_NULL)
      return NCSCC_RC_FAILURE;

   memset((char *)&indx, '\0', sizeof(AVD_SG_SU_RANK_INDX));

   indx.sg_name_net.length = m_NCS_OS_HTONS(su->sg_name.length);

   memcpy(indx.sg_name_net.value,su->sg_name.value,su->sg_name.length);

   indx.su_rank_net = m_NCS_OS_HTONL(su->rank);

   rank_elt = avd_sg_su_rank_struc_find(cb, indx);

   if(rank_elt == AVD_SG_SU_RANK_NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   if(ncs_patricia_tree_del(&cb->sg_su_rank_anchor,&rank_elt->tree_node)
                      != NCSCC_RC_SUCCESS)
   {
      /* log error */
      return NCSCC_RC_FAILURE;
   }

   m_MMGR_FREE_AVD_SG_SU_RANK(rank_elt);

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfsgsurankentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_S_G_S_U_RANK_ENTRY_ID table. This is the SG_SU Rank table. The
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


uns32 saamfsgsurankentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg,
                             NCSCONTEXT* data)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SG_SU_RANK_INDX   indx;
   uns16 len;
   uns32 i;
   AVD_SG_SU_RANK *   rank_elt = AVD_SG_SU_RANK_NULL;

   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   memset(&indx, '\0', sizeof(AVD_SG_SU_RANK_INDX));

   /* Prepare the SG-SU Rank database key from the instant ID */
   len = (SaUint16T)arg->i_idx.i_inst_ids[0];

   indx.sg_name_net.length  = m_NCS_OS_HTONS(len);
   
   for(i = 0; i < len; i++)
   {
      indx.sg_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }
   
   if (arg->i_idx.i_inst_len > len + 1)
   {
      indx.su_rank_net = m_NCS_OS_HTONL((SaUint32T)arg->i_idx.i_inst_ids[len + 1]);
   }

   rank_elt = avd_sg_su_rank_struc_find(avd_cb,indx);

   if (rank_elt == AVD_SG_SU_RANK_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)rank_elt;

   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: saamfsgsurankentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_AMF_S_G_S_U_RANK_ENTRY_ID table. This is the SG-SU Rank table. The
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


uns32 saamfsgsurankentry_extract(NCSMIB_PARAM_VAL* param,
                                 NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                                 NCSCONTEXT buffer)
{
   AVD_SG_SU_RANK *   rank_elt = (AVD_SG_SU_RANK *)data;

   if (rank_elt == AVD_SG_SU_RANK_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   switch(param->i_param_id)
   {

   case saAmfSGSURankSUName_ID:
      m_AVSV_OCTVAL_TO_PARAM(param, buffer, rank_elt->su_name.length,
                             rank_elt->su_name.value);
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
 * Function: saamfsgsurankentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_S_G_S_U_RANK_ENTRY_ID table. This is the SG-SU Rank table. The
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

uns32 saamfsgsurankentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
                              NCSCONTEXT* data, uns32* next_inst_id,
                              uns32 *next_inst_id_len)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_SG_SU_RANK_INDX   indx;
   uns16 len;
   uns32 i;
   AVD_SG_SU_RANK *   rank_elt = AVD_SG_SU_RANK_NULL;

   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   memset(&indx, '\0', sizeof(AVD_SG_SU_RANK_INDX));

   if (arg->i_idx.i_inst_len != 0)
   {

      /* Prepare the SG-SU Rank database key from the instant ID */
      len = (SaUint16T)arg->i_idx.i_inst_ids[0];

      indx.sg_name_net.length  = m_NCS_OS_HTONS(len);

      for(i = 0; i < len; i++)
      {
         indx.sg_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
        
      if (arg->i_idx.i_inst_len > len + 1)
      {

         indx.su_rank_net = m_NCS_OS_HTONL((SaUint32T)arg->i_idx.i_inst_ids[len + 1]);
      }
   }

   rank_elt = avd_sg_su_rank_struc_find_next(avd_cb,indx);

   if (rank_elt == AVD_SG_SU_RANK_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)rank_elt;

   /* Prepare the instant ID from the SG name and SU rank */
   len = m_NCS_OS_NTOHS(rank_elt->indx.sg_name_net.length);

   *next_inst_id_len = len + 1 + 1;

   next_inst_id[0] = len;

   for(i = 0; i < len; i++)
   {
      next_inst_id[i + 1] = (uns32)(rank_elt->indx.sg_name_net.value[i]);
   }

   next_inst_id[len+1] = m_NCS_OS_NTOHL(rank_elt->indx.su_rank_net);

   *data = (NCSCONTEXT)rank_elt;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfsgsurankentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_S_G_S_U_RANK_ENTRY_ID table. This is the SG_SU Rank table. The
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


uns32 saamfsgsurankentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                             NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{
   return NCSCC_RC_FAILURE;
}



/*****************************************************************************
 * Function: saamfsgsurankentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_S_G_S_U_RANK_ENTRY_ID table. This is the SG-SU Rank table. The
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


uns32 saamfsgsurankentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_SUCCESS;
}
