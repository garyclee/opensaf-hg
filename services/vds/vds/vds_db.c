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

  MODULE NAME:       vds_db.c


  DESCRIPTION:

******************************************************************************
*/

#include "vds.h"

/* Static function proto-type */

static VDS_VDEST_DB_INFO *vds_new_vdest(VDS_CB *vds_cb,
                                        NCSVDA_VDEST_CREATE_INFO *info);
static VDS_VDEST_DB_INFO *vds_lookup_by_vdest_id(VDS_CB *vds_cb);
static uns32 vds_free_vdest_assignment(VDS_CB *vds_cb,
                                       VDS_VDEST_DB_INFO **io_db_node,
                                       VDS_VDEST_ADEST_INFO **pp_adest_node,
                                       NCS_BOOL honor_persistent_flag,
                                       NCS_BOOL ckpt);
static VDS_VDEST_ADEST_INFO **vds_srch_adest_in_adest_list(VDS_VDEST_ADEST_INFO **p_head,
                                                           MDS_DEST *adest);
static VDS_VDEST_DB_INFO *vds_getnext_by_vdest_id(VDS_CB *vds_cb,
                                                  MDS_DEST *vdest_id);


/****************************************************************************
  Name          :  vds_process_vdest_create

  Description   :  This function generates and returns the vdest_id for the
                   given vdest_name.
                   - It also checkpoints the respective data.

  Arguments     :  vds_cb - pointer to VDS Control block
                   src_adest - given adest node 
                   info - request info to create VDEST

  Return Values :  Success: NCSCC_RC_SUCCESS
                   Failure: NCSCC_RC_FAILURE  

  Notes         : None
******************************************************************************/
uns32 vds_process_vdest_create(VDS_CB *vds_cb,
                               MDS_DEST *src_adest,
                               NCSVDA_VDEST_CREATE_INFO *info)
{
   VDS_VDEST_DB_INFO    *vdest_db_node = NULL;
   VDS_VDEST_ADEST_INFO *adest_node = NULL, **pp_adest_node = NULL;
   VDS_CKPT_OPERATIONS  vds_ckpt_write_flag = VDS_CKPT_OPERA_OVERWRITE;
   uns32 rc = NCSCC_RC_SUCCESS;


   /* BEWARE: SaNameT can be non-printable. The following trace is for
    * debugging purposes only */

   VDS_TRACE1_ARG3("vds_process_vdest_create(Name=\"%s Adest=%lld\")\n",
                   info->info.named.i_name.value,*src_adest);

   /* Check DB to see if there is a VDEST (string) already exists */
   vdest_db_node = (VDS_VDEST_DB_INFO*)ncs_patricia_tree_get(&vds_cb->vdest_name_db,
                                                    (uns8 *)&info->info.named.i_name);
   if (vdest_db_node != NULL)
   {
      /* Check if there is an anchor already associated to this ADEST */
      pp_adest_node = vds_srch_adest_in_adest_list(&vdest_db_node->adest_list,
                                                   src_adest);
      if (pp_adest_node != NULL)
      {
         info->info.named.o_vdest = vdest_db_node->vdest_id;
         return NCSCC_RC_SUCCESS;
      }

      /* log message VDest Name and Adest already exits */
   }
   else
   {
      /*No VDEST already. Will need to create one */
      vdest_db_node = vds_new_vdest(vds_cb, info);
      if (vdest_db_node == NULL)
      {
         /*log message Unable to create vdest_db_node*/     
         return NCSCC_RC_FAILURE;
      }
      
       /* log message vdest name does not exits */
         
      /* if VDEST, ADEST  already exits  simply smile. 
       *    
       * if new VDEST is  created and new adest node is added 
       * then  new section will be written (vds_ckpt_write_flag is WRITE )
       * to check point with vdest_id as that section id. 
       * 
       * if VDEST already exits and new  adest node is added 
       * (vds_ckpt_write_flag is OVER_WRITE ) a section is overwritten 
       * in check point using vdest_id as section_id.
       */

      vds_ckpt_write_flag = VDS_CKPT_OPERA_WRITE;
   }
   
   
   adest_node = vds_new_vdest_instance(vdest_db_node, src_adest);
   if (adest_node == NULL)
   {
      /*adest creation failed */     
      return NCSCC_RC_FAILURE;
   }

   info->info.named.o_vdest = vdest_db_node->vdest_id;
   
   
   /*Now checkpoint the data (write/overwrite) */
   rc = vds_ckpt_write(vds_cb, vdest_db_node, vds_ckpt_write_flag);
   
   if (rc != NCSCC_RC_SUCCESS)
   {
      vds_process_vdest_destroy(vds_cb,
                                src_adest,
                                (NCSCONTEXT)vdest_db_node,
                                FALSE);
       m_VDS_LOG_CKPT(VDS_LOG_CKPT_WRITE,
                           VDS_LOG_CKPT_FAILURE,
                                  NCSFL_SEV_ERROR, rc);
      
      return rc;  
   }

   m_VDS_LOG_CKPT(VDS_LOG_CKPT_WRITE,
                       VDS_LOG_CKPT_SUCCESS,
                              NCSFL_SEV_INFO, rc);
   return rc;
}


/****************************************************************************
  Name          : vds_process_vdest_lookup

  Description   : Returns vdest_id for given VDEST(string) if it exits 
                  in VDS database. 

  Arguments     : vds_cb - pointer to VDS control block
                  info   - Lookup request for vdest name in the vdests db 

  Return Values : Success: NCSCC_RC_SUCCESS
                  Failure: NCSCC_RC_FAILURE  

  Notes         : None
******************************************************************************/
uns32 vds_process_vdest_lookup(VDS_CB *vds_cb, NCSVDA_VDEST_LOOKUP_INFO *info)
{
   VDS_VDEST_DB_INFO *vdest_db_node = NULL;

   VDS_TRACE1_ARG1("vds_process_vdest_lookup\n");

   /*Gets the patrica node with vdest_name as key from db*/
   vdest_db_node = (VDS_VDEST_DB_INFO*)ncs_patricia_tree_get(&vds_cb->vdest_name_db,
                                                    (uns8 *)&info->i_name);
   if (vdest_db_node == NULL)
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   
   info->o_vdest = vdest_db_node->vdest_id;
   
   m_VDS_LOG_VDEST(info->i_name.value,m_MDS_GET_VDEST_ID_FROM_MDS_DEST(info->o_vdest),"LOOKUP");

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          : vds_process_vdest_destroy

  Description   : De-allocates given ADEST node and also VDEST node if no more  
                  adests present.

  Arguments     : vds_cb - pointer to VDS Control block
                  src_adest - given adest node 
                  info - request info to destroy VDEST.

  Return Values : Success: NCSCC_RC_SUCCESS
                  Failure: NCSCC_RC_FAILURE  

  Notes         : None
******************************************************************************/
uns32 vds_process_vdest_destroy(VDS_CB *vds_cb,
                                MDS_DEST *src_adest,
                                NCSCONTEXT db_info,
                                NCS_BOOL external)
{
   VDS_VDEST_DB_INFO  *vdest_db_node = NULL;
   VDS_VDEST_ADEST_INFO **pp_adest_node;
   NCSVDA_VDEST_DESTROY_INFO *info = NULL;
   uns32 rc = NCSCC_RC_SUCCESS;



   m_VDS_LOG_VDEST("VDEST DESTROY",(long)100,"DESTROY");

   if (external)
   {
      info = (NCSVDA_VDEST_DESTROY_INFO *)db_info;

      VDS_TRACE1_ARG3("vds_process_vdest_destroy(Name=\"%s Adest=%lld\")\n",
                   info->i_name.value,*src_adest);

      vdest_db_node = (VDS_VDEST_DB_INFO*)ncs_patricia_tree_get(&vds_cb->vdest_name_db,
                                                    (uns8 *)&info->i_name);
   }
   else
      vdest_db_node = (VDS_VDEST_DB_INFO *)db_info;



   if (vdest_db_node == NULL)
   {
      /* This VDEST is not in use */
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   pp_adest_node = vds_srch_adest_in_adest_list(&vdest_db_node->adest_list,
                                                src_adest);
   if (pp_adest_node == NULL)
   {
      return NCSCC_RC_FAILURE;
   }
       
   /* Return the <i_vdest> value for VDA's sake */
   if (external)
      info->i_vdest = vdest_db_node->vdest_id;
      
   rc = vds_free_vdest_assignment(vds_cb, &vdest_db_node, pp_adest_node, FALSE, external);

   return rc;
}


/****************************************************************************
  Name          :  vds_new_vdest_instance

  Description   :  Allocates ADEST node and adds it to adest_list. 

  Arguments     :  db_node - pointer to VDS_DB_INFO_NODE. 
                   adest   - pointer to adest. 

  Return Values :  Success : allocated VDS_VDEST_ADEST_INFO node. 
                   Failure : NULL

  Notes         :  None
******************************************************************************/
VDS_VDEST_ADEST_INFO *vds_new_vdest_instance(VDS_VDEST_DB_INFO *db_node,
                                             MDS_DEST *adest)
{
   VDS_VDEST_ADEST_INFO *p_adest_node = NULL;

   VDS_TRACE1_ARG1("vds_new_vdest_instance\n");

   p_adest_node = m_MMGR_ALLOC_VDS_ADEST_INFO;
   if (p_adest_node == NULL)
   {
       m_VDS_LOG_MEM(VDS_LOG_MEM_VDS_ADEST_INFO,
                             VDS_LOG_MEM_ALLOC_FAILURE,
                                   NCSFL_SEV_ERROR);
      return NULL;
   }

   m_NCS_OS_MEMSET(p_adest_node, 0, sizeof(*p_adest_node));

   p_adest_node->adest = *adest;

   /*Insert new node into the list*/
   p_adest_node->next  = db_node->adest_list;
   db_node->adest_list = p_adest_node;
    
   /*log message Adest create*/ 
   return p_adest_node;
}


/****************************************************************************
  Name          :  vds_new_vdest

  Description   :  This routine allocates new VDEST_DB_INFO_NODE, fills 
                   vdest_id and updates latest_allocated_vdest in VDS control 
                   block. 

  Arguments     :  vds_cb - pointer to VDS control block. 
                   info   -  pointer to request to create new VDEST structure.

  Return Values :  Success : VDS_VDEST_DB_INFO node. 
                   Failure : NULL 

  Notes         :  None
******************************************************************************/
static VDS_VDEST_DB_INFO  *vds_new_vdest(VDS_CB *vds_cb, 
                                         NCSVDA_VDEST_CREATE_INFO *info)
{
   VDS_VDEST_DB_INFO *db_info = NULL;
   uns32 rc = NCSCC_RC_SUCCESS;

   VDS_TRACE1_ARG1("vds_new_vdest\n");

   db_info = m_MMGR_ALLOC_VDS_DB_INFO;
   if (db_info == NULL)
   {
      m_VDS_LOG_MEM(VDS_LOG_MEM_VDS_DB_INFO,
                         VDS_LOG_MEM_ALLOC_FAILURE,
                                 NCSFL_SEV_ERROR);
      return NULL;
   }

   m_NCS_OS_MEMSET(db_info, 0, sizeof(*db_info));
   db_info->vdest_name = info->info.named.i_name;
   db_info->persistent = info->i_persistent;
   db_info->adest_list = NULL;
   db_info->name_pat_node.key_info = (uns8*)&db_info->vdest_name;

   rc = ncs_patricia_tree_add(&vds_cb->vdest_name_db, &db_info->name_pat_node);

   if(rc != NCSCC_RC_SUCCESS)
   {
     m_VDS_LOG_TREE(VDS_LOG_PAT_ADD_NAME,
                               VDS_LOG_PAT_FAILURE,
                                       NCSFL_SEV_ERROR, rc);
   }
   else
     m_VDS_LOG_TREE(VDS_LOG_PAT_ADD_NAME,
                             VDS_LOG_PAT_SUCCESS,
                                     NCSFL_SEV_INFO, rc);



   /* find an unused VDEST */
   do
   {
      vds_cb->latest_allocated_vdest++;
      if (m_MDS_GET_VDEST_ID_FROM_MDS_DEST(vds_cb->latest_allocated_vdest)>= NCSMDS_MAX_VDEST)
      {
         /* VDEST_ID have wrapped around */
         vds_cb->latest_allocated_vdest = NCSVDA_UNNAMED_MAX + 1;
      }
   } while (vds_lookup_by_vdest_id(vds_cb) != NULL);

   
   db_info->vdest_id = vds_cb->latest_allocated_vdest;
   db_info->id_pat_node.key_info = (uns8*)&db_info->vdest_id;
   
   if(db_info->persistent == FALSE)
   {
    m_VDS_LOG_VDEST(db_info->vdest_name.value,m_MDS_GET_VDEST_ID_FROM_MDS_DEST(vds_cb->latest_allocated_vdest),"FALSE");
   }
   else
   {
    m_VDS_LOG_VDEST(db_info->vdest_name.value,m_MDS_GET_VDEST_ID_FROM_MDS_DEST(vds_cb->latest_allocated_vdest),"TRUE");
   }
   
   rc = ncs_patricia_tree_add(&vds_cb->vdest_id_db, &db_info->id_pat_node);
   if(rc != NCSCC_RC_SUCCESS)
   {
     m_VDS_LOG_TREE(VDS_LOG_PAT_ADD_ID,
                             VDS_LOG_PAT_FAILURE,
                                       NCSFL_SEV_ERROR, rc);
   }
   else
     m_VDS_LOG_TREE(VDS_LOG_PAT_ADD_ID,
                             VDS_LOG_PAT_SUCCESS,
                                      NCSFL_SEV_INFO, rc);

   return db_info;
}


/****************************************************************************
  Name          : vds_lookup_by_vdest_id

  Description   : Returns vdest_db_info node identified by 
                  latest_allocated_vdest_id. 

  Arguments     : vds_cb - pointer to VDS control block.

  Return Values : Success : VDS_VDEST_DB_INFO node with vdest_id
                            (i.e latest allocated vdest_id).
                  Failure : NULL

  Notes         : None
******************************************************************************/
static VDS_VDEST_DB_INFO *vds_lookup_by_vdest_id(VDS_CB *vds_cb)
{
   NCS_PATRICIA_NODE *tmp = NULL;
   MDS_DEST *vdest_id;

   vdest_id = &vds_cb->latest_allocated_vdest;

   /* Gives Patricaia node with vdest_id as key */
   tmp = ncs_patricia_tree_get(&vds_cb->vdest_id_db,
                              (uns8*)vdest_id);
   if (tmp == NULL)
   {
      return NULL;
   }
 
   /* returns vdest_db_info_node identified by the  patricia node */
   return (VDS_VDEST_DB_INFO*)(((char*)tmp) - ID_PAT_NODE_OFFSET);
}


/****************************************************************************
  Name          : vds_getnext_by_vdest_id

  Description   : Gives next particia node from the patricia tree depending 
                  on vdest_id which acts as key.

  Arguments     : vds_cb     - pointer to VDS control block
                  io_db_node - pointer to vdest_id.

  Return Values : Success : NCSCC_RC_SUCCESS.
                  Failure : NCSCC_RC_FAILURE.

  Notes         : None.
******************************************************************************/
VDS_VDEST_DB_INFO *vds_getnext_by_vdest_id(VDS_CB *vds_cb, MDS_DEST *vdest_id)
{
   NCS_PATRICIA_NODE *tmp;
 
   tmp = ncs_patricia_tree_getnext(&vds_cb->vdest_id_db,
                                   (uns8*)vdest_id);
   if (tmp == NULL)
   {
      return NULL;
   }

   return (VDS_VDEST_DB_INFO*)(((char*)tmp) - ID_PAT_NODE_OFFSET);
}


/****************************************************************************
  Name          : vds_clean_non_persistent vdests.

  Description   : De-allocates adest nodes with given adest in all vdests.

  Arguments     : vds_cb - pointer to VDS control block.
                  adest_down - pointer to adest that went down.

  Return Values : None.

  Notes         : None.
******************************************************************************/
void vds_clean_non_persistent_vdests(VDS_CB *vds_cb, MDS_DEST *adest_down)
{
   MDS_DEST          *p_curr = NULL, curr;
   VDS_VDEST_DB_INFO *db_node = NULL;
   VDS_VDEST_ADEST_INFO **pp_adest_node;


   VDS_TRACE1_ARG1("vds_clean_non_persistent_vdest\n");
   
   p_curr = NULL;
   
   /* Gives root node from the tree with key vdest_id */
   db_node = vds_getnext_by_vdest_id(vds_cb, NULL);

   while (db_node != NULL)
   {
      curr = db_node->vdest_id;

      /* Searches adest_list  for node with adest value as adest_down  */
      pp_adest_node = vds_srch_adest_in_adest_list(&db_node->adest_list,
                                                   adest_down);
      if (pp_adest_node != NULL)
      {
         /* Free VDEST assignments to the ADEST went down. But set
            the "honor-persistent " flag to TRUE, so that persistent VDEST 
            are not destroyed */
        vds_free_vdest_assignment(vds_cb, &db_node, pp_adest_node, TRUE, TRUE);    
      }

      db_node = vds_getnext_by_vdest_id(vds_cb, &curr);
   }

   return;
}


/****************************************************************************
  Name          : vds_free_vdest_assignment

  Description   : De-allocates given ADEST node and also VDEST node if no more  
                  adests present.

  Arguments     : vds_cb - pointer to VDS control block
                  io_db_node - double pointer to VDS_VDEST_DB_INFO node
                  pp_adest_node - double pointer to adest_list 
                  honor_persistent_flag - persistent flag 

  Return Values : Success : NCSCC_RC_SUCCESS
                  Failure : NCSCC_RC_FAILURE 
  Notes         : None
******************************************************************************/
static uns32 vds_free_vdest_assignment(VDS_CB *vds_cb,
                                       VDS_VDEST_DB_INFO **io_db_node,
                                       VDS_VDEST_ADEST_INFO **pp_adest_node,
                                       NCS_BOOL honor_persistent_flag,
                                       NCS_BOOL ckpt)
{
   VDS_VDEST_ADEST_INFO *tmp_adest_node;
   VDS_VDEST_DB_INFO    *db_node = *io_db_node;
   uns32 rc = NCSCC_RC_SUCCESS;
   
   VDS_TRACE1_ARG1("vds_free_vdest_assignment\n");
   
   if (pp_adest_node == NULL)
      return NCSCC_RC_FAILURE;

   /* Detach this node from list and free it up*/
   tmp_adest_node = *pp_adest_node;
   
   (*pp_adest_node) = (*pp_adest_node)->next;
   
   if (db_node->adest_list == NULL)
   {
      if ((honor_persistent_flag == FALSE) ||
          (db_node->persistent == FALSE))
      {
         /* Deletes the section that had checkpointed with 
            this vdest_id as section id */
         if (ckpt)
         {
            rc = vds_ckpt_dbinfo_delete(vds_cb, &db_node->vdest_id);
            if (rc != NCSCC_RC_SUCCESS)
            {
                m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_DELETE,
                                    VDS_LOG_CKPT_FAILURE,
                                       NCSFL_SEV_ERROR, rc);
                /* Restore adest_node info */
                db_node->adest_list = tmp_adest_node;

                /* return from here */
                return rc;
            }
         }   

         /* Delete from VDEST-NAME-DB */
         ncs_patricia_tree_del(&vds_cb->vdest_name_db,
                               &db_node->name_pat_node);

         /* Delete from VDEST-DB */
         ncs_patricia_tree_del(&vds_cb->vdest_id_db,
                               &db_node->id_pat_node);

         /* Free the node */
         m_MMGR_FREE_VDS_DB_INFO(db_node);
         *io_db_node = db_node = NULL; /* precaution */
      }
   }
   else
   {
      /* Overwrites checkpointed  section with  modified adest_list  
         with this vdest_id */ 
         
      if (ckpt)
      {        
         rc = vds_ckpt_write(vds_cb, db_node, VDS_CKPT_OPERA_OVERWRITE);

         if (rc != NCSCC_RC_SUCCESS)
         {
            m_VDS_LOG_CKPT(VDS_LOG_CKPT_OVERWRITE,
                                   VDS_LOG_CKPT_FAILURE,
                                           NCSFL_SEV_ERROR, rc);
            /* Restore adest_list */
            tmp_adest_node->next = db_node->adest_list;
            db_node->adest_list = tmp_adest_node;

            /* return from here */
            return rc;
         }
         m_VDS_LOG_CKPT(VDS_LOG_CKPT_OVERWRITE,
                                  VDS_LOG_CKPT_SUCCESS,
                                          NCSFL_SEV_INFO, rc);
      }     
   }
   
   m_MMGR_FREE_VDS_ADEST_INFO(tmp_adest_node);
   
   tmp_adest_node = NULL; /* precaution */

   return rc;
}


/****************************************************************************
  Name          : vds_srch_adest_in_adest_list

  Description   : searches adest_list for a given adest 

  Arguments     : p_head - double pointer to adest_list   
                  adest  - pointer to adest 

  Return Values : Success :  adest node( VDS_VDEST_ADEST_INFO **).
                  Failure :  NULL.  

  Notes         : None
******************************************************************************/
static VDS_VDEST_ADEST_INFO 
**vds_srch_adest_in_adest_list(VDS_VDEST_ADEST_INFO **p_head,
                               MDS_DEST *adest)
{
   VDS_VDEST_ADEST_INFO **walk;

   /* searches adest_list for a given adest */
   for (walk = p_head; (*walk) != NULL; walk = &(*walk)->next)
   {
      if (m_NCS_MDS_DEST_EQUAL(&(*walk)->adest, adest))
         return walk;
   }

   return NULL;
}

