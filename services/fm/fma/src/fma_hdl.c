/*****************************************************************************
 *                                                                           *
 * NOTICE TO PROGRAMMERS:  RESTRICTED USE.                                   *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED TO YOU AND YOUR COMPANY UNDER A LICENSE         *
 * AGREEMENT FROM EMERSON, INC.                                              *
 *                                                                           *
 * THE TERMS OF THE LICENSE AGREEMENT RESTRICT THE USE OF THIS SOFTWARE TO   *
 * SPECIFIC PROJECTS ("LICENSEE APPLICATIONS") IDENTIFIED IN THE AGREEMENT.  *
 *                                                                           *
 * IF YOU ARE UNSURE WHETHER YOU ARE AUTHORIZED TO USE THIS SOFTWARE ON YOUR *
 * PROJECT,  PLEASE CHECK WITH YOUR MANAGEMENT.                              *
 *                                                                           *
 *****************************************************************************


..............................................................................

DESCRIPTION:

******************************************************************************
*/

#include "fma.h"

static void fma_hdl_cbk_list_del (FMA_CB *cb, FMA_PEND_CBK *list);
static void fma_hdl_pend_resp_list_del (FMA_CB *cb, FMA_PEND_CBK *list);
static uns32 fma_hdl_callbk_dispatch_one (FMA_CB           **cb,
                                     FMA_HDL_REC      **hdl_rec);
static uns32 fma_hdl_callbk_dispatch_all (FMA_CB           **cb,
                                     FMA_HDL_REC      **hdl_rec);
static uns32 fma_hdl_callbk_dispatch_block (FMA_CB           **cb,
                                     FMA_HDL_REC      **hdl_rec);



/****************************************************************************
  Name          : fma_hdl_rec_add

  Description   :

  Arguments     :

  Return Values :

  Notes         :
******************************************************************************/
FMA_HDL_REC *fma_hdl_rec_add(FMA_CB *cb,FMA_HDL_DB *hdl_db,const fmCallbacksT *fmaCallback)
{
    FMA_HDL_REC *hdl_rec = 0;
    
    m_FMA_LOG_FUNC_ENTRY("fma_hdl_rec_add");
    /** Allocate memory for hdl_rec **/
    hdl_rec = m_MMGR_ALLOC_FMA_HDL_REC;

    if ( !hdl_rec )
    {
        m_FMA_LOG_MEM(FMA_LOG_HDL_REC_ALLOC,FMA_LOG_MEM_ALLOC_FAILURE,NCSFL_SEV_EMERGENCY);
        goto error; 
    }
    m_NCS_OS_MEMSET(hdl_rec, '\0', sizeof(FMA_HDL_REC));
   
    /** Associate with hdl manager **/
    hdl_rec->hdl = ncshm_create_hdl(cb->pool_id,NCS_SERVICE_ID_FMA,(NCSCONTEXT)hdl_rec);
    if ( !(hdl_rec->hdl) )
    {
        m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_CREATE,FMA_LOG_HDL_DB_FAILURE,hdl_rec->hdl,NCSFL_SEV_CRITICAL);
        goto error;
    }
    /** Creat selection object **/
    m_NCS_SEL_OBJ_CREATE(&hdl_rec->sel_obj);
    if (!m_GET_FD_FROM_SEL_OBJ(hdl_rec->sel_obj))
      goto error;

   /** Store the registered callbacks **/
   if (fmaCallback)
       m_NCS_OS_MEMCPY((void *)&hdl_rec->reg_cbk, (void *)fmaCallback,
                       sizeof(fmCallbacksT));

   hdl_rec->hdl_node.key_info = (uns8 *)&hdl_rec->hdl; 
   /** Add the record to the hdl database **/
   if (ncs_patricia_tree_add(&hdl_db->hdl_db_anchor, &hdl_rec->hdl_node)
       != NCSCC_RC_SUCCESS)
   {
       m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_ADD, FMA_LOG_HDL_DB_FAILURE,hdl_rec->hdl,NCSFL_SEV_INFO);
       goto error;
   }
   /** update the no of records **/
   hdl_db->num++;
  
   m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_ADD, FMA_LOG_HDL_DB_SUCCESS,hdl_rec->hdl,NCSFL_SEV_INFO);
                     
   return hdl_rec;

error:
   if (hdl_rec)
   {
     /** remove the association with hdl-mngr **/
      if (hdl_rec->hdl)
         ncshm_destroy_hdl(NCS_SERVICE_ID_FMA, hdl_rec->hdl);

      /** remove the selection object **/
      if (m_GET_FD_FROM_SEL_OBJ(hdl_rec->sel_obj))
            m_NCS_SEL_OBJ_DESTROY(hdl_rec->sel_obj);

      m_MMGR_FREE_FMA_HDL_REC(hdl_rec);
      hdl_rec = NULL;
   }
   return 0;
}

/****************************************************************************
  Name          : fma_hdl_rec_del

  Description   :

  Arguments     :

  Return Values :

  Notes         :
******************************************************************************/
void fma_hdl_rec_del (FMA_CB *cb, FMA_HDL_DB *hdl_db, FMA_HDL_REC *hdl_rec)
{
   m_FMA_LOG_FUNC_ENTRY("fma_hdl_rec_del");

   /* release the handle before destroying it or destroy will block */
   ncshm_give_hdl(hdl_rec->hdl);

   /* remove the association with hdl-mngr */
   ncshm_destroy_hdl(NCS_SERVICE_ID_FMA, hdl_rec->hdl);

   /** Remove the hdl record from the database **/
   ncs_patricia_tree_del(&hdl_db->hdl_db_anchor, &hdl_rec->hdl_node);
  
   /* clean the pend callback & resp list */
   fma_hdl_cbk_list_del(cb, &hdl_rec->pend_cbk);
   fma_hdl_pend_resp_list_del(cb, (FMA_PEND_CBK *)&hdl_rec->pend_resp);

   /* destroy the selection object */
   if (m_GET_FD_FROM_SEL_OBJ(hdl_rec->sel_obj))
   {
       m_NCS_SEL_OBJ_RAISE_OPERATION_SHUT(&hdl_rec->sel_obj);

      m_NCS_SEL_OBJ_RMV_OPERATION_SHUT(&hdl_rec->sel_obj);

   }

   /* free the hdl rec */
   m_MMGR_FREE_FMA_HDL_REC(hdl_rec);
   hdl_rec = NULL;

   /* update the no of records */
   hdl_db->num--;

    m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_DEL, FMA_LOG_HDL_DB_SUCCESS, 0,NCSFL_SEV_INFO);
   return;
 
}
/****************************************************************************
  Name          : fma_hdl_cbk_list_del

  Description   :

  Arguments     :

  Return Values :

  Notes         :
******************************************************************************/
static void fma_hdl_cbk_list_del (FMA_CB *cb, FMA_PEND_CBK *list)
{
   FMA_PEND_CBK_REC *rec = 0;
   
   m_FMA_LOG_FUNC_ENTRY("fma_hdl_cbk_list_del");
   /* pop & delete all the records */
   do
   {
      m_FMA_HDL_PEND_CBK_GET(list, rec);
      if (!rec) break;

      /* destroy the invocation handle and delete the record */
      if(rec->cbk_info->inv)
          ncshm_destroy_hdl(NCS_SERVICE_ID_FMA,rec->cbk_info->inv);

      m_MMGR_FREE_FMA_CBK_INFO(rec->cbk_info);
      m_MMGR_FREE_FMA_PEND_CBK_REC(rec);
      rec = NULL;
   } while (1);

   m_FMA_ASSERT((!list->num) && (!list->head) && (!list->tail));

   return;

}
/****************************************************************************
  Name          : fma_hdl_pend_resp_list_del

  Description   :

  Arguments     :

  Return Values :

  Notes         :
******************************************************************************/
static void fma_hdl_pend_resp_list_del (FMA_CB *cb, FMA_PEND_CBK *list)
{
   FMA_PEND_CBK_REC *rec = 0;

   m_FMA_LOG_FUNC_ENTRY("fma_hdl_pend_resp_list_del");
   /* pop & delete all the records */
   do
   {
      m_FMA_HDL_PEND_RESP_GET(list, rec);
      if (!rec) break;

      /* delete the record */
      if(rec->cbk_info->inv)
          ncshm_destroy_hdl(NCS_SERVICE_ID_FMA, rec->cbk_info->inv);  

      m_MMGR_FREE_FMA_CBK_INFO(rec->cbk_info);
      m_MMGR_FREE_FMA_PEND_CBK_REC(rec);
      rec = NULL;
   } while (1);

   m_FMA_ASSERT((!list->num) && (!list->head) && (!list->tail));

   return;

}
/****************************************************************************
  Name          : fma_hdl_db_init
      
  Description   : 
   
  Arguments     : 
 
  Return Values : 
   
  Notes         : 
******************************************************************************/
uns32 fma_hdl_db_init (FMA_HDL_DB *hdl_db)
{  
   NCS_PATRICIA_PARAMS param;
   uns32 rc = NCSCC_RC_SUCCESS;
    
   m_FMA_LOG_FUNC_ENTRY("fma_hdl_db_init");   
   param.key_size = sizeof(uns32);
   param.info_size = 0;
   
   rc = ncs_patricia_tree_init(&hdl_db->hdl_db_anchor, &param);
   if ( rc == NCSCC_RC_SUCCESS )
   { 
      hdl_db->num = 0;
      m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_CREATE,FMA_LOG_HDL_DB_SUCCESS,0,NCSFL_SEV_INFO);
   }
      
   return rc;
}  

/****************************************************************************
  Name          : fma_hdl_db_del
 
  Description   : 
 
  Arguments     : 
    
  Return Values : None
 
  Notes         : None
******************************************************************************/
void fma_hdl_db_del (FMA_CB *cb)
{
   FMA_HDL_DB  *hdl_db = &cb->hdl_db;
   FMA_HDL_REC *hdl_rec = 0;
   NCS_PATRICIA_NODE *node = 0;
   uns8 *key;
   
   m_FMA_LOG_FUNC_ENTRY("fma_hdl_db_del");
   node = ncs_patricia_tree_getnext(&(hdl_db->hdl_db_anchor),NULL);
   while ( node )
   {
      hdl_rec = (FMA_HDL_REC*)ncshm_take_hdl( NCS_SERVICE_ID_FMA, *(uns32 *)(node->key_info) );
      key = node->key_info;
      fma_hdl_rec_del(cb, hdl_db, hdl_rec);
/*    node = ncs_patricia_tree_getnext(&(cb->hdl_db.hdl_db_anchor),(const uns8*)key); */
      node = ncs_patricia_tree_getnext(&(cb->hdl_db.hdl_db_anchor),NULL);
   }

   m_FMA_ASSERT(!hdl_db->num);

   /* destroy the hdl db tree */
   ncs_patricia_tree_destroy(&hdl_db->hdl_db_anchor);
   m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_DESTROY,FMA_LOG_HDL_DB_SUCCESS,0,NCSFL_SEV_INFO);
   return;

}


/****************************************************************************
  Name          : fma_hdl_callbk_dispatch

  Description   :

  Arguments     :

  Return Values : None

  Notes         : None
******************************************************************************/

uns32 fma_hdl_callbk_dispatch (FMA_CB           **cb,
                            FMA_HDL_REC      **hdl_rec,
                            SaDispatchFlagsT flags)
{
   uns32 rc = SA_AIS_OK;
   
   m_FMA_LOG_FUNC_ENTRY("fma_hdl_callbk_dispatch");
   switch (flags)
   {
   case SA_DISPATCH_ONE:
       rc = fma_hdl_callbk_dispatch_one(cb, hdl_rec);
      break;

   case SA_DISPATCH_ALL:
       rc = fma_hdl_callbk_dispatch_all(cb, hdl_rec);
      break;

   case SA_DISPATCH_BLOCKING:
       rc = fma_hdl_callbk_dispatch_block(cb, hdl_rec);
      break;

   default:
        m_FMA_LOG_API(FMA_LOG_API_DISPATCH, FMA_LOG_API_ERR_SA_INVALID_PARAM, 0, NCSFL_SEV_INFO);
        rc = SA_AIS_ERR_INVALID_PARAM;
   } 

   return rc;

}

/****************************************************************************
  Name          : fma_hdl_callbk_dispatch_one

  Description   :

  Arguments     :

  Return Values : None

  Notes         : None
******************************************************************************/

static uns32 fma_hdl_callbk_dispatch_one (FMA_CB **cb, FMA_HDL_REC **hdl_rec)
{
   FMA_PEND_CBK_REC *rec = NULL;
   FMA_PEND_CBK *pend_cbk = &(*hdl_rec)->pend_cbk;
   FMA_PEND_RESP *pend_resp = &(*hdl_rec)->pend_resp;
/*   uns32 hdl = (*hdl_rec)->hdl;  */
   fmCallbacksT reg_cbk;
   uns32 rc = SA_AIS_OK;

   m_FMA_LOG_FUNC_ENTRY("fma_hdl_callbk_dispatch_one");   

   m_NCS_MEMSET(&reg_cbk, 0, sizeof(fmCallbacksT));
   m_NCS_MEMCPY(&reg_cbk, &(*hdl_rec)->reg_cbk,sizeof(fmCallbacksT));
   
   /* Pop the rec from the pending callback list */
   m_FMA_HDL_PEND_CBK_GET(pend_cbk,rec);
   if ( rec )
   {
   /** Reset the selection object **/
   if ( m_NCS_SEL_OBJ_RMV_IND((*hdl_rec)->sel_obj, TRUE, TRUE) == -1 )
         m_FMA_ASSERT(0);
  
   /* Add the rec in resp list as we expect a response for all the callbacks */ 
   m_FMA_HDL_PEND_RESP_ADD(pend_resp, (FMA_PEND_RESP_REC *)rec);
   /** Need to release the lock for calling the callback, otherwise deadlock on arriving another callback. **/
    m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
/* commented the following - not needed */
/*    ncshm_give_hdl(hdl); */
    
    switch(rec->cbk_info->cbk_type)
     {
         case NODE_RESET_IND:
             reg_cbk.fmNodeResetIndCallback(rec->cbk_info->inv,rec->cbk_info->node_reset_info);
         break;
         case SWITCHOVER_REQ:
             reg_cbk.fmSysManSwitchReqCallback(rec->cbk_info->inv);
         break;
         default:
              m_FMA_LOG_API(FMA_LOG_API_DISPATCH, FMA_LOG_API_ERR_SA_INVALID_PARAM, 0, NCSFL_SEV_INFO);
              rc = SA_AIS_ERR_INVALID_PARAM;
     }
     
     m_NCS_LOCK(&(*cb)->lock, NCS_LOCK_WRITE);

/* commented the following - not needed */
/*
     if( 0 == (*hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_FMA, hdl)))
     {
         if(rec->cbk_info)
             m_MMGR_FREE_FMA_CBK_INFO(rec->cbk_info);
         m_MMGR_FREE_FMA_PEND_CBK_REC(rec);
         rec = NULL;
     }
*/
   }
   return rc;
   
}
                            
/****************************************************************************
  Name          : fma_hdl_callbk_dispatch_all

  Description   :

  Arguments     :

  Return Values : None

  Notes         : None
******************************************************************************/

static uns32 fma_hdl_callbk_dispatch_all (FMA_CB           **cb,
                               FMA_HDL_REC      **hdl_rec)
{
    FMA_PEND_CBK_REC *rec = NULL;
   FMA_PEND_CBK *pend_cbk = &(*hdl_rec)->pend_cbk;
   FMA_PEND_RESP *pend_resp = &(*hdl_rec)->pend_resp;
/*   uns32 hdl = (*hdl_rec)->hdl; */
   fmCallbacksT reg_cbk;
   
   m_FMA_LOG_FUNC_ENTRY("fma_hdl_callbk_dispatch_all");

   m_NCS_MEMSET(&reg_cbk, 0, sizeof(fmCallbacksT));
   m_NCS_MEMCPY(&reg_cbk, &(*hdl_rec)->reg_cbk,sizeof(fmCallbacksT));

   /** Pop all the pending rec from the pending list and process all the callbacks **/
   m_FMA_HDL_PEND_CBK_GET(pend_cbk,rec);
   while (rec)
   {
      /** Reset the selection object **/
   if ( m_NCS_SEL_OBJ_RMV_IND((*hdl_rec)->sel_obj, TRUE, TRUE) == -1 )
         m_FMA_ASSERT(0);

   /* Add the rec in resp list as we expect a response for all the callbacks */
   m_FMA_HDL_PEND_RESP_ADD(pend_resp, (FMA_PEND_RESP_REC *)rec);
   /** Need to release the lock for calling the callback, otherwise deadlock on arriving another callback. **/
    m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);

/* commented the following - not needed */
/*    ncshm_give_hdl(hdl); */

    switch(rec->cbk_info->cbk_type)
     {
         case NODE_RESET_IND:
             reg_cbk.fmNodeResetIndCallback(rec->cbk_info->inv,rec->cbk_info->node_reset_info);
         break;
         case SWITCHOVER_REQ:
             reg_cbk.fmSysManSwitchReqCallback(rec->cbk_info->inv);
         break;
         default:
                m_FMA_LOG_API(FMA_LOG_API_DISPATCH, FMA_LOG_API_ERR_SA_INVALID_PARAM, 0, NCSFL_SEV_INFO);
     }

     m_NCS_LOCK(&(*cb)->lock, NCS_LOCK_WRITE);

/* commented the following - not needed */
/*
      if( 0 == (*hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_FMA, hdl)))
      {
         if(rec->cbk_info)
             m_MMGR_FREE_FMA_CBK_INFO(rec->cbk_info);
         m_MMGR_FREE_FMA_PEND_CBK_REC(rec);
         rec = NULL;
         break;
      }
*/
      m_FMA_HDL_PEND_CBK_GET(pend_cbk,rec);
   }
   return SA_AIS_OK;
}
/****************************************************************************
  Name          : fma_hdl_callbk_dispatch_block

  Description   :

  Arguments     :

  Return Values : None

  Notes         : None
******************************************************************************/

static uns32 fma_hdl_callbk_dispatch_block (FMA_CB           **cb,
                                FMA_HDL_REC      **hdl_rec)
{
   FMA_PEND_CBK_REC *rec = NULL;
   FMA_PEND_CBK *pend_cbk = &(*hdl_rec)->pend_cbk;
   FMA_PEND_RESP *pend_resp = &(*hdl_rec)->pend_resp;
/*   uns32 hdl = (*hdl_rec)->hdl; */
   fmCallbacksT reg_cbk;
   uns32 rc = SA_AIS_OK;

   m_FMA_LOG_FUNC_ENTRY("fma_hdl_callbk_dispatch_block");

   NCS_SEL_OBJ_SET  all_sel_obj;
   NCS_SEL_OBJ      sel_obj = (*hdl_rec)->sel_obj;   
  
   m_NCS_MEMSET(&reg_cbk, 0, sizeof(fmCallbacksT));
   m_NCS_MEMCPY(&reg_cbk, &(*hdl_rec)->reg_cbk,sizeof(fmCallbacksT));
   
   m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
/* commented the following */
/*   ncshm_give_hdl(hdl); */

   m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
   m_NCS_SEL_OBJ_SET(sel_obj,&all_sel_obj);

   while(m_NCS_SEL_OBJ_SELECT(sel_obj,&all_sel_obj,0,0,0) != -1)
   {
       m_NCS_LOCK(&(*cb)->lock, NCS_LOCK_WRITE);
/* commented the following */
/*       *hdl_rec = (FMA_HDL_REC *)ncshm_take_hdl(NCS_SERVICE_ID_FMA, hdl);

      if(!(*hdl_rec))
      {  
         rc = SA_AIS_ERR_BAD_HANDLE;
         m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_GET,FMA_LOG_HDL_DB_FAILURE,hdl,NCSFL_SEV_CRITICAL);
         m_FMA_LOG_API(FMA_LOG_API_DISPATCH, FMA_LOG_API_ERR_SA_BAD_HANDLE, 0, NCSFL_SEV_INFO);

         m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
         break;
      }
*/       
      if (!m_NCS_SEL_OBJ_ISSET(sel_obj,&all_sel_obj))
      {
         m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
         break;
      }
      
      /* Pop the rec from the pending callback list */
      m_FMA_HDL_PEND_CBK_GET(pend_cbk,rec);
      if ( !rec )
      {
          m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
          break;
      }
      
       /** Reset the selection object **/
       if ( m_NCS_SEL_OBJ_RMV_IND((*hdl_rec)->sel_obj, TRUE, TRUE) == -1 )
           m_FMA_ASSERT(0);

       /* Add the rec in resp list as we expect a response for all the callbacks */
        m_FMA_HDL_PEND_RESP_ADD(pend_resp, (FMA_PEND_RESP_REC *)rec);
        /** Need to release the lock for calling the callback, otherwise deadlock on arriving another callback. **/
        m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE);
/* commented the following - not needed */
/*        ncshm_give_hdl(hdl); */

        switch(rec->cbk_info->cbk_type)
        {
            case NODE_RESET_IND:
               reg_cbk.fmNodeResetIndCallback(rec->cbk_info->inv,rec->cbk_info->node_reset_info);
            break;
            case SWITCHOVER_REQ:
                reg_cbk.fmSysManSwitchReqCallback(rec->cbk_info->inv);
            break;
            default:
                m_FMA_LOG_API(FMA_LOG_API_DISPATCH, FMA_LOG_API_ERR_SA_INVALID_PARAM, 0, NCSFL_SEV_NOTICE); 
        }

/* commented the following - not needed */
/*
        m_NCS_LOCK(&(*cb)->lock, NCS_LOCK_WRITE);

        if( 0 == (*hdl_rec =(FMA_HDL_REC *) ncshm_take_hdl(NCS_SERVICE_ID_FMA, hdl)))
        {
            if(rec->cbk_info)
               m_MMGR_FREE_FMA_CBK_INFO(rec->cbk_info);
            m_MMGR_FREE_FMA_PEND_CBK_REC(rec);
            rec = NULL;
            break;
        }
        else */
            m_NCS_SEL_OBJ_SET(sel_obj,&all_sel_obj);

/*        ncshm_give_hdl(hdl); 
        m_NCS_UNLOCK(&(*cb)->lock, NCS_LOCK_WRITE); */
   
   }
   
/*   if(*hdl_rec)
      ncshm_give_hdl(hdl);
*/

   m_NCS_LOCK(&(*cb)->lock, NCS_LOCK_WRITE);
/*   *hdl_rec = (FMA_HDL_REC *)ncshm_take_hdl(NCS_SERVICE_ID_FMA, hdl); */

   return rc;

}
/****************************************************************************
  Name          : fma_hdl_pend_resp_get

  Description   : 

  Arguments     : 

  Return Values : 

  Notes         : None.
******************************************************************************/
FMA_PEND_RESP_REC *fma_hdl_pend_resp_get(FMA_PEND_RESP *list, SaInvocationT key)
{
   m_FMA_LOG_FUNC_ENTRY("fma_hdl_pend_resp_get");

   if (!((list)->head))
   {   
       m_FMA_ASSERT(!((list)->num));
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
         /** parse thru the single list (head is diff from other elements) and
             search for rec matching inv handle number. **/
          
         FMA_PEND_RESP_REC *tmp_rec = list->head;
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
  Name          : fma_hdl_pend_resp_pop

  Description   : 

  Arguments     : 

  Return Values : 

  Notes         : None.
******************************************************************************/
FMA_PEND_RESP_REC *fma_hdl_pend_resp_pop(FMA_PEND_RESP *list, SaInvocationT key)
{
   m_FMA_LOG_FUNC_ENTRY("fma_hdl_pend_resp_pop");

   if (!((list)->head))
   {   
       m_FMA_ASSERT(!((list)->num));
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
         FMA_PEND_RESP_REC *tmp_rec = list->head;
         FMA_PEND_RESP_REC *p_tmp_rec = NULL;
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

