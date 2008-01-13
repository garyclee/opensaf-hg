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


#include "cpd.h"

static uns32 cpd_get_ckpt_and_node_name(const uns32* i_inst_ids, uns32 i_inst_len,SaNameT *ckpt_name,SaNameT *node_name);


/**********************************************************************
 * Name  : cpd_get_the_node_name
 *
 * Description : get the node_name from the mib key (ckpt_name + node_name )
 *
 *  i_inst_ids = instance_id's , if key is name then  [ length of name + name string ]

***********************************************************************/

uns32 cpd_get_ckpt_and_node_name(const uns32* i_inst_ids, uns32 i_inst_len,SaNameT *ckpt_name,SaNameT *node_name)
{
   uns32 length=0,*start_ptr,counter=0,node_name_len=0;
 /*  uns8 *name,*n_name; */
   
   start_ptr = (uns32 *)i_inst_ids;
   length = *start_ptr;
   start_ptr++;
/*   name = (uns8 *)m_MMGR_ALLOC_CPD_DEFAULT(sizeof(uns8)*(length+1));
   if(name==NULL)
   {
     return NCSCC_RC_FAILURE;
   }
   
   m_NCS_OS_MEMSET(name,'\0',length+1); */

   for(;counter<length;counter++)
   {
      ckpt_name->value[counter] = (uns8)*start_ptr;
      /*      *name=(uns8)*start_ptr;
       ckpt_name->value[counter] = *name;
       name++; */
       start_ptr++;
   }
   ckpt_name->length = length;

   node_name_len = *start_ptr;
   start_ptr++;
  
 /*  n_name = (uns8 *)m_MMGR_ALLOC_CPD_DEFAULT(sizeof(uns8)*(length+1));
   if(n_name==NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(n_name,'\0',node_name_len+1);*/
   counter=0;
   for(;counter < node_name_len;counter++)
   {
      node_name->value[counter] = (uns8)*start_ptr;
     /*      *n_name = (uns8 *)*start_ptr; 
      node_name->value[counter] = *n_name;
      n_name++; */
      start_ptr++;
   }
   node_name->length = node_name_len;
  
   return NCSCC_RC_SUCCESS;
}



/***********************************************************************
 * Name :  sackptnodereplicalocentry_get
 *
 * Description : 
 *
  ALGO : 1. get the map_info from cpsv_map_from_instid()

***********************************************************************/
uns32 sackptnodereplicalocentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
   CPD_CB *cpd_cb;
   CPD_REP_KEY_INFO     key_info;   
   uns32  rc = NCSCC_RC_SUCCESS;
   SaNameT       ckpt_name,node_name;
   CPD_CKPT_REPLOC_INFO *rep_info=NULL;

   m_NCS_OS_MEMSET(&node_name,'\0',sizeof(SaNameT));
   m_NCS_OS_MEMSET(&ckpt_name,0,sizeof(SaNameT));
   m_NCS_OS_MEMSET(&key_info,0,sizeof(CPD_REP_KEY_INFO));

   /* STEP 1 : get the cpd_cb */
   cpd_cb = (CPD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CPD, (uns32)arg->i_mib_key);
   
   if (cpd_cb == NULL)
      return NCSCC_RC_NO_INSTANCE; 


   rc = cpd_get_ckpt_and_node_name(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len,&ckpt_name,&node_name);
   if(rc != NCSCC_RC_SUCCESS)
   {
      ncshm_give_hdl(cpd_cb->cpd_hdl);
      return rc;
   }

   key_info.ckpt_name = ckpt_name;
   key_info.node_name = node_name;

   cpd_ckpt_reploc_get(&cpd_cb->ckpt_reploc_tree,&key_info,&rep_info); 
   if(rep_info)
   { 
      *data = (NCSCONTEXT)rep_info;
   }    
   else
   {
      ncshm_give_hdl(cpd_cb->cpd_hdl);
      return NCSCC_RC_NO_INSTANCE;
   }
   return rc;
}


/******************************************************************************
 * Name  : sackptnodereplicalocentry_extract
 *
 * Description :
 *
 *****************************************************************************/
uns32 sackptnodereplicalocentry_extract(NCSMIB_PARAM_VAL *param, NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer)
{
   uns32 rc=NCSCC_RC_SUCCESS;
   CPD_CKPT_REPLOC_INFO *rep_info = NULL;

   rep_info = (CPD_CKPT_REPLOC_INFO *)data; 
   if(rep_info == NULL)
      return NCSCC_RC_NO_INSTANCE;   
 
                                                
   switch(var_info->param_id)
   {
                                                                 
      case saCkptCheckpointNameLoc_ID:
         param->i_fmat_id = NCSMIB_FMAT_OCT;
         param->i_length  = rep_info->rep_key.ckpt_name.length;
         m_NCS_MEMCPY((uns8 *)buffer,rep_info->rep_key.ckpt_name.value,param->i_length);
         param->info.i_oct     = (uns8*)buffer;
         break;

      case saCkptNodeNameLoc_ID:
         param->i_fmat_id = NCSMIB_FMAT_OCT;
         param->i_length = rep_info->rep_key.node_name.length;
         m_NCS_MEMCPY((uns8 *)buffer,rep_info->rep_key.node_name.value,param->i_length);
         param->info.i_oct     = (uns8*)buffer;
         break;  
     
      default:
         rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);
   }
                                                                                                                             
   return rc;
}                                                                                   
 

/*****************************************************************************
 * Name  :  sackptnodereplicalocentry_next
 *
 * Description :
 *
 * 1 ckpt created on 2 nodes SCXB & PB respectively. nref will be  pb -> scxb 
******************************************************************************/
uns32 sackptnodereplicalocentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data,
                                     uns32* next_inst_id,uns32 *next_inst_id_len)
{
   CPD_CKPT_REPLOC_INFO *rep_info=NULL;
   CPD_REP_KEY_INFO     key_info;
   uns32 counter,next_counter,total_length=0,rc=NCSCC_RC_SUCCESS;
   uns32 ckpt_name_length=0,node_name_length=0;
   SaNameT  ckpt_name,node_name;


   m_NCS_OS_MEMSET(&key_info,0,sizeof(CPD_REP_KEY_INFO)); 
   m_NCS_OS_MEMSET(&ckpt_name,0,sizeof(SaNameT));
   m_NCS_OS_MEMSET(&node_name,0,sizeof(SaNameT));

   CPD_CB *cpd_cb = NULL;
   /* Get the CB pointer from the CB handle */
   cpd_cb = (CPD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CPD, (uns32)arg->i_mib_key);
   if (cpd_cb == NULL)
      return NCSCC_RC_NO_INSTANCE;
 
   if(arg->i_idx.i_inst_len == 0)
   {
     /* 1st cp_node will be obtained */
      cpd_ckpt_reploc_getnext(&cpd_cb->ckpt_reploc_tree, NULL,&rep_info);
      if(rep_info)
      {
         key_info = rep_info->rep_key;
         *data = (NCSCONTEXT)rep_info;
      }
      else
      {
         ncshm_give_hdl(cpd_cb->cpd_hdl); 
         return NCSCC_RC_NO_INSTANCE; 
      }
   }
   else
   {
      rc = cpd_get_ckpt_and_node_name(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len,&ckpt_name,&node_name);
      if(rc != NCSCC_RC_SUCCESS)
         return NCSCC_RC_FAILURE;      
      
      key_info.ckpt_name = ckpt_name;
      key_info.node_name = node_name;
      cpd_ckpt_reploc_getnext(&cpd_cb->ckpt_reploc_tree,&key_info,&rep_info);      
      if(rep_info)
      {
         *data = (NCSCONTEXT)rep_info;
      }
      else
      {
         ncshm_give_hdl(cpd_cb->cpd_hdl);
         return NCSCC_RC_NO_INSTANCE; 
      }
   }   
      
   /* converting the ckpt_name length from NTOHS */
   ckpt_name_length = m_NCS_OS_NTOHS(rep_info->rep_key.ckpt_name.length);

   next_inst_id[0] = ckpt_name_length;
   total_length = total_length + next_inst_id[0];
   for(counter=0;counter < ckpt_name_length;counter++)
   {
      next_inst_id[counter+1] = (uns32)rep_info->rep_key.ckpt_name.value[counter];
   }
   node_name_length = m_NCS_OS_NTOHS(rep_info->rep_key.node_name.length); 
   next_inst_id[counter+1] = node_name_length;
   total_length = total_length + node_name_length;
   counter++;
   for(next_counter=0;next_counter < node_name_length;next_counter++)
   {
      next_inst_id[counter+1] = (uns32)rep_info->rep_key.node_name.value[next_counter];
      counter++;
   }
   *next_inst_id_len = total_length+2;


   return NCSCC_RC_SUCCESS;
}

/***************************************************************************
 *  Name  :  sackptnodereplicalocentry_setrow                                        
 *  
 *  Description :
***************************************************************************/
uns32 sackptnodereplicalocentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                                   NCSMIB_SETROW_PARAM_VAL* params,
                                   struct ncsmib_obj_info* obj_info,
                                   NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}                                          



/**************************************************************************
 * Name  : sackptnodereplicalocentry_set
 *
 * Description :
 *
 * Notes : As all the objects are read only there is no need of Set
**************************************************************************/
uns32 sackptnodereplicalocentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,NCSMIB_VAR_INFO* var_info,NCS_BOOL test_flag)
{
   return NCSCC_RC_FAILURE;
                                                                                
}




