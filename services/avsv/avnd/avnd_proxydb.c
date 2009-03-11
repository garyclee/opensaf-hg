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

  DESCRIPTION: This file contains internode & external proxy component's
               data base creation, accessing and deletion of functions.

  FUNCTIONS INCLUDED in this module:
  
****************************************************************************/
#include "avnd.h"

/******************************************************************************
  Name          : avnd_nodeid_mdsdest_rec_add
 
  Description   : This routine adds NODE_ID to MDS_DEST record for a particular
                  AvND.
 
  Arguments     : cb  - ptr to the AvND control block.
                  mds_dest - Mds dest of AvND coming up.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_nodeid_mdsdest_rec_add (AVND_CB *cb, MDS_DEST mds_dest)
{
   AVND_NODEID_TO_MDSDEST_MAP * rec = NULL;
   NODE_ID   node_id  = 0; 
   uns32     res      = NCSCC_RC_SUCCESS;

   node_id = m_NCS_NODE_ID_FROM_MDS_DEST(mds_dest);

   rec = (AVND_NODEID_TO_MDSDEST_MAP *)ncs_patricia_tree_get(&cb->nodeid_mdsdest_db, 
                                       (uns8 *)&(node_id));
   if(rec != NULL)
   {
       m_AVND_AVND_ERR_LOG(
                   "nodeid_mdsdest rec already exists, Rec Add Failed: MdsDest and NodeId are",
                    NULL,mds_dest,node_id,0,0);
       return NCSCC_RC_FAILURE; 
   }
   else
   {
      rec = m_MMGR_ALLOC_AVND_NODEID_MDSDEST;

      if(rec == NULL)
      {
        return NCSCC_RC_FAILURE;
      }
      else
      {
        rec->node_id = node_id;
        rec->mds_dest = mds_dest;
        rec->tree_node.bit = 0;
        rec->tree_node.key_info = (uns8*)&(rec->node_id);

        res = ncs_patricia_tree_add(&cb->nodeid_mdsdest_db, &rec->tree_node);

        if ( NCSCC_RC_SUCCESS != res )
        {
           m_AVND_AVND_ERR_LOG(
                 "Couldn't add nodeid_mdsdest rec, patricia add failed: MdsDest and NodeId are",
                  NULL,mds_dest,node_id,0,0);
           m_MMGR_FREE_AVND_NODEID_MDSDEST(rec); 
           return res;
       }

       } /* Else of if(rec == NULL)  */

   } /* Else of if(rec != NULL)  */

   m_AVND_AVND_SUCC_LOG("nodeid_mdsdest rec added: MdsDest and NodeId are",
                         NULL,mds_dest,node_id,0,0);
 return res;

}

/******************************************************************************
  Name          : avnd_nodeid_mdsdest_rec_del
 
  Description   : This routine delete NODE_ID to MDS_DEST record for a particular
                  AvND.
 
  Arguments     : cb  - ptr to the AvND control block.
                  mds_dest - Mds dest of AvND coming up.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_nodeid_mdsdest_rec_del (AVND_CB *cb, MDS_DEST mds_dest)
{
   AVND_NODEID_TO_MDSDEST_MAP * rec = NULL;
   NODE_ID   node_id  = 0; 
   uns32     res      = NCSCC_RC_SUCCESS;

   node_id = m_NCS_NODE_ID_FROM_MDS_DEST(mds_dest);

   rec = (AVND_NODEID_TO_MDSDEST_MAP *)ncs_patricia_tree_get(&cb->nodeid_mdsdest_db, 
                                         (uns8 *)&(node_id));
   if(rec == NULL)
   {
       m_AVND_AVND_ERR_LOG(
               "nodeid_mdsdest rec doesn't exist, Rec del failed: MdsDest and NodeId are",
                NULL,mds_dest,node_id,0,0);
       return NCSCC_RC_FAILURE; 
   }
   else
   {
       res = ncs_patricia_tree_del(&cb->nodeid_mdsdest_db, &rec->tree_node);

       if ( NCSCC_RC_SUCCESS != res )
       {
           m_AVND_AVND_ERR_LOG(
           "Couldn't del nodeid_mdsdest rec, patricia del failed: MdsDest,NodeId and res are",
                  NULL,mds_dest,node_id,res,0);
            return res;
       }

   } /* Else of if(rec == NULL) */

   m_MMGR_FREE_AVND_NODEID_MDSDEST(rec); 

   m_AVND_AVND_SUCC_LOG("nodeid_mdsdest rec deleted: MdsDest and NodeId are",
                        NULL,mds_dest,node_id,0,0);
   return res;
}

/******************************************************************************
  Name          : avnd_get_mds_dest_from_nodeid
 
  Description   : This routine return mds dest of AvND for a particular node id.
 
  Arguments     : cb  - ptr to the AvND control block.
                  mds_dest - Mds dest of AvND coming up.
 
  Return Values : 0/mds dest.
 
  Notes         : None
******************************************************************************/
MDS_DEST avnd_get_mds_dest_from_nodeid (AVND_CB *cb, NODE_ID node_id)
{
   AVND_NODEID_TO_MDSDEST_MAP * rec = NULL;

   rec = (AVND_NODEID_TO_MDSDEST_MAP *)ncs_patricia_tree_get(&cb->nodeid_mdsdest_db, 
                                       (uns8 *)&(node_id));
   if(rec == NULL)
   {
       m_AVND_AVND_ERR_LOG(
               "nodeid_mdsdest rec doesn't exist, Rec get failed: NodeId is",
                NULL,node_id,0,0,0);
       return 0; 
   }
   
   return rec->mds_dest;
}

/****************************************************************************
  Name          : avnd_nodeid_to_mdsdest_map_db_init
 
  Description   : This routine initializes the node_id to mds dest map database.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_nodeid_to_mdsdest_map_db_init(AVND_CB *cb)
{
   NCS_PATRICIA_PARAMS params;
   uns32               rc = NCSCC_RC_SUCCESS;

   m_NCS_OS_MEMSET(&params, 0, sizeof(NCS_PATRICIA_PARAMS));

   params.key_size = sizeof(NODE_ID);
   rc = ncs_patricia_tree_init(&cb->nodeid_mdsdest_db, &params);

   if (NCSCC_RC_SUCCESS != rc)
   {
       m_AVND_AVND_ERR_LOG("nodeid_mdsdest_db initialization failed. Rc is ",
                           NULL,rc,0,0,0);
   }

   return rc;
}

/****************************************************************************
  Name          : avnd_nodeid_to_mdsdest_map_db_destroy
 
  Description   : This routine destroys the node_id to mds dest mapping database.
                  It deletes all the mapping records from the database.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_nodeid_to_mdsdest_map_db_destroy(AVND_CB *cb)
{
   AVND_NODEID_TO_MDSDEST_MAP *mapping = 0;
   uns32   rc = NCSCC_RC_SUCCESS;

   /* scan & delete each su */
   while ( 0 != (mapping = 
         (AVND_NODEID_TO_MDSDEST_MAP *)ncs_patricia_tree_getnext(&cb->nodeid_mdsdest_db, 
         (uns8 *)0)) )
   {
      /* delete the record */
      rc = avnd_nodeid_mdsdest_rec_del(cb, mapping->node_id);
      if ( NCSCC_RC_SUCCESS != rc ) goto err;
   }

   /* finally destroy patricia tree */
   rc = ncs_patricia_tree_destroy(&cb->nodeid_mdsdest_db);
   if ( NCSCC_RC_SUCCESS != rc ) goto err;

   return rc;

err:

   m_AVND_AVND_ERR_LOG("nodeid_to_mdsdest_map_db_destroy failed. rc is",
                       NULL,rc,0,0,0);
   return rc;
}

/****************************************************************************
  Name          : avnd_internode_avail_comp_db_init
 
  Description   : This routine initializes the available internode components database.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_internode_avail_comp_db_init(AVND_CB *cb)
{
   NCS_PATRICIA_PARAMS params;
   uns32               rc = NCSCC_RC_SUCCESS;

   m_NCS_OS_MEMSET(&params, 0, sizeof(NCS_PATRICIA_PARAMS));

   params.key_size = sizeof(SaNameT);
   rc = ncs_patricia_tree_init(&cb->internode_avail_comp_db, &params);

   if (NCSCC_RC_SUCCESS != rc)
   {
      m_AVND_AVND_ERR_LOG("internode_avail_comp_db initialization failed. Rc is ",
                          NULL,rc,0,0,0);
   }

   return rc;
}

/****************************************************************************
  Name          : avnd_internode_avail_comp_db_destroy
 
  Description   : This routine destroys the available internode components database.
                  It deletes all the components records from the database.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_internode_avail_comp_db_destroy(AVND_CB *cb)
{
   AVND_COMP *comp = 0;
   uns32   rc = NCSCC_RC_SUCCESS;

   /* scan & delete each su */
   while ( 0 != (comp = 
            (AVND_COMP *)ncs_patricia_tree_getnext(&cb->internode_avail_comp_db, 
            (uns8 *)0)) )
   {
      /* delete the record */
      m_AVND_SEND_CKPT_UPDT_ASYNC_RMV(cb, comp, AVND_CKPT_COMP_CONFIG);
      rc = avnd_internode_comp_del(cb, &cb->internode_avail_comp_db, &comp->name_net);
      if ( NCSCC_RC_SUCCESS != rc ) goto err;
   }

   /* finally destroy patricia tree */
   rc = ncs_patricia_tree_destroy(&cb->internode_avail_comp_db);
   if ( NCSCC_RC_SUCCESS != rc ) goto err;

   return rc;

err:

   m_AVND_AVND_ERR_LOG("internode_avail_comp_db_destroy failed. rc is",NULL,rc,0,0,0);
   return rc;
}

/******************************************************************************
  Name          : avnd_internode_comp_add
 
  Description   : This routine adds an internode component in 
                  internode_avail_comp_db.
 
  Arguments     : ptree   - ptr to the patricia tree of data base.
                  name    - ptr to the component name.
                  node id - node id of the component.
                  rc      - out param for operation result
                  pxy_for_ext_comp - Whether this is a proxy for external
                  component.
                  comp_is_proxy - Whether internode component is proxy or
                                  proxied. 1 is for proxy, 0 is for proxied.
 
  Return Values : Pointer to AVND_COMP data structure
 
  Notes         : None
******************************************************************************/
AVND_COMP * avnd_internode_comp_add (NCS_PATRICIA_TREE *ptree, SaNameT *name,
                                     NODE_ID  node_id, uns32 * rc, 
                                     NCS_BOOL pxy_for_ext_comp, NCS_BOOL comp_is_proxy)
{
   AVND_COMP *comp = 0;

   *rc = SA_AIS_OK;
   
   /* verify if this component is already present in the db */
   if (NULL != (comp = m_AVND_COMPDB_REC_GET(*ptree, *name)))
   {
      /* This is a proxy and already proxying at least one component. 
         So, no problem.*/
      *rc = SA_AIS_ERR_EXIST;
      m_AVND_AVND_DEBUG_LOG("avnd_internode_comp_add already exists. Comp and NodeId are",
                       name,node_id,0,0,0);
      return comp;
   }

   /* a fresh comp... */
   comp = m_MMGR_ALLOC_AVND_COMP;
   if (!comp) 
   {
     *rc = SA_AIS_ERR_NO_MEMORY;
     goto err;
   }
   
   m_NCS_OS_MEMSET(comp, 0, sizeof(AVND_COMP));
       
   /* update the comp-name (patricia key) */
   memcpy(&comp->name_net, name, sizeof(SaNameT));
       
   comp->pres = NCS_PRES_UNINSTANTIATED;

   if(0 == node_id)
   {
     /* This means this is an external component. */
     m_AVND_COMP_TYPE_SET_EXT_CLUSTER(comp);
   }

   m_AVND_COMP_TYPE_SET_INTER_NODE(comp);
  
   if(TRUE == pxy_for_ext_comp)
   {
     m_AVND_PROXY_FOR_EXT_COMP_SET(comp);
   }

   if(TRUE == comp_is_proxy)
   {
     m_AVND_COMP_TYPE_PROXY_SET(comp);
   }
   else if(FALSE == comp_is_proxy)
   {
     m_AVND_COMP_TYPE_PROXIED_SET(comp);
   }

   comp->node_id = node_id;

   /* initialize proxied list */
   avnd_pxied_list_init(comp); 
   
   /* Add to the patricia tree. */
   comp->tree_node.bit = 0;
   comp->tree_node.key_info = (uns8*)&comp->name_net;
   *rc = ncs_patricia_tree_add(ptree, &comp->tree_node);
   if ( NCSCC_RC_SUCCESS != *rc )
   {
      *rc = SA_AIS_ERR_NO_MEMORY;
      goto err;
   }

   m_AVND_AVND_DEBUG_LOG(
    "avnd_internode_comp_add:Comp, nodeid, pxy_for_ext_comp and comp_is_proxy", 
                         &comp->name_net,node_id,pxy_for_ext_comp,comp_is_proxy,0);
return comp;
   
err:

   if (comp) 
   {
     m_MMGR_FREE_AVND_COMP(comp);
   }

   m_AVND_AVND_ERR_LOG("avnd_internode_comp_add failed. Comp and NodeId are",
                       name,node_id,0,0,0);
return 0;

}

/******************************************************************************
  Name          : avnd_internode_comp_del
 
  Description   : This routine deletes an internode component from internode_avail_comp_db.
 
  Arguments     : ptree  - ptr to the patricia tree of data base.
                  name_net - ptr to the component name.
                        
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 avnd_internode_comp_del (AVND_CB *cb, NCS_PATRICIA_TREE *ptree, SaNameT *name_net)
{
   AVND_COMP *comp = 0;
   uns32         rc = NCSCC_RC_SUCCESS;
   AVND_COMP_CBK *cbk_rec = NULL, *temp_cbk_ptr = NULL;

   /* get the comp */
   comp = m_AVND_COMPDB_REC_GET(*ptree, *name_net);
   if (!comp)
   {
      rc = AVND_ERR_NO_COMP;
      m_AVND_AVND_ERR_LOG("internode_comp_del failed. Rec doesn't exist. Comp is",
                           name_net,0,0,0,0);
      goto err;
   }
   m_AVND_AVND_ENTRY_LOG("avnd_internode_comp_del:Comp, nodeid and comp_type", 
                         &comp->name_net,comp->node_id,comp->comp_type,0,0);

/*  Delete the callbacks if any. */
    cbk_rec = comp->cbk_list;
    while(cbk_rec)
    {
      temp_cbk_ptr = cbk_rec->next;
      m_AVND_SEND_CKPT_UPDT_ASYNC_RMV(cb, cbk_rec, AVND_CKPT_COMP_CBK_REC);
      avnd_comp_cbq_rec_del(cb, comp, cbk_rec);
      cbk_rec = temp_cbk_ptr;
    }

   /* 
   * Remove from the patricia tree.
   */
   rc = ncs_patricia_tree_del(ptree, &comp->tree_node);
   if ( NCSCC_RC_SUCCESS != rc )
   {
      rc = AVND_ERR_TREE;
      goto err;
   }

   /* free the memory */
   if(comp) m_MMGR_FREE_AVND_COMP(comp);
return rc;

err:
   
   /* free the memory */
   if(comp) m_MMGR_FREE_AVND_COMP(comp);
   
   m_AVND_AVND_ERR_LOG("internode_comp_del failed. Comp and rc are",name_net,rc,0,0,0);

   return rc;

}

