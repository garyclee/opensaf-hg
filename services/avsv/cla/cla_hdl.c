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

  DESCRIPTION:

  This file contains library routines for handle database.
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#include "cla.h"

static uns32 cla_hdl_cbk_dispatch_one(CLA_CB **, CLA_HDL_REC **);
static uns32 cla_hdl_cbk_dispatch_all(CLA_CB **, CLA_HDL_REC **);
static uns32 cla_hdl_cbk_dispatch_block(CLA_CB **, CLA_HDL_REC **);
static void cla_hdl_cbk_rec_prc (AVSV_CLM_CBK_INFO *, SaClmCallbacksT *);


/****************************************************************************
  Name          : cla_hdl_init
 
  Description   : This routine initializes the handle database.
 
  Arguments     : hdl_db  - ptr to the handle database
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 cla_hdl_init (CLA_HDL_DB *hdl_db)
{
   NCS_PATRICIA_PARAMS param;
   uns32               rc = NCSCC_RC_SUCCESS;

   /* init the hdl db tree */
   param.key_size = sizeof(uns32);
   param.info_size = 0;

   rc = ncs_patricia_tree_init(&hdl_db->hdl_db_anchor, &param);
   if (NCSCC_RC_SUCCESS == rc)
      hdl_db->num = 0;

   return rc;
}


/****************************************************************************
  Name          : cla_hdl_del
 
  Description   : This routine deletes the handle database.
 
  Arguments     : cb  - ptr to the CLA control block
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
void cla_hdl_del (CLA_CB *cb)
{
   CLA_HDL_DB  *hdl_db = &cb->hdl_db;
   CLA_HDL_REC *hdl_rec = 0;

   /* scan the entire handle db & delete each record */
   while( (hdl_rec = (CLA_HDL_REC *)
                     ncs_patricia_tree_getnext(&hdl_db->hdl_db_anchor, 0) ) )
   {      
      cla_hdl_rec_del(cb, hdl_db, hdl_rec);
   }

   /* there shouldn't be any record left */
   m_AVSV_ASSERT(!hdl_db->num);

   /* destroy the hdl db tree */
   ncs_patricia_tree_destroy(&hdl_db->hdl_db_anchor);

   return;
}


/****************************************************************************
  Name          : cla_hdl_rec_del
 
  Description   : This routine deletes the handle record.
 
  Arguments     : cb      - ptr tot he CLA control block
                  hdl_db  - ptr to the hdl db
                  hdl_rec - ptr to the hdl record
 
  Return Values : None
 
  Notes         : The selection object is destroyed after all the means to 
                  access the handle record (ie. hdl db tree or hdl mngr) is 
                  removed. This is to disallow the waiting thread to access 
                  the hdl rec while other thread executes saClmFinalize on it.
******************************************************************************/
void cla_hdl_rec_del (CLA_CB *cb, CLA_HDL_DB *hdl_db, CLA_HDL_REC *hdl_rec)
{
   uns32 hdl = 0; 
   hdl = hdl_rec->hdl;

   /* pop the hdl rec */
   ncs_patricia_tree_del(&hdl_db->hdl_db_anchor, &hdl_rec->hdl_node);

   /* clean the pend callbk list */
   cla_hdl_cbk_list_del(cb, hdl_rec);

   /* clean the track notification buffer TBD */

   /* destroy the selection object */
   if (m_GET_FD_FROM_SEL_OBJ(hdl_rec->sel_obj))
   {
      m_NCS_SEL_OBJ_RAISE_OPERATION_SHUT(&hdl_rec->sel_obj);
     
      /* remove the association with hdl-mngr */
      ncshm_destroy_hdl(NCS_SERVICE_ID_CLA, hdl_rec->hdl);
     
      m_NCS_SEL_OBJ_RMV_OPERATION_SHUT(&hdl_rec->sel_obj);
   }
   /* free the hdl rec */
   m_MMGR_FREE_CLA_HDL_REC(hdl_rec);

   /* update the no of records */
   hdl_db->num--;

   m_CLA_LOG_HDL_DB(CLA_LOG_HDL_DB_REC_DEL, CLA_LOG_HDL_DB_SUCCESS, 
                    hdl, NCSFL_SEV_INFO);

   return;
}


/****************************************************************************
  Name          : cla_hdl_cbk_list_del
 
  Description   : This routine scans the pending callbk list & deletes all 
                  the records.
 
  Arguments     : cb   - ptr to the CLA control block
                  list - ptr to the pending callbk list
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
void cla_hdl_cbk_list_del (CLA_CB *cb, CLA_HDL_REC *hdl_rec )
{
   CLA_PEND_CBK *list = NULL;
   CLA_PEND_CBK_REC *rec = 0;

   if(hdl_rec == NULL)
      return;

   list = &hdl_rec->pend_cbk;

   /* pop & delete all the records */
   do
   {
      m_CLA_HDL_PEND_CBK_POP(list, rec);
      if (!rec) break;

      /* remove the selection object indication */
      if ( -1 == m_NCS_SEL_OBJ_RMV_IND(hdl_rec->sel_obj, TRUE, TRUE) )
      {
         /* log */
         m_AVSV_ASSERT(0);
      }

      /* clean the record */
      m_MMGR_FREE_AVSV_CLM_CBK_INFO(rec->cbk_info); /* clean the notify buffer too TBD */

      /* delete the record */
      m_MMGR_FREE_CLA_PEND_CBK_REC(rec);
   } while (1);

   /* there shouldn't be any record left */
   m_AVSV_ASSERT((!list->num) && (!list->head) && (!list->tail));

   return;
}


/****************************************************************************
  Name          : cla_hdl_rec_add
 
  Description   : This routine adds the handle record to the handle db.
 
  Arguments     : cb       - ptr tot he CLA control block
                  hdl_db   - ptr to the hdl db
                  reg_cbks - ptr to the set of registered callbacks
 
  Return Values : ptr to the handle record
 
  Notes         : None
******************************************************************************/
CLA_HDL_REC *cla_hdl_rec_add (CLA_CB *cb, 
                              CLA_HDL_DB *hdl_db, 
                              const SaClmCallbacksT *reg_cbks)
{
   CLA_HDL_REC *rec = 0;

   /* allocate the hdl rec */
   if ( !(rec = m_MMGR_ALLOC_CLA_HDL_REC) )
       goto error;

   m_NCS_OS_MEMSET(rec, 0, sizeof(CLA_HDL_REC));

   /* create the association with hdl-mngr */
   if ( !(rec->hdl = ncshm_create_hdl(cb->pool_id, NCS_SERVICE_ID_CLA, 
                                      (NCSCONTEXT)rec)) )
      goto error;

   /* create the selection object & store it in hdl rec */
   m_NCS_SEL_OBJ_CREATE(&rec->sel_obj);
   if (!m_GET_FD_FROM_SEL_OBJ(rec->sel_obj))
      goto error;
   
   /* store the registered callbacks */
   if (reg_cbks)
       m_NCS_OS_MEMCPY((void *)&rec->reg_cbk, (void *)reg_cbks, 
                       sizeof(SaClmCallbacksT));

   /* add the record to the hdl db */
   rec->hdl_node.key_info = (uns8 *)&rec->hdl;
   if (ncs_patricia_tree_add(&hdl_db->hdl_db_anchor, &rec->hdl_node)
       != NCSCC_RC_SUCCESS)
       goto error;

   /* update the no of records */
   hdl_db->num++;

   m_CLA_LOG_HDL_DB(CLA_LOG_HDL_DB_REC_ADD, CLA_LOG_HDL_DB_SUCCESS, 
                    rec->hdl, NCSFL_SEV_INFO);

   return rec;

error:
   if (rec) 
   {
     /* remove the association with hdl-mngr */
      if (rec->hdl)
         ncshm_destroy_hdl(NCS_SERVICE_ID_CLA, rec->hdl);

      /* destroy the selection object */
      if (m_GET_FD_FROM_SEL_OBJ(rec->sel_obj))
         m_NCS_SEL_OBJ_DESTROY(rec->sel_obj);

      m_MMGR_FREE_CLA_HDL_REC(rec);
   }
   return 0;
}


/****************************************************************************
  Name          : cla_hdl_cbk_param_add
 
  Description   : This routine adds the callback parameters to the pending 
                  callback list.
 
  Arguments     : cb       - ptr to the CLA control block
                  hdl_rec  - ptr to the handle record
                  cbk_info - ptr to the callback parameters
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : This routine reuses the callback info ptr that is received 
                  from MDS thus avoiding an extra copy.
******************************************************************************/
uns32 cla_hdl_cbk_param_add (CLA_CB *cb, CLA_HDL_REC *hdl_rec, 
                             AVSV_CLM_CBK_INFO *cbk_info)
{
   CLA_PEND_CBK_REC *rec = 0;
   uns32            rc = NCSCC_RC_SUCCESS;

   /* allocate the callbk rec */
   if ( !(rec = m_MMGR_ALLOC_CLA_PEND_CBK_REC) )
   {
      rc = NCSCC_RC_FAILURE; /* Need to define RC enums TBD */
      goto done;
   }
   m_NCS_OS_MEMSET(rec, 0, sizeof(CLA_PEND_CBK_REC));

   /* populate the callbk parameters */
   rec->cbk_info = cbk_info;

   /* now push it to the pending list */
   m_CLA_HDL_PEND_CBK_PUSH(&(hdl_rec->pend_cbk), rec);

   /* send an indication to the application */
   if ( NCSCC_RC_SUCCESS != (rc = m_NCS_SEL_OBJ_IND(hdl_rec->sel_obj)) )
   {
      /* log */
      m_AVSV_ASSERT(0);
   }

done:
   if ( (NCSCC_RC_SUCCESS != rc) && rec )
      m_MMGR_FREE_CLA_PEND_CBK_REC(rec);

   m_CLA_LOG_HDL_DB(CLA_LOG_HDL_DB_REC_CBK_ADD, CLA_LOG_HDL_DB_SUCCESS, 
                    hdl_rec->hdl, NCSFL_SEV_INFO);

   return rc;
}


/****************************************************************************
  Name          : cla_hdl_cbk_dispatch
 
  Description   : This routine dispatches the pending callbacks as per the 
                  dispatch flags.
 
  Arguments     : cb      - ptr to the CLA control block
                  hdl_rec - ptr to the handle record
                  flags   - dispatch flags
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 cla_hdl_cbk_dispatch (CLA_CB           **cb, 
                            CLA_HDL_REC      **hdl_rec, 
                            SaDispatchFlagsT flags)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   switch (flags)
   {
   case SA_DISPATCH_ONE:
       rc = cla_hdl_cbk_dispatch_one(cb, hdl_rec);
      break;

   case SA_DISPATCH_ALL:
       rc = cla_hdl_cbk_dispatch_all(cb, hdl_rec);
      break;

   case SA_DISPATCH_BLOCKING:
       rc = cla_hdl_cbk_dispatch_block(cb, hdl_rec);
      break;

   default:
      m_AVSV_ASSERT(0);
   } /* switch */

   return rc;
}


/****************************************************************************
  Name          : cla_hdl_cbk_dispatch_one
 
  Description   : This routine dispatches one pending callback.
 
  Arguments     : cb      - ptr to the CLA control block
                  hdl_rec - ptr to the handle record
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 cla_hdl_cbk_dispatch_one (CLA_CB **cb, CLA_HDL_REC **hdl_rec)
{
   CLA_PEND_CBK     *list = &(*hdl_rec)->pend_cbk;
   CLA_PEND_CBK_REC *rec = 0;
   uns32 hdl = (*hdl_rec)->hdl;
   SaClmCallbacksT reg_cbk;
   uns32 rc = NCSCC_RC_SUCCESS;

   m_NCS_MEMSET(&reg_cbk, 0, sizeof(SaClmCallbacksT));
   m_NCS_MEMCPY(&reg_cbk, &(*hdl_rec)->reg_cbk,sizeof(SaClmCallbacksT));

   /* pop the rec from the list */
   m_CLA_HDL_PEND_CBK_POP(list, rec);
   if (rec)
   {
       /* remove the selection object indication */
      if ( -1 == m_NCS_SEL_OBJ_RMV_IND((*hdl_rec)->sel_obj, TRUE, TRUE) )
      {
         /* log */
         m_AVSV_ASSERT(0);
      }

     /* release the cb lock & return the hdls to the hdl-mngr */
      m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
      ncshm_give_hdl(hdl);
                        
      /* process the callback list record */
      cla_hdl_cbk_rec_prc(rec->cbk_info, &reg_cbk);

      /* now that we are done with this rec, free the resources */
      m_MMGR_FREE_AVSV_CLM_CBK_INFO(rec->cbk_info); /* clean the notify buffer too TBD */
      m_MMGR_FREE_CLA_PEND_CBK_REC(rec);

      m_NCS_LOCK(&(*cb)->lock, NCS_LOCK_WRITE);
      *hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_CLA, hdl);
   }

   return rc;
}


/****************************************************************************
  Name          : cla_hdl_cbk_dispatch_all
 
  Description   : This routine dispatches all the pending callbacks.
 
  Arguments     : cb      - ptr to the CLA control block
                  hdl_rec - ptr to the handle record
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 cla_hdl_cbk_dispatch_all (CLA_CB **cb, CLA_HDL_REC **hdl_rec)
{
   CLA_PEND_CBK     *list = &(*hdl_rec)->pend_cbk;
   CLA_PEND_CBK_REC *rec = 0;
   uns32 hdl = (*hdl_rec)->hdl;
   SaClmCallbacksT reg_cbk;
   uns32 rc = NCSCC_RC_SUCCESS;

   m_NCS_MEMSET(&reg_cbk, 0, sizeof(SaClmCallbacksT));
   m_NCS_MEMCPY(&reg_cbk, &(*hdl_rec)->reg_cbk,sizeof(SaClmCallbacksT));

   /* pop all the records from the list & process them */
   do
   {
      m_CLA_HDL_PEND_CBK_POP(list, rec);
      if (!rec) break;

      /* remove the selection object indication */
      if ( -1 == m_NCS_SEL_OBJ_RMV_IND((*hdl_rec)->sel_obj, TRUE, TRUE) )
      {
         /* log */
         m_AVSV_ASSERT(0);
      }

      /* release the cb lock & return the hdls to the hdl-mngr */
      m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
      ncshm_give_hdl(hdl);

      /* process the callback list record */
      cla_hdl_cbk_rec_prc(rec->cbk_info, &reg_cbk);

      /* now that we are done with this rec, free the resources */
      m_MMGR_FREE_AVSV_CLM_CBK_INFO(rec->cbk_info); /* clean the notify buffer too TBD */
      m_MMGR_FREE_CLA_PEND_CBK_REC(rec);

      m_NCS_LOCK(&(*cb)->lock, NCS_LOCK_WRITE);

      /* is it finalized ? */
      if(!(*hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_CLA, hdl)))
         break;

   } while (1);

   return rc;
}


/****************************************************************************
  Name          : cla_hdl_cbk_dispatch_block
 
  Description   : This routine blocks forever for receiving indications from 
                  AvND. The routine returns when saClmFinalize is executed on 
                  the handle.
 
  Arguments     : cb      - ptr to the CLA control block
                  hdl_rec - ptr to the handle record
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 cla_hdl_cbk_dispatch_block (CLA_CB **cb, CLA_HDL_REC **hdl_rec)
{
   CLA_PEND_CBK     *list = &(*hdl_rec)->pend_cbk;
   uns32            hdl = (*hdl_rec)->hdl;
   CLA_PEND_CBK_REC *rec = 0;
   NCS_SEL_OBJ_SET  all_sel_obj;
   NCS_SEL_OBJ      sel_obj = (*hdl_rec)->sel_obj;
   SaClmCallbacksT  reg_cbk;
   uns32            rc = SA_AIS_OK;

   m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
   m_NCS_SEL_OBJ_SET(sel_obj,&all_sel_obj); 
   
   m_NCS_MEMSET(&reg_cbk, 0, sizeof(SaClmCallbacksT));
   m_NCS_MEMCPY(&reg_cbk, &(*hdl_rec)->reg_cbk,sizeof(SaClmCallbacksT));

   /* release all lock and handle - we are abt to go into deep sleep */
   m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);

   while(m_NCS_SEL_OBJ_SELECT(sel_obj,&all_sel_obj,0,0,0) != -1)
   {
      ncshm_give_hdl(hdl);
      m_NCS_LOCK(&(*cb)->lock, NCS_LOCK_WRITE);
    
      /* get all handle and lock */
      *hdl_rec = (CLA_HDL_REC *)ncshm_take_hdl(NCS_SERVICE_ID_CLA, hdl);
   
      if(!(*hdl_rec))
      {
         m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
         break;
      }
   
      if (!m_NCS_SEL_OBJ_ISSET(sel_obj,&all_sel_obj))
      {
         rc = SA_AIS_ERR_LIBRARY;
         m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
         break;
      }

      /* pop rec */
      m_CLA_HDL_PEND_CBK_POP(list, rec);
      if (!rec) 
      {
         m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
         break;
      }

      /* remove the selection object indication */
      if ( -1 == m_NCS_SEL_OBJ_RMV_IND((*hdl_rec)->sel_obj, TRUE, TRUE) )
         m_AVSV_ASSERT(0);
         
      /* release the cb lock & return the hdls to the hdl-mngr */
      ncshm_give_hdl(hdl);
      m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);

      /* process the callback list record */
      cla_hdl_cbk_rec_prc(rec->cbk_info,&reg_cbk);

      /* now that we are done with this rec, free the resources */
      m_MMGR_FREE_AVSV_CLM_CBK_INFO(rec->cbk_info); /* clean the notify buffer too TBD */
      m_MMGR_FREE_CLA_PEND_CBK_REC(rec);

      m_NCS_LOCK(&(*cb)->lock, NCS_LOCK_WRITE);
      *hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_CLA, hdl);
      if(*hdl_rec)
      {
         m_NCS_SEL_OBJ_SET(sel_obj,&all_sel_obj);
      }else
      {
         m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
         break;
      }
      m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
   }/*while*/
   if(*hdl_rec) 
      ncshm_give_hdl(hdl); 

   m_NCS_LOCK(&(*cb)->lock, NCS_LOCK_WRITE);
   *hdl_rec = (CLA_HDL_REC *)ncshm_take_hdl(NCS_SERVICE_ID_CLA, hdl);

   return rc;
}


/****************************************************************************
  Name          : cla_hdl_cbk_rec_prc
 
  Description   : This routine invokes the registered callback routine & frees
                  the callback record resources.
 
  Arguments     : cb      - ptr to the CLA control block
                  rec     - ptr to the callback record
                  reg_cbk - ptr to the registered callbacks
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
void cla_hdl_cbk_rec_prc ( AVSV_CLM_CBK_INFO *info, SaClmCallbacksT  *reg_cbk)
{

   /* invoke the corresponding callback */
   switch (info->type)
   {
   case AVSV_CLM_CBK_TRACK:
      {
         AVSV_CLM_CBK_TRACK_PARAM *track = &info->param.track;

         if (reg_cbk->saClmClusterTrackCallback)
             reg_cbk->saClmClusterTrackCallback(&track->notify, 
                                                track->mem_num,
                                                track->err);
      }
      break;

   case AVSV_CLM_CBK_NODE_ASYNC_GET:
      {
         AVSV_CLM_CBK_NODE_ASYNC_GET_PARAM *node_get = &info->param.node_get;

         if (reg_cbk->saClmClusterNodeGetCallback)
             reg_cbk->saClmClusterNodeGetCallback(node_get->inv,
                                                  &node_get->node,
                                                  node_get->err);
      }
      break;

   default:
      m_AVSV_ASSERT(0);
      break;
   } /* switch */

   return;
}
