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

#include "ava.h"

static uns32 ava_hdl_cbk_dispatch_one(AVA_CB **, AVA_HDL_REC **);
static uns32 ava_hdl_cbk_dispatch_all(AVA_CB **, AVA_HDL_REC **);
static uns32 ava_hdl_cbk_dispatch_block(AVA_CB **, AVA_HDL_REC **);

static void ava_hdl_cbk_rec_prc (AVSV_AMF_CBK_INFO *, SaAmfCallbacksT *);


static void ava_hdl_cbk_list_del(AVA_CB *, AVA_PEND_CBK *);
static void ava_hdl_pend_resp_list_del(AVA_CB *, AVA_PEND_CBK *);

/****************************************************************************
  Name          : ava_hdl_init
 
  Description   : This routine initializes the handle database.
 
  Arguments     : hdl_db  - ptr to the handle database
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 ava_hdl_init (AVA_HDL_DB *hdl_db)
{
   NCS_PATRICIA_PARAMS param;
   uns32               rc = NCSCC_RC_SUCCESS;

   memset(&param, 0, sizeof(NCS_PATRICIA_PARAMS));

   /* init the hdl db tree */
   param.key_size = sizeof(uns32);
   param.info_size = 0;

   rc = ncs_patricia_tree_init(&hdl_db->hdl_db_anchor, &param);
   if (NCSCC_RC_SUCCESS == rc)
      hdl_db->num = 0;

   return rc;
}


/****************************************************************************
  Name          : ava_hdl_del
 
  Description   : This routine deletes the handle database.
 
  Arguments     : cb  - ptr to the AvA control block
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
void ava_hdl_del (AVA_CB *cb)
{
   AVA_HDL_DB  *hdl_db = &cb->hdl_db;
   AVA_HDL_REC *hdl_rec = 0;

   /* scan the entire handle db & delete each record */
   while( (hdl_rec = (AVA_HDL_REC *)
                     ncs_patricia_tree_getnext(&hdl_db->hdl_db_anchor, 0) ) )
   {      
      ava_hdl_rec_del(cb, hdl_db, hdl_rec);
   }

   /* there shouldn't be any record left */
   assert(!hdl_db->num);

   /* destroy the hdl db tree */
   ncs_patricia_tree_destroy(&hdl_db->hdl_db_anchor);

   return;
}


/****************************************************************************
  Name          : ava_hdl_rec_del
 
  Description   : This routine deletes the handle record.
 
  Arguments     : cb      - ptr tot he AvA control block
                  hdl_db  - ptr to the hdl db
                  hdl_rec - ptr to the hdl record
 
  Return Values : None
 
  Notes         : The selection object is destroyed after all the means to 
                  access the handle record (ie. hdl db tree or hdl mngr) is 
                  removed. This is to disallow the waiting thread to access 
                  the hdl rec while other thread executes saAmfFinalize on it.
******************************************************************************/
void ava_hdl_rec_del (AVA_CB *cb, AVA_HDL_DB *hdl_db, AVA_HDL_REC *hdl_rec)
{
   uns32 hdl = hdl_rec->hdl;

   /* pop the hdl rec */
   ncs_patricia_tree_del(&hdl_db->hdl_db_anchor, &hdl_rec->hdl_node);

   /* clean the pend callbk & resp list */
   ava_hdl_cbk_list_del(cb, &hdl_rec->pend_cbk);
   ava_hdl_pend_resp_list_del(cb, (AVA_PEND_CBK *)&hdl_rec->pend_resp);

   /* destroy the selection object */
   if (m_GET_FD_FROM_SEL_OBJ(hdl_rec->sel_obj))
   {
       m_NCS_SEL_OBJ_RAISE_OPERATION_SHUT(&hdl_rec->sel_obj);

      /* remove the association with hdl-mngr */
      ncshm_destroy_hdl(NCS_SERVICE_ID_AVA, hdl_rec->hdl);

      m_NCS_SEL_OBJ_RMV_OPERATION_SHUT(&hdl_rec->sel_obj);

   }

   /* free the hdl rec */
   free(hdl_rec);

   /* update the no of records */
   hdl_db->num--;

   m_AVA_LOG_HDL_DB(AVA_LOG_HDL_DB_REC_DEL, AVA_LOG_HDL_DB_SUCCESS, 
                    hdl, NCSFL_SEV_INFO);

   return;
}


/****************************************************************************
  Name          : ava_hdl_cbk_list_del
 
  Description   : This routine scans the pending callbk list & deletes all 
                  the records.
 
  Arguments     : cb   - ptr to the AvA control block
                  list - ptr to the pending callbk list
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
void ava_hdl_cbk_list_del (AVA_CB *cb, AVA_PEND_CBK *list)
{
   AVA_PEND_CBK_REC *rec = 0;

   /* pop & delete all the records */
   do
   {
      m_AVA_HDL_PEND_CBK_POP(list, rec);
      if (!rec) break;

      /* delete the record */
      ava_hdl_cbk_rec_del(rec);
   } while (1);

   /* there shouldn't be any record left */
   assert((!list->num) && (!list->head) && (!list->tail));

   return;
}


/****************************************************************************
  Name          : ava_hdl_cbk_rec_del
 
  Description   : This routine deletes the specified callback record in the 
                  pending callback list.
 
  Arguments     : rec - ptr to the pending callbk rec
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
void ava_hdl_cbk_rec_del (AVA_PEND_CBK_REC *rec)
{
   /* delete the callback info */
   if (rec->cbk_info)
      avsv_amf_cbk_free(rec->cbk_info);
   
   /* delete the record */
   free(rec);
}


/****************************************************************************
  Name          : ava_hdl_rec_add
 
  Description   : This routine adds the handle record to the handle db.
 
  Arguments     : cb       - ptr tot he AvA control block
                  hdl_db   - ptr to the hdl db
                  reg_cbks - ptr to the set of registered callbacks
 
  Return Values : ptr to the handle record
 
  Notes         : None
******************************************************************************/
AVA_HDL_REC *ava_hdl_rec_add (AVA_CB *cb, 
                              AVA_HDL_DB *hdl_db, 
                              const SaAmfCallbacksT *reg_cbks)
{
   AVA_HDL_REC *rec = 0;

   /* allocate the hdl rec */
   if ( !(rec = calloc(1, sizeof(AVA_HDL_REC))) )
       goto error;

   /* create the association with hdl-mngr */
   if ( !(rec->hdl = ncshm_create_hdl(cb->pool_id, NCS_SERVICE_ID_AVA, 
                                      (NCSCONTEXT)rec)) )
      goto error;

   /* create the selection object & store it in hdl rec */
   m_NCS_SEL_OBJ_CREATE(&rec->sel_obj);
   if (!m_GET_FD_FROM_SEL_OBJ(rec->sel_obj))
      goto error;
   
   /* store the registered callbacks */
   if (reg_cbks)
       memcpy((void *)&rec->reg_cbk, (void *)reg_cbks, 
                       sizeof(SaAmfCallbacksT));

   /* add the record to the hdl db */
   rec->hdl_node.key_info = (uns8 *)&rec->hdl;
   if (ncs_patricia_tree_add(&hdl_db->hdl_db_anchor, &rec->hdl_node)
       != NCSCC_RC_SUCCESS)
       goto error;

   /* update the no of records */
   hdl_db->num++;

   m_AVA_LOG_HDL_DB(AVA_LOG_HDL_DB_REC_ADD, AVA_LOG_HDL_DB_SUCCESS, 
                    rec->hdl, NCSFL_SEV_INFO);

   return rec;

error:
   if (rec) 
   {
     /* remove the association with hdl-mngr */
      if (rec->hdl)
         ncshm_destroy_hdl(NCS_SERVICE_ID_AVA, rec->hdl);

      /* destroy the selection object */
      if (m_GET_FD_FROM_SEL_OBJ(rec->sel_obj))
         m_NCS_SEL_OBJ_DESTROY(rec->sel_obj);

      free(rec);
   }
   return 0;
}


/****************************************************************************
  Name          : ava_hdl_cbk_param_add
 
  Description   : This routine adds the callback parameters to the pending 
                  callback list.
 
  Arguments     : cb       - ptr to the AvA control block
                  hdl_rec  - ptr to the handle record
                  cbk_info - ptr to the callback parameters
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : This routine reuses the callback info ptr that is received 
                  from MDS thus avoiding an extra copy.
******************************************************************************/
uns32 ava_hdl_cbk_param_add (AVA_CB *cb, AVA_HDL_REC *hdl_rec, 
                             AVSV_AMF_CBK_INFO *cbk_info)
{
   AVA_PEND_CBK_REC *rec = 0;
   uns32            rc = NCSCC_RC_SUCCESS;

   /* allocate the callbk rec */
   if ( !(rec = calloc(1, sizeof(AVA_PEND_CBK_REC))) )
   {
      rc = NCSCC_RC_FAILURE;
      goto done;
   }

   /* populate the callbk parameters */
   rec->cbk_info = cbk_info;

   /* now push it to the pending list */
   m_AVA_HDL_PEND_CBK_PUSH(&(hdl_rec->pend_cbk), rec);

   /* send an indication to the application */
   if ( NCSCC_RC_SUCCESS != (rc = m_NCS_SEL_OBJ_IND(hdl_rec->sel_obj)) )
   {
      /* log */
      assert(0);
   }

done:
   if ( (NCSCC_RC_SUCCESS != rc) && rec )
      ava_hdl_cbk_rec_del(rec);

   m_AVA_LOG_HDL_DB(AVA_LOG_HDL_DB_REC_CBK_ADD, AVA_LOG_HDL_DB_SUCCESS, 
                    hdl_rec->hdl, NCSFL_SEV_INFO);

   return rc;
}


/****************************************************************************
  Name          : ava_hdl_cbk_dispatch
 
  Description   : This routine dispatches the pending callbacks as per the 
                  dispatch flags.
 
  Arguments     : cb      - ptr to the AvA control block
                  hdl_rec - ptr to the handle record
                  flags   - dispatch flags
 
  Return Values : SA_AIS_OK/SA_AIS_ERR_<CODE>
 
  Notes         : None
******************************************************************************/
uns32 ava_hdl_cbk_dispatch (AVA_CB           **cb, 
                            AVA_HDL_REC      **hdl_rec, 
                            SaDispatchFlagsT flags)
{
   uns32 rc = SA_AIS_OK;

   switch (flags)
   {
   case SA_DISPATCH_ONE:
       rc = ava_hdl_cbk_dispatch_one(cb, hdl_rec);
      break;

   case SA_DISPATCH_ALL:
       rc = ava_hdl_cbk_dispatch_all(cb, hdl_rec);
      break;

   case SA_DISPATCH_BLOCKING:
       rc = ava_hdl_cbk_dispatch_block(cb, hdl_rec);
      break;

   default:
      assert(0);
   } /* switch */

   return rc;
}


/****************************************************************************
  Name          : ava_hdl_cbk_dispatch_one
 
  Description   : This routine dispatches one pending callback.
 
  Arguments     : cb      - ptr to the AvA control block
                  hdl_rec - ptr to the handle record
 
  Return Values : SA_AIS_OK/SA_AIS_ERR_<CODE>
 
  Notes         : None.
******************************************************************************/
uns32 ava_hdl_cbk_dispatch_one (AVA_CB **cb, AVA_HDL_REC **hdl_rec)
{
   AVA_PEND_CBK     *list = &(*hdl_rec)->pend_cbk;
   AVA_PEND_RESP    *list_resp = &(*hdl_rec)->pend_resp;
   AVA_PEND_CBK_REC *rec = 0;
   uns32 hdl = (*hdl_rec)->hdl;
   SaAmfCallbacksT reg_cbk; 
   uns32 rc = SA_AIS_OK;

   memset(&reg_cbk, 0, sizeof(SaAmfCallbacksT));
   memcpy(&reg_cbk, &(*hdl_rec)->reg_cbk,sizeof(SaAmfCallbacksT));

   /* pop the rec from the list */
   m_AVA_HDL_PEND_CBK_POP(list, rec);
   if (rec)
   {

      if(rec->cbk_info->type != AVSV_AMF_PG_TRACK)
      {
         /* push this record into pending response list */
         m_AVA_HDL_PEND_RESP_PUSH(list_resp, (AVA_PEND_RESP_REC *)rec);
         m_AVA_HDL_CBK_REC_IN_DISPATCH_SET(rec);
      }

      /* remove the selection object indication */
      if ( -1 == m_NCS_SEL_OBJ_RMV_IND((*hdl_rec)->sel_obj, TRUE, TRUE) )
         assert(0);

      /* release the cb lock & return the hdls to the hdl-mngr */
      m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
      ncshm_give_hdl(hdl);

      /* process the callback list record */
      ava_hdl_cbk_rec_prc(rec->cbk_info,&reg_cbk);

      m_NCS_LOCK(&(*cb)->lock, NCS_LOCK_WRITE);

      if( 0 == (*hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_AVA, hdl)))
      {
         /* hdl is already finalized */
         ava_hdl_cbk_rec_del(rec);
         return rc;
      }

      /* if we are done with this rec, free it */
      if((rec->cbk_info->type != AVSV_AMF_PG_TRACK) && 
                        m_AVA_HDL_IS_CBK_RESP_DONE(rec) )
      {
         m_AVA_HDL_PEND_RESP_POP(list_resp, rec, rec->cbk_info->inv);
         ava_hdl_cbk_rec_del(rec);
      }
      else if(rec->cbk_info->type == AVSV_AMF_PG_TRACK)
      {
         /* PG Track cbk do not have any response */
         ava_hdl_cbk_rec_del(rec);
      }
      else
      {
         m_AVA_HDL_CBK_REC_IN_DISPATCH_RESET(rec);
      }

   }

   return rc;
}


/****************************************************************************
  Name          : ava_hdl_cbk_dispatch_all
 
  Description   : This routine dispatches all the pending callbacks.
 
  Arguments     : cb      - ptr to the AvA control block
                  hdl_rec - ptr to the handle record
 
  Return Values : SA_AIS_OK/SA_AIS_ERR_<CODE>
 
  Notes         : Refer to the notes in ava_hdl_cbk_dispatch_one().
******************************************************************************/
uns32 ava_hdl_cbk_dispatch_all (AVA_CB **cb, AVA_HDL_REC **hdl_rec)
{
   AVA_PEND_CBK     *list = &(*hdl_rec)->pend_cbk;
   AVA_PEND_RESP    *list_resp = &(*hdl_rec)->pend_resp;
   AVA_PEND_CBK_REC *rec = 0;
   uns32 hdl = (*hdl_rec)->hdl;
   SaAmfCallbacksT reg_cbk; 
   uns32 rc = SA_AIS_OK;

   memset(&reg_cbk, 0,sizeof(SaAmfCallbacksT));
   memcpy(&reg_cbk, &(*hdl_rec)->reg_cbk,sizeof(SaAmfCallbacksT));

   /* pop all the records from the list & process them */
   do
   {
      m_AVA_HDL_PEND_CBK_POP(list, rec);
      if (!rec) break;

      if(rec->cbk_info->type != AVSV_AMF_PG_TRACK)
      {
         /* push this record into pending response list */
         m_AVA_HDL_PEND_RESP_PUSH(list_resp, (AVA_PEND_RESP_REC *)rec);
         m_AVA_HDL_CBK_REC_IN_DISPATCH_SET(rec);
      }

      /* remove the selection object indication */
      if ( -1 == m_NCS_SEL_OBJ_RMV_IND((*hdl_rec)->sel_obj, TRUE, TRUE) )
         assert(0);

      /* release the cb lock & return the hdls to the hdl-mngr */
      m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
      ncshm_give_hdl(hdl);

      /* process the callback list record */
      ava_hdl_cbk_rec_prc(rec->cbk_info,&reg_cbk);

      m_NCS_LOCK(&(*cb)->lock, NCS_LOCK_WRITE);

      /* is it finalized ? */
      if(!(*hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_AVA, hdl)))
      {
         ava_hdl_cbk_rec_del(rec);
         break;
      }

      /* if we are done with this rec, free it */
      if((rec->cbk_info->type != AVSV_AMF_PG_TRACK) && 
                        m_AVA_HDL_IS_CBK_RESP_DONE(rec) )
      {
         m_AVA_HDL_PEND_RESP_POP(list_resp, rec, rec->cbk_info->inv);
         ava_hdl_cbk_rec_del(rec);
      }
      else if(rec->cbk_info->type == AVSV_AMF_PG_TRACK)
      {
         /* PG Track cbk do not have any response */
         ava_hdl_cbk_rec_del(rec);
      }
      else
      {
         m_AVA_HDL_CBK_REC_IN_DISPATCH_RESET(rec);
      }

   } while (1);

   return rc;
}


/****************************************************************************
  Name          : ava_hdl_cbk_dispatch_block
 
  Description   : This routine blocks forever for receiving indications from 
                  AvND. The routine returns when saAmfFinalize is executed on 
                  the handle.
 
  Arguments     : cb      - ptr to the AvA control block
                  hdl_rec - ptr to the handle record
 
  Return Values : SA_AIS_OK/SA_AIS_ERR_<CODE>
 
  Notes         : None
******************************************************************************/
uns32 ava_hdl_cbk_dispatch_block (AVA_CB **cb, AVA_HDL_REC **hdl_rec)
{
   AVA_PEND_CBK     *list = &(*hdl_rec)->pend_cbk;
   AVA_PEND_RESP    *list_resp = &(*hdl_rec)->pend_resp;
   uns32            hdl = (*hdl_rec)->hdl;
   AVA_PEND_CBK_REC *rec = 0;
   NCS_SEL_OBJ_SET  all_sel_obj;
   NCS_SEL_OBJ      sel_obj = (*hdl_rec)->sel_obj;
   SaAmfCallbacksT reg_cbk; 
   uns32            rc = SA_AIS_OK;

   m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
   m_NCS_SEL_OBJ_SET(sel_obj,&all_sel_obj); 
   
   memset(&reg_cbk, 0,sizeof(SaAmfCallbacksT));
   memcpy(&reg_cbk, &(*hdl_rec)->reg_cbk,sizeof(SaAmfCallbacksT));

   /* release all lock and handle - we are abt to go into deep sleep */
   m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);

   while(m_NCS_SEL_OBJ_SELECT(sel_obj,&all_sel_obj,0,0,0) != -1)
   {
      ncshm_give_hdl(hdl);

      m_NCS_LOCK(&(*cb)->lock, NCS_LOCK_WRITE);

      /* get all handle and lock */
      *hdl_rec = (AVA_HDL_REC *)ncshm_take_hdl(NCS_SERVICE_ID_AVA, hdl);

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
      m_AVA_HDL_PEND_CBK_POP(list, rec);
      if (!rec) 
      {
         m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
         break;
      }

      if(rec->cbk_info->type != AVSV_AMF_PG_TRACK)
      {
         /* push this record into pending response list */
         m_AVA_HDL_PEND_RESP_PUSH(list_resp, (AVA_PEND_RESP_REC *)rec);
         m_AVA_HDL_CBK_REC_IN_DISPATCH_SET(rec);
      }

      /* remove the selection object indication */
      if ( -1 == m_NCS_SEL_OBJ_RMV_IND((*hdl_rec)->sel_obj, TRUE, TRUE) )
         assert(0);
         
      /* release the cb lock & return the hdls to the hdl-mngr */
      ncshm_give_hdl(hdl);
      m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);

      /* process the callback list record */
      ava_hdl_cbk_rec_prc(rec->cbk_info,&reg_cbk);

      /* take cb lock, so that any call to SaAmfResponce() will block */
       m_NCS_LOCK(&(*cb)->lock, NCS_LOCK_WRITE);

      *hdl_rec = (AVA_HDL_REC *) ncshm_take_hdl(NCS_SERVICE_ID_AVA, hdl);
      if(*hdl_rec)
      {
         m_NCS_SEL_OBJ_SET(sel_obj,&all_sel_obj);
      }else
      {
         ava_hdl_cbk_rec_del(rec);
         m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
         break;
      }

      /* if we are done with this rec, free it */
      if((rec->cbk_info->type != AVSV_AMF_PG_TRACK) && 
                        m_AVA_HDL_IS_CBK_RESP_DONE(rec) )
      {
         m_AVA_HDL_PEND_RESP_POP(list_resp, rec, rec->cbk_info->inv);
         ava_hdl_cbk_rec_del(rec);
      }
      else if(rec->cbk_info->type == AVSV_AMF_PG_TRACK)
      {
         /* PG Track cbk do not have any response */
         ava_hdl_cbk_rec_del(rec);
      }
      else
      {
         m_AVA_HDL_CBK_REC_IN_DISPATCH_RESET(rec);
      }

      /* end of another critical section */
       m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
       
   }/*while*/
  
   if(*hdl_rec) 
      ncshm_give_hdl(hdl); 


   m_NCS_LOCK(&(*cb)->lock, NCS_LOCK_WRITE);
   *hdl_rec = (AVA_HDL_REC *)ncshm_take_hdl(NCS_SERVICE_ID_AVA, hdl);

   return rc;
}


/****************************************************************************
  Name          : ava_hdl_cbk_rec_prc
 
  Description   : This routine invokes the registered callback routine.
 
  Arguments     : cb      - ptr to the AvA control block
                  hdl_rec - ptr to the hdl rec
                  cbk_rec - ptr to the callback record
 
  Return Values : None
 
  Notes         : It may so happen that the callbacks that are dispatched may 
                  finalize on the amf handle. Release AMF handle to the handle
                  manager before dispatching. Else Finalize blocks while 
                  destroying the association with handle manager.
******************************************************************************/
void ava_hdl_cbk_rec_prc (AVSV_AMF_CBK_INFO *info, SaAmfCallbacksT *reg_cbk)
{

   /* invoke the corresponding callback */
   switch (info->type)
   {
   case AVSV_AMF_HC:
      {
         AVSV_AMF_HC_PARAM *hc = &info->param.hc;

         if (reg_cbk->saAmfHealthcheckCallback)
         {
            hc->comp_name.length = hc->comp_name.length;
            reg_cbk->saAmfHealthcheckCallback(info->inv, 
                                               &hc->comp_name, 
                                               &hc->hc_key);
         }
      }
      break;

   case AVSV_AMF_COMP_TERM:
      {
         AVSV_AMF_COMP_TERM_PARAM *comp_term = &info->param.comp_term;

         if (reg_cbk->saAmfComponentTerminateCallback)
         {
            comp_term->comp_name.length = comp_term->comp_name.length;
            reg_cbk->saAmfComponentTerminateCallback(info->inv, 
                                                     &comp_term->comp_name);
         }
      }
      break;

   case AVSV_AMF_CSI_SET:
      {
         AVSV_AMF_CSI_SET_PARAM *csi_set = &info->param.csi_set;

         if (reg_cbk->saAmfCSISetCallback)
         {
            csi_set->comp_name.length = csi_set->comp_name.length;

            if(SA_AMF_CSI_TARGET_ALL != csi_set->csi_desc.csiFlags)
            {
               csi_set->csi_desc.csiName.length = csi_set->csi_desc.csiName.length;
            }

            if((SA_AMF_HA_ACTIVE == csi_set->ha) &&
	       (SA_AMF_CSI_NEW_ASSIGN != csi_set->csi_desc.csiStateDescriptor.activeDescriptor.transitionDescriptor))
            {
               csi_set->csi_desc.csiStateDescriptor.activeDescriptor.activeCompName.length =
		       csi_set->csi_desc.csiStateDescriptor.activeDescriptor.activeCompName.length;
              
            }else if (SA_AMF_HA_STANDBY == csi_set->ha)
            {
               csi_set->csi_desc.csiStateDescriptor.standbyDescriptor.activeCompName.length =
		       csi_set->csi_desc.csiStateDescriptor.standbyDescriptor.activeCompName.length;
            }
 
            reg_cbk->saAmfCSISetCallback(info->inv, 
                                         &csi_set->comp_name,
                                         csi_set->ha,
                                         csi_set->csi_desc);
         }
      }
      break;

   case AVSV_AMF_CSI_REM:
      {
         AVSV_AMF_CSI_REM_PARAM *csi_rem = &info->param.csi_rem;

         if (reg_cbk->saAmfCSIRemoveCallback)
         {
            csi_rem->comp_name.length = csi_rem->comp_name.length;
            csi_rem->csi_name.length = csi_rem->csi_name.length;
            reg_cbk->saAmfCSIRemoveCallback(info->inv, 
                                            &csi_rem->comp_name,
                                            &csi_rem->csi_name,
                                            csi_rem->csi_flags);
         }
      }
      break;

   case AVSV_AMF_PG_TRACK:
      {
         AVSV_AMF_PG_TRACK_PARAM *pg_track = &info->param.pg_track;
         SaAmfProtectionGroupNotificationBufferT buf;
         uns32 i = 0;

         if (reg_cbk->saAmfProtectionGroupTrackCallback)
         {
            pg_track->csi_name.length = pg_track->csi_name.length;

            /* convert comp-name len into host order */
            for (i = 0; i < pg_track->buf.numberOfItems; i++)
               pg_track->buf.notification[i].member.compName.length = 
		    pg_track->buf.notification[i].member.compName.length;

            /* copy the contents into a malloced buffer.. appl frees it */
            buf.numberOfItems = pg_track->buf.numberOfItems;
            buf.notification = 0;

            buf.notification = malloc(buf.numberOfItems * sizeof(SaAmfProtectionGroupNotificationT));
            if (buf.notification)
            {
               memcpy(buf.notification, pg_track->buf.notification,
                               buf.numberOfItems * sizeof(SaAmfProtectionGroupNotificationT));

               reg_cbk->saAmfProtectionGroupTrackCallback(&pg_track->csi_name,
                                                          &buf,
                                                          pg_track->mem_num,
                                                          pg_track->err);
            }else
       {
               pg_track->err = SA_AIS_ERR_NO_MEMORY;
               reg_cbk->saAmfProtectionGroupTrackCallback(&pg_track->csi_name,
                                                          0, 
                                                          0,
                                                          pg_track->err);
            }   
         }
      }
      break;

   case AVSV_AMF_PXIED_COMP_INST:
      {
         AVSV_AMF_PXIED_COMP_INST_PARAM *comp_inst = &info->param.pxied_comp_inst;

         if (reg_cbk->saAmfProxiedComponentInstantiateCallback)
         {
            comp_inst->comp_name.length = comp_inst->comp_name.length;
            reg_cbk->saAmfProxiedComponentInstantiateCallback(info->inv, 
                                                     &comp_inst->comp_name);
         }
      }
      break;
      
   case AVSV_AMF_PXIED_COMP_CLEAN:
      {
         AVSV_AMF_PXIED_COMP_CLEAN_PARAM *comp_clean = &info->param.pxied_comp_clean;

         if (reg_cbk->saAmfProxiedComponentCleanupCallback)
         {
            comp_clean->comp_name.length = comp_clean->comp_name.length;
            reg_cbk->saAmfProxiedComponentCleanupCallback(info->inv, 
                                                     &comp_clean->comp_name);
         }
      }
      break;

   default:
      assert(0);
      break;
   } /* switch */

   return;
}

/****************************************************************************
  Name          : ava_hdl_pend_resp_pop 
 
  Description   : This routine pops a matching rec from list of pending 
                  responses.
 
  Arguments     : list  - ptr to list of pend resp in HDL_REC 
                  key   - invocation number of the callback 
 
  Return Values : ptr to AVA_PEND_RESP_REC 
 
  Notes         : None. 
******************************************************************************/
AVA_PEND_RESP_REC *ava_hdl_pend_resp_pop(AVA_PEND_RESP *list, SaInvocationT key)
{
   if (!((list)->head))
   { 
       assert(!((list)->num));
       return NULL; 
   } 
   else 
   {
      if(key == 0)
      { 
         return NULL; 
      }
      else
      {
         AVA_PEND_RESP_REC *tmp_rec = list->head; 
         AVA_PEND_RESP_REC *p_tmp_rec = NULL;
         while(tmp_rec != NULL)
         {
            /* Found a match */
            if(tmp_rec->cbk_info->inv == key )
            {
               /* if this is the last rec */
               if(list->tail == tmp_rec)
               {
                  /* copy the prev rec to tail */
                  list->tail = p_tmp_rec;

                  if(list->tail == NULL)
                     list->head = NULL;
                  else
                     list->tail->next = NULL;

                  list->num--;
                  return tmp_rec;
               }
               else
               {
                  /* if this is the first rec */
                  if(list->head == tmp_rec)
                     list->head = tmp_rec->next;
                  else
                     p_tmp_rec->next = tmp_rec->next;

                  tmp_rec->next = NULL;
                  list->num--;
                  return tmp_rec;

               }
            }
            /* move on to next element */
            p_tmp_rec = tmp_rec;
            tmp_rec = tmp_rec->next;
         }

         return NULL;
      }/* else */
   }
}

/****************************************************************************
  Name          : ava_hdl_pend_resp_get 
 
  Description   : This routine gets a matching rec from list of pending 
                  responses.
 
  Arguments     : list  - ptr to list of pend resp in HDL_REC 
                  key   - invocation number of the callback 
 
  Return Values : ptr to AVA_PEND_RESP_REC 
 
  Notes         : None. 
******************************************************************************/
AVA_PEND_RESP_REC *ava_hdl_pend_resp_get(AVA_PEND_RESP *list, SaInvocationT key)
{
   if (!((list)->head))
   { 
       assert(!((list)->num));
       return NULL; 
   } 
   else 
   {
      if(key == 0)
      { 
         return NULL; 
      }
      else
      {
         /* parse thru the single list (head is diff from other elements) and
          *  search for rec matching inv handle number. 
          */
         AVA_PEND_RESP_REC *tmp_rec = list->head; 
         while(tmp_rec != NULL)
         {
            if(tmp_rec->cbk_info->inv == key )
               return tmp_rec;
            tmp_rec = tmp_rec->next;
         }
         return NULL;
      }/* else */
   }
}


/****************************************************************************
  Name          : ava_hdl_pend_resp_list_del
 
  Description   : This routine scans the pending response list & deletes all 
                  the records.
 
  Arguments     : cb   - ptr to the AvA control block
                  list - ptr to the pending response list
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
void ava_hdl_pend_resp_list_del (AVA_CB *cb, AVA_PEND_CBK *list)
{
   AVA_PEND_CBK_REC *rec = 0;

   /* pop & delete all the records */
   do
   {
      m_AVA_HDL_PEND_CBK_POP(list, rec);
      if (!rec) break;

      /* delete the record */
      if(!m_AVA_HDL_IS_CBK_REC_IN_DISPATCH(rec))
      ava_hdl_cbk_rec_del(rec);
   } while (1);

   /* there shouldn't be any record left */
   assert((!list->num) && (!list->head) && (!list->tail));

   return;
}



