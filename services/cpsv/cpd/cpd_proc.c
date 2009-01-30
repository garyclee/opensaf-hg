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
  FILE NAME: cpd_proc.c

  DESCRIPTION: CPD Event handling routines

  FUNCTIONS INCLUDED in this module:
  cpd_proc_non_colloc_ckpt_create .........
******************************************************************************/

#include "cpd.h"

/****************************************************************************
 * Name          : cpd_noncolloc_ckpt_rep_create
 *
 * Description   : This routine will run the policy to create the non-collacated
 *                 ckpt replicas.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 cpd_noncolloc_ckpt_rep_create(CPD_CB *cb, 
                                    MDS_DEST *cpnd_dest,
                                    CPD_CKPT_INFO_NODE    *ckpt_node,
                                    CPD_CKPT_MAP_INFO     *map_info)
{
   CPSV_EVT              send_evt;
   uns32                 rc;
   CPSV_D2ND_CKPT_INFO   *d2nd_info = NULL;
   SaNameT  ckpt_name;

   m_NCS_OS_MEMSET(&ckpt_name,0,sizeof(SaNameT));

   /* Update the database with new replica */
   rc = cpd_ckpt_db_entry_update(cb, cpnd_dest,  NULL, &ckpt_node, &map_info);
   
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_CPD_CL(CPD_DB_UPD_FAILED,CPD_FC_DB,NCSFL_SEV_ERROR,__FILE__,__LINE__);
      return rc;
   }
  
   cpd_a2s_ckpt_dest_add(cb,ckpt_node,cpnd_dest);
 
   /* Send the Replica create info to CPND */
   m_NCS_OS_MEMSET(&send_evt, 0, sizeof(CPSV_EVT));
   send_evt.type = CPSV_EVT_TYPE_CPND;
   send_evt.info.cpnd.type = CPND_EVT_D2ND_CKPT_CREATE;

   ckpt_name = map_info->ckpt_name;
   ckpt_name.length = m_NCS_OS_NTOHS(ckpt_name.length);
   send_evt.info.cpnd.info.ckpt_create.ckpt_name = ckpt_name;

   d2nd_info = &send_evt.info.cpnd.info.ckpt_create.ckpt_info;
   
   d2nd_info->error = SA_AIS_OK;
   d2nd_info->ckpt_id = ckpt_node->ckpt_id;
   d2nd_info->is_active_exists = ckpt_node->is_active_exists;
   if(d2nd_info->is_active_exists)
       d2nd_info->active_dest = ckpt_node->active_dest;
   d2nd_info->attributes = map_info->attributes;
   d2nd_info->ckpt_rep_create = TRUE;
   
   if(ckpt_node->dest_cnt)
   {
      CPD_NODE_REF_INFO *node_list = ckpt_node->node_list;
      uns32 i=0;
      
      d2nd_info->dest_cnt = ckpt_node->dest_cnt;
      d2nd_info->dest_list = 
        m_MMGR_ALLOC_CPSV_CPND_DEST_INFO(ckpt_node->dest_cnt);
      
      if(d2nd_info->dest_list == NULL)
      {
        rc = NCSCC_RC_OUT_OF_MEM;
        return rc;
      }
      else
      {
         m_NCS_OS_MEMSET(d2nd_info->dest_list, 0, 
                          (sizeof(CPSV_CPND_DEST_INFO)*ckpt_node->dest_cnt));
         
         for(i=0; i< ckpt_node->dest_cnt; i++)
         {
            d2nd_info->dest_list[i].dest = node_list->dest;
            node_list = node_list->next;
         }
      }
   }
   
   rc = cpd_mds_msg_send(cb, NCSMDS_SVC_ID_CPND, *cpnd_dest, &send_evt);
  
   if(d2nd_info->dest_list)
      m_MMGR_FREE_CPSV_CPND_DEST_INFO(d2nd_info->dest_list); 
   /* Send the replica add info to other CPNDs */
   m_NCS_OS_MEMSET(&send_evt, 0, sizeof(CPSV_EVT));
   send_evt.type = CPSV_EVT_TYPE_CPND;
   send_evt.info.cpnd.type = CPND_EVT_D2ND_CKPT_REP_ADD;
   send_evt.info.cpnd.info.ckpt_add.ckpt_id = ckpt_node->ckpt_id;
   send_evt.info.cpnd.info.ckpt_add.mds_dest = *cpnd_dest;
   send_evt.info.cpnd.info.ckpt_add.is_cpnd_restart = FALSE;
      
   rc = cpd_mds_bcast_send(cb, &send_evt, NCSMDS_SVC_ID_CPND);
   
   m_LOG_CPD_FFCL(CPD_REP_ADD_SUCCESS,CPD_FC_DB,NCSFL_SEV_INFO,ckpt_node->ckpt_id, \
                      *cpnd_dest,__FILE__,__LINE__);
   return rc;
}


/****************************************************************************
 * Name          : cpd_ckpt_db_entry_update
 *
 * Description   : This routine will update the CPD database when a new ckpt
 *                 got created.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 cpd_ckpt_db_entry_update(CPD_CB *cb, 
                               MDS_DEST *cpnd_dest,
                               CPSV_ND2D_CKPT_CREATE *ckpt_create,
                               CPD_CKPT_INFO_NODE **o_ckpt_node, 
                               CPD_CKPT_MAP_INFO **io_map_info)
{
   CPD_CPND_INFO_NODE   *node_info=NULL;
   CPD_CKPT_INFO_NODE   *ckpt_node=NULL;
   CPD_CKPT_MAP_INFO    *map_info=NULL;
   CPD_CKPT_REPLOC_INFO *reploc_info = NULL; 
   CPD_NODE_REF_INFO    *nref_info = NULL;
   CPD_CKPT_REF_INFO    *cref_info = NULL;
   uns32                proc_rc = NCSCC_RC_SUCCESS;
   SaCkptCheckpointHandleT ckpt_id=0;
   NCS_BOOL             add_flag = TRUE;
   SaClmClusterNodeT    cluster_node;   
   NODE_ID              key;
   SaClmNodeIdT         node_id;  
   SaNameT              ckpt_name;
   CPD_REP_KEY_INFO     key_info;

   m_NCS_OS_MEMSET(&ckpt_name,0,sizeof(SaNameT));
   m_NCS_OS_MEMSET(&cluster_node,0,sizeof(SaClmClusterNodeT));
   m_NCS_OS_MEMSET(&key_info,0,sizeof(CPD_REP_KEY_INFO));
   /* Upfront allocate all the memory required for this ckpt */
   if(*io_map_info == NULL)
   {
      map_info = m_MMGR_ALLOC_CPD_CKPT_MAP_INFO;
      if(map_info == NULL)
      {
         m_LOG_CPD_CL(CPD_DB_ADD_FAILED,CPD_FC_DB,NCSFL_SEV_ERROR,__FILE__,__LINE__);
         goto free_mem;
      }
      ckpt_node = m_MMGR_ALLOC_CPD_CKPT_INFO_NODE;
      if(ckpt_node == NULL)
      {
         m_LOG_CPD_CL(CPD_DB_ADD_FAILED,CPD_FC_DB,NCSFL_SEV_ERROR,__FILE__,__LINE__);
         goto free_mem;
      }
   }
   else 
   {
     map_info = *io_map_info;
   }
   
   nref_info = m_MMGR_ALLOC_CPD_NODE_REF_INFO;
   cref_info = m_MMGR_ALLOC_CPD_CKPT_REF_INFO;
   
   if(!(nref_info && cref_info))
   {
      m_LOG_CPD_CL(CPD_DB_ADD_FAILED,CPD_FC_DB,NCSFL_SEV_ERROR,__FILE__,__LINE__);
      proc_rc = NCSCC_RC_OUT_OF_MEM;
      goto free_mem;  
   }
   
   /* Get the CPD_CPND_INFO_NODE (CPND from where this ckpt is created) */
   proc_rc = cpd_cpnd_info_node_find_add(&cb->cpnd_tree, 
                                             cpnd_dest, &node_info, &add_flag);
   if(!node_info) 
   {
      m_LOG_CPD_FCL(CPD_DB_ADD_FAILED,CPD_FC_DB,NCSFL_SEV_ERROR,*cpnd_dest,__FILE__,__LINE__);
      proc_rc = NCSCC_RC_OUT_OF_MEM;
      goto free_mem;
   }

   key = m_NCS_NODE_ID_FROM_MDS_DEST(*cpnd_dest);
   node_id = key;
/*   Processing for the Node name , with CLM  */

   if (saClmClusterNodeGet(cb->clm_hdl,node_id,NCS_SAF_ACCEPT_TIME,&cluster_node)!= SA_AIS_OK) 
   {
      proc_rc = NCSCC_RC_FAILURE;
      m_LOG_CPD_LCL(CPD_DB_ADD_FAILED,CPD_FC_DB,NCSFL_SEV_ERROR,node_id,__FILE__,__LINE__);
      m_NCS_CONS_PRINTF("CLUSTER NODE GET FAILED\n");
      goto free_mem;
   }
 
   node_info->node_name = cluster_node.nodeName;

   key_info.node_name = cluster_node.nodeName;
   if(ckpt_create != NULL)
   {
     key_info.ckpt_name = ckpt_create->ckpt_name;
   }
   else
   {
     ckpt_name = (*io_map_info)->ckpt_name;
     ckpt_name.length = m_NCS_OS_NTOHS(ckpt_name.length);
     key_info.ckpt_name = ckpt_name;
   }

   cpd_ckpt_reploc_get(&cb->ckpt_reploc_tree,&key_info,&reploc_info);
   if(reploc_info == NULL)
   {
      reploc_info = m_MMGR_ALLOC_CPD_CKPT_REPLOC_INFO;
      if(reploc_info == NULL)
         goto free_mem;

      m_NCS_OS_MEMSET(reploc_info,0,sizeof(CPD_CKPT_REPLOC_INFO));

      reploc_info->rep_key.node_name = cluster_node.nodeName;

      if(ckpt_create != NULL)
      {
         reploc_info->rep_key.ckpt_name = ckpt_create->ckpt_name;
    
         if(!m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(&ckpt_create->attributes)) 
            reploc_info->rep_type  = REP_NONCOLL;
         else
         {
            if((ckpt_create->attributes.creationFlags & SA_CKPT_WR_ALL_REPLICAS)&&(m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(&ckpt_create->attributes)))
               reploc_info->rep_type = REP_SYNCUPD;
            
            if((ckpt_create->attributes.creationFlags & SA_CKPT_WR_ACTIVE_REPLICA)&&(m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(&ckpt_create->attributes))) 
            reploc_info->rep_type = REP_NOTACTIVE;
            if((ckpt_create->attributes.creationFlags & SA_CKPT_WR_ACTIVE_REPLICA_WEAK) && (m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(&ckpt_create->attributes)))
            reploc_info->rep_type = REP_NOTACTIVE;
         } 
      }
      else
      { 
         reploc_info->rep_key.ckpt_name = ckpt_name;

         if(!m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(&(*io_map_info)->attributes))
            reploc_info->rep_type = REP_NONCOLL;
         else
         {
            if(((*io_map_info)->attributes.creationFlags & SA_CKPT_WR_ALL_REPLICAS)&&(m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(&(*io_map_info)->attributes)))
              reploc_info->rep_type = REP_SYNCUPD;
            if(((*io_map_info)->attributes.creationFlags & SA_CKPT_WR_ACTIVE_REPLICA)&&(m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(&(*io_map_info)->attributes)))
              reploc_info->rep_type = REP_NOTACTIVE;
            if(((*io_map_info)->attributes.creationFlags & SA_CKPT_WR_ACTIVE_REPLICA_WEAK)&&(m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(&(*io_map_info)->attributes)))
              reploc_info->rep_type = REP_NOTACTIVE;
         }

      } 

      proc_rc = cpd_ckpt_reploc_node_add(&cb->ckpt_reploc_tree, reploc_info); 
      if(proc_rc != NCSCC_RC_SUCCESS)
      {
         /* goto reploc_node_add_fail; */
         m_LOG_CPD_CL(CPD_DB_ADD_FAILED,CPD_FC_DB,NCSFL_SEV_ERROR,__FILE__,__LINE__);
      }
   }   

   if(*io_map_info)
   {
      ckpt_id = (*io_map_info)->ckpt_id;
      /* This checkpoint already available with CPD, Get the ckpt_node */
      cpd_ckpt_node_get(&cb->ckpt_tree, 
                    &ckpt_id, &ckpt_node);
      
      if(ckpt_node == NULL)
      {
         /* This should not happen, some thing seriously wrong with the CPD
            data base */
         /* The right thing is crash the CPD TBD */
         m_LOG_CPD_FCL(CPD_DB_ADD_FAILED,CPD_FC_DB,NCSFL_SEV_ERROR,ckpt_id,__FILE__,__LINE__);
         return NCSCC_RC_FAILURE;
      }
      
      *o_ckpt_node = ckpt_node;
   }
   else
   {
      /* Fill the Map Info */
      m_NCS_OS_MEMSET(map_info, 0, sizeof(CPD_CKPT_MAP_INFO));
   /*   m_NCS_OS_MEMCPY(&map_info->ckpt_name,&ckpt_create->ckpt_name,sizeof(ckpt_create->ckpt_name.length)); */
      map_info->ckpt_name = ckpt_create->ckpt_name; 
      map_info->attributes = ckpt_create->attributes;
      map_info->ckpt_id = cb->nxt_ckpt_id++;
      
      proc_rc = cpd_ckpt_map_node_add(&cb->ckpt_map_tree, map_info);
      if(proc_rc != NCSCC_RC_SUCCESS)
      {
         m_LOG_CPD_FCL(CPD_DB_ADD_FAILED,CPD_FC_DB,NCSFL_SEV_ERROR,map_info->ckpt_id,__FILE__,__LINE__);
         goto map_node_add_fail;
      }

      m_NCS_OS_MEMSET(ckpt_node, 0, sizeof(CPD_CKPT_INFO_NODE));
      ckpt_node->ckpt_id = map_info->ckpt_id;
      ckpt_node->ckpt_name = ckpt_create->ckpt_name;
      ckpt_node->is_unlink_set = FALSE;
      ckpt_node->attributes = ckpt_create->attributes;
      if(ckpt_node->attributes.maxSections == 1)
        ckpt_node->num_sections = 1;
      ckpt_node->ret_time = ckpt_create->attributes.retentionDuration;
      m_GET_TIME_STAMP(ckpt_node->create_time); 
      ckpt_node->ckpt_flags = ckpt_create->ckpt_flags;
      /* Select the active replica */
      if(!(m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(&ckpt_create->attributes) && 
         m_IS_ASYNC_UPDATE_OPTION(&ckpt_create->attributes)))
      {
         /* The policy is to make the first replica as active replica */
         ckpt_node->is_active_exists = TRUE;
         ckpt_node->active_dest = *cpnd_dest;
      }
     
      if((!m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(&ckpt_create->attributes))&&(m_CPND_IS_ON_SCXB(cb->cpd_self_id,cpd_get_slot_sub_id_from_mds_dest(*cpnd_dest))))
      {
         if(!ckpt_node->ckpt_on_scxb1)
           ckpt_node->ckpt_on_scxb1 = (uns32)cpd_get_slot_sub_id_from_mds_dest(*cpnd_dest);
         else
            ckpt_node->ckpt_on_scxb2 = (uns32)cpd_get_slot_sub_id_from_mds_dest(*cpnd_dest);
      }
      if((!m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(&ckpt_create->attributes))&&m_CPND_IS_ON_SCXB(cb->cpd_remote_id,cpd_get_slot_sub_id_from_mds_dest(*cpnd_dest)))
      {
         if(!ckpt_node->ckpt_on_scxb1)
           ckpt_node->ckpt_on_scxb1 = (uns32)cpd_get_slot_sub_id_from_mds_dest(*cpnd_dest);
         else
            ckpt_node->ckpt_on_scxb2 = (uns32)cpd_get_slot_sub_id_from_mds_dest(*cpnd_dest);
      }
      proc_rc = cpd_ckpt_node_add(&cb->ckpt_tree, ckpt_node);
      if(proc_rc != NCSCC_RC_SUCCESS)
      {
          m_LOG_CPD_FCL(CPD_DB_ADD_FAILED,CPD_FC_DB,NCSFL_SEV_ERROR,ckpt_node->ckpt_id,__FILE__,__LINE__);
          goto ckpt_node_add_fail;
      }
   
   
       *io_map_info = map_info;
       *o_ckpt_node = ckpt_node;
   }
  

 
   /* Add the CPND Details (CPND reference) to the ckpt node */
   m_NCS_OS_MEMSET(nref_info, 0, sizeof(CPD_NODE_REF_INFO));
   nref_info->dest = *cpnd_dest;
   cpd_node_ref_info_add(ckpt_node, nref_info);
   
   /* Add the ckpt reference to the CPND node info */
   m_NCS_OS_MEMSET(cref_info, 0, sizeof(CPD_CKPT_REF_INFO));
   cref_info->ckpt_node = ckpt_node;
   cpd_ckpt_ref_info_add(node_info, cref_info);

#if 0
   m_LOG_CPD_CFCL(CPD_DB_ADD_SUCCESS,CPD_FC_DB,NCSFL_SEV_INFO,map_info->ckpt_name.value,\
     map_info->ckpt_id,__FILE__,__LINE__);
#endif
   return NCSCC_RC_SUCCESS;


ckpt_node_add_fail:   
map_node_add_fail:
   /* This is the unexpected failure case, (Process TBD ) */
   m_NCS_CONS_PRINTF("UNEXPECTED FAILURE\n");
   return proc_rc;
   
free_mem:
  if(cref_info)
     m_MMGR_FREE_CPD_CKPT_REF_INFO(cref_info);
  
  if(nref_info)
     m_MMGR_FREE_CPD_NODE_REF_INFO(nref_info);
 
  if(*io_map_info == NULL)
  { 
     if(ckpt_node)
        m_MMGR_FREE_CPD_CKPT_INFO_NODE(ckpt_node);
  
     if(map_info)
        m_MMGR_FREE_CPD_CKPT_MAP_INFO(map_info);
  }
  
  return proc_rc;
   
}

/****************************************************************************
 * Name          : cpd_noncolloc_ckpt_rep_delete
 *
 * Description   : This routine will run the policy to create the non-collacated
 *                 ckpt replicas.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 cpd_noncolloc_ckpt_rep_delete(CPD_CB *cb, 
                                    CPD_CKPT_INFO_NODE    *ckpt_node,
                                    CPD_CKPT_MAP_INFO     *map_info)
{
   CPSV_EVT             send_evt;
   CPD_NODE_REF_INFO    *nref_info, *nref_next;
   CPD_CKPT_REF_INFO    *cref_info = NULL;
   CPD_CPND_INFO_NODE   *node_info=NULL;
   uns32 rc=NCSCC_RC_SUCCESS;
   NCS_BOOL ckptid_flag = FALSE;
   SaNameT       ckpt_name,node_name;
   CPD_REP_KEY_INFO  key_info;
   CPD_CKPT_REPLOC_INFO *rep_info = NULL; 
 
   m_NCS_OS_MEMSET(&ckpt_name,0,sizeof(SaNameT));
   m_NCS_OS_MEMSET(&node_name,0,sizeof(SaNameT));
   m_NCS_OS_MEMSET(&key_info,0,sizeof(CPD_REP_KEY_INFO));

   /* Only the created replicas are present, need to delete them */
   nref_info = ckpt_node->node_list;
   while(nref_info)
   {
      nref_next = nref_info->next;
      node_info=NULL;
      
      cpd_cpnd_info_node_get(&cb->cpnd_tree, 
                                  &nref_info->dest, &node_info);
   
      if(node_info)
      {
         /* Remove the ckpt reference from the node_info */
         for(cref_info = node_info->ckpt_ref_list; 
            cref_info != NULL; 
            cref_info = cref_info->next)
         {
            if(cref_info->ckpt_node == ckpt_node)
            {
               cpd_ckpt_ref_info_del(node_info, cref_info);
               break;
            }
         }
         m_NCS_OS_MEMSET(&key_info,0,sizeof(CPD_REP_KEY_INFO));
         m_NCS_OS_MEMSET(&ckpt_name,0,sizeof(SaNameT));
         m_NCS_OS_MEMSET(&node_name,0,sizeof(SaNameT));
         
         ckpt_name = ckpt_node->ckpt_name;
         key_info.ckpt_name = ckpt_name;
          
         node_name = node_info->node_name;

         cpd_a2s_ckpt_dest_del(cb,ckpt_node->ckpt_id,&nref_info->dest,ckptid_flag); 
      
         /* TBD Send the Replica Delete event to CPND */
          /* Send the Replica create info to CPND */
         m_NCS_OS_MEMSET(&send_evt, 0, sizeof(CPSV_EVT));
         send_evt.type = CPSV_EVT_TYPE_CPND;
         send_evt.info.cpnd.type = CPND_EVT_D2ND_CKPT_DESTROY;
         send_evt.info.cpnd.info.ckpt_destroy.ckpt_id = ckpt_node->ckpt_id;
   
         if(cpd_mds_msg_send(cb, NCSMDS_SVC_ID_CPND, nref_info->dest, &send_evt) == NCSCC_RC_SUCCESS)
            m_LOG_CPD_FFCL(CPD_NON_COLOC_CKPT_DESTROY_SUCCESS,CPD_FC_HDLN,NCSFL_SEV_INFO, \
                                 ckpt_node->ckpt_id, nref_info->dest,__FILE__,__LINE__);

         /* Delete the node info, incase there are no ckpts on that CPND */
         if(node_info->ckpt_cnt == 0)
            cpd_cpnd_info_node_delete(cb, node_info);
      }
     
       key_info.node_name = node_name;
   /*  key_info.node_name.length = m_NCS_OS_NTOHS(node_name.length); */
       cpd_ckpt_reploc_get(&cb->ckpt_reploc_tree,&key_info,&rep_info);
       if(rep_info) 
       {
          cpd_ckpt_reploc_node_delete(cb,rep_info);
       }


 
      m_MMGR_FREE_CPD_NODE_REF_INFO(nref_info);
      nref_info = nref_next;
   }
   ckpt_node->node_list = nref_info;
   

   /* Mark the dest_cnt as zero, so that the ckpt_node will get deleted */
   ckpt_node->dest_cnt = 0;  

   return rc;
}

/****************************************************************************
 * Name          : cpd_process_ckpt_delete
 *
 * Description   : This routine will update the CPD database when a existing
 *                 CKPT got deleted.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 cpd_process_ckpt_delete(CPD_CB *cb, 
                              CPD_CKPT_INFO_NODE  *ckpt_node,
                              CPSV_SEND_INFO *sinfo,
                              NCS_BOOL *o_ckpt_node_deleted,
                              NCS_BOOL *o_is_active_changed)
{
   CPD_NODE_REF_INFO    *nref_info;
   CPD_CKPT_REF_INFO    *cref_info = NULL;
   NCS_BOOL             match_found = FALSE;
   CPD_CKPT_MAP_INFO    *map_info=NULL;
   CPD_CPND_INFO_NODE   *node_info=NULL;
   SaCkptCheckpointCreationAttributesT create_attr;
   SaNameT       ckpt_name,node_name;
   CPD_REP_KEY_INFO  key_info;   
   CPD_CKPT_REPLOC_INFO *rep_info = NULL; 

   m_NCS_OS_MEMSET(&ckpt_name,0,sizeof(SaNameT));
   m_NCS_OS_MEMSET(&node_name,0,sizeof(SaNameT));
   m_NCS_OS_MEMSET(&key_info,0,sizeof(CPD_REP_KEY_INFO));
   m_NCS_OS_MEMSET(&create_attr,0,sizeof(SaCkptCheckpointCreationAttributesT));


  if(ckpt_node->is_unlink_set != TRUE)
  { 
     cpd_ckpt_map_node_get(&cb->ckpt_map_tree, &ckpt_node->ckpt_name, &map_info);
     if(map_info == NULL)
     {
        m_LOG_CPD_FCL(CPD_DB_DEL_FAILED,CPD_FC_DB,NCSFL_SEV_ERROR,ckpt_node->ckpt_id,__FILE__,__LINE__);
        return NCSCC_RC_FAILURE;
     }
  }
  
  ckpt_name = ckpt_node->ckpt_name;


  for(nref_info = ckpt_node->node_list; 
        nref_info != NULL; 
        nref_info = nref_info->next)
  {
     if(m_NCS_MDS_DEST_EQUAL(&nref_info->dest, &sinfo->dest))
     {
         /* MDS Dest is matching, Free this node */
           match_found = TRUE;
           break;
     }
  }
   
  if(match_found == FALSE)
  {
     m_LOG_CPD_FCL(CPD_DB_DEL_FAILED,CPD_FC_DB,NCSFL_SEV_ERROR,ckpt_node->ckpt_id,__FILE__,__LINE__);
     return NCSCC_RC_FAILURE;
  }
  /* Get the creation attributes */
  if(ckpt_node->is_unlink_set)
     create_attr = ckpt_node->attributes;
  else
     create_attr = map_info->attributes;
   
   /* Remove the node reference from the ckpt_node */
   cpd_node_ref_info_del(ckpt_node, nref_info);
 
   if(ckpt_node->dest_cnt)
   {
      /* Inform other CPNDs, that the replica received from this (sinfo->dest)
           CPND is deleted */ 
      *o_ckpt_node_deleted = TRUE;

      /* If the deleted replica is active, Select the new active replica */
      if(m_NCS_MDS_DEST_NODEID_EQUAL(m_NCS_NODE_ID_FROM_MDS_DEST(ckpt_node->active_dest), m_NCS_NODE_ID_FROM_MDS_DEST(sinfo->dest)))
      {
         ckpt_node->is_active_exists = FALSE;
         m_NCS_OS_MEMSET(&ckpt_node->active_dest, 0, sizeof(MDS_DEST));
      }
      
         
      /* Select the New Active Dest */
      if(ckpt_node->is_active_exists == FALSE)
      {
         nref_info = ckpt_node->node_list;
      
         /* Non-collocated so make scxb as active */  
         if(!m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(&ckpt_node->attributes)){
             while(nref_info){
               if((m_CPND_IS_ON_SCXB(cb->cpd_self_id,cpd_get_slot_sub_id_from_mds_dest(nref_info->dest)))||
                    (m_CPND_IS_ON_SCXB(cb->cpd_remote_id,cpd_get_slot_sub_id_from_mds_dest(nref_info->dest)))){
                 ckpt_node->is_active_exists = TRUE;
                 ckpt_node->active_dest = nref_info->dest;
                 *o_is_active_changed = TRUE;
                 break;
               }
                nref_info = nref_info->next;
             }
          }
          else{
                if(!m_IS_ASYNC_UPDATE_OPTION(&create_attr))
                {       
                   /* The policy is to select the next replica as the active replica,
                   This is avilable at last node in the ckpt_node->node_list */
                   while(nref_info)
                   {
                     if(nref_info->next == NULL)
                       break;   /* This is the last node */
                     else
                       nref_info = nref_info->next;
                   }
            
                   if(nref_info)
                   {
                      ckpt_node->is_active_exists = TRUE;
                      ckpt_node->active_dest = nref_info->dest;
                      *o_is_active_changed = TRUE;
                   }
                }  
         
                else
                {
                   ckpt_node->is_active_exists = FALSE;
                   m_NCS_MEMSET(&ckpt_node->active_dest, 0, sizeof(MDS_DEST));
            
                   if(nref_info)
                   {
                      /* Need to send this info to CPNDs */
                      *o_is_active_changed = TRUE;
                   }
                }
             }
       }
   }
      
   /* Before deleting the ckpt_node, Get the CPND_INFO_NODE */
   cpd_cpnd_info_node_get(&cb->cpnd_tree, 
                                  &sinfo->dest, &node_info);
   
   if(node_info)
   {
      node_name = node_info->node_name;
      /* Remove the ckpt reference from the node_info */
      for(cref_info = node_info->ckpt_ref_list; 
         cref_info != NULL; 
         cref_info = cref_info->next)
      {
         if(cref_info->ckpt_node == ckpt_node)
         {
            cpd_ckpt_ref_info_del(node_info, cref_info);
            break;
         }
      }

      /* Delete the node info, incase there are no ckpts on that CPND */
      if(node_info->ckpt_cnt == 0)
         cpd_cpnd_info_node_delete(cb, node_info);
   }
  
   if(!m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(&create_attr))
   {
         if((ckpt_node->ckpt_on_scxb1 == 0)&&(ckpt_node->ckpt_on_scxb2 == 0))
         {
           if(!cpd_is_noncollocated_replica_present_on_payload(cb , ckpt_node))
           {
               cpd_noncolloc_ckpt_rep_delete(cb, ckpt_node, map_info);
           }
         } 
   }
   
   if(ckpt_node->dest_cnt == 0)
   {
      *o_ckpt_node_deleted = FALSE;
      *o_is_active_changed = FALSE;
      m_LOG_CPD_FCL(CPD_CKPT_DEL_SUCCESS,CPD_FC_DB,NCSFL_SEV_INFO,ckpt_node->ckpt_id,\
                     __FILE__,__LINE__);
      /* Remove the ckpt_node */
      cpd_ckpt_node_delete(cb, ckpt_node);
      
      /* Delete the ckpt_node & MAP Node */
      if(map_info)
      {
         cpd_ckpt_map_node_delete(cb, map_info);
      }
   }
   /* This is to delete the node from reploc_tree */
   key_info.ckpt_name = ckpt_name;
   key_info.node_name = node_name;
 /*  key_info.node_name.length = m_NCS_OS_NTOHS(node_name.length);*/
   cpd_ckpt_reploc_get(&cb->ckpt_reploc_tree,&key_info,&rep_info);
   if(rep_info)   
   {
      cpd_ckpt_reploc_node_delete(cb,rep_info);
   } 

             
   return NCSCC_RC_SUCCESS;  
}


/****************************************************************************
 * Name          : cpd_process_cpnd_down
 *
 * Description   : This routine will process the CPND down event.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 cpd_process_cpnd_down(CPD_CB *cb, MDS_DEST *cpnd_dest)
{
   CPD_CPND_INFO_NODE   *cpnd_info = NULL;
   CPD_CKPT_REF_INFO    *cref_info;
   uns32                proc_rc;
   CPD_CKPT_MAP_INFO    *map_info=NULL;
   NCS_BOOL             ckptid_flag = TRUE;
   NCS_BOOL             add_flag = FALSE;
   CPD_CKPT_INFO_NODE   *ckpt_node=NULL;
   CPD_REP_KEY_INFO  key_info;
   CPD_CKPT_REPLOC_INFO *rep_info = NULL;

   m_NCS_OS_MEMSET(&key_info,0,sizeof(CPD_REP_KEY_INFO));
  
 
  /* cpd_cpnd_info_node_get(&cb->cpnd_tree, cpnd_dest, &cpnd_info); */
   cpd_cpnd_info_node_find_add(&cb->cpnd_tree,cpnd_dest, &cpnd_info, &add_flag);
   if(!cpnd_info)
      return NCSCC_RC_SUCCESS;
   
   cref_info = cpnd_info->ckpt_ref_list;
   
   while(cref_info)
   {
      CPD_NODE_REF_INFO    *nref_info;
      NCS_BOOL             match_found = FALSE;
      CPSV_EVT             send_evt;

      /* Update the ckpt reference list in the cpnd node after removing the first cref_info */
      cpnd_info->ckpt_ref_list = cref_info->next;
      
      ckpt_node =  cref_info->ckpt_node;
      for(nref_info = ckpt_node->node_list; 
          nref_info != NULL; 
          nref_info = nref_info->next)
      {
         if(m_NCS_MDS_DEST_NODEID_EQUAL(m_NCS_NODE_ID_FROM_MDS_DEST(nref_info->dest), m_NCS_NODE_ID_FROM_MDS_DEST(*cpnd_dest)))
         {
            /* MDS Dest is matching, Free this node */
            match_found = TRUE;
            break;
         }
      }
      
      key_info.ckpt_name = ckpt_node->ckpt_name;
      key_info.node_name = cpnd_info->node_name;
   
      if(match_found == TRUE)
      {
         /* Remove the node reference from the ckpt_node */
         cpd_node_ref_info_del(ckpt_node, nref_info);
    
         if(!(m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(&ckpt_node->attributes)))
         {
       
             if(m_CPND_IS_ON_SCXB(ckpt_node->ckpt_on_scxb1,cpd_get_slot_sub_id_from_mds_dest(*cpnd_dest)))
             {
                ckpt_node->ckpt_on_scxb1 = 0;
             }
             if(m_CPND_IS_ON_SCXB(ckpt_node->ckpt_on_scxb2,cpd_get_slot_sub_id_from_mds_dest(*cpnd_dest)))
             {
                ckpt_node->ckpt_on_scxb2 = 0;
             }

#if 0
            if(ckpt_node->dest_cnt == 1) /* only standby scxb exists */
            {
              if((ckpt_node->ckpt_on_scxb1 == 0)&&(ckpt_node->ckpt_on_scxb2 == 0))
              {
                 cpd_noncolloc_ckpt_rep_delete(cb, ckpt_node, map_info);
              }
            }
            else if(ckpt_node->dest_cnt > 1)
            {
               if(!m_CPND_IS_ON_SCXB(cb->cpd_self_id,cpd_get_slot_sub_id_from_mds_dest(*cpnd_dest)) && \
                  !m_CPND_IS_ON_SCXB(cb->cpd_remote_id,cpd_get_slot_sub_id_from_mds_dest(*cpnd_dest)))
               { /* Payload */
                  if((ckpt_node->ckpt_on_scxb1 == 0) && (ckpt_node->ckpt_on_scxb2 == 0))
                  {
                     if(ckpt_node->dest_cnt <= 2)
                       cpd_noncolloc_ckpt_rep_delete(cb, ckpt_node, map_info);
                  }

               }
            }
#endif      
             if((ckpt_node->ckpt_on_scxb1 == 0)&&(ckpt_node->ckpt_on_scxb2 == 0))
             {
               if(!cpd_is_noncollocated_replica_present_on_payload(cb , ckpt_node))
               {
                  cpd_noncolloc_ckpt_rep_delete(cb, ckpt_node, map_info);
               }
             }

         }

         /* Before delete ckpt_node, update send_evt */
         m_NCS_OS_MEMSET(&send_evt, 0, sizeof(CPSV_EVT));
         send_evt.type = CPSV_EVT_TYPE_CPND;
         send_evt.info.cpnd.type = CPND_EVT_D2ND_CKPT_REP_DEL;
         send_evt.info.cpnd.info.ckpt_del.ckpt_id = ckpt_node->ckpt_id;
         send_evt.info.cpnd.info.ckpt_del.mds_dest = *cpnd_dest;
         if(ckpt_node->dest_cnt == 0)
         {
            m_LOG_CPD_FCL(CPD_CKPT_DEL_SUCCESS,CPD_FC_DB,NCSFL_SEV_INFO,ckpt_node->ckpt_id,\
                     __FILE__,__LINE__);
            cpd_ckpt_map_node_get(&cb->ckpt_map_tree, &ckpt_node->ckpt_name, &map_info);

            /* BUG FIX FOR SNMP MIBS */
        /*    ckpt_node->ckpt_name.length = m_NCS_OS_NTOHS(ckpt_node->ckpt_name.length);  */
            /* Remove the ckpt_node */
            proc_rc = cpd_ckpt_node_delete(cb, ckpt_node);

            if(map_info)
            {
               proc_rc = cpd_ckpt_map_node_delete(cb, map_info);
            }
         }
         else
         {
             /* Broadcast the info to all CPNDs */
             proc_rc = cpd_mds_bcast_send(cb, &send_evt, NCSMDS_SVC_ID_CPND);
             m_LOG_CPD_FFCL(CPD_REP_DEL_SUCCESS,CPD_FC_DB,NCSFL_SEV_INFO,ckpt_node->ckpt_id,*cpnd_dest,\
                            __FILE__,__LINE__);
         
             if(m_NCS_MDS_DEST_NODEID_EQUAL(m_NCS_NODE_ID_FROM_MDS_DEST(ckpt_node->active_dest),m_NCS_NODE_ID_FROM_MDS_DEST( *cpnd_dest)))
             {
                ckpt_node->is_active_exists = FALSE;
                m_NCS_OS_MEMSET(&ckpt_node->active_dest, 0, sizeof(MDS_DEST));
             }

             if(ckpt_node->is_active_exists == FALSE)
             {
                 CPD_NODE_REF_INFO *nref_info2;
                 nref_info2 = ckpt_node->node_list;
                 
                 /* If it is non-collocated then select the scxb dest as active */
                 if(!m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(&ckpt_node->attributes)){                     
                    while(nref_info2){
                       if((m_CPND_IS_ON_SCXB(cb->cpd_self_id,cpd_get_slot_sub_id_from_mds_dest(nref_info2->dest)))||
                             (m_CPND_IS_ON_SCXB(cb->cpd_remote_id,cpd_get_slot_sub_id_from_mds_dest(nref_info2->dest)))){
                          ckpt_node->is_active_exists = TRUE;
                          ckpt_node->active_dest = nref_info2->dest; 
                          break;
                       }  
                       nref_info2 = nref_info2->next;
                    }   
                 }        
                 else{                         
                       if(!m_IS_ASYNC_UPDATE_OPTION(&ckpt_node->attributes))
                       {
                          /* The policy is to select the next replica as the active
                          replica, This is avilable at last node in the ckpt_node->node_list */
                          while(nref_info2)
                          {
                            if(nref_info2->next == NULL)
                              break;   /* This is the last node */
                            else
                              nref_info2 = nref_info2->next;
                          }
                                                                                
                          if(nref_info2)
                          {
                           ckpt_node->is_active_exists = TRUE;
                           ckpt_node->active_dest = nref_info2->dest;
                          }
                       }
                     }
               
                /* Chosen the active dest, broadcast to all CPNDs */
                m_NCS_OS_MEMSET(&send_evt, 0, sizeof(CPSV_EVT));
                send_evt.type = CPSV_EVT_TYPE_CPND;
                send_evt.info.cpnd.type = CPND_EVT_D2ND_CKPT_ACTIVE_SET;
                send_evt.info.cpnd.info.active_set.ckpt_id = ckpt_node->ckpt_id;
                send_evt.info.cpnd.info.active_set.mds_dest = ckpt_node->active_dest;

                proc_rc = cpd_mds_bcast_send(cb, &send_evt, NCSMDS_SVC_ID_CPND);
                m_LOG_CPD_FFCL(CPD_CKPT_ACTIVE_CHANGE_SUCCESS,CPD_FC_HDLN,NCSFL_SEV_INFO,ckpt_node->ckpt_id,\
                               ckpt_node->active_dest,__FILE__,__LINE__);

                /*To broadcast the active MDS_DEST info of ckpt to all CPA's*/
                m_NCS_OS_MEMSET(&send_evt, 0, sizeof(CPSV_EVT));
                send_evt.type = CPSV_EVT_TYPE_CPA;
                send_evt.info.cpa.type = CPA_EVT_D2A_ACT_CKPT_INFO_BCAST_SEND;
                send_evt.info.cpa.info.ackpt_info.ckpt_id = ckpt_node->ckpt_id;
                send_evt.info.cpa.info.ackpt_info.mds_dest = ckpt_node->active_dest;
                proc_rc = cpd_mds_bcast_send(cb, &send_evt, NCSMDS_SVC_ID_CPA);
#if 0
                m_NCS_CONS_PRINTF("BLADE DOWN EVT ckptid = %llu mds_dest = %llu\n",ckpt_node->ckpt_id, ckpt_node->active_dest);
#endif
            }   /* if(ckpt_node->is_active_exists == FALSE) */
         }  /* else of if(ckpt_node->dest_cnt == 0) */
      }   /* if(match_found == TRUE) */
         /* Send it to CPD(s), by sending ckpt_id = 0 */
      
        /* This is to delete the node from reploc_tree */
      cpd_ckpt_reploc_get(&cb->ckpt_reploc_tree,&key_info,&rep_info);
      if(rep_info)
      {
         cpd_ckpt_reploc_node_delete(cb,rep_info);
      }
      
      m_MMGR_FREE_CPD_CKPT_REF_INFO(cref_info);

      cref_info = cpnd_info->ckpt_ref_list;
   }

   cpd_a2s_ckpt_dest_del(cb,0,cpnd_dest,ckptid_flag);

   cpnd_info->ckpt_ref_list = 0;
   cpd_cpnd_info_node_delete(cb, cpnd_info);
   return NCSCC_RC_SUCCESS;
}


/*************************************************************************************
 * Name           : cpd_proc_active_set
 *
 * Description    : This routine is to set the active replica 
 *
 * Return Values  :
 *
 * Notes          : None
**************************************************************************************/

uns32  cpd_proc_active_set(CPD_CB *cb,SaCkptCheckpointHandleT ckpt_id,MDS_DEST mds_dest,CPD_CKPT_INFO_NODE **ckpt_node)
{
   SaAisErrorT         rc=SA_AIS_OK;
   CPD_REP_KEY_INFO  key_info;
   CPD_CKPT_REPLOC_INFO *rep_info = NULL;
   CPD_CPND_INFO_NODE *cpnd_info_node;
   SaNameT node_name;
   
   m_NCS_OS_MEMSET(&node_name,0,sizeof(SaNameT));
   m_NCS_OS_MEMSET(&key_info,0,sizeof(CPD_REP_KEY_INFO));

   cpd_ckpt_node_get(&cb->ckpt_tree,&ckpt_id, ckpt_node);
   if((*ckpt_node) == NULL)
   {
      m_LOG_CPD_FCL(CPD_CKPT_INFO_NODE_GET_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,ckpt_id,__FILE__,__LINE__);

      return SA_AIS_ERR_NOT_EXIST;
   }
/* Update the Active Replica Info */

 
   if( (*ckpt_node)->is_active_exists)
   {
      if(((*ckpt_node)->active_dest) != mds_dest)      
      {
         cpd_cpnd_info_node_get(&cb->cpnd_tree,&((*ckpt_node)->active_dest),&cpnd_info_node);
         if(cpnd_info_node)
         {
            key_info.ckpt_name        = (*ckpt_node)->ckpt_name;
            key_info.node_name        = cpnd_info_node->node_name;
            cpd_ckpt_reploc_get(&cb->ckpt_reploc_tree,&key_info,&rep_info);
            if(rep_info)
            {
               rep_info->rep_type = 2;
            }
            m_NCS_OS_MEMSET(&key_info,0,sizeof(CPD_REP_KEY_INFO));
         }
      }
   }


   (*ckpt_node)->is_active_exists = TRUE;
   (*ckpt_node)->active_dest = mds_dest;

   if(mds_dest)
   {
      cpd_cpnd_info_node_get(&cb->cpnd_tree,&mds_dest,&cpnd_info_node); 
      if(!cpnd_info_node){ 
         m_LOG_CPD_FCL(CPD_CPND_NODE_DOES_NOT_EXIST,CPD_FC_HDLN,NCSFL_SEV_ERROR,mds_dest,__FILE__,__LINE__);
         return rc;
      }
      key_info.ckpt_name        = (*ckpt_node)->ckpt_name;
      key_info.node_name        = cpnd_info_node->node_name; 
  /*  key_info.node_name.length = m_NCS_OS_NTOHS(cpnd_info_node->node_name.length);*/
      cpd_ckpt_reploc_get(&cb->ckpt_reploc_tree,&key_info,&rep_info);
      if(rep_info)
      {
         rep_info->rep_type = 1;
      }
   }
   return rc;
}



/******************************************************************************************
 * Name          : cpd_proc_retention_set
 *
 * Description   : This routine will set the retention duration
 *
 * Return Values : 
 *
 * Notes         : None
******************************************************************************************/

uns32 cpd_proc_retention_set(CPD_CB *cb, SaCkptCheckpointHandleT ckpt_id,SaTimeT reten_time,CPD_CKPT_INFO_NODE **ckpt_node)
{
    SaAisErrorT       rc = SA_AIS_OK;
    cpd_ckpt_node_get(&cb->ckpt_tree,&ckpt_id, ckpt_node);

   if((*ckpt_node) == NULL)
   {
      m_LOG_CPD_FCL(CPD_CKPT_INFO_NODE_GET_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,ckpt_id,__FILE__,__LINE__);
      return SA_AIS_ERR_NOT_EXIST;
   }

   /* Update the retention Time */
   (*ckpt_node)->ret_time = reten_time;
   return rc;
}


/**************************************************************************************
 * Name          : cpd_proc_unlink_set
 *
 * Description   : This routine will set the unlink flag of the checkpoint
 *
 * Return Values :
 *
 * Notes         : None
**************************************************************************************/

uns32 cpd_proc_unlink_set(CPD_CB *cb,CPD_CKPT_INFO_NODE **ckpt_node,CPD_CKPT_MAP_INFO *map_info,SaNameT *ckpt_name)
{
   SaAisErrorT         rc=SA_AIS_OK;   

   /* Get the Map Info & Ckpt info records */
   cpd_ckpt_map_node_get(&cb->ckpt_map_tree, ckpt_name, &map_info); 

   if(map_info)
   {
      /* BUG FIX FOR SNMP MIBS */
 /*   ckpt_name->length =  m_NCS_OS_NTOHS(ckpt_name->length);   */
      cpd_ckpt_node_get(&cb->ckpt_tree, &map_info->ckpt_id, ckpt_node);
   }
   else
   {
      m_LOG_CPD_CCL(CPD_PROC_UNLINK_SET_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,ckpt_name->value,__FILE__,__LINE__);
      /* There is no checkpoint opened with this name */
      return SA_AIS_ERR_NOT_EXIST;
   }

   if((*ckpt_node) == 0)
   {
      /* This should not happen, Incorrect CPD database 
       Handling, TBD */
       m_LOG_CPD_CCL(CPD_PROC_UNLINK_SET_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,ckpt_name->value,__FILE__,__LINE__);
       return SA_AIS_ERR_NOT_EXIST;
   }

   (*ckpt_node)->is_unlink_set = TRUE;
   (*ckpt_node)->attributes = map_info->attributes;
    /* Delete the MAP Info */
   rc = cpd_ckpt_map_node_delete(cb, map_info);
   return rc;
}  

/****************************************************************************
 * Name          : cpd_is_noncollocated_replica_present_on_payload
 *
 * Description   : This is the function dumps the Ckpt Replica Info
 * Arguments     : cb       - CPD Control Block pointer
 *                 ckpt_node  - pointer to checkpoint node
 *
 * Return Values : TRUE/FALSE
 *****************************************************************************/
NCS_BOOL cpd_is_noncollocated_replica_present_on_payload(CPD_CB *cb, CPD_CKPT_INFO_NODE *ckpt_node)
{
   CPD_NODE_REF_INFO    *nref_info = ckpt_node->node_list;
   
     while(nref_info)
     {
         /* Check if a replica is present on one of the payload blades */
         if((!m_CPND_IS_ON_SCXB(cb->cpd_self_id, cpd_get_slot_sub_id_from_mds_dest(nref_info->dest))) && \
           (!m_CPND_IS_ON_SCXB(cb->cpd_remote_id,cpd_get_slot_sub_id_from_mds_dest(nref_info->dest)))) {
               return TRUE;
         }

         nref_info = nref_info->next;
     }

   return FALSE;
}


/****************************************************************************
 * Name          : cpd_cb_dump
 *
 * Description   : This routine is used for debugging.
 *
 * Return Values : NCSCC_RC_SUCCESS/Error.
 *
 * Notes         : None.
 *****************************************************************************/
void cpd_cb_dump(void)
{
   CPD_CB *cb;

   m_CPD_RETRIEVE_CB(cb);

   if(cb == NULL) {
      return;
   }        

 /* # if ( CPSV_DEBUG == 1)  */
   m_NCS_CONS_PRINTF("\n\n");   
   m_NCS_CONS_PRINTF("*****************Printing CPD CB Dump******************");
   m_NCS_CONS_PRINTF("\n MDS Handle:             %x", (uns32)cb->mds_handle);
   m_NCS_CONS_PRINTF("\n Handle Manager Pool ID: %d", cb->hm_poolid);
   m_NCS_CONS_PRINTF("\n Handle Manager Handle:  %d", cb->cpd_hdl);
   m_NCS_CONS_PRINTF("\n CPD State:  %d",cb->ha_state); 
   m_NCS_CONS_PRINTF("ACTIVE : ENC ASYNC UPDATE COUNT %d\n",cb->cpd_sync_cnt);
   m_NCS_CONS_PRINTF("STANDBY : DEC ASYNC UPDATE COUNT %d\n",cb->sync_upd_cnt);
   if(cb->is_cpnd_tree_up)
   {
      m_NCS_CONS_PRINTF("\n+++++++++++++CPND Tree is UP+++++++++++++++++++++++++++");
      m_NCS_CONS_PRINTF("\nNumber of nodes in CPND Tree:  %d", cb->cpnd_tree.n_nodes);
      
      /* Print the CPND Details */
      if(cb->cpnd_tree.n_nodes >0)
      {
         MDS_DEST key;
         CPD_CPND_INFO_NODE *cpnd_info_node;
         CPD_CKPT_REF_INFO  *list=NULL;
         m_NCS_OS_MEMSET(&key, 0, sizeof(MDS_DEST));
         cpnd_info_node = 
          (CPD_CPND_INFO_NODE *)ncs_patricia_tree_getnext(&cb->cpnd_tree, (uns8*)&key);
         while(cpnd_info_node)
         {
            key = cpnd_info_node->cpnd_dest;
            m_NCS_CONS_PRINTF("\n------------------------------------------------------");
            m_NCS_CONS_PRINTF("\n MDS Node ID:  = %d", m_NCS_NODE_ID_FROM_MDS_DEST(cpnd_info_node->cpnd_dest));
            m_NCS_CONS_PRINTF("\n Number of CKPTs  = %d", cpnd_info_node->ckpt_cnt);
            list = cpnd_info_node->ckpt_ref_list;
            m_NCS_CONS_PRINTF("\n List of CKPT IDs: ");
            while(list && cpnd_info_node->ckpt_cnt > 0)
            {
               m_NCS_CONS_PRINTF(" %d ", (uns32)list->ckpt_node->ckpt_id);
               list = list->next;
            }
            m_NCS_CONS_PRINTF("\n End of CKPT IDs List ");
           cpnd_info_node = 
           (CPD_CPND_INFO_NODE *)ncs_patricia_tree_getnext(&cb->cpnd_tree, (uns8*)&key);
         }
         m_NCS_CONS_PRINTF("\n End of CPND Info");
      }
      m_NCS_CONS_PRINTF("\n End of CPND info nodes ");
   }
   
   /* Print the Checkpoint Details */
   if(cb->is_ckpt_tree_up)
   {
      m_NCS_CONS_PRINTF("\n+++++++++++++CKPT Tree is UP+++++++++++++++++++++++++++");
      m_NCS_CONS_PRINTF("\nNumber of nodes in CKPT Tree:  %d", cb->ckpt_tree.n_nodes);
      
      /* Print the CKPT Details */
      if(cb->ckpt_tree.n_nodes > 0)
      {
         SaCkptCheckpointHandleT  prev_ckpt_id=0;
         CPD_CKPT_INFO_NODE *ckpt_node;
         CPD_NODE_REF_INFO  *list;
         
         /* Get the First Node */
         ckpt_node = (CPD_CKPT_INFO_NODE *)ncs_patricia_tree_getnext(&cb->ckpt_tree,
                                                   (uns8*)&prev_ckpt_id);
         
         while(ckpt_node)
         {
            uns32 i=0;
            prev_ckpt_id = ckpt_node->ckpt_id;
            
            m_NCS_CONS_PRINTF("\n------------------------------------------------------");
            m_NCS_CONS_PRINTF("\n CKPT ID:  = %d", (uns32)ckpt_node->ckpt_id);
            m_NCS_CONS_PRINTF("\n CKPT Name len  = %d", ckpt_node->ckpt_name.length);
            m_NCS_CONS_PRINTF("\n CKPT Name: ");
            for(i=0; i < ckpt_node->ckpt_name.length; i++)
            {
               m_NCS_CONS_PRINTF("%c", ckpt_node->ckpt_name.value[i]);
            }
            
            m_NCS_CONS_PRINTF("\n UNLINK = %d, Active Exists = %d", ckpt_node->is_unlink_set, 
                                                        ckpt_node->is_active_exists);
            if(ckpt_node->is_unlink_set)
            {
               m_NCS_CONS_PRINTF("\n Create Attributes");
               m_NCS_CONS_PRINTF("\n creationFlags: %d, ", (uns32)ckpt_node->attributes.creationFlags);
               m_NCS_CONS_PRINTF("\n retentionDuration: %d, ", (uns32)ckpt_node->attributes.retentionDuration);
               m_NCS_CONS_PRINTF("\n maxSections: %ld, ", ckpt_node->attributes.maxSections);
               m_NCS_CONS_PRINTF("\n maxSectionSize: %d, ", (uns32)ckpt_node->attributes.maxSectionSize);
               m_NCS_CONS_PRINTF("\n maxSectionIdSize: %d, ", (uns32)ckpt_node->attributes.maxSectionIdSize);
            }
            
            if(ckpt_node->is_active_exists)
               m_NCS_CONS_PRINTF("\n Active DEST NODE ID: %d", m_NCS_NODE_ID_FROM_MDS_DEST(ckpt_node->active_dest));

            m_NCS_CONS_PRINTF("Reteintion Time = %d", (uns32)ckpt_node->ret_time);

            list = ckpt_node->node_list;
            m_NCS_CONS_PRINTF("No of Dest: %d", ckpt_node->dest_cnt);
            m_NCS_CONS_PRINTF("\n List of NODEs: ");
            while(list && ckpt_node->dest_cnt > 0)
            {
               m_NCS_CONS_PRINTF(" %d, ", m_NCS_NODE_ID_FROM_MDS_DEST(list->dest));
               list = list->next;
            }
            m_NCS_CONS_PRINTF("\n End of CKPT NODE s List ");
            ckpt_node = (CPD_CKPT_INFO_NODE *)ncs_patricia_tree_getnext(&cb->ckpt_tree,
                                                   (uns8*)&prev_ckpt_id);
         }
         m_NCS_CONS_PRINTF("\n End of CKPT Info");
      }
      m_NCS_CONS_PRINTF("\n End of CKPT nodes information ");
   }
   
      /* Print the Checkpoint Details */
   if(cb->is_ckpt_map_up)
   {
      m_NCS_CONS_PRINTF("\n+++++++++++++CKPT MAP Tree is UP+++++++++++++++++++++++++++");
      m_NCS_CONS_PRINTF("\nNumber of nodes in CKPT Map Tree:  %d", cb->ckpt_map_tree.n_nodes);
      
      /* Print the CKPT Details */
      if (cb->ckpt_map_tree.n_nodes > 0)
      {
         CPD_CKPT_MAP_INFO *ckpt_map_node=NULL;
         SaNameT     name;
         
         m_NCS_OS_MEMSET(&name, 0, sizeof(SaNameT));
   
         /* Get the First Node */
         ckpt_map_node = (CPD_CKPT_MAP_INFO *)ncs_patricia_tree_getnext(&cb->ckpt_map_tree,
                                                   (uns8*)name.value);
         while(ckpt_map_node != NULL)
         {
            uns32 i;
            
            name = ckpt_map_node->ckpt_name;
         
            m_NCS_CONS_PRINTF("\n------------------------------------------------------");
            m_NCS_CONS_PRINTF("\n CKPT Name len  = %d", ckpt_map_node->ckpt_name.length);
            m_NCS_CONS_PRINTF("\n CKPT Name: ");
            for(i=0; i<ckpt_map_node->ckpt_name.length; i++)
            {
               m_NCS_CONS_PRINTF("%c", ckpt_map_node->ckpt_name.value[i]);
            }
            
            m_NCS_CONS_PRINTF("\n CKPT ID:  = %d", (uns32)ckpt_map_node->ckpt_id);
            
            m_NCS_CONS_PRINTF("\n Create Attributes");
            m_NCS_CONS_PRINTF("\n creationFlags: %d, ", (uns32)ckpt_map_node->attributes.creationFlags);
            m_NCS_CONS_PRINTF("\n retentionDuration: %d, ", (uns32)ckpt_map_node->attributes.retentionDuration);
            m_NCS_CONS_PRINTF("\n maxSections: %ld, ", ckpt_map_node->attributes.maxSections);
            m_NCS_CONS_PRINTF("\n maxSectionSize: %d, ", (uns32)ckpt_map_node->attributes.maxSectionSize);
            m_NCS_CONS_PRINTF("\n maxSectionIdSize: %d, ", (uns32)ckpt_map_node->attributes.maxSectionIdSize);
            ckpt_map_node = (CPD_CKPT_MAP_INFO *)ncs_patricia_tree_getnext(&cb->ckpt_map_tree,(uns8*)name.value);
         }
         m_NCS_CONS_PRINTF("\n End of CKPT Info");
      }
      m_NCS_CONS_PRINTF("\n End of CKPT nodes information \n");
   }  
   m_NCS_CONS_PRINTF("*****************End of CPD CB Dump******************");
   m_NCS_CONS_PRINTF("\n\n");   
/* #endif    */
   return;  
}




