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

uns32 cpsv_map_from_instid(const uns32* i_inst_ids, uns32 i_inst_len, CPD_CKPT_MAP_INFO *map_info,CPD_CB *cb,int32 type);

static uns32 cpsv_maprec_getnext(CPD_CB *cb,SaNameT *ckpt_name,CPD_CKPT_MAP_INFO *map_info);




uns32 cpsv_map_from_instid(const uns32* i_inst_ids, uns32 i_inst_len, CPD_CKPT_MAP_INFO *map_info,CPD_CB *cb,int32 type)
{
   SaNameT ckpt_name;
   m_NCS_OS_MEMSET(&ckpt_name,'\0',sizeof(SaNameT));
   uns32 length,*start_ptr,counter=0;
/*   uns8 *name;    */
   CPD_CKPT_MAP_INFO *map=NULL;

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
    /*  *name=(uns8)*start_ptr;     */
       ckpt_name.value[counter] =(uns8)*start_ptr;
    /*   name++; */
       start_ptr++;
   }
   ckpt_name.length = length;

  
   
/*   cpd_ckpt_map_node_getnext(&cb->ckpt_map_tree,&ckpt_name,&map); */
   if( 1 == type)
   {
     /* give map_node_getnext instead of patricia tree getnext */
      cpd_ckpt_map_node_getnext(&cb->ckpt_map_tree,&ckpt_name,&map);  
/*      map = (CPD_CKPT_MAP_INFO *)ncs_patricia_tree_getnext(&cb->ckpt_map_tree,(uns8*)&ckpt_name.value);  */
      if(map != NULL)
      {
         *map_info = *map;
      }
      else
      {
         return NCSCC_RC_NO_INSTANCE;
      }
   }
   if(2 == type)
   {
      cpd_ckpt_map_node_get(&cb->ckpt_map_tree,&ckpt_name,&map);
      if(map != NULL)
      {
         *map_info = *map;
      }
      else
      {
          return NCSCC_RC_NO_INSTANCE;
      }
    }
   return NCSCC_RC_SUCCESS;
}  


/****************************************************************************************
 *   
 * Name         :  sackptcheckpointentry_get
 *
 * Description  : This is used to get the Checkpoint Node data
 ***************************************************************************************/

uns32 sackptcheckpointentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
   CPD_CB    *cpd_cb = NULL;
   uns32     rc = NCSCC_RC_SUCCESS;
   CPD_CKPT_MAP_INFO map_info;
   CPD_CKPT_INFO_NODE *ckpt_node=NULL; 

   m_NCS_OS_MEMSET(&map_info,'\0',sizeof(CPD_CKPT_MAP_INFO)); 
   
   /* Get the CB pointer from the CB handle */
   cpd_cb = (CPD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CPD, (uns32)arg->i_mib_key); 
   
   if (cpd_cb == NULL)
      return NCSCC_RC_NO_INSTANCE;

    rc = cpsv_map_from_instid(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len, &map_info,cpd_cb,2);
    if( rc != NCSCC_RC_SUCCESS)
    {
      /* Log the error */
       ncshm_give_hdl(cpd_cb->cpd_hdl);
       return rc;
    } 

    cpd_ckpt_node_get(&cpd_cb->ckpt_tree,&map_info.ckpt_id,&ckpt_node);
    if (rc != NCSCC_RC_SUCCESS)
    {
       m_LOG_CPD_FCL(CPD_CKPT_INFO_NODE_GET_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,map_info.ckpt_id,__FILE__,__LINE__);
       ncshm_give_hdl(cpd_cb->cpd_hdl);
       return rc;
    }
  
    *data = (NCSCONTEXT)ckpt_node;
    return NCSCC_RC_SUCCESS;
} 


uns32 cpsv_maprec_getnext(CPD_CB *cb,SaNameT *ckpt_name,CPD_CKPT_MAP_INFO *map_info)
{
   CPD_CKPT_MAP_INFO *map;
   if (ckpt_name)
   {
       cpd_ckpt_map_node_getnext(&cb->ckpt_map_tree,ckpt_name,&map);
   /*    ckpt_name->length = m_NCS_OS_NTOHS(ckpt_name->length);  */
 /*       map = (CPD_CKPT_MAP_INFO *)ncs_patricia_tree_getnext(&cb->ckpt_map_tree,(uns8*)&ckpt_name); */
       if(map != NULL)
       {
          *map_info = *map;
       }
   }
   else
   {
       /* Get the first node */
      cpd_ckpt_map_node_getnext(&cb->ckpt_map_tree,NULL,&map); 
  /*    map = (CPD_CKPT_MAP_INFO *)ncs_patricia_tree_getnext(&cb->ckpt_map_tree,(uns8*)NULL); */
      if(map != NULL)
      {
         *map_info = *map;
      }
      else
      {
        
         return NCSCC_RC_NO_INSTANCE;
      }
   }  

   if ( map_info == NULL)
      return NCSCC_RC_NO_INSTANCE;
 
   return NCSCC_RC_SUCCESS;
}


/*********************************************************************
 * Name        : sackptcheckpointentry_next
 *
 * Description : To get the next entry 
 *
 ********************************************************************/

uns32 sackptcheckpointentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data, uns32* next_inst_id,
                                 uns32 *next_inst_id_len)
{
   CPD_CB    *cpd_cb = NULL;
   uns32     rc = NCSCC_RC_SUCCESS,counter;
   CPD_CKPT_MAP_INFO map_info;
   CPD_CKPT_INFO_NODE *ckpt_node=NULL;
   uns16 length=0;

   m_NCS_OS_MEMSET(&map_info,'\0',sizeof(CPD_CKPT_MAP_INFO));
   /* Get the CB pointer from the CB handle */
   cpd_cb = (CPD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CPD, (uns32)arg->i_mib_key);

   if (cpd_cb == NULL)
      return NCSCC_RC_NO_INSTANCE;

   if ( arg->i_idx.i_inst_len == 0)
   {
     /* Get the first node */
      rc = cpsv_maprec_getnext(cpd_cb,(SaNameT*)NULL,&map_info);
      if(rc == NCSCC_RC_NO_INSTANCE)
        return rc;
   }
   else
   {
      rc = cpsv_map_from_instid(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len, &map_info,cpd_cb,1);
      if( rc != NCSCC_RC_SUCCESS)
      {
         /* Log the error */
         ncshm_give_hdl(cpd_cb->cpd_hdl);
         return rc;
      }
   }
   ckpt_node = (CPD_CKPT_INFO_NODE *)ncs_patricia_tree_get(&cpd_cb->ckpt_tree,(uns8*)&map_info.ckpt_id); 
    
   if (ckpt_node == NULL)
   {
      m_LOG_CPD_FCL(CPD_CKPT_INFO_NODE_GET_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,map_info.ckpt_id,__FILE__,__LINE__);
      rc = NCSCC_RC_FAILURE;
      return rc;
   }
  
   length = m_NCS_OS_NTOHS(map_info.ckpt_name.length);  

   next_inst_id[0] = length;
   for(counter=0;counter< length;counter++)
   {
      *(next_inst_id+counter+1) = (uns32)map_info.ckpt_name.value[counter];
   }
   *next_inst_id_len = length+1;
   
 /*  *data =(NCSCONTEXT)ckpt_node; */
   *data =(NCSCONTEXT)ckpt_node;

   ncshm_give_hdl(cpd_cb->cpd_hdl);
   return rc;
}


/**********************************************************************
 * Name        : sackptcheckpointentry_extract
 *
 * Description :
 *
 *********************************************************************/

uns32 sackptcheckpointentry_extract(NCSMIB_PARAM_VAL *param, NCSMIB_VAR_INFO *var_info, NCSCONTEXT data, NCSCONTEXT buffer)
{

   uns32 rc=NCSCC_RC_SUCCESS;
/*   uns64 *ckpt_size = NULL; */
   uns64 ckpt_size;
   CPD_CKPT_INFO_NODE *ckpt_node=NULL;
   CPD_CB *cpd_cb;
   CPSV_EVT send_evt;          
   CPSV_EVT *out_evt=NULL;
   SaTimeT creation_time=0,retention_time=0;
                                                                
   m_CPD_RETRIEVE_CB(cpd_cb);  /* finally give up the handle */
   if(cpd_cb == NULL)
   {
      m_LOG_CPD_CL(CPD_CB_HDL_TAKE_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__);
      return (NCSCC_RC_FAILURE);
   }
                                                                                                                          
   ckpt_node = (CPD_CKPT_INFO_NODE *)data; 

   switch(var_info->param_id)
   {
      case saCkptCheckpointSize_ID:
        /*  ckpt_size = (uns64 *)m_MMGR_ALLOC_CPD_DEFAULT(sizeof(uns64));
          if(ckpt_size==NULL)
          {
             return NCSCC_RC_FAILURE;
          }
          m_NCS_OS_MEMSET(ckpt_size,0,sizeof(uns64));*/
          ckpt_size = (uns64)ckpt_node->attributes.checkpointSize;
          m_CPSV_UNS64_TO_PARAM(param,buffer,ckpt_size);             
          break;

     case saCkptCheckpointUsedSize_ID:
          m_NCS_OS_MEMSET(&send_evt,0,sizeof(CPSV_EVT));
   
          send_evt.type = CPSV_EVT_TYPE_CPND;
          send_evt.info.cpnd.type = CPND_EVT_D2ND_CKPT_SIZE;
          send_evt.info.cpnd.info.ckpt_mem_size.ckpt_id = ckpt_node->ckpt_id;
             

          if(ckpt_node->active_dest == 0)
          {
             ckpt_node->ckpt_used_size = 0;
             ckpt_size = (uns64)ckpt_node->ckpt_used_size;
             m_CPSV_UNS64_TO_PARAM(param,buffer,ckpt_size);
          }
          else
          {
             rc = cpd_mds_msg_sync_send(cpd_cb,NCSMDS_SVC_ID_CPND,ckpt_node->active_dest ,&send_evt,&out_evt,CPSV_WAIT_TIME);
             if (rc != NCSCC_RC_SUCCESS) {
               m_LOG_CPD_FCL(CPD_MDS_SEND_FAIL,CPD_FC_HDLN,NCSFL_SEV_ERROR,ckpt_node->active_dest,__FILE__,__LINE__);               
             }
          
             if(out_evt == NULL)
             {
                /* LOG */
                return NCSCC_RC_NO_INSTANCE;
             }
             if(out_evt->info.cpd.info.ckpt_mem_used.error != SA_AIS_OK)
             {
                m_MMGR_FREE_CPSV_EVT(out_evt, NCS_SERVICE_ID_CPD);
                return NCSCC_RC_NO_INSTANCE;
             }
             ckpt_node->ckpt_used_size = out_evt->info.cpd.info.ckpt_mem_used.ckpt_used_size;
             /*  ckpt_size = (uns64 *)m_MMGR_ALLOC_CPD_DEFAULT(sizeof(uns64));
             if(ckpt_size==NULL)
             {
               return NCSCC_RC_FAILURE;
             }
             m_NCS_OS_MEMSET(ckpt_size,0,sizeof(uns64)); */
             ckpt_size = (uns64)ckpt_node->ckpt_used_size;
             m_CPSV_UNS64_TO_PARAM(param,buffer,ckpt_size);
             m_MMGR_FREE_CPSV_EVT(out_evt, NCS_SERVICE_ID_CPD);
          }
             break;
        

     case saCkptCheckpointMaxSectionSize_ID:
        /*  ckpt_size = (uns64 *)m_MMGR_ALLOC_CPD_DEFAULT(sizeof(uns64));
          if(ckpt_size==NULL)
          {
             return NCSCC_RC_FAILURE;
          }
          m_NCS_OS_MEMSET(ckpt_size,0,sizeof(uns64)); */
          ckpt_size = (uns64)ckpt_node->attributes.maxSectionSize;
          m_CPSV_UNS64_TO_PARAM(param,buffer,ckpt_size);
          break;
   
     case saCkptCheckpointMaxSectionIdSize_ID:
        /*  ckpt_size = (uns64 *)m_MMGR_ALLOC_CPD_DEFAULT(sizeof(uns64));
          if(ckpt_size==NULL)
          {
             return NCSCC_RC_FAILURE;
          }
          m_NCS_OS_MEMSET(ckpt_size,0,sizeof(uns64));*/
          ckpt_size = (uns64)ckpt_node->attributes.maxSectionIdSize;
          m_CPSV_UNS64_TO_PARAM(param,buffer,ckpt_size);
          break;
    
     case saCkptCheckpointIsCollocated_ID:
          param->i_fmat_id  = NCSMIB_FMAT_BOOL;
          param->i_length   = 1;
          param->info.i_int = (ckpt_node->attributes.creationFlags & 0x8 ) ? 1:2;
          break;       
   
     case saCkptCheckpointUpdateOption_ID:
         param->i_fmat_id  = NCSMIB_FMAT_INT;
         param->i_length   = 1;
         if(ckpt_node->attributes.creationFlags & 0x1)
            param->info.i_int = 1;
         if(ckpt_node->attributes.creationFlags & 0x2)
            param->info.i_int = 2;
         if(ckpt_node->attributes.creationFlags & 0x4)   
            param->info.i_int = 3;
         break; 

      case saCkptCheckpointCreateTime_ID:
         creation_time = ckpt_node->create_time;
         creation_time = creation_time * 1000000000;
         m_CPSV_UNS64_TO_PARAM(param,buffer,creation_time);
         break; 

       case saCkptCheckpointRetDuration_ID:
         retention_time =  ckpt_node->ret_time;
         m_CPSV_UNS64_TO_PARAM(param,buffer,retention_time);
         break;

       case saCkptCheckpointNumSections_ID:
         m_NCS_OS_MEMSET(&send_evt,0,sizeof(CPSV_EVT));

         send_evt.type = CPSV_EVT_TYPE_CPND;
         send_evt.info.cpnd.type = CPND_EVT_D2ND_CKPT_NUM_SECTIONS;
         send_evt.info.cpnd.info.ckpt_sections.ckpt_id = ckpt_node->ckpt_id;

         if(ckpt_node->active_dest == 0)
         {
             ckpt_node->num_sections = 0;
             m_LOG_CPD_FCL(CPD_CPND_NODE_DOES_NOT_EXIST,CPD_FC_HDLN,NCSFL_SEV_ERROR,ckpt_node->active_dest,__FILE__,__LINE__);
         }
         else
         {
             rc = cpd_mds_msg_sync_send(cpd_cb,NCSMDS_SVC_ID_CPND,ckpt_node->active_dest ,&send_evt,&out_evt,CPSV_WAIT_TIME);
             if(rc != NCSCC_RC_SUCCESS) 
             {
               m_LOG_CPD_FCL(CPD_MDS_SEND_FAIL,CPD_FC_HDLN,NCSFL_SEV_ERROR,ckpt_node->active_dest,__FILE__,__LINE__);
             }

             if(out_evt == NULL)
             {
               m_LOG_CPD_FCL(CPD_MDS_SEND_FAIL,CPD_FC_HDLN,NCSFL_SEV_ERROR,ckpt_node->active_dest,__FILE__,__LINE__);
             }
             else
             {
                 if(out_evt->info.cpd.info.ckpt_created_sections.error == SA_AIS_ERR_NOT_EXIST)
                 {
                   m_LOG_CPD_FCL(CPD_CPND_NODE_DOES_NOT_EXIST,CPD_FC_HDLN,NCSFL_SEV_ERROR,ckpt_node->active_dest,__FILE__,__LINE__);
                 }
 
                 ckpt_node->num_sections = out_evt->info.cpd.info.ckpt_created_sections.ckpt_num_sections;
                 m_MMGR_FREE_CPSV_EVT(out_evt, NCS_SERVICE_ID_CPD);
             }
         }
         rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);
             break;

      default:
           rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);
   } 
    return rc;
}


/**********************************************************************
 * Name   : sackptcheckpointentry_set
 *
 * Description :
 *
 *********************************************************************/

uns32  sackptcheckpointentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,NCSMIB_VAR_INFO* var_info,NCS_BOOL test_flag)
{
   return NCSCC_RC_FAILURE;

}


/**********************************************************************
 * Name        : sackptcheckpointentry_setrow
 *
 * Description : 
 *
 **********************************************************************/

uns32 sackptcheckpointentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                                   NCSMIB_SETROW_PARAM_VAL* params,
                                   struct ncsmib_obj_info* obj_info,
                                   NCS_BOOL testrow_flag)
{
    return NCSCC_RC_FAILURE;
}
