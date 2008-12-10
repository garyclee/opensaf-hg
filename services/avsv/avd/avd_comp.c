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
  the component database on the AVD. It also deals with all the MIB operations
  like set,get,getnext etc related to the component table.


..............................................................................

  FUNCTIONS INCLUDED in this module:

  avd_comp_struc_crt - creates and adds comp structure to database.
  avd_comp_struc_find - Finds comp structure from the database.
  avd_comp_struc_find_next - Finds the next comp structure from the database.  
  avd_comp_struc_del - deletes and frees comp structure from database.
  avd_comp_del_su_list - deletes comp structure from SU list of comps.
  saamfcomptableentry_get - This function is one of the get processing
                        routines for objects in SA_AMF_COMP_TABLE_ENTRY_ID table.
  saamfcomptableentry_extract - This function is one of the get processing
                            function for objects in SA_AMF_COMP_TABLE_ENTRY_ID
                            table.
  saamfcomptableentry_set - This function is the set processing for objects in
                         SA_AMF_COMP_TABLE_ENTRY_ID table.
  saamfcomptableentry_next - This function is the next processing for objects in
                         SA_AMF_COMP_TABLE_ENTRY_ID table.
  saamfcomptableentry_setrow - This function is the setrow processing for
                           objects in SA_AMF_COMP_TABLE_ENTRY_ID table.
  avd_comp_ack_msg - This function processes the acknowledgment received for
                   a component related message from AVND for operator related
                   addition.


  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"


/*****************************************************************************
 * Function: avd_comp_struc_crt
 *
 * Purpose:  This function will create and add a AVD_COMP structure to the
 * tree if an element with comp_name key value doesn't exist in the tree.
 *
 * Input: cb - the AVD control block
 *        comp_name - The component name of the component that needs to be
 *                    added.
 *
 * Returns: The pointer to AVD_COMP structure allocated and added.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_COMP * avd_comp_struc_crt(AVD_CL_CB *cb,SaNameT comp_name, NCS_BOOL ckpt)
{
   AVD_COMP *comp;

   if( (comp_name.length <= 8) || (m_NCS_OS_STRNCMP(comp_name.value,"safComp=",8) != 0) )
   {
      return AVD_COMP_NULL;
   }

   /* Allocate a new block structure now
    */
   if ((comp = m_MMGR_ALLOC_AVD_COMP) == AVD_COMP_NULL)
   {
      /* log an error */
      m_AVD_LOG_MEM_FAIL(AVD_COMP_ALLOC_FAILED);
      return AVD_COMP_NULL;
   }

   m_NCS_MEMSET((char *)comp, '\0', sizeof(AVD_COMP));

   if (ckpt)
   {
      m_NCS_MEMCPY(comp->comp_info.name_net.value,comp_name.value,
         m_NCS_OS_NTOHS(comp_name.length));
      comp->comp_info.name_net.length = comp_name.length;
   }
   else
   {
      m_NCS_MEMCPY(comp->comp_info.name_net.value,comp_name.value,comp_name.length);
      comp->comp_info.name_net.length = m_HTON_SANAMET_LEN(comp_name.length);
      
      comp->comp_info.cap = NCS_COMP_CAPABILITY_1_ACTIVE_OR_1_STANDBY;
      comp->comp_info.category = NCS_COMP_TYPE_NON_SAF;
      comp->comp_info.def_recvr = SA_AMF_COMPONENT_RESTART;
      comp->comp_info.inst_level = 1;
      comp->row_status = NCS_ROW_NOT_READY;
      
      comp->comp_info.comp_restart = TRUE;
      comp->nodefail_cleanfail = FALSE;
      comp->oper_state = NCS_OPER_STATE_DISABLE;
      comp->readiness_state = NCS_OUT_OF_SERVICE;
      comp->pres_state = NCS_PRES_UNINSTANTIATED;
   }

   comp->comp_info.max_num_inst = AVSV_MAX_INST;
   comp->comp_info.max_num_amstart = AVSV_MAX_AMSTART;
   
   comp->comp_info.init_time = AVSV_INST_TIMEOUT;
   comp->comp_info.term_time = AVSV_TERM_TIMEOUT;
   comp->comp_info.clean_time = AVSV_CLEAN_TIMEOUT;
   comp->comp_info.amstart_time = AVSV_AM_START_TIMEOUT;
   comp->comp_info.amstop_time = AVSV_AM_STOP_TIMEOUT;

   comp->comp_info.terminate_callback_timeout= AVSV_TERM_CB_TIMEOUT;
   comp->comp_info.csi_set_callback_timeout = AVSV_CSI_SET_CB_TIMEOUT;
   comp->comp_info.quiescing_complete_timeout = AVSV_QUES_DONE_TIMEOUT;
   comp->comp_info.csi_rmv_callback_timeout = AVSV_CSI_RMV_CB_TIMEOUT;
   comp->comp_info.proxied_inst_callback_timeout = AVSV_PROX_INST_CB_TIMEOUT;
   comp->comp_info.proxied_clean_callback_timeout = AVSV_PROX_CL_CB_TIMEOT;

   /* Init pointers */
   comp->su = AVD_SU_NULL;
   comp->su_comp_next = AVD_COMP_NULL;


   comp->tree_node.key_info = (uns8*)&(comp->comp_info.name_net);
   comp->tree_node.bit   = 0;
   comp->tree_node.left  = NCS_PATRICIA_NODE_NULL;
   comp->tree_node.right = NCS_PATRICIA_NODE_NULL;

   if( ncs_patricia_tree_add(&cb->comp_anchor,&comp->tree_node) 
                      != NCSCC_RC_SUCCESS)
   {
      /* log an error */
      m_MMGR_FREE_AVD_COMP(comp);
      return AVD_COMP_NULL;
   }


   return comp;
   
}


/*****************************************************************************
 * Function: avd_comp_struc_find
 *
 * Purpose:  This function will find a AVD_COMP structure in the
 * tree with comp_name value as key.
 *
 * Input: cb - the AVD control block
 *        comp_name - The name of the component.
 *        host_order - Flag indicating if the length is host order
 *
 * Returns: The pointer to AVD_COMP structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_COMP * avd_comp_struc_find(AVD_CL_CB *cb,SaNameT comp_name,NCS_BOOL host_order)
{
   AVD_COMP *comp;
   SaNameT  lcomp_name;

   m_NCS_MEMSET((char *)&lcomp_name, '\0', sizeof(SaNameT));
   lcomp_name.length = (host_order == FALSE) ? comp_name.length :  
                                           m_HTON_SANAMET_LEN(comp_name.length);

   m_NCS_MEMCPY(lcomp_name.value,comp_name.value,m_NCS_OS_NTOHS(lcomp_name.length));

   comp = (AVD_COMP *)ncs_patricia_tree_get(&cb->comp_anchor, (uns8 *)&lcomp_name);

   return comp;
}


/*****************************************************************************
 * Function: avd_comp_struc_find_next
 *
 * Purpose:  This function will find the next AVD_COMP structure in the
 * tree whose comp_name value is next of the given comp_name value.
 *
 * Input: cb - the AVD control block
 *        comp_name - The name of the component.
 *        host_order - Flag indicating if the length is host order
 *
 * Returns: The pointer to AVD_COMP structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_COMP * avd_comp_struc_find_next(AVD_CL_CB *cb,SaNameT comp_name,NCS_BOOL host_order)
{
   AVD_COMP *comp;
   SaNameT  lcomp_name;

   m_NCS_MEMSET((char *)&lcomp_name, '\0', sizeof(SaNameT));
   lcomp_name.length = (host_order == FALSE) ? comp_name.length :  
                                           m_HTON_SANAMET_LEN(comp_name.length);

   m_NCS_MEMCPY(lcomp_name.value,comp_name.value,m_NCS_OS_NTOHS(lcomp_name.length));
   comp = (AVD_COMP *)ncs_patricia_tree_getnext(&cb->comp_anchor, 
                                                 (uns8*)&lcomp_name);

   return comp;
}


/*****************************************************************************
 * Function: avd_comp_struc_del
 *
 * Purpose:  This function will delete and free AVD_COMP structure from 
 * the tree. If the node need to be deleted with saNameT, first do a find
 * and call this function with the AVD_COMP pointer.
 *
 * Input: cb - the AVD control block
 *        comp - The component structure that needs to be deleted.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_comp_struc_del(AVD_CL_CB *cb,AVD_COMP *comp)
{

   if (comp == AVD_COMP_NULL)
      return NCSCC_RC_FAILURE;

   if(ncs_patricia_tree_del(&cb->comp_anchor,&comp->tree_node) 
                      != NCSCC_RC_SUCCESS)
   {
      /* log error */
      return NCSCC_RC_FAILURE;
   }

   m_MMGR_FREE_AVD_COMP(comp);
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: avd_comp_del_su_list
 *
 * Purpose:  This function will del the given comp from the SU list, and fill
 * the components pointer with NULL
 *
 * Input: cb - the AVD control block
 *        comp - The component pointer
 *
 * Returns: None. 
 *
 * NOTES: None
 *
 * 
 **************************************************************************/

void avd_comp_del_su_list(AVD_CL_CB *cb,AVD_COMP *comp)
{
   AVD_COMP *i_comp = AVD_COMP_NULL;
   AVD_COMP *prev_comp = AVD_COMP_NULL;

   if (comp->su != AVD_SU_NULL)
   {
      /* remove COMP from SU */
      i_comp = comp->su->list_of_comp;

      while ((i_comp != AVD_COMP_NULL) && (i_comp != comp))
      {
         prev_comp = i_comp;
         i_comp = i_comp->su_comp_next;
      }

      if(i_comp != comp)
      {
         /* Log a fatal error */
      }else
      {
         if (prev_comp == AVD_COMP_NULL)
         {
            comp->su->list_of_comp = comp->su_comp_next;
         }else
         {
            prev_comp->su_comp_next = comp->su_comp_next;
         }                     
      }

      comp->su_comp_next = AVD_COMP_NULL;
      comp->su = AVD_SU_NULL;
   } /* if (comp->su != AVD_SU_NULL) */ 
               
}



/*****************************************************************************
 * Function: saamfcomptableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_COMP_TABLE_ENTRY_ID table. This is the component table. The
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

uns32 saamfcomptableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                  NCSCONTEXT* data)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_COMP      *comp;
   SaNameT       comp_name;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   m_NCS_MEMSET(&comp_name, '\0', sizeof(SaNameT));
   
   /* Prepare the component database key from the instant ID */
   comp_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < comp_name.length; i++)
   {
      comp_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   comp = avd_comp_struc_find(avd_cb,comp_name,TRUE);

   if (comp == AVD_COMP_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)comp;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 * Function: saamfcomptableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_AMF_COMP_TABLE_ENTRY_ID table. This is the component table. The
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

uns32 saamfcomptableentry_extract(NCSMIB_PARAM_VAL* param, 
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer)
{
   AVD_COMP      *comp = (AVD_COMP *)data;
   SaNameT       empty_name;
   AVD_SU_SI_REL *temp_susi_list = AVD_SU_SI_REL_NULL;

   if (comp == AVD_COMP_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }
   switch(param->i_param_id)
   {
   /* CLC commands */
   case saAmfCompInstantiateCmd_ID:
      m_AVSV_OCTVAL_TO_PARAM(param, buffer, ((uns16)comp->comp_info.init_len),
                             comp->comp_info.init_info);
      break;
   case saAmfCompTerminateCmd_ID:
      m_AVSV_OCTVAL_TO_PARAM(param, buffer, ((uns16)comp->comp_info.term_len),
                             comp->comp_info.term_info);
      break;
   case saAmfCompCleanupCmd_ID:
      m_AVSV_OCTVAL_TO_PARAM(param, buffer, ((uns16)comp->comp_info.clean_len),
                             comp->comp_info.clean_info);
      break;
   case saAmfCompAmStartCmd_ID:
      m_AVSV_OCTVAL_TO_PARAM(param, buffer, ((uns16)comp->comp_info.amstart_len),
                             comp->comp_info.amstart_info);
      break;
   case saAmfCompAmStopCmd_ID:
      m_AVSV_OCTVAL_TO_PARAM(param, buffer, ((uns16)comp->comp_info.amstop_len),
                             comp->comp_info.amstop_info);
      break;

   /* CLC commands TIMEOUT values */
   case saAmfCompInstantiateTimeout_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,comp->comp_info.init_time);
      break;
   case saAmfCompDelayBetweenInstantiateAttempts_ID:
      m_AVSV_UNS64_TO_PARAM(param, buffer, comp->inst_retry_delay);
      break;
   case saAmfCompTerminateTimeout_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,comp->comp_info.term_time);
      break;
   case saAmfCompCleanupTimeout_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,comp->comp_info.clean_time);
      break;
   case saAmfCompAmStartTimeout_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,comp->comp_info.amstart_time);
      break;
   case saAmfCompAmStopTimeout_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,comp->comp_info.amstop_time);
      break;
   case saAmfCompTerminateCallbackTimeOut_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,comp->comp_info.terminate_callback_timeout);
      break;
   case saAmfCompCSISetCallbackTimeout_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,comp->comp_info.csi_set_callback_timeout);
      break;
   case saAmfCompQuiescingCompleteTimeout_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,comp->comp_info.quiescing_complete_timeout);
      break;
   case saAmfCompCSIRmvCallbackTimeout_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,comp->comp_info.csi_rmv_callback_timeout);
      break;
   case saAmfCompProxiedCompInstantiateCallbackTimeout_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,comp->comp_info.proxied_inst_callback_timeout);
      break;
   case saAmfCompProxiedCompCleanupCallbackTimeout_ID:
      m_AVSV_UNS64_TO_PARAM(param,buffer,comp->comp_info.proxied_clean_callback_timeout);
      break;
   case saAmfCompNodeRebootCleanupFail_ID:
      param->i_fmat_id = NCSMIB_FMAT_INT; 
      param->i_length = 1; 
      param->info.i_int = (comp->nodefail_cleanfail == FALSE) ? NCS_SNMP_FALSE:NCS_SNMP_TRUE;
      break;
   case saAmfCompDisableRestart_ID:
      param->i_fmat_id = NCSMIB_FMAT_INT; 
      param->i_length = 1; 
      param->info.i_int = (comp->comp_info.comp_restart == FALSE) ? NCS_SNMP_FALSE:NCS_SNMP_TRUE;
      break;
   /* proxy name */
   case saAmfCompCurrProxyName_ID:
      if (comp->proxy_comp_name_net.length != 0)
      {
         m_AVSV_OCTVAL_TO_PARAM(param, buffer,
                           m_NCS_OS_NTOHS(comp->proxy_comp_name_net.length),
                             comp->proxy_comp_name_net.value);
      }else
      {
         empty_name.length = 0;
         m_AVSV_LENVAL_TO_PARAM(param,buffer,empty_name);
      }   
      break;
   case saAmfCompAMEnable_ID:
      param->i_fmat_id = NCSMIB_FMAT_INT; 
      param->i_length = 1; 
      param->info.i_int = (comp->comp_info.am_enable == FALSE) ? NCS_SNMP_FALSE:NCS_SNMP_TRUE;
      break;

   case saAmfCompNumCurrActiveCsi_ID:
      param->i_fmat_id = NCSMIB_FMAT_INT;
      param->i_length = 1;
      param->info.i_int = 0;
     
      /* Count the num of active comp->su->list_of_susi->ha_state */
      if(comp->su != AVD_SU_NULL)
      {
         temp_susi_list = comp->su->list_of_susi;
         while(temp_susi_list != AVD_SU_SI_REL_NULL)
         {
            if(temp_susi_list->state != SA_AMF_HA_STANDBY)
               param->info.i_int = param->info.i_int + 1;
            temp_susi_list = temp_susi_list->su_next;
         }
      }
 
      break;
   case saAmfCompNumCurrStandbyCsi_ID:
      param->i_fmat_id = NCSMIB_FMAT_INT;
      param->i_length = 1;
      param->info.i_int = 0;

      /*Count the num of standby comp->su->list_of_susi->ha_state */
      if(comp->su != AVD_SU_NULL)
      {
         temp_susi_list = comp->su->list_of_susi;
         while(temp_susi_list != AVD_SU_SI_REL_NULL)
         {
            if(temp_susi_list->state == SA_AMF_HA_STANDBY)
               param->info.i_int = param->info.i_int + 1;
            temp_susi_list = temp_susi_list->su_next;
         }
      } 
      break;
      
   case saAmfCompReadinessState_ID:
      param->i_fmat_id = NCSMIB_FMAT_INT;
      param->i_length = 1;
      param->info.i_int = NCS_OUT_OF_SERVICE;
      if(comp->su != AVD_SU_NULL)
      {
         if( (comp->oper_state == NCS_OPER_STATE_ENABLE) && (comp->su->readiness_state == NCS_IN_SERVICE) )
         {
            param->info.i_int = NCS_IN_SERVICE;
         }
         else if( (comp->oper_state == NCS_OPER_STATE_ENABLE) && (comp->su->readiness_state == NCS_STOPPING) )
         {
            param->info.i_int = NCS_STOPPING;
         }
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
 * Function: saamfcomptableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_COMP_TABLE_ENTRY_ID table. This is the component table. The
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

uns32 saamfcomptableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_COMP      *comp = AVD_COMP_NULL, *i_comp=AVD_COMP_NULL;
   SaNameT       comp_name;
   SaNameT       temp_name;
   uns32         i, temp_param_id=0;
   NCS_BOOL      val_same_flag = FALSE;
   uns32         rc, min_si=0;
   NCS_BOOL      isPre;
   NCSMIBLIB_REQ_INFO temp_mib_req;
   AVSV_PARAM_INFO param;
  
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_INV_VAL;  
   }

   temp_param_id = arg->req.info.set_req.i_param_val.i_param_id;

   m_NCS_MEMSET(&comp_name, '\0', sizeof(SaNameT));
   
   /* Prepare the component database key from the instant ID */
   comp_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
   for(i = 0; i < comp_name.length; i++)
   {
      comp_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   comp = avd_comp_struc_find(avd_cb,comp_name,TRUE);

   if (comp == AVD_COMP_NULL)
   {
      if((arg->req.info.set_req.i_param_val.i_param_id == saAmfCompRowStatus_ID)
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

      comp = avd_comp_struc_crt(avd_cb,comp_name, FALSE);

      m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

      if (comp == AVD_COMP_NULL)
      {
         /* Invalid instance object */
         return NCSCC_RC_NO_INSTANCE; 
      }

   }else /* if (comp == AVD_COMP_NULL) */
   {
      /* The record is already available */

      if(arg->req.info.set_req.i_param_val.i_param_id == saAmfCompRowStatus_ID)
      {
         /* This is a row status operation */
         if (arg->req.info.set_req.i_param_val.info.i_int == (uns32)comp->row_status)
         {
            /* row status object is same so nothing to be done. */
            return NCSCC_RC_SUCCESS;
         }

         switch(arg->req.info.set_req.i_param_val.info.i_int)
         {
         case NCS_ROW_ACTIVE:
            /* validate the structure to see if the row can be made active */
            if ((comp->comp_info.category == NCS_COMP_TYPE_SA_AWARE) &&
               (comp->comp_info.init_len == 0))
            {
               /* All the mandatory fields are not filled
                */
               /* log information error */
               return NCSCC_RC_INV_VAL;
            }
            else if((comp->comp_info.category == NCS_COMP_TYPE_NON_SAF) &&
                 ((comp->comp_info.init_len == 0) ||
                 (comp->comp_info.term_len == 0)))
            {
               /* All the mandatory fields are not filled
                */
               /* log information error */
               return NCSCC_RC_INV_VAL;
            }
   
            if ((comp->comp_info.clean_len == 0) ||
               (comp->comp_info.max_num_inst == 0)) 
            {
               /* All the mandatory fields are not filled
                */
               /* log information error */
               return NCSCC_RC_INV_VAL;
            }

            if((comp->comp_info.category == NCS_COMP_TYPE_SA_AWARE) ||
               (comp->comp_info.category == NCS_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE) ||
               (comp->comp_info.category == NCS_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE) )
            {

               if(comp->comp_info.cap == NCS_COMP_CAPABILITY_1_ACTIVE_OR_Y_STANDBY )
               {
                  comp->max_num_csi_actv = 1;
               }
               else if((comp->comp_info.cap == NCS_COMP_CAPABILITY_1_ACTIVE_OR_1_STANDBY ) ||
                     (comp->comp_info.cap == NCS_COMP_CAPABILITY_1_ACTIVE ))
               {
                  comp->max_num_csi_actv = 1;
                  comp->max_num_csi_stdby = 1;
               }
               else if(comp->comp_info.cap == NCS_COMP_CAPABILITY_X_ACTIVE )
               {
                  comp->max_num_csi_stdby = comp->max_num_csi_actv;
               }

               if((comp->max_num_csi_actv == 0) ||
                  (comp->max_num_csi_stdby == 0) )
                  return NCSCC_RC_INV_VAL;
            }
            /* check that the SU is present and is active.
             */

            /* get the SU name*/
            avsv_cpy_SU_DN_from_DN(&temp_name, &comp_name);

            if(temp_name.length == 0)
            {
               /* log information error */
               return NCSCC_RC_INV_VAL;
            }

            /* check that the SU is present and is active.
             */

            if ((comp->su = avd_su_struc_find(avd_cb,temp_name,TRUE)) 
                                          == AVD_SU_NULL)
               return NCSCC_RC_INV_VAL;
      
            if (comp->su->row_status != NCS_ROW_ACTIVE)
            {
               comp->su = AVD_SU_NULL;
               return NCSCC_RC_INV_VAL;
            }


            /* The component should belong to the proper SG */
            if(( (comp->su->sg_of_su->su_redundancy_model == AVSV_SG_RED_MODL_NWAY) &&
                ((comp->comp_info.cap != NCS_COMP_CAPABILITY_X_ACTIVE_AND_Y_STANDBY) || (comp->comp_info.category == NCS_COMP_TYPE_NON_SAF))) ||
                  ((comp->su->sg_of_su->su_redundancy_model == AVSV_SG_RED_MODL_NPM) &&
                                   (comp->comp_info.cap == NCS_COMP_CAPABILITY_1_ACTIVE_OR_1_STANDBY)) )
               return NCSCC_RC_INV_VAL;


            /* Should not have NPI NCS SUs */
            if( (comp->comp_info.category != NCS_COMP_TYPE_SA_AWARE ) &&
                (comp->su->sg_of_su->sg_ncs_spec == SA_TRUE) && 
                (comp->su->su_preinstan == SA_FALSE) &&
                 (comp->su->list_of_comp == AVD_COMP_NULL) )
            {
               /* log information error */
               comp->su = AVD_SU_NULL;
               return NCSCC_RC_INV_VAL;
            }

            if((comp->su->curr_num_comp + 1) > comp->su->num_of_comp)
            {
               /* log information error */
               return NCSCC_RC_INV_VAL;
            }

            if(comp->comp_info.am_enable == TRUE)
            {
               if( (comp->comp_info.amstop_len == 0) ||
                   (comp->comp_info.amstart_len == 0) ||
                   (comp->comp_info.amstart_time == 0) ||
                   (comp->comp_info.amstop_time == 0) )
               {
                  return NCSCC_RC_INV_VAL;
               }
            }

            if(test_flag == TRUE)
            {
               return NCSCC_RC_SUCCESS;
            }
  
            /* add to the list of SU  */
            i_comp = comp->su->list_of_comp;
            if((i_comp ==AVD_COMP_NULL) || 
              (i_comp->comp_info.inst_level >= comp->comp_info.inst_level))
            {
               comp->su->list_of_comp = comp;
               comp->su_comp_next = i_comp;
            }
            else
            {
               while((i_comp->su_comp_next != AVD_COMP_NULL) && 
                    (i_comp->su_comp_next->comp_info.inst_level < comp->comp_info.inst_level))
                    i_comp = i_comp->su_comp_next;

                    comp->su_comp_next = i_comp->su_comp_next;
                    i_comp->su_comp_next = comp;
            }

            
            /* set the value, checkpoint the entire record.
             */
            comp->row_status = NCS_ROW_ACTIVE;

            /* update the SU information about the component */
            comp->su->curr_num_comp ++;

            /* check if the
             * corresponding node is UP send the component information
             * to the Node.
             */
            if((comp->su->su_on_node->node_state == AVD_AVND_STATE_PRESENT) ||
               (comp->su->su_on_node->node_state == AVD_AVND_STATE_NO_CONFIG) ||
               (comp->su->su_on_node->node_state == AVD_AVND_STATE_NCS_INIT))
            { 
               if (avd_snd_comp_msg(avd_cb,comp) != NCSCC_RC_SUCCESS)
               {
                  /* the SU will never get to readiness state in service */
                  /* Log an internal error */
                  comp->su->curr_num_comp --;
                  avd_comp_del_su_list(avd_cb,comp);
                  comp->row_status = NCS_ROW_NOT_READY;
                  return NCSCC_RC_INV_VAL;
               }
            }
            
            m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(cb, comp, AVSV_CKPT_AVD_COMP_CONFIG);

            /* Verify if the SUs preinstan value need to be changed */
            if(comp->comp_info.category >= NCS_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE) 
            {
               isPre = FALSE;
               i_comp = comp->su->list_of_comp;
               while(i_comp)
               { 
                  if(i_comp->comp_info.category < NCS_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE) 
                  {
                     isPre = TRUE;
                     break;
                  }
                  i_comp = i_comp->su_comp_next;
               }
               if(isPre == FALSE)
               {
                  comp->su->su_preinstan = FALSE;
               }
               comp->max_num_csi_actv = 1;
               comp->max_num_csi_stdby = 1;
            }
            else
            {
               comp->su->su_preinstan = TRUE;
            }

            
            if( (comp->max_num_csi_actv < comp->su->si_max_active) || 
                (comp->su->si_max_active == 0) )
            {
               comp->su->si_max_active = comp->max_num_csi_actv;
            }

            if( (comp->max_num_csi_stdby < comp->su->si_max_standby) || 
                (comp->su->si_max_standby== 0) )
            {
               comp->su->si_max_standby = comp->max_num_csi_stdby;
            }
            
            m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (comp->su), AVSV_CKPT_AVD_SU_CONFIG);

            return NCSCC_RC_SUCCESS;
            break;

         case NCS_ROW_NOT_IN_SERVICE:
         case NCS_ROW_DESTROY:
            
            /* check if it is active currently */

            if(comp->row_status == NCS_ROW_ACTIVE)
            {

               /* Check to see that the SU of which the component is a
                * part is in admin locked state, in term state with
                * no assignments before
                * making the row status as not in service or delete 
                */
               if((comp->su->sg_of_su->sg_ncs_spec == TRUE) ||
                  (comp->su->admin_state != NCS_ADMIN_STATE_LOCK) ||
                  (comp->su->pres_state != NCS_PRES_UNINSTANTIATED) ||
                  (comp->su->list_of_susi != AVD_SU_SI_REL_NULL) ||
                  ((comp->su->su_preinstan == TRUE) && (comp->su->term_state != TRUE)))
               {
                  /* log information error */
                  return NCSCC_RC_INV_VAL;
               }

               if(test_flag == TRUE)
               {
                  return NCSCC_RC_SUCCESS;
               }

               /* verify if the max ACTIVE and STANDBY SIs of the SU 
               ** need to be changed
               */
               if(comp->max_num_csi_actv == comp->su->si_max_active)
               {
                  /* find the number and set it */
                  min_si = 0; 
                  i_comp = comp->su->list_of_comp;
                  while(i_comp)
                  { 
                     if(i_comp != comp) 
                     {
                        if(min_si > i_comp->max_num_csi_actv) 
                           min_si = i_comp->max_num_csi_actv;
                        else if(min_si == 0)
                           min_si = i_comp->max_num_csi_actv;
                     }
                     i_comp = i_comp->su_comp_next;
                  }
                  /* Now we have the min value. set it */
                  comp->su->si_max_active = min_si;
               }

               /* FOR STANDBY count */
               if(comp->max_num_csi_stdby == comp->su->si_max_standby)
               {
                  /* find the number and set it */
                  min_si = 0;
                  i_comp = comp->su->list_of_comp;
                  while(i_comp)
                  { 
                     if(i_comp != comp)
                     {
                        if(min_si > i_comp->max_num_csi_stdby)
                           min_si = i_comp->max_num_csi_stdby;
                        else if(min_si == 0)
                           min_si = i_comp->max_num_csi_stdby;
                     }
                     i_comp = i_comp->su_comp_next;
                  }
                  /* Now we have the min value. set it */
                  comp->su->si_max_standby = min_si;
               }

               /* Verify if the SUs preinstan value need to be changed */
               if(comp->comp_info.category < NCS_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE) 
               {
                  isPre = FALSE;
                  i_comp = comp->su->list_of_comp;
                  while(i_comp)
                  { 
                     if( (comp->comp_info.category < NCS_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE) 
                        && (i_comp != comp) )
                     {
                        isPre = TRUE;
                        break;
                     }
                     i_comp = i_comp->su_comp_next;
                  } /* end while */

                  if(isPre == TRUE)
                  {
                     comp->su->su_preinstan = TRUE;
                  }
                  else
                  {
                     comp->su->su_preinstan = FALSE;
                  }
               }

               if(comp->su->curr_num_comp == 1)
               {
                  /* This comp will be deleted so revert these to def val*/
                  comp->su->si_max_active = 0;
                  comp->su->si_max_standby = 0;
                  comp->su->su_preinstan = TRUE;
               }

              
               /* send a message to the AVND deleting the
                * component.
                */
               if((comp->su->su_on_node->node_state == AVD_AVND_STATE_PRESENT) ||
                  (comp->su->su_on_node->node_state == AVD_AVND_STATE_NO_CONFIG) ||
                  (comp->su->su_on_node->node_state == AVD_AVND_STATE_NCS_INIT))
               {
                  m_NCS_MEMSET(((uns8 *)&param),'\0',sizeof(AVSV_PARAM_INFO));
                  param.act = AVSV_OBJ_OPR_DEL;
                  param.name_net = comp->comp_info.name_net;
                  param.table_id = NCSMIB_TBL_AVSV_AMF_COMP;
                  avd_snd_op_req_msg(avd_cb,comp->su->su_on_node,&param);
               }

               /* decrement the active component number of this SU */
               comp->su->curr_num_comp --;
               
               m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (comp->su), AVSV_CKPT_AVD_SU_CONFIG);
               /* we need to delete this comp structure on the
                * standby AVD. check point to the standby AVD that this
                * record need to be deleted
                */

               m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, comp, AVSV_CKPT_AVD_COMP_CONFIG);

               avd_comp_del_su_list(avd_cb,comp);

            } /* if(comp->row_status == NCS_ROW_ACTIVE) */

            if(test_flag == TRUE)
            {
               return NCSCC_RC_SUCCESS;
            }

            if(arg->req.info.set_req.i_param_val.info.i_int
                  == NCS_ROW_DESTROY)
            {
               /* delete and free the structure */
               m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

               /* remove the component from the SU list. 
                */

               avd_comp_del_su_list(avd_cb,comp);
               
               avd_comp_struc_del(avd_cb,comp);

               m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

               return NCSCC_RC_SUCCESS;

            } /* if(arg->req.info.set_req.i_param_val.info.i_int
                  == NCS_ROW_DESTROY) */

            comp->row_status = arg->req.info.set_req.i_param_val.info.i_int;
            return NCSCC_RC_SUCCESS;

            break;
         default:
            m_AVD_LOG_INVALID_VAL_ERROR(arg->req.info.set_req.i_param_val.info.i_int);
            /* Invalid row status object */
            return NCSCC_RC_INV_VAL;
            break;
         } /* switch(arg->req.info.set_req.i_param_val.info.i_int) */

      } /* if(arg->req.info.set_req.i_param_val.i_param_id == ncsCompRowStatus_ID) */

   } /* if (comp == AVD_COMP_NULL) */

   /* We now have the component block */
   if(test_flag == TRUE)
   {
      return NCSCC_RC_SUCCESS;
   }

   
   if(comp->row_status == NCS_ROW_ACTIVE)
   {
      if((comp->su->su_on_node->node_state == AVD_AVND_STATE_PRESENT) ||
         (comp->su->su_on_node->node_state == AVD_AVND_STATE_NO_CONFIG) ||
         (comp->su->su_on_node->node_state == AVD_AVND_STATE_NCS_INIT))
      {
         m_NCS_MEMSET(((uns8 *)&param),'\0',sizeof(AVSV_PARAM_INFO));
         param.table_id = NCSMIB_TBL_AVSV_AMF_COMP;
         param.obj_id = arg->req.info.set_req.i_param_val.i_param_id;   
         param.act = AVSV_OBJ_OPR_MOD;
         param.name_net = comp->comp_info.name_net;

         switch(arg->req.info.set_req.i_param_val.i_param_id)
         {
         case saAmfCompInstantiateCmd_ID:
           param.value_len = arg->req.info.set_req.i_param_val.i_length;
           m_NCS_MEMCPY(&param.value[0],
                        arg->req.info.set_req.i_param_val.info.i_oct,
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.init_len = arg->req.info.set_req.i_param_val.i_length;
               m_NCS_MEMCPY(comp->comp_info.init_info,
                            arg->req.info.set_req.i_param_val.info.i_oct,
                            comp->comp_info.init_len);
            break;
         case saAmfCompTerminateCmd_ID:
           param.value_len = arg->req.info.set_req.i_param_val.i_length;
           m_NCS_MEMCPY(&param.value[0],
                        arg->req.info.set_req.i_param_val.info.i_oct,
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.term_len = arg->req.info.set_req.i_param_val.i_length;
               m_NCS_MEMCPY(comp->comp_info.term_info,
                            arg->req.info.set_req.i_param_val.info.i_oct,
                            comp->comp_info.term_len);
            break;
         case saAmfCompCleanupCmd_ID:
           param.value_len = arg->req.info.set_req.i_param_val.i_length;
           m_NCS_MEMCPY(&param.value[0],
                        arg->req.info.set_req.i_param_val.info.i_oct,
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.clean_len = arg->req.info.set_req.i_param_val.i_length;
               m_NCS_MEMCPY(comp->comp_info.clean_info,
                            arg->req.info.set_req.i_param_val.info.i_oct,
                            comp->comp_info.clean_len);
            break;
         case saAmfCompAmStartCmd_ID:
           param.value_len = arg->req.info.set_req.i_param_val.i_length;
           m_NCS_MEMCPY(&param.value[0],
                        arg->req.info.set_req.i_param_val.info.i_oct,
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.amstart_len = arg->req.info.set_req.i_param_val.i_length;
               m_NCS_MEMCPY(comp->comp_info.amstart_info,
                            arg->req.info.set_req.i_param_val.info.i_oct,
                            comp->comp_info.amstart_len);
            break;
         case saAmfCompAmStopCmd_ID:
           param.value_len = arg->req.info.set_req.i_param_val.i_length;
           m_NCS_MEMCPY(&param.value[0],
                        arg->req.info.set_req.i_param_val.info.i_oct,
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.amstop_len = arg->req.info.set_req.i_param_val.i_length;
               m_NCS_MEMCPY(comp->comp_info.amstop_info,
                            arg->req.info.set_req.i_param_val.info.i_oct,
                            comp->comp_info.amstop_len);
            break;
         case saAmfCompInstantiateTimeout_ID:
           param.value_len = sizeof(SaTimeT);
           m_NCS_MEMCPY(&param.value[0], 
                        arg->req.info.set_req.i_param_val.info.i_oct, 
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.init_time = 
                    m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);

            break;
         case saAmfCompDelayBetweenInstantiateAttempts_ID:
           param.value_len = sizeof(SaTimeT);
           m_NCS_MEMCPY(&param.value[0], 
                        arg->req.info.set_req.i_param_val.info.i_oct, 
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->inst_retry_delay = 
                    m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
            break;
         case saAmfCompTerminateTimeout_ID:
           param.value_len = sizeof(SaTimeT);
           m_NCS_MEMCPY(&param.value[0], 
                        arg->req.info.set_req.i_param_val.info.i_oct, 
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.term_time = 
                    m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
            break;
         case saAmfCompCleanupTimeout_ID:
           param.value_len = sizeof(SaTimeT);
           m_NCS_MEMCPY(&param.value[0], 
                        arg->req.info.set_req.i_param_val.info.i_oct, 
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.clean_time = 
                    m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
            break;
         case saAmfCompAmStartTimeout_ID:
           param.value_len = sizeof(SaTimeT);
           m_NCS_MEMCPY(&param.value[0], 
                        arg->req.info.set_req.i_param_val.info.i_oct, 
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.amstart_time = 
                    m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
            break;
         case saAmfCompAmStopTimeout_ID:
           param.value_len = sizeof(SaTimeT);
           m_NCS_MEMCPY(&param.value[0], 
                        arg->req.info.set_req.i_param_val.info.i_oct, 
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.amstop_time = 
                    m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
            break;
         case saAmfCompTerminateCallbackTimeOut_ID:
           param.value_len = sizeof(SaTimeT);
           m_NCS_MEMCPY(&param.value[0], 
                        arg->req.info.set_req.i_param_val.info.i_oct, 
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.terminate_callback_timeout = 
                    m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
            break;
         case saAmfCompCSISetCallbackTimeout_ID:
           param.value_len = sizeof(SaTimeT);
           m_NCS_MEMCPY(&param.value[0],
                        arg->req.info.set_req.i_param_val.info.i_oct,
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.csi_set_callback_timeout =
                    m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
            break;
         case saAmfCompQuiescingCompleteTimeout_ID:
           param.value_len = sizeof(SaTimeT);
           m_NCS_MEMCPY(&param.value[0],
                        arg->req.info.set_req.i_param_val.info.i_oct,
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.quiescing_complete_timeout =
                    m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
            break;
         case saAmfCompCSIRmvCallbackTimeout_ID:
           param.value_len = sizeof(SaTimeT);
           m_NCS_MEMCPY(&param.value[0],
                        arg->req.info.set_req.i_param_val.info.i_oct,
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.csi_rmv_callback_timeout =
                    m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
            break;
         case saAmfCompProxiedCompInstantiateCallbackTimeout_ID:
           param.value_len = sizeof(SaTimeT);
           m_NCS_MEMCPY(&param.value[0],
                        arg->req.info.set_req.i_param_val.info.i_oct,
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.proxied_inst_callback_timeout =
                    m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
            break;
         case saAmfCompProxiedCompCleanupCallbackTimeout_ID:
           param.value_len = sizeof(SaTimeT);
           m_NCS_MEMCPY(&param.value[0],
                        arg->req.info.set_req.i_param_val.info.i_oct,
                        param.value_len);

            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.proxied_clean_callback_timeout =
                    m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
            break;
         case saAmfCompNodeRebootCleanupFail_ID:
            param.value_len = 1;
            param.value[0] = (uns8)arg->req.info.set_req.i_param_val.info.i_int;    
            
            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->nodefail_cleanfail = (arg->req.info.set_req.i_param_val.info.i_int == NCS_SNMP_TRUE) ? TRUE : FALSE; 
            break;
         case saAmfCompRecoveryOnError_ID:
            if(arg->req.info.set_req.i_param_val.info.i_int == SA_AMF_NO_RECOMMENDATION)
            {
               return NCSCC_RC_INV_VAL;
            }
            param.value_len = sizeof(uns32);
            m_NCS_OS_HTONL_P(&param.value[0], arg->req.info.set_req.i_param_val.info.i_int);
            
            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.def_recvr = arg->req.info.set_req.i_param_val.info.i_int;
            break;
         case saAmfCompNumMaxInstantiate_ID:
            param.value_len = sizeof(uns32);
            m_NCS_OS_HTONL_P(&param.value[0], arg->req.info.set_req.i_param_val.info.i_int);
            
            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.max_num_inst = arg->req.info.set_req.i_param_val.info.i_int;
            break;
         case saAmfCompNumMaxInstantiateWithDelay_ID:
            param.value_len = sizeof(uns32);
            m_NCS_OS_HTONL_P(&param.value[0], arg->req.info.set_req.i_param_val.info.i_int);
            
            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->max_num_inst_delay = arg->req.info.set_req.i_param_val.info.i_int;
            break;
         case saAmfCompNumMaxAmStartAttempts_ID:
            param.value_len = sizeof(uns32);
            m_NCS_OS_HTONL_P(&param.value[0], arg->req.info.set_req.i_param_val.info.i_int);
            
            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.max_num_amstart = arg->req.info.set_req.i_param_val.info.i_int;
            break;
         case saAmfCompNumMaxAmStopAttempts_ID:
            param.value_len = sizeof(uns32);
            m_NCS_OS_HTONL_P(&param.value[0], arg->req.info.set_req.i_param_val.info.i_int);
            
            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.max_num_amstop = arg->req.info.set_req.i_param_val.info.i_int;
            break;

         case saAmfCompAMEnable_ID:
            if(comp->comp_info.am_enable == (arg->req.info.set_req.i_param_val.info.i_int == NCS_SNMP_TRUE) ? TRUE : FALSE)
               break;

            if(arg->req.info.set_req.i_param_val.info.i_int == NCS_SNMP_TRUE)
            {
               if( (comp->comp_info.amstop_len == 0) ||
                   (comp->comp_info.amstart_len == 0) ||
                   (comp->comp_info.amstart_time == 0) ||
                   (comp->comp_info.amstop_time == 0) )
               {
                  return NCSCC_RC_INV_VAL;
               }
            }

            param.value_len = 1;
            param.value[0] = (arg->req.info.set_req.i_param_val.info.i_int == NCS_SNMP_TRUE) ? TRUE : FALSE;    
            
            rc = avd_snd_op_req_msg(avd_cb, comp->su->su_on_node, &param);
            if( rc != NCSCC_RC_SUCCESS)
               return NCSCC_RC_INV_VAL;
            else
               comp->comp_info.am_enable = (arg->req.info.set_req.i_param_val.info.i_int == NCS_SNMP_TRUE) ? TRUE : FALSE; 
            break;
         default:
            /* when row status is active we don't allow any other MIB object to be
             * modified.
             */
            return NCSCC_RC_INV_VAL;
         }
         
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVSV_CKPT_AVD_COMP_CONFIG);
         return NCSCC_RC_SUCCESS;
      } /* NOT ABSENT */
      else if( (temp_param_id == saAmfCompInstantiateTimeout_ID) ||
               (temp_param_id == saAmfCompDelayBetweenInstantiateAttempts_ID) ||
               (temp_param_id == saAmfCompTerminateTimeout_ID) ||
               (temp_param_id == saAmfCompCleanupTimeout_ID) ||
               (temp_param_id == saAmfCompAmStartTimeout_ID) ||
               (temp_param_id == saAmfCompAmStopTimeout_ID) ||
               (temp_param_id == saAmfCompTerminateCallbackTimeOut_ID) ||
               (temp_param_id == saAmfCompCSISetCallbackTimeout_ID) ||
               (temp_param_id == saAmfCompQuiescingCompleteTimeout_ID) ||
               (temp_param_id == saAmfCompCSIRmvCallbackTimeout_ID) ||
               (temp_param_id == saAmfCompProxiedCompInstantiateCallbackTimeout_ID) ||
               (temp_param_id == saAmfCompProxiedCompCleanupCallbackTimeout_ID) ||
               (temp_param_id == saAmfCompNodeRebootCleanupFail_ID) ||
               (temp_param_id == saAmfCompRecoveryOnError_ID) ||
               (temp_param_id == saAmfCompNumMaxInstantiate_ID) ||
               (temp_param_id == saAmfCompNumMaxInstantiateWithDelay_ID) ||
               (temp_param_id == saAmfCompNumMaxAmStartAttempts_ID) ||
               (temp_param_id == saAmfCompNumMaxAmStopAttempts_ID) )

      {
         /* if node is absent we need to set the value and hence fall thru */
      }
      else
      {
         /* Other values cant be set when row status is active irrespective 
          * of node state */
         return NCSCC_RC_INV_VAL;
      }
   } /*if(comp->row_status == NCS_ROW_ACTIVE)*/

   switch(arg->req.info.set_req.i_param_val.i_param_id)
   {
   case saAmfCompRowStatus_ID:
      /* fill the row status value */
      if(arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT)
      {
         comp->row_status = arg->req.info.set_req.i_param_val.info.i_int;
      }
      break;
   case saAmfCompInstantiateCmd_ID:
      comp->comp_info.init_len = arg->req.info.set_req.i_param_val.i_length;
      m_NCS_MEMCPY(comp->comp_info.init_info,
                   arg->req.info.set_req.i_param_val.info.i_oct,
                   comp->comp_info.init_len);
      break;
   case saAmfCompTerminateCmd_ID:
      comp->comp_info.term_len = arg->req.info.set_req.i_param_val.i_length;
      m_NCS_MEMCPY(comp->comp_info.term_info,
                   arg->req.info.set_req.i_param_val.info.i_oct,
                   comp->comp_info.term_len);
      break;
   case saAmfCompCleanupCmd_ID:
      comp->comp_info.clean_len = arg->req.info.set_req.i_param_val.i_length;
      m_NCS_MEMCPY(comp->comp_info.clean_info,
                   arg->req.info.set_req.i_param_val.info.i_oct,
                   comp->comp_info.clean_len);
      break;
   case saAmfCompAmStartCmd_ID:
      comp->comp_info.amstart_len = arg->req.info.set_req.i_param_val.i_length;
      m_NCS_MEMCPY(comp->comp_info.amstart_info,
                   arg->req.info.set_req.i_param_val.info.i_oct,
                   comp->comp_info.amstart_len );
      break;
   case saAmfCompAmStopCmd_ID:
      comp->comp_info.amstop_len = arg->req.info.set_req.i_param_val.i_length;
      m_NCS_MEMCPY(comp->comp_info.amstop_info,
                   arg->req.info.set_req.i_param_val.info.i_oct,
                   comp->comp_info.amstop_len);
      break;
   case saAmfCompInstantiateTimeout_ID:
      comp->comp_info.init_time = 
              m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
      break;
   case saAmfCompDelayBetweenInstantiateAttempts_ID:
      comp->inst_retry_delay = 
              m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
      break;
   case saAmfCompTerminateTimeout_ID:
      comp->comp_info.term_time = 
              m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
      break;
   case saAmfCompCleanupTimeout_ID:
      comp->comp_info.clean_time = 
              m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
      break;
   case saAmfCompAmStartTimeout_ID:
      comp->comp_info.amstart_time = 
              m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
      break;
   case saAmfCompAmStopTimeout_ID:
      comp->comp_info.amstop_time = 
              m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
      break;
   case saAmfCompTerminateCallbackTimeOut_ID:
      comp->comp_info.terminate_callback_timeout =
              m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
      break;
   case saAmfCompCSISetCallbackTimeout_ID:
      comp->comp_info.csi_set_callback_timeout =
              m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
      break;
   case saAmfCompQuiescingCompleteTimeout_ID:
      comp->comp_info.quiescing_complete_timeout =
              m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
      break;
   case saAmfCompCSIRmvCallbackTimeout_ID:
      comp->comp_info.csi_rmv_callback_timeout =
              m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
      break;
   case saAmfCompProxiedCompInstantiateCallbackTimeout_ID:
      comp->comp_info.proxied_inst_callback_timeout =
              m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
      break;
   case saAmfCompProxiedCompCleanupCallbackTimeout_ID:
      comp->comp_info.proxied_clean_callback_timeout =
              m_NCS_OS_NTOHLL_P(arg->req.info.set_req.i_param_val.info.i_oct);
      break;
   case saAmfCompNodeRebootCleanupFail_ID:
      comp->nodefail_cleanfail = (arg->req.info.set_req.i_param_val.info.i_int == NCS_SNMP_TRUE) ? TRUE:FALSE; 
         break;
   case saAmfCompDisableRestart_ID:
      comp->comp_info.comp_restart = (arg->req.info.set_req.i_param_val.info.i_int == NCS_SNMP_TRUE) ? TRUE:FALSE; 
         break;
   case saAmfCompAMEnable_ID:
      comp->comp_info.am_enable = (arg->req.info.set_req.i_param_val.info.i_int == NCS_SNMP_TRUE) ? TRUE : FALSE; 
      break;
   case saAmfCompRecoveryOnError_ID:
      if(arg->req.info.set_req.i_param_val.info.i_int == SA_AMF_NO_RECOMMENDATION)
      {
         return NCSCC_RC_INV_VAL;
      }
      else
      {
         comp->comp_info.def_recvr = arg->req.info.set_req.i_param_val.info.i_int;
      }
   default:
      m_NCS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 

      temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
      temp_mib_req.info.i_set_util_info.param = &(arg->req.info.set_req.i_param_val);
      temp_mib_req.info.i_set_util_info.var_info = var_info;
      temp_mib_req.info.i_set_util_info.data = comp;
      temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

      /* call the mib routine handler */ 
      if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
      {
         return rc;
      }
      break;
   } /* switch(arg->req.info.set_req.i_param_val.i_param_id) */
   
   if(comp->row_status == NCS_ROW_ACTIVE)
   {
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVSV_CKPT_AVD_COMP_CONFIG);
   }
   
   return NCSCC_RC_SUCCESS;

}



/*****************************************************************************
 * Function: saamfcomptableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_COMP_TABLE_ENTRY_ID table. This is the component table. The
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

uns32 saamfcomptableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_COMP      *comp;
   SaNameT       comp_name;
   uns32         i;
   
   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;  
   }

   m_NCS_MEMSET(&comp_name, '\0', sizeof(SaNameT));

   if (arg->i_idx.i_inst_len != 0)
   {   
      /* Prepare the component database key from the instant ID */
      comp_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
      for(i = 0; i < comp_name.length; i++)
      {
         comp_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
   }

   comp = avd_comp_struc_find_next(avd_cb,comp_name,TRUE);

   if (comp == AVD_COMP_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the instant ID from the component name */

   *next_inst_id_len = m_NCS_OS_NTOHS(comp->comp_info.name_net.length) + 1;

   next_inst_id[0] = *next_inst_id_len -1;
   for(i = 0; i < next_inst_id[0]; i++)
   {
      next_inst_id[i + 1] = (uns32)(comp->comp_info.name_net.value[i]);
   }

   *data = (NCSCONTEXT)comp;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: saamfcomptableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_COMP_TABLE_ENTRY_ID table. This is the component table. The
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

uns32 saamfcomptableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: avd_comp_ack_msg
 *
 * Purpose:  This function processes the acknowledgment received for
 *          a non proxied component or proxied component related message 
 *          from AVND for operator related changes. If the message
 *          acknowledges success nothing to be done. If failure is
 *          reported a error is logged.
 *
 * Input: cb - the AVD control block
 *        ack_msg - The acknowledgement message received from the AVND.
 *
 * Returns: none.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_comp_ack_msg(AVD_CL_CB *cb,AVD_DND_MSG *ack_msg)
{

   AVD_COMP        *comp, *i_comp;   
   AVSV_PARAM_INFO param;
   AVD_AVND        *avnd;
   uns32           min_si=0;
   NCS_BOOL        isPre;

   /* check the ack message for errors. If error find the component that
    * has error.
    */

   if (ack_msg->msg_info.n2d_reg_comp.error != NCSCC_RC_SUCCESS)
   {
      /* Find the component */
      comp = avd_comp_struc_find(cb,ack_msg->msg_info.n2d_reg_comp.comp_name_net,FALSE);
      if ((comp == AVD_COMP_NULL) || (comp->row_status != NCS_ROW_ACTIVE))
      {
         /* The component has already been deleted. there is nothing
          * that can be done.
          */

         /* Log an information error that the component is
          * deleted.
          */
         return;
      }

      /* log an fatal error as normally this shouldnt happen.
       */
      m_AVD_LOG_INVALID_VAL_ERROR(ack_msg->msg_info.n2d_reg_comp.error);

      /* Make the row status as not in service to indicate that. It is
       * not active.
       */

      /* verify if the max ACTIVE and STANDBY SIs of the SU 
      ** need to be changed
      */
      if(comp->max_num_csi_actv == comp->su->si_max_active)
      {
         /* find the number and set it */
         min_si = 0; 
         i_comp = comp->su->list_of_comp;
         while(i_comp)
         { 
            if(i_comp != comp) 
            {
               if(min_si > i_comp->max_num_csi_actv) 
                  min_si = i_comp->max_num_csi_actv;
               else if(min_si == 0)
                  min_si = i_comp->max_num_csi_actv;
            }
            i_comp = i_comp->su_comp_next;
         }
         /* Now we have the min value. set it */
         comp->su->si_max_active = min_si;
      }

      /* FOR STANDBY count */
      if(comp->max_num_csi_stdby == comp->su->si_max_standby)
      {
         /* find the number and set it */
         min_si = 0;
         i_comp = comp->su->list_of_comp;
         while(i_comp)
         { 
            if(i_comp != comp)
            {
               if(min_si > i_comp->max_num_csi_stdby)
                  min_si = i_comp->max_num_csi_stdby;
               else if(min_si == 0)
                  min_si = i_comp->max_num_csi_stdby;
            }
            i_comp = i_comp->su_comp_next;
         }
         /* Now we have the min value. set it */
         comp->su->si_max_standby = min_si;
      }

      /* Verify if the SUs preinstan value need to be changed */
      if(comp->comp_info.category < NCS_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE) 
      {
         isPre = FALSE;
         i_comp = comp->su->list_of_comp;
         while(i_comp)
         { 
            if( (comp->comp_info.category < NCS_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE)
               && (i_comp != comp) )
            {
               isPre = TRUE;
               break;
            }
            i_comp = i_comp->su_comp_next;
         } /* end while */

         if(isPre == TRUE)
         {
            comp->su->su_preinstan = TRUE;
         }
         else
         {
            comp->su->su_preinstan = FALSE;
         }
      }

      if(comp->su->curr_num_comp == 1)
      {
         /* This comp will be deleted so revert these to def val*/
         comp->su->si_max_active = 0;
         comp->su->si_max_standby = 0;
         comp->su->su_preinstan = TRUE;
      }

      /* decrement the active component number of this SU */
      comp->su->curr_num_comp --;
      
      m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, (comp->su), AVSV_CKPT_AVD_SU_CONFIG);

      avd_comp_del_su_list(cb,comp);

      m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, comp, AVSV_CKPT_AVD_COMP_CONFIG);
      comp->row_status = NCS_ROW_NOT_IN_SERVICE;
      return;
   }

   /* Log an information error that the node was updated with the
    * information of the component.
    */
   /* Find the component */
   comp = avd_comp_struc_find(cb,ack_msg->msg_info.n2d_reg_comp.comp_name_net,FALSE);
   if (comp == AVD_COMP_NULL)
   {
      /* The comp has already been deleted. there is nothing
       * that can be done.
       */

      /* Log an information error that the comp is
       * deleted.
       */
      /* send a delete message to the AvND for the comp. */
      m_NCS_MEMSET(((uns8 *)&param),'\0',sizeof(AVSV_PARAM_INFO));
      param.act = AVSV_OBJ_OPR_DEL;
      param.name_net = ack_msg->msg_info.n2d_reg_comp.comp_name_net;
      param.table_id = NCSMIB_TBL_AVSV_AMF_COMP;
      avnd = avd_avnd_struc_find_nodeid(cb,ack_msg->msg_info.n2d_reg_comp.node_id);      
      avd_snd_op_req_msg(cb,avnd,&param);
      return;
   }else if (comp->row_status != NCS_ROW_ACTIVE)
   {
      /* Log an information error that the comp is
       * Not in service.
       */
      return;
   }
   
   if(comp->su->curr_num_comp == comp->su->num_of_comp)
   {
      if(comp->su->su_preinstan == TRUE)
      {
         avd_sg_app_su_inst_func(cb, comp->su->sg_of_su);
      }
      else
      {
         comp->oper_state = NCS_OPER_STATE_ENABLE;
         comp->su->oper_state = NCS_OPER_STATE_ENABLE;

         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp, AVSV_CKPT_COMP_OPER_STATE);
         m_AVSV_SEND_CKPT_UPDT_ASYNC_UPDT(cb, comp->su, AVSV_CKPT_SU_OPER_STATE);
         
         if(m_AVD_APP_SU_IS_INSVC(comp->su))
         {
            m_AVD_SET_SU_REDINESS(cb,(comp->su),NCS_IN_SERVICE);
            switch(comp->su->sg_of_su->su_redundancy_model)
            {
            case AVSV_SG_RED_MODL_2N:
               avd_sg_2n_su_insvc_func(cb, comp->su);
               break;

            case AVSV_SG_RED_MODL_NWAYACTV:
               avd_sg_nacvred_su_insvc_func(cb, comp->su);
               break;            

            case AVSV_SG_RED_MODL_NWAY:
               avd_sg_nway_su_insvc_func(cb, comp->su);
               break; 

            case AVSV_SG_RED_MODL_NPM:
               avd_sg_npm_su_insvc_func(cb, comp->su);
               break;             

            case AVSV_SG_RED_MODL_NORED:
            default:
               avd_sg_nored_su_insvc_func(cb, comp->su);
               break;
            }            
         }
      }
   }
   return;
}



/*****************************************************************************
 * Function: avd_comp_cs_type_struc_crt
 *
 * Purpose:  This function will create and add a AVD_COMP_CS_TYPE structure to the
 * tree if an element with specified key value doesn't exist in the tree.
 *
 * Input: cb   - the AVD control block
 *        indx - 
 *
 * Returns: The pointer to AVD_COMP_CS_TYPE structure allocated and added.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_COMP_CS_TYPE * avd_comp_cs_type_struc_crt(AVD_CL_CB *cb, AVD_COMP_CS_TYPE_INDX indx)
{
   AVD_COMP_CS_TYPE *cst = AVD_COMP_CS_TYPE_NULL;

   /* Allocate a new block structure now
    */
   if ((cst = m_MMGR_ALLOC_AVD_COMP_CS_TYPE) == AVD_COMP_CS_TYPE_NULL)
   {
      /* log an error */
      m_AVD_LOG_MEM_FAIL(AVD_COMP_CS_TYPE_ALLOC_FAILED);
      return AVD_COMP_CS_TYPE_NULL;
   }

   m_NCS_MEMSET((char *)cst, '\0', sizeof(AVD_COMP_CS_TYPE));

   cst->indx.comp_name_net.length = indx.comp_name_net.length;
   m_NCS_MEMCPY(cst->indx.comp_name_net.value,indx.comp_name_net.value,
                            m_NCS_OS_NTOHS(indx.comp_name_net.length));
   
   cst->indx.csi_type_name_net.length = indx.csi_type_name_net.length;
   m_NCS_MEMCPY(cst->indx.csi_type_name_net.value, indx.csi_type_name_net.value, 
                                 m_NCS_OS_NTOHS(indx.csi_type_name_net.length));
  
   cst->row_status = NCS_ROW_NOT_READY;

   cst->tree_node.key_info = (uns8*)(&cst->indx);
   cst->tree_node.bit   = 0;
   cst->tree_node.left  = NCS_PATRICIA_NODE_NULL;
   cst->tree_node.right = NCS_PATRICIA_NODE_NULL;

   if( ncs_patricia_tree_add(&cb->comp_cs_type_anchor,&cst->tree_node) 
                      != NCSCC_RC_SUCCESS)
   {
      /* log an error */
      m_MMGR_FREE_AVD_COMP_CS_TYPE(cst);
      return AVD_COMP_CS_TYPE_NULL;
   }


   return cst;
   
}


/*****************************************************************************
 * Function: avd_comp_cs_type_struc_find
 *
 * Purpose:  This function will find a AVD_COMP_CS_TYPE structure in the
 * tree with component name and CSI Type name as key.
 *
 * Input: cb   - the AVD control block
 *        indx - 
 *
 * Returns: The pointer to AVD_COMP_CS_TYPE structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_COMP_CS_TYPE * avd_comp_cs_type_struc_find(AVD_CL_CB *cb, AVD_COMP_CS_TYPE_INDX indx)
{
   AVD_COMP_CS_TYPE *cst = AVD_COMP_CS_TYPE_NULL;
   cst = (AVD_COMP_CS_TYPE *)ncs_patricia_tree_get(&cb->comp_cs_type_anchor, (uns8*)&indx);

   return cst;
}


/*****************************************************************************
 * Function: avd_comp_cs_type_struc_find_next
 *  
 * Purpose:  This function will find a AVD_COMP_CS_TYPE structure in the
 * tree whose index is the next from specified component name and CSI Type name.
 *
 * Input: cb   - the AVD control block
 *        indx - 
 *
 * Returns: The pointer to AVD_COMP_CS_TYPE structure found in the tree.
 *
 * NOTES:
 *
 * 
 **************************************************************************/

AVD_COMP_CS_TYPE * avd_comp_cs_type_struc_find_next(AVD_CL_CB *cb, AVD_COMP_CS_TYPE_INDX indx)
{
   AVD_COMP_CS_TYPE *cst = AVD_COMP_CS_TYPE_NULL;
   cst = (AVD_COMP_CS_TYPE *)ncs_patricia_tree_getnext(&cb->comp_cs_type_anchor, (uns8*)&indx);

   return cst; 
}

/*****************************************************************************
 * Function: avd_comp_cs_type_find_match
 *  
 * Purpose:  This function will verify the the component and CSI are related
 *  in the table.
 *
 * Input: cb   - the AVD control block
 *        csi -  The CSI whose type need to be matched with the components CSI types list
 *        comp - The component whose list need to be searched.
 *
 * Returns: NCSCC_RC_SUCCESS, NCS_RC_FAILURE.
 *
 * NOTES: This function will be used only while assigning new susi.
 *        In this funtion we will find a match only if the matching comp_cs_type
 *        row status is active.
 **************************************************************************/

uns32  avd_comp_cs_type_find_match(AVD_CL_CB *cb, AVD_CSI *csi,AVD_COMP *comp)
{
   AVD_COMP_CS_TYPE_INDX i_idx;
   AVD_COMP_CS_TYPE *cst = AVD_COMP_CS_TYPE_NULL;
   SaNameT  csi_type_net;

   m_NCS_MEMSET((uns8 *)&i_idx,'\0',sizeof(i_idx));
   i_idx.comp_name_net = comp->comp_info.name_net;
   csi_type_net = csi->csi_type;
   csi_type_net.length = m_NCS_OS_HTONS(csi_type_net.length);
   cst = ((AVD_COMP_CS_TYPE *)ncs_patricia_tree_getnext(&cb->comp_cs_type_anchor, (uns8*)&i_idx));
   while( (cst != AVD_COMP_CS_TYPE_NULL) && (m_CMP_NORDER_SANAMET(comp->comp_info.name_net,cst->indx.comp_name_net) == 0))
   {
     if ((m_CMP_NORDER_SANAMET(csi_type_net,cst->indx.csi_type_name_net) == 0) && (cst->row_status == NCS_ROW_ACTIVE))
        return NCSCC_RC_SUCCESS;

     cst = ((AVD_COMP_CS_TYPE *)ncs_patricia_tree_getnext(&cb->comp_cs_type_anchor, (uns8*)&(cst->indx)));
   }

   return NCSCC_RC_FAILURE;
}

/*****************************************************************************
 * Function: avd_comp_cs_type_struc_del
 *
 * Purpose:  This function will delete and free AVD_COMP_CS_TYPE structure from 
 * the tree.
 *
 * Input: cb  - the AVD control block
 *        cst - 
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE  
 *
 * NOTES:
 *
 * 
 **************************************************************************/

uns32 avd_comp_cs_type_struc_del(AVD_CL_CB *cb, AVD_COMP_CS_TYPE *cst)
{
   if (cst == AVD_COMP_CS_TYPE_NULL)
      return NCSCC_RC_FAILURE;

   if(ncs_patricia_tree_del(&cb->comp_cs_type_anchor,&cst->tree_node)
                      != NCSCC_RC_SUCCESS)
   {
      /* log error */
      return NCSCC_RC_FAILURE;
   }

   m_MMGR_FREE_AVD_COMP_CS_TYPE(cst);
   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: saamfcompcstypesupportedtableentry_get
 *
 * Purpose:  This function is one of the get processing routines for objects
 * in SA_AMF_COMP_C_S_TYPE_SUPPORTED_TABLE_ENTRY_ID table. This is the CompCSTypeSupported table.
 * The name of this function is generated by the MIBLIB tool. This function
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

uns32 saamfcompcstypesupportedtableentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg,
                                            NCSCONTEXT* data)
{
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_COMP_CS_TYPE_INDX   indx;
   uns16 comp_len;
   uns16 csi_len;
   uns32 i;
   AVD_COMP_CS_TYPE        *cst = AVD_COMP_CS_TYPE_NULL;

   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   m_NCS_MEMSET((char*)&indx, '\0', sizeof(AVD_COMP_CS_TYPE_INDX));

   /* Prepare the comp csi type database key from the instant ID */
   comp_len = (SaUint16T)arg->i_idx.i_inst_ids[0];
   indx.comp_name_net.length  = m_NCS_OS_HTONS(comp_len);

   for(i = 0; i < comp_len; i++)
   {
      indx.comp_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   csi_len = (SaUint16T)arg->i_idx.i_inst_ids[comp_len + 1];
   indx.csi_type_name_net.length = m_NCS_OS_HTONS(csi_len);

   for(i = 0; i < csi_len; i++)
   {
      indx.csi_type_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[comp_len + 1 + 1 + i]);
   }

   cst = avd_comp_cs_type_struc_find(avd_cb,indx);

   if (cst == AVD_COMP_CS_TYPE_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)cst;

   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: saamfcompcstypesupportedtableentry_extract
 *
 * Purpose:  This function is one of the get processing function for objects in
 * SA_AMF_COMP_C_S_TYPE_SUPPORTED_TABLE_ENTRY_ID table. This is the CompCSTypeSupported table.
 * The name of this function is generated by the MIBLIB tool. This function
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

uns32 saamfcompcstypesupportedtableentry_extract(NCSMIB_PARAM_VAL* param,
                              NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                              NCSCONTEXT buffer)
{
   AVD_COMP_CS_TYPE     *cst = (AVD_COMP_CS_TYPE *)data;

   if (cst == AVD_COMP_CS_TYPE_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }
   switch(param->i_param_id)
   {

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
 * Function: saamfcompcstypesupportedtableentry_set
 *
 * Purpose:  This function is the set processing for objects in
 * SA_AMF_COMP_C_S_TYPE_SUPPORTED_TABLE_ENTRY_ID table. This is the CompCSTypeSupported table.
 * The name of this function is generated by the MIBLIB tool. This function
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

uns32 saamfcompcstypesupportedtableentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                              NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{

   AVD_CL_CB              *avd_cb = (AVD_CL_CB *)cb;
   AVD_COMP_CS_TYPE_INDX  indx;
   uns16 comp_len;
   uns16 csi_len;
   uns32 i;
   AVD_COMP_CS_TYPE       *cst = AVD_COMP_CS_TYPE_NULL;
   AVD_COMP               *comp = AVD_COMP_NULL;

   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_INV_VAL;
   }

   m_NCS_MEMSET((char*)&indx, '\0', sizeof(AVD_COMP_CS_TYPE_INDX));

   /* Prepare the comp csi type database key from the instant ID */
   comp_len = (SaUint16T)arg->i_idx.i_inst_ids[0];
   indx.comp_name_net.length  = m_NCS_OS_HTONS(comp_len);

   for(i = 0; i < comp_len; i++)
   {
      indx.comp_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   csi_len = (SaUint16T)arg->i_idx.i_inst_ids[comp_len + 1];
   indx.csi_type_name_net.length = m_NCS_OS_HTONS(csi_len);

   for(i = 0; i < csi_len; i++)
   {
      indx.csi_type_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[comp_len + 1 + 1 + i]);
   }

   cst = avd_comp_cs_type_struc_find(avd_cb,indx);

   if (cst == AVD_COMP_CS_TYPE_NULL)
   {
      if((arg->req.info.set_req.i_param_val.i_param_id == saAmfCompCSTypeSupportedRowStatus_ID)
         && (arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_WAIT)
         && (arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_CREATE_AND_GO))
      {
         if((arg->req.info.set_req.i_param_val.info.i_int != NCS_ROW_ACTIVE)
           || (avd_cb->init_state >= AVD_CFG_DONE))
         {
            /* Invalid row status object */
            return NCSCC_RC_INV_VAL;
         }
      }

      if(test_flag == TRUE)
      {
         return NCSCC_RC_SUCCESS;
      }

      m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

      cst = avd_comp_cs_type_struc_crt(avd_cb,indx);

      m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);

      if(cst == AVD_COMP_CS_TYPE_NULL)
      {
         /* Invalid instance object */
         return NCSCC_RC_NO_INSTANCE;
      }

   }else  /* The record is already available */
   {
      if(arg->req.info.set_req.i_param_val.i_param_id == saAmfCompCSTypeSupportedRowStatus_ID)
      {
         /* This is a row status operation */
         if (arg->req.info.set_req.i_param_val.info.i_int == (uns32)cst->row_status)
         {
            /* row status object is same so nothing to be done. */
            return NCSCC_RC_SUCCESS;
         }
         if(test_flag == TRUE)
         {
            return NCSCC_RC_SUCCESS;
         }
         switch(arg->req.info.set_req.i_param_val.info.i_int)
         {
         case NCS_ROW_ACTIVE:
            
            cst->row_status = arg->req.info.set_req.i_param_val.info.i_int;
            m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(cb, cst, AVSV_CKPT_AVD_COMP_CS_TYPE_CONFIG);
            return NCSCC_RC_SUCCESS;
            break;

         case NCS_ROW_NOT_IN_SERVICE:
          
            if(cst->row_status == NCS_ROW_ACTIVE)
            { 
               comp =  avd_comp_struc_find(cb, indx.comp_name_net, FALSE);

               /* hope this comp does not have any assignment */
               if(comp && (comp->row_status == NCS_ROW_ACTIVE) && (comp->su)
                     && (comp->su->list_of_susi != AVD_SU_SI_REL_NULL))
                  return NCSCC_RC_INV_VAL;
                     
               m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, cst, AVSV_CKPT_AVD_COMP_CS_TYPE_CONFIG);
            }
            cst->row_status = arg->req.info.set_req.i_param_val.info.i_int;
            return NCSCC_RC_SUCCESS;
            break;

         case NCS_ROW_DESTROY:
            
            if(cst->row_status == NCS_ROW_ACTIVE)
            {
               comp =  avd_comp_struc_find(cb, indx.comp_name_net, FALSE);

               /* hope this comp does not have any assignment */
               if(comp && (comp->row_status == NCS_ROW_ACTIVE) && (comp->su)
                     && (comp->su->list_of_susi != AVD_SU_SI_REL_NULL))
                  return NCSCC_RC_INV_VAL;

               m_AVSV_SEND_CKPT_UPDT_ASYNC_RMV(cb, cst, AVSV_CKPT_AVD_COMP_CS_TYPE_CONFIG);
            }

            /* delete and free the structure */
            m_AVD_CB_LOCK(avd_cb, NCS_LOCK_WRITE);

            avd_comp_cs_type_struc_del(avd_cb, cst);

            m_AVD_CB_UNLOCK(avd_cb, NCS_LOCK_WRITE);
  
            return NCSCC_RC_SUCCESS;
            break;

         default:

            m_AVD_LOG_INVALID_VAL_ERROR(arg->req.info.set_req.i_param_val.info.i_int);
            /* Invalid row status object */
            return NCSCC_RC_INV_VAL;
            break;
         }
      }
   }

   if(test_flag == TRUE)
   {
      return NCSCC_RC_SUCCESS;
   }

   if(arg->req.info.set_req.i_param_val.i_param_id == saAmfCompCSTypeSupportedRowStatus_ID)
   {
      /* fill the row status value */
      if((arg->req.info.set_req.i_param_val.info.i_int == NCS_ROW_ACTIVE) ||
         (arg->req.info.set_req.i_param_val.info.i_int == NCS_ROW_CREATE_AND_GO) )
      {
         cst->row_status = NCS_ROW_ACTIVE;
         m_AVSV_SEND_CKPT_UPDT_ASYNC_ADD(cb, cst, AVSV_CKPT_AVD_COMP_CS_TYPE_CONFIG);
      }


   }  
   else
   {
       /* Invalid Object ID */
       return NCSCC_RC_INV_VAL;
   }

   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: saamfcompcstypesupportedtableentry_next
 *
 * Purpose:  This function is the next processing for objects in
 * SA_AMF_COMP_C_S_TYPE_SUPPORTED_TABLE_ENTRY_ID table. This is the CompCSTypeSupported table.
 * The name of this function is generated by the MIBLIB tool. This function
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

uns32 saamfcompcstypesupportedtableentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{  
   AVD_CL_CB     *avd_cb = (AVD_CL_CB *)cb;
   AVD_COMP_CS_TYPE_INDX  indx;
   uns16 comp_len;
   uns16 csi_len;
   uns32 i;    
   AVD_COMP_CS_TYPE     *cst = AVD_COMP_CS_TYPE_NULL;

   if (avd_cb->cluster_admin_state != NCS_ADMIN_STATE_UNLOCK)
   {
      /* Invalid operation */
      return NCSCC_RC_NO_INSTANCE;
   }

   m_NCS_MEMSET((char*)&indx, '\0', sizeof(AVD_COMP_CS_TYPE_INDX));

   if(arg->i_idx.i_inst_len != 0)
   {
      /* Prepare the comp csi type database key from the instant ID */
      comp_len = (SaUint16T)arg->i_idx.i_inst_ids[0];
      indx.comp_name_net.length  = m_NCS_OS_HTONS(comp_len);
   
      for(i = 0; i < comp_len; i++)
      {
         indx.comp_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
   
      if(arg->i_idx.i_inst_len  > comp_len + 1 )
      {
         csi_len = (SaUint16T)arg->i_idx.i_inst_ids[comp_len + 1];
         indx.csi_type_name_net.length = m_NCS_OS_HTONS(csi_len);
   
         for(i = 0; i < csi_len; i++)
         {
            indx.csi_type_name_net.value[i] = (uns8)(arg->i_idx.i_inst_ids[comp_len + 1 + 1 + i]);
         }
      }
   }


   cst = avd_comp_cs_type_struc_find_next(avd_cb, indx);

   if (cst == AVD_COMP_CS_TYPE_NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* Prepare the instant ID from the component and csi type name */

   comp_len = m_NCS_OS_NTOHS(cst->indx.comp_name_net.length);
   csi_len  = m_NCS_OS_NTOHS(cst->indx.csi_type_name_net.length);

   *next_inst_id_len = comp_len + 1 + csi_len + 1;

   next_inst_id[0] = comp_len;
   for(i = 0; i < comp_len; i++)
   {
      next_inst_id[i + 1] = (uns32)(cst->indx.comp_name_net.value[i]);
   }

   next_inst_id[comp_len + 1] = csi_len;
   for(i = 0; i < csi_len; i++)
   {
      next_inst_id[comp_len + 1 + i + 1] = (uns32)(cst->indx.csi_type_name_net.value[i]);
   }

   *data = (NCSCONTEXT)cst;

   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: saamfcompcstypesupportedtableentry_setrow
 *
 * Purpose:  This function is the setrow processing for objects in
 * SA_AMF_COMP_C_S_TYPE_SUPPORTED_TABLE_ENTRY_ID table. This is the CompCSTypeSupported table.
 * The name of this function is generated by the MIBLIB tool. This function
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


uns32 saamfcompcstypesupportedtableentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{  
   return NCSCC_RC_SUCCESS;
}


























