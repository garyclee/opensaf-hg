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

  This file contains functions that assist the library routines
..............................................................................

  FUNCTIONS INCLUDED in this module:
  

******************************************************************************
*/

#include "gla.h"

void gla_process_callback (GLA_CB            *gla_cb,
                           GLA_CLIENT_INFO   *client_info,
                           GLSV_GLA_CALLBACK_INFO   *callback);

/****************************************************************************
  Name          : gla_process_callback
 
  Description   : This routine invokes the registered callback routine.
 
  Arguments     : cb  - ptr to the AvA control block
                  rec - ptr to the callback record
                  reg_callbk - ptr to the registered callbacks
 
  Return Values : None
 
  Notes         : None
******************************************************************************/
void gla_process_callback (GLA_CB            *gla_cb,
                           GLA_CLIENT_INFO   *client_info,
                           GLSV_GLA_CALLBACK_INFO   *callback)
{
   GLA_RESOURCE_ID_INFO *res_id_node = NULL;


   /* invoke the corresponding callback */
   switch (callback->callback_type)
   {
   case GLSV_LOCK_RES_OPEN_CBK:
      {
         GLSV_LOCK_RES_OPEN_PARAM *param = &callback->params.res_open;
         if (client_info->lckCallbk.saLckResourceOpenCallback)
         {
         if ( param->error == SA_AIS_OK )
          {
             /* add the resource id to the local tree */
             /* retrieve Resorce hdl record */
             res_id_node = (GLA_RESOURCE_ID_INFO *)ncshm_take_hdl(
                              NCS_SERVICE_ID_GLA,  callback->resourceId);

             if(res_id_node)
             {
                GLA_CLIENT_RES_INFO *client_res_info = NULL;

                res_id_node->gbl_res_id = param->resourceId;
                res_id_node->lock_handle_id = client_info->lock_handle_id;

                client_res_info = gla_client_res_tree_find_and_add(client_info,res_id_node->gbl_res_id, FALSE);
                if(client_res_info)
                    client_res_info->lcl_res_cnt++;
                else
                {
                   client_res_info = gla_client_res_tree_find_and_add(client_info,res_id_node->gbl_res_id, TRUE);
                   if(client_res_info)
                        client_res_info->lcl_res_cnt++;
                }

                ncshm_give_hdl(callback->resourceId);
                client_info->lckCallbk.saLckResourceOpenCallback(param->invocation,
                                                                 callback->resourceId,param->error);
             }
           }
          else
          {
               res_id_node = (GLA_RESOURCE_ID_INFO *)ncshm_take_hdl(
                              NCS_SERVICE_ID_GLA,  callback->resourceId);
                     /* delete the resource node */
               if(res_id_node)
                  gla_res_tree_delete_node(gla_cb,res_id_node);

               client_info->lckCallbk.saLckResourceOpenCallback(param->invocation,
                          param->resourceId,
                          param->error);
          }
          
        }
      }
      break;
      
   case GLSV_LOCK_GRANT_CBK:
         {
          GLSV_LOCK_GRANT_PARAM *param = &callback->params.lck_grant;
          if (client_info->lckCallbk.saLckLockGrantCallback)
          {
            GLA_LOCK_ID_INFO *lock_id_node = NULL;
            res_id_node = (GLA_RESOURCE_ID_INFO *)ncshm_take_hdl(
                              NCS_SERVICE_ID_GLA, param->resourceId);
            if(res_id_node)
            {
               ncshm_give_hdl(param->resourceId);
               if(param->error == SA_AIS_ERR_TRY_AGAIN)
               {
                  /* get the lock id tree and delete */
                       /* retrieve Lock hdl record */
                  lock_id_node  = (GLA_LOCK_ID_INFO *)ncshm_take_hdl(
                                                NCS_SERVICE_ID_GLA, param->lcl_lockId);

                  if(lock_id_node)
                  {
                      gla_lock_tree_delete_node(gla_cb,lock_id_node);

                  }
                  client_info->lckCallbk.saLckLockGrantCallback(param->invocation,0,param->error);
               }
               else
               {
                 /***********/
                 if(param->lockStatus != SA_LCK_LOCK_GRANTED)
                 {
                       /* retrieve Lock hdl record */
                   lock_id_node  = (GLA_LOCK_ID_INFO *)ncshm_take_hdl(
                                                NCS_SERVICE_ID_GLA, param->lcl_lockId);


                   if(lock_id_node)
                   {
                     gla_lock_tree_delete_node(gla_cb,lock_id_node);

                     client_info->lckCallbk.saLckLockGrantCallback(param->invocation,
                                                                   param->lockStatus,
                                                                   param->error);
                  }
                  else
                  {
                     client_info->lckCallbk.saLckLockGrantCallback(param->invocation,0,
                                                                   SA_AIS_ERR_NOT_EXIST);
                  }
               }
               else
               {
                       /* retrieve Lock hdl record */
                   lock_id_node  = (GLA_LOCK_ID_INFO *)ncshm_take_hdl(
                                                NCS_SERVICE_ID_GLA, param->lcl_lockId);


                  if(lock_id_node)
                  {

                     lock_id_node->gbl_lock_id = param->lockId;
                      
                     ncshm_give_hdl(param->lcl_lockId);
 
                     client_info->lckCallbk.saLckLockGrantCallback(param->invocation,
                                                                  param->lockStatus,
                                                                  param->error);
                }
                else
                {
                   client_info->lckCallbk.saLckLockGrantCallback(param->invocation,
                                                                 0,
                                                                 SA_AIS_ERR_NOT_EXIST);
                }
              }
           }
         }
        }
      }

      break;
   case GLSV_LOCK_WAITER_CBK:
      {
         GLSV_LOCK_WAITER_PARAM *param = &callback->params.lck_wait;
         if (client_info->lckCallbk.saLckLockWaiterCallback)
         {
               GLA_LOCK_ID_INFO *lock_id_node = NULL;
               lock_id_node  = (GLA_LOCK_ID_INFO *)ncshm_take_hdl(
                                                NCS_SERVICE_ID_GLA, param->lcl_lockId);

               if(lock_id_node)
               {
                  ncshm_give_hdl(param->lcl_lockId);
                  client_info->lckCallbk.saLckLockWaiterCallback(param->wait_signal,
                     param->lcl_lockId,
                     param->modeHeld,
                     param->modeRequested);
               }
         }
      }

      break;
      
   case GLSV_LOCK_UNLOCK_CBK:
      {

         GLSV_LOCK_UNLOCK_PARAM *param = &callback->params.unlock;
         if (client_info->lckCallbk.saLckResourceUnlockCallback)
         {
            GLA_LOCK_ID_INFO *lock_id_node = NULL;
            lock_id_node  = (GLA_LOCK_ID_INFO *)ncshm_take_hdl(
                                                NCS_SERVICE_ID_GLA, param->lockId);

            
            if(lock_id_node)
            {
              if( param->error ==  SA_AIS_ERR_TRY_AGAIN)
              {
                  ncshm_give_hdl(param->lockId);
                  client_info->lckCallbk.saLckResourceUnlockCallback(param->invocation,
                                                                     param->error); 
              }
              else
              {
                  gla_lock_tree_delete_node(gla_cb,lock_id_node);
                /* call the callback */
                  client_info->lckCallbk.saLckResourceUnlockCallback(param->invocation,
                                                                     param->error);
              }
            }
            else
            {
                  client_info->lckCallbk.saLckResourceUnlockCallback(param->invocation,
                  SA_AIS_ERR_NOT_EXIST);

            }
         }
      }
      break;
   default:
      break;
   }
   /* free the callback info */
   if(callback)
      m_MMGR_FREE_GLA_CALLBACK_INFO(callback);
}

/****************************************************************************
  Name          : gla_hdl_callbk_dispatch_one
 
  Description   : This routine dispatches one pending callback.
 
  Arguments     : cb      - ptr to the GLA control block
                  client_info - ptr to the client info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 gla_hdl_callbk_dispatch_one (GLA_CB            *cb, 
                                   GLA_CLIENT_INFO   *client_info)
{
   GLSV_GLA_CALLBACK_INFO   *callback;
   GLA_RESOURCE_ID_INFO     *res_id_node = NULL;
   uns32 rc = SA_AIS_OK;

   /* get it from the queue */
   callback = glsv_gla_callback_queue_read(client_info);
   if(callback)
   {
      while(callback)
      {
        res_id_node = (GLA_RESOURCE_ID_INFO *)ncshm_take_hdl(
                              NCS_SERVICE_ID_GLA,  callback->resourceId);

        if(res_id_node)
        { 
           ncshm_give_hdl(callback->resourceId);
           /* process the callback */
           gla_process_callback(cb,client_info,callback);
           break;
        }
        else
          callback = glsv_gla_callback_queue_read(client_info);
      }
   }
   else
   {
      m_LOG_GLA_API(GLA_API_LCK_DISPATCH_FAIL,NCSFL_SEV_INFO);
   }
   return rc;
}

/****************************************************************************
  Name          : gla_hdl_callbk_dispatch_all
 
  Description   : This routine dispatches all pending callback.
 
  Arguments     : cb      - ptr to the GLA control block
                  client_info - ptr to the client info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 gla_hdl_callbk_dispatch_all(GLA_CB  *cb,GLA_CLIENT_INFO *client_info)
{
   GLSV_GLA_CALLBACK_INFO   *callback;

   while((callback = glsv_gla_callback_queue_read(client_info)))
   {
      gla_process_callback(cb,client_info,callback);
   }

   
   return SA_AIS_OK;
}


/****************************************************************************
  Name          : gla_hdl_callbk_dispatch_block
 
  Description   : This routine dispatches all pending callback.
 
  Arguments     : cb      - ptr to the GLA control block
                  client_info - ptr to the client info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 gla_hdl_callbk_dispatch_block(GLA_CB  *gla_cb,GLA_CLIENT_INFO *client_info)
{
   GLSV_GLA_CALLBACK_INFO   *callback = NULL;
   SaLckHandleT             hdl = client_info->lock_handle_id;  
   uns32 rc = SA_AIS_OK;
#if 0
   NCS_SEL_OBJ_SET          all_sel_obj;
   NCS_SEL_OBJ              sel_obj = m_NCS_IPC_GET_SEL_OBJ(&client_info->callbk_mbx);
   m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
   m_NCS_SEL_OBJ_SET(sel_obj,&all_sel_obj); 

   while(m_NCS_SEL_OBJ_SELECT(sel_obj,&all_sel_obj,0,0,0) != -1)
   {
      if (m_NCS_SEL_OBJ_ISSET(sel_obj,&all_sel_obj))
      {
         if(gla_client_tree_find_and_add(gla_cb,hdl,FALSE))
         {
            callback = glsv_gla_callback_queue_read(client_info);
            if(callback)
               gla_process_callback(gla_cb,client_info,callback);
         }
      }
      else
      {
         /* check to see the validity of the handle */
         if(gla_client_tree_find_and_add(gla_cb,hdl,FALSE))
         {
            /* Big amiss happened, so delete the client*/
            m_LOG_GLA_API(GLA_DISPATCH_BLOCK_CLIENT_DESTROYED,NCSFL_SEV_ERROR);
            if(client_info)
            {
               m_NCS_LOCK(&gla_cb->cb_lock, NCS_LOCK_WRITE);
               gla_client_tree_delete_node(gla_cb, client_info);
               m_NCS_UNLOCK(&gla_cb->cb_lock, NCS_LOCK_WRITE);
            }
            return SA_AIS_ERR_LIBRARY;
         }
         else
         {
            /* another thread executed the saLckFinalize - so everything is fine */
            return SA_AIS_OK;
         }

      }
      m_NCS_SEL_OBJ_SET(sel_obj,&all_sel_obj);
   }
#endif
  for(;;)
   {
      if (NULL != (callback = (GLSV_GLA_CALLBACK_INFO *)
               m_NCS_IPC_RECEIVE(&client_info->callbk_mbx, callback)))
      {
         gla_process_callback(gla_cb,client_info,callback);

         /* check to see the validity of the hdl.*/
         if (NULL == gla_client_tree_find_and_add(gla_cb,hdl,FALSE))
             return rc;
      }
      else
         return rc; /* FIX to handle finalize clean up of mbx */
   }

   return SA_AIS_OK;
}
