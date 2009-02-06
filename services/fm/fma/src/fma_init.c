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

#include "fma.h"

/** Global: CB handle returned by hdl manager **/
uns32 gl_fma_hdl = 0;
void *g_fma_task_hdl = NULL;
 
static uns32 fma_create(NCS_LIB_CREATE *create_info);
static uns32 fma_destroy(NCS_LIB_DESTROY *destroy_info);
static uns32 fma_hpl_init(void);
static uns32 fma_hpl_finalize(void);

static uns32 lib_use_count = 0;


/****************************************************************************
  Name          : fma_fm_node_reset_ind

  Description   : Processes the node_reset indication message.

  Arguments     : cb - pointer to the FMA control block.
                  phy_addr - physical address of the entity

  Return Values : NCSCC_RC_SUCCESS

  Notes         : None.
******************************************************************************/
static uns32 fma_fm_node_reset_ind(FMA_CB *cb, FMA_FM_PHY_ADDR phy_addr)
{
   NCS_PATRICIA_NODE *node = NULL;
   FMA_HDL_REC *hdl_rec = NULL;
   FMA_PEND_CBK_REC *pend_cbk_rec = NULL;

   m_FMA_LOG_FUNC_ENTRY("fma_fm_node_reset_ind");

   node = ncs_patricia_tree_getnext(&(cb->hdl_db.hdl_db_anchor),NULL);
   while (node)
   {
      hdl_rec = (FMA_HDL_REC*)ncshm_take_hdl(NCS_SERVICE_ID_FMA, 
                                             *(uns32 *)(node->key_info));
      if(!hdl_rec)
      {
         m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_GET, FMA_LOG_HDL_DB_FAILURE, 
                          *(uns32 *)(node->key_info), NCSFL_SEV_CRITICAL);
         goto hdl_rec_take_hdl_failed;
      }

      pend_cbk_rec = m_MMGR_ALLOC_FMA_PEND_CBK_REC;
      if (!pend_cbk_rec)
      {
         m_FMA_LOG_MEM(FMA_LOG_PEND_CBK_REC_ALLOC,FMA_LOG_MEM_ALLOC_FAILURE,
                       NCSFL_SEV_EMERGENCY);
         goto hdl_rec_mem_alloc_failed;
      }

      m_NCS_OS_MEMSET(pend_cbk_rec, 0, sizeof(FMA_PEND_CBK_REC));

      pend_cbk_rec->cbk_info = m_MMGR_ALLOC_FMA_CBK_INFO;
      if (!(pend_cbk_rec->cbk_info))
      {
         m_FMA_LOG_MEM(FMA_LOG_CBK_INFO_ALLOC,FMA_LOG_MEM_ALLOC_FAILURE,NCSFL_SEV_EMERGENCY);
         goto hdl_rec_cbk_info_mem_alloc_failed;
      }

      m_NCS_OS_MEMSET(pend_cbk_rec->cbk_info, 0, sizeof(FMA_CBK_INFO));

      /* Populate the pending callback structure */ 
      fma_get_ent_path_from_slot_site(&(pend_cbk_rec->cbk_info->node_reset_info), 
                                      cb, phy_addr.slot, phy_addr.site);
      pend_cbk_rec->cbk_info->cbk_type = NODE_RESET_IND;

      /* This hdl is used as invocation object for calling callback */
      pend_cbk_rec->cbk_info->inv = ncshm_create_hdl(cb->pool_id, NCS_SERVICE_ID_FMA,
                                                     (NCSCONTEXT)pend_cbk_rec);
      if(!(pend_cbk_rec->cbk_info->inv))
      {
         m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_CBK_ADD,FMA_LOG_HDL_DB_FAILURE,0,
                          NCSFL_SEV_CRITICAL);
         goto pend_cbk_rec_inv_hdl_create_failed;
      }

      /* Add the pending callback to the list */
      m_FMA_HDL_PEND_CBK_ADD(&(hdl_rec->pend_cbk), pend_cbk_rec);
      
      /* Set the FD of the rec */
      m_NCS_SEL_OBJ_IND(hdl_rec->sel_obj);      

      ncshm_give_hdl(*(uns32 *)(node->key_info));

      node = ncs_patricia_tree_getnext(&(cb->hdl_db.hdl_db_anchor),
                                       (const uns8*)node->key_info);
   } 

   return NCSCC_RC_SUCCESS;

pend_cbk_rec_inv_hdl_create_failed:
   m_MMGR_FREE_FMA_CBK_INFO(pend_cbk_rec->cbk_info);

hdl_rec_cbk_info_mem_alloc_failed:
   m_MMGR_FREE_FMA_PEND_CBK_REC(pend_cbk_rec);

hdl_rec_mem_alloc_failed:
   ncshm_give_hdl(*(uns32 *)(node->key_info));

hdl_rec_take_hdl_failed: 
   return NCSCC_RC_FAILURE;
}


/****************************************************************************
  Name          : fma_fm_switchover_req

  Description   : Processes the switchover request message.

  Arguments     : None.

  Return Values : NCSCC_RC_SUCCESS

  Notes         : NCSCC_RC_SUCCESS
******************************************************************************/
static uns32 fma_fm_switchover_req(void)
{
   NCS_PATRICIA_NODE *node = NULL;
   FMA_HDL_REC *hdl_rec = NULL;
   FMA_PEND_CBK_REC *pend_cbk_rec = NULL;
   FMA_CB *cb;  
 
   m_FMA_LOG_FUNC_ENTRY("fma_fm_switchover_req");

   cb = (FMA_CB*)ncshm_take_hdl(NCS_SERVICE_ID_FMA, gl_fma_hdl);
   if (!cb)
   {
      m_FMA_LOG_CB(FMA_LOG_CB_RETRIEVE, FMA_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      goto fma_cb_take_hdl_failed;
   }

   node = ncs_patricia_tree_getnext(&(cb->hdl_db.hdl_db_anchor),NULL);
   while (node)
   {
      hdl_rec = (FMA_HDL_REC*)ncshm_take_hdl(NCS_SERVICE_ID_FMA, 
                                             *(uns32 *)(node->key_info));
      if(!hdl_rec)
      {
         m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_GET, FMA_LOG_HDL_DB_FAILURE, 
                          *(uns32 *)(node->key_info), NCSFL_SEV_CRITICAL);
         goto hdl_rec_take_hdl_failed;
      }

      pend_cbk_rec = m_MMGR_ALLOC_FMA_PEND_CBK_REC;
      if (!pend_cbk_rec)
      {
         m_FMA_LOG_MEM(FMA_LOG_PEND_CBK_REC_ALLOC,FMA_LOG_MEM_ALLOC_FAILURE,
                       NCSFL_SEV_EMERGENCY);
         goto hdl_rec_mem_alloc_failed;
      }

      m_NCS_OS_MEMSET(pend_cbk_rec, 0, sizeof(FMA_PEND_CBK_REC));

      pend_cbk_rec->cbk_info = m_MMGR_ALLOC_FMA_CBK_INFO;
      if (!(pend_cbk_rec->cbk_info))
      {
         m_FMA_LOG_MEM(FMA_LOG_CBK_INFO_ALLOC,FMA_LOG_MEM_ALLOC_FAILURE,NCSFL_SEV_EMERGENCY);
         goto hdl_rec_cbk_info_mem_alloc_failed;
      }

      m_NCS_OS_MEMSET(pend_cbk_rec->cbk_info, 0, sizeof(FMA_CBK_INFO));
       
      /* Populate the pending callback structure */
      pend_cbk_rec->cbk_info->cbk_type = SWITCHOVER_REQ;

      /* This handle is used as invocation object for calling callback */
      pend_cbk_rec->cbk_info->inv = ncshm_create_hdl(cb->pool_id, NCS_SERVICE_ID_FMA,
                                                     (NCSCONTEXT)pend_cbk_rec);
      if(!(pend_cbk_rec->cbk_info->inv))
      {
         m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_REC_CBK_ADD,FMA_LOG_HDL_DB_FAILURE,0,NCSFL_SEV_CRITICAL);
         goto pend_cbk_rec_inv_hdl_create_failed;
      }

      /* Add the pending callback to the list */
      m_FMA_HDL_PEND_CBK_ADD(&(hdl_rec->pend_cbk), pend_cbk_rec);

      /* Set the FD of the rec */
       m_NCS_SEL_OBJ_IND(hdl_rec->sel_obj);

      ncshm_give_hdl(*(uns32 *)(node->key_info));

      node = ncs_patricia_tree_getnext(&(cb->hdl_db.hdl_db_anchor),(const uns8*)node->key_info);
   }

   ncshm_give_hdl(gl_fma_hdl);

   return NCSCC_RC_SUCCESS;

pend_cbk_rec_inv_hdl_create_failed:
   m_MMGR_FREE_FMA_CBK_INFO(pend_cbk_rec->cbk_info);
hdl_rec_cbk_info_mem_alloc_failed:
   m_MMGR_FREE_FMA_PEND_CBK_REC(pend_cbk_rec);
hdl_rec_mem_alloc_failed:
   ncshm_give_hdl(*(uns32 *)(node->key_info));
hdl_rec_take_hdl_failed:
   ncshm_give_hdl(gl_fma_hdl);
fma_cb_take_hdl_failed:
   return NCSCC_RC_FAILURE;
}


/****************************************************************************
  Name          : fma_mbx_msg_handler
   
  Description   : Processes the current mailbox event based on message type. 
      
  Arguments     : cb - pointer to the FMA control block.
                  fma_mbx_evt - pointer to the FMA_MBX_EVT_T in the mailbox. 
   
  Return Values : None. 
   
  Notes         : None.
******************************************************************************/
static void fma_mbx_msg_handler(FMA_CB *cb,FMA_MBX_EVT_T *fma_mbx_evt)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   FMA_FM_MSG *msg;


   m_FMA_LOG_FUNC_ENTRY("fma_mbx_msg_handler");

   if(fma_mbx_evt == NULL)
      return;

   if (cb == NULL)
      return;
    
   msg = fma_mbx_evt->msg;
   switch ( msg->msg_type)
   {
   case FMA_FM_EVT_NODE_RESET_IND:
       rc = fma_fm_node_reset_ind(cb, msg->info.phy_addr);
       if(NCSCC_RC_SUCCESS != rc)
          m_FMA_LOG_CBK(FMA_LOG_CBK_NODE_RESET_IND, FMA_LOG_CBK_FAILURE, NCSFL_SEV_CRITICAL);
       else
          m_FMA_LOG_CBK(FMA_LOG_CBK_NODE_RESET_IND, FMA_LOG_CBK_SUCCESS, NCSFL_SEV_INFO);
       break;

   case FMA_FM_EVT_SYSMAN_SWITCH_REQ:
       rc = fma_fm_switchover_req();
       if(NCSCC_RC_SUCCESS != rc)
          m_FMA_LOG_CBK(FMA_LOG_CBK_SWOVER_REQ, FMA_LOG_CBK_FAILURE, NCSFL_SEV_CRITICAL);
       else
          m_FMA_LOG_CBK(FMA_LOG_CBK_SWOVER_REQ, FMA_LOG_CBK_SUCCESS, NCSFL_SEV_INFO);
       break;

   default:
       m_FMA_LOG_MISC(FMA_INVALID_PARAM,NCSFL_SEV_INFO,"fma_mbx_msg_handler");
       break;
   }  

   if (NULL != fma_mbx_evt)
   {
      m_MMGR_FREE_FMA_FM_MSG(fma_mbx_evt->msg);
      m_MMGR_FREE_FMA_MBX_EVT(fma_mbx_evt);
   }

   return;
}


/****************************************************************************
  Name          : fma_main_proc
 
  Description   : Main thread of FMA.
 
  Arguments     : Pointer to the fma global init handle. 
 
  Return Values : None. 
 
  Notes         : None.
******************************************************************************/
static void fma_main_proc( uns32 *fma_init_hdl)
{
   FMA_CB *cb;
   NCS_SEL_OBJ      mbx_sel_obj;
   NCS_SEL_OBJ_SET  temp_sel_obj_set;
   FMA_MBX_EVT_T    *fma_mbx_evt; 
   uns32            msg;

   USE(msg);
   
   m_FMA_LOG_FUNC_ENTRY("fma_main_proc");

   if (fma_init_hdl == NULL)
      return ;
 
   if (*fma_init_hdl != gl_fma_hdl)
      return ;

   /** Get FMA CB **/
   cb = ncshm_take_hdl(NCS_SERVICE_ID_FMA, gl_fma_hdl);    
   if (cb == NULL)
   {
      m_FMA_LOG_CB(FMA_LOG_CB_RETRIEVE, FMA_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return ;
   }
   
   /** Get mailbox selection object **/
   mbx_sel_obj = m_NCS_IPC_GET_SEL_OBJ(&cb->mbx);

   /** Reset the selection object to ZERO**/
   m_NCS_SEL_OBJ_ZERO(&cb->sel_obj);  

   /** Set the mbx selection object in FMA_CB selection object set **/
   m_NCS_SEL_OBJ_SET(mbx_sel_obj, &cb->sel_obj);

   temp_sel_obj_set = cb->sel_obj;
   ncshm_give_hdl(gl_fma_hdl);

   /** Wait infinitely till something written on mbx fd that is FMA receives 
       msg from FM **/ 
   while((m_NCS_SEL_OBJ_SELECT(mbx_sel_obj,&temp_sel_obj_set,NULL,NULL,NULL)!= -1))
   {
      if (m_NCS_SEL_OBJ_ISSET(mbx_sel_obj, &temp_sel_obj_set))
      {
         cb = ncshm_take_hdl(NCS_SERVICE_ID_FMA, gl_fma_hdl);
         if (cb == NULL)
            continue;

         fma_mbx_evt = (FMA_MBX_EVT_T*) m_NCS_IPC_NON_BLK_RECEIVE(&(cb->mbx), msg);
         if (fma_mbx_evt)
         {
            fma_mbx_msg_handler(cb, fma_mbx_evt);      
         }

         ncshm_give_hdl(gl_fma_hdl);
      }      
      temp_sel_obj_set = cb->sel_obj;
   }
    
   m_NCS_SYSLOG(NCS_LOG_INFO, "Exiting from FMA Thread. \n");

   return ;
}


/****************************************************************************
  Name          : fma_lib_req
 
  Description   : Initializes the fma library & creates the fma thread.
                  
  Arguments     : req_info - pointer to the single entry API req info.
 
  Return Values : NCSCC_RC_SUCCESS/error_code
 
  Notes         : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
******************************************************************************/
uns32 fma_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   uns32 rc = NCSCC_RC_SUCCESS;
  
   m_FMA_LOG_FUNC_ENTRY("fma_lib_req"); 

   switch (req_info->i_op)
   {
   case NCS_LIB_REQ_CREATE:
       rc =  fma_create(&req_info->info.create);
       if (rc == NCSCC_RC_SUCCESS)
          m_FMA_LOG_SEAPI(FMA_LOG_SEAPI_CREATE,FMA_LOG_SEAPI_SUCCESS,NCSFL_SEV_NOTICE);
       else
          m_FMA_LOG_SEAPI(FMA_LOG_SEAPI_CREATE,FMA_LOG_SEAPI_FAILURE,NCSFL_SEV_CRITICAL);
       break;
 
   case NCS_LIB_REQ_DESTROY:
        rc = fma_destroy(&req_info->info.destroy);
        if (rc == NCSCC_RC_SUCCESS)
           m_NCS_CONS_PRINTF("FMA Destroy Success\n");
        else
        {
           m_FMA_LOG_SEAPI(FMA_LOG_SEAPI_DESTROY,FMA_LOG_SEAPI_FAILURE,NCSFL_SEV_CRITICAL);
           m_NCS_CONS_PRINTF("FMA Destroy Failure\n");
        }
        break;
          
   case NCS_LIB_REQ_INSTANTIATE:
       m_FMA_LOG_SEAPI(FMA_LOG_SEAPI_INSTANTIATE, FMA_LOG_SEAPI_SUCCESS,NCSFL_SEV_NOTICE);
       rc = NCSCC_RC_SUCCESS;
       break;
         
   case NCS_LIB_REQ_UNINSTANTIATE:
       m_FMA_LOG_SEAPI(FMA_LOG_SEAPI_UNINSTANTIATE, FMA_LOG_SEAPI_SUCCESS,NCSFL_SEV_NOTICE);
       rc = NCSCC_RC_SUCCESS;
       break;

   default:
       m_FMA_LOG_MISC(FMA_INVALID_PARAM,NCSFL_SEV_INFO,"fma_lib_req");
       break;
   }
 
   return rc;
}


/****************************************************************************
  Name          : fma_create
 
  Description   : Initializes the core agents required for FMA & creates the
                  FMA thread.
 
  Arguments     : create_info - ptr to the NCS_LIB_CREATE
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
static uns32 fma_create (NCS_LIB_CREATE *create_info)
{
   FMA_CB *cb = 0;
   uns32 rc = NCSCC_RC_SUCCESS;
   int argc = 0;
   char **argv;
   NCS_LIB_CREATE hisv_create_info;
 
   m_FMA_LOG_FUNC_ENTRY("fma_create");

   if(lib_use_count)
   {
      lib_use_count++;
      return NCSCC_RC_SUCCESS;
   }
    
   /* Start NCS agents */
   if (ncs_agents_startup(argc, argv) != NCSCC_RC_SUCCESS)
   {
      rc = NCSCC_RC_FAILURE;
      goto fma_ncs_agents_startup_fail;
   }
    
   /**  Registering with logging subsystem: DLSv **/
   if (fma_log_reg() != NCSCC_RC_SUCCESS)
   {
      rc = NCSCC_RC_FAILURE;
      goto fma_log_reg_fail;
   }
    
   /** Allocate memory for control block **/
   cb = m_MMGR_ALLOC_FMA_CB;
   if (cb == NULL)
   {
      m_FMA_LOG_MEM(FMA_LOG_CB_ALLOC,FMA_LOG_MEM_ALLOC_FAILURE,NCSFL_SEV_CRITICAL);
      m_FMA_LOG_CB(FMA_LOG_CB_CREATE,FMA_LOG_CB_FAILURE,NCSFL_SEV_CRITICAL);
      rc = NCSCC_RC_FAILURE;
      goto fma_cb_alloc_fail;
   }

   m_NCS_OS_MEMSET(cb, 0, sizeof(FMA_CB));
    
   cb->pool_id = NCS_HM_POOL_ID_COMMON;
    
   /** create the association with hdl-mngr **/   
   if (!(cb->cb_hdl = ncshm_create_hdl(cb->pool_id, 
                                       NCS_SERVICE_ID_FMA, 
                                       (NCSCONTEXT)cb)))
   {
      m_FMA_LOG_CB(FMA_LOG_CB_HDL_ASS_CREATE,FMA_LOG_MEM_ALLOC_FAILURE,NCSFL_SEV_CRITICAL); 
      rc = NCSCC_RC_FAILURE;
      goto cb_hdl_create_fail;
   }
    
   /** store the hdl in the global variable **/
   gl_fma_hdl = cb->cb_hdl;

   /** store my node id **/
   cb->my_node_id = m_NCS_GET_NODE_ID;
   
   /* Set the is_platform flag to FALSE, default environment is PC */
   cb->is_platform = FALSE;

   /** Initialize the hpl library */
   if (fma_hpl_init() != NCSCC_RC_SUCCESS)
      goto cb_mbx_create_fail;

   /** Create mailbox for processing msg from FM **/
   if (m_NCS_IPC_CREATE (&cb->mbx) != NCSCC_RC_SUCCESS)
   {
      m_FMA_LOG_MBX(FMA_LOG_MBX_CREATE,FMA_LOG_MBX_FAILURE,NCSFL_SEV_CRITICAL); 
      rc = NCSCC_RC_FAILURE;
      goto cb_mbx_create_fail;
   }

   /** Increment the referrence count **/
   m_NCS_IPC_ATTACH(&cb->mbx);
    
   if (m_NCS_TASK_CREATE((NCS_OS_CB)fma_main_proc,
                        &gl_fma_hdl,
                        NCS_FMA_NAME,
                        NCS_FMA_PRIORITY,
                        NCS_FMA_STACK_SIZE,
                        &g_fma_task_hdl) != NCSCC_RC_SUCCESS)
   {
      m_FMA_LOG_TASK(FMA_LOG_TASK_CREATE,FMA_LOG_MBX_FAILURE,NCSFL_SEV_CRITICAL); 
      rc = NCSCC_RC_FAILURE;
      goto task_create_fail;
   }

   /** initialize the fma cb lock **/
   if(m_NCS_LOCK_INIT(&cb->lock) != NCSCC_RC_SUCCESS)
   {
      m_FMA_LOG_LOCK(FMA_LOG_LOCK_INIT,FMA_LOG_LOCK_FAILURE,NCSFL_SEV_CRITICAL);
      rc = NCSCC_RC_FAILURE;
      goto lock_init_fail;
   }
    
   /** Get FMA handle database : Initialize the patricia tree **/ 
   if (fma_hdl_db_init( &cb->hdl_db ) != NCSCC_RC_SUCCESS)
   {
      m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_CREATE,FMA_LOG_HDL_DB_FAILURE,0,NCSFL_SEV_CRITICAL);
      rc = NCSCC_RC_FAILURE;
      goto hdl_db_create_fail;
   }
   
   /** Register with MDS **/
   if (fma_mds_reg(cb) != NCSCC_RC_SUCCESS)
   {
      m_FMA_LOG_MDS(FMA_LOG_MDS_REG, FMA_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL); 
      rc = NCSCC_RC_FAILURE;
      goto mds_reg_fail;
   }   

   if (m_NCS_TASK_START (g_fma_task_hdl) != NCSCC_RC_SUCCESS )
   {
      m_FMA_LOG_TASK(FMA_LOG_TASK_START,FMA_LOG_MBX_FAILURE,NCSFL_SEV_CRITICAL);
      rc = NCSCC_RC_FAILURE;
      goto task_start_fail;
   }
   
   lib_use_count++;

   return rc;
    
task_start_fail:
   fma_mds_dereg(cb);
mds_reg_fail:
   fma_hdl_db_del(cb);    
hdl_db_create_fail: 
   m_NCS_LOCK_DESTROY(&cb->lock); 
lock_init_fail:
   m_NCS_TASK_RELEASE(g_fma_task_hdl);
task_create_fail:
   m_NCS_IPC_RELEASE(&cb->mbx, NULL);
cb_mbx_create_fail:
   ncshm_destroy_hdl(NCS_SERVICE_ID_FMA, cb->cb_hdl);
   gl_fma_hdl = 0; 
cb_hdl_create_fail:   
   m_MMGR_FREE_FMA_CB(cb);
   cb = NULL;
fma_cb_alloc_fail:
   fma_log_dereg();
fma_log_reg_fail:
fma_ncs_agents_startup_fail:
   return rc;
}


/****************************************************************************
  Name          : fma_destroy
 
  Description   : Destroy the FMA thread & library.
 
  Arguments     : create_info - ptr to the NCS_LIB_DESTROY
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
static uns32 fma_destroy (NCS_LIB_DESTROY *destroy_info)
{
   FMA_CB *cb = 0;
   uns32 rc = NCSCC_RC_SUCCESS ;   
   int argc = 0;
   char **argv; 
    
   m_FMA_LOG_FUNC_ENTRY("fma_destroy"); 

   if(!lib_use_count)
   {
      m_NCS_CONS_PRINTF("\nFMA library not yet created...! \n");
      return NCSCC_RC_FAILURE;
   }
    
   if(lib_use_count > 1)
   {
      lib_use_count--;
      return NCSCC_RC_SUCCESS;
   }
    
   cb = (FMA_CB*)ncshm_take_hdl(NCS_SERVICE_ID_FMA, gl_fma_hdl);
   if (!cb)
   {
      m_FMA_LOG_CB(FMA_LOG_CB_RETRIEVE, FMA_LOG_CB_FAILURE, NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;  
   }
   
   /* Finalize the hpl library */
   if (fma_hpl_finalize() != NCSCC_RC_SUCCESS)
      m_NCS_CONS_PRINTF("Hpl library finalize failed\n");

   /** Free all rec and destroy hdl-db patricia tree **/
   fma_hdl_db_del(cb);
   m_FMA_LOG_HDL_DB(FMA_LOG_HDL_DB_DESTROY,FMA_LOG_HDL_DB_SUCCESS,0,NCSFL_SEV_INFO);   
    
   /** Unregister with MDS **/
   rc = fma_mds_dereg(cb);
   if (!rc)
      m_FMA_LOG_MDS(FMA_LOG_MDS_UNREG, FMA_LOG_MDS_FAILURE, NCSFL_SEV_CRITICAL);        
    
   /** Destroy the lock **/
   m_NCS_LOCK_DESTROY(&cb->lock);
   m_FMA_LOG_LOCK(FMA_LOG_LOCK_FINALIZE,FMA_LOG_LOCK_SUCCESS,NCSFL_SEV_INFO);
    
   /** Return FMA CB **/
   ncshm_give_hdl(gl_fma_hdl);
    
   /* Destory FMA_CB hdl */
   ncshm_destroy_hdl(NCS_SERVICE_ID_FMA, cb->cb_hdl);
   m_FMA_LOG_CB(FMA_LOG_CB_HDL_ASS_REMOVE,FMA_LOG_MEM_ALLOC_SUCCESS,NCSFL_SEV_INFO);
   gl_fma_hdl = 0;
    
   /* Free FMA CB */
   m_MMGR_FREE_FMA_CB(cb);
   cb = NULL;
   m_FMA_LOG_CB(FMA_LOG_CB_DESTROY,FMA_LOG_CB_SUCCESS,NCSFL_SEV_INFO);
    
   /* Dergister with DTSV */
   fma_log_dereg();
    
   /* Shutdown NCS agents */
   ncs_agents_shutdown(argc, argv);
    
   if(rc == NCSCC_RC_SUCCESS)
      lib_use_count--;
    
   return rc;
}


/****************************************************************************
  Name          : fma_get_ent_path_from_slot_site
 
  Description   : Constructs an entity path using the slot & site values.
 
  Arguments     : o_ent_path - pointer to the entity path structure.
                  cb - Pointer to Control Block
                  slot - physical slot id.
                  site - sub slot id.
 
  Return Values : None.
 
  Notes         : None
******************************************************************************/
void fma_get_ent_path_from_slot_site(SaHpiEntityPathT *o_ent_path, FMA_CB *cb,
                                     uns8 slot, uns8 site)
{
   uns8 shelf  = 0;
   uns32 count = 0;
   uns32 rc = NCSCC_RC_FAILURE;
   SaHpiEntityPathT temp_epath;

   /* Retrieve the shelf id/chassis id from node_id */
   m_NCS_GET_PHYINFO_FROM_NODE_ID(cb->my_node_id, &shelf, NULL, NULL);

   m_NCS_MEMSET(o_ent_path, 0, sizeof(SaHpiEntityPathT));
   m_NCS_MEMSET(&temp_epath, 0, sizeof(SaHpiEntityPathT));

   if ((0 != site) && (15 != site))
   {
      /* Fill the entity type for site (i.e. subslot/amc card).
       * This is the same as the value configured in /etc/opensaf/subslot_id
       */
      o_ent_path->Entry[count].EntityType = SAHPI_ENT_CHASSIS_SPECIFIC + 7;
      o_ent_path->Entry[count++].EntityLocation = (SaHpiEntityLocationT)site;
   }

   if (cb->is_platform == TRUE)
   {
      /* Attempt to lookup the array-based entity-path first using the HISv lookup fn */
      rc = hpl_entity_path_lookup(HPL_EPATH_FLAG_ARRAY, shelf, slot, (uns8 *) &temp_epath);
   }

   if ((rc == NCSCC_RC_SUCCESS) && (temp_epath.Entry[0].EntityType != 0)) {

       m_NCS_CONS_PRINTF("HPL lookup of entity path successful\n");
       /* Copy the epath dynamically obtained from HPL<->HISV<->HPI */
      o_ent_path->Entry[count].EntityType = temp_epath.Entry[0].EntityType;
      o_ent_path->Entry[count++].EntityLocation = (SaHpiEntityLocationT)temp_epath.Entry[0].EntityLocation;

      o_ent_path->Entry[count].EntityType = temp_epath.Entry[1].EntityType;
      o_ent_path->Entry[count++].EntityLocation = (SaHpiEntityLocationT)temp_epath.Entry[1].EntityLocation;

      o_ent_path->Entry[count].EntityType = temp_epath.Entry[2].EntityType;
      o_ent_path->Entry[count++].EntityLocation = (SaHpiEntityLocationT)temp_epath.Entry[2].EntityLocation;
   }
   else
   {
      /* Fill the default epath */
      o_ent_path->Entry[count].EntityType = SAHPI_ENT_PHYSICAL_SLOT;
      o_ent_path->Entry[count++].EntityLocation = (SaHpiEntityLocationT)slot;

      o_ent_path->Entry[count].EntityType  = SAHPI_ENT_ADVANCEDTCA_CHASSIS;
      o_ent_path->Entry[count++].EntityLocation = (SaHpiEntityLocationT)shelf;

      o_ent_path->Entry[count].EntityType  = SAHPI_ENT_ROOT;
      o_ent_path->Entry[count++].EntityLocation = 0;
   }
   return;
}


/****************************************************************************
  Name          : fma_get_slot_site_from_ent_path
 
  Description   : Extract slot & subslot id from entity path.
 
  Arguments     : ent_path - entitypath structure
                  o_slot - pointer to slot id
                  o_site - pointer to subslot.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 fma_get_slot_site_from_ent_path(SaHpiEntityPathT ent_path, uns8 *o_slot,
                                      uns8 *o_site)
{
   uns32 tmp_var;

   *o_slot = 0;
   *o_site = 0xf;
    
   /* reach to the root element of the entity path */
   for (tmp_var = 0; tmp_var < SAHPI_MAX_ENTITY_PATH; tmp_var++)
   {
      if (ent_path.Entry[tmp_var].EntityType == SAHPI_ENT_ROOT)
         break;
    
      if ((ent_path.Entry[tmp_var].EntityType == SAHPI_ENT_PHYSICAL_SLOT) ||
          (ent_path.Entry[tmp_var].EntityType == SAHPI_ENT_SYSTEM_BLADE)  ||
          (ent_path.Entry[tmp_var].EntityType == SAHPI_ENT_SWITCH_BLADE))
           *o_slot = ent_path.Entry[tmp_var].EntityLocation;
        
      /* enum name for site id (subslot/AMC card) is TBD. for now use the offset 7 */
      if (ent_path.Entry[tmp_var].EntityType == (SAHPI_ENT_CHASSIS_SPECIFIC + 7))
         *o_site = ent_path.Entry[tmp_var].EntityLocation;
   }
    
   if ((tmp_var == 0) || (tmp_var > SAHPI_MAX_ENTITY_PATH))
      return NCSCC_RC_FAILURE;
   
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : fma_hpl_init 
  
  Description   : Initializes the HPL client library.
     
  Arguments     : None.
  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  
  Notes         : None. 
 *****************************************************************************/
static uns32 fma_hpl_init(void)
{
   NCS_LIB_REQ_INFO    req_info;
   uns32   rc = NCSCC_RC_SUCCESS;

   /* Initialize with HPL.*/
   m_NCS_OS_MEMSET(&req_info, '\0', sizeof(req_info));
   req_info.i_op = NCS_LIB_REQ_CREATE;
   rc = ncs_hpl_lib_req(&req_info);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("hpl lib init failed\n ");
      return rc;
   }

   return rc;
}

/****************************************************************************
  Name          : fma_hpl_finalize
 
  Description   : Finalizes the HPL client library.
 
  Arguments     : None.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  
  Notes         : None. 
 ****************************************************************************/
static uns32 fma_hpl_finalize(void)
{
   NCS_LIB_REQ_INFO    req_info;
   uns32   rc = NCSCC_RC_SUCCESS;

   /* Initialize with HPL.*/
   m_NCS_OS_MEMSET(&req_info, '\0', sizeof(req_info));
   req_info.i_op = NCS_LIB_REQ_DESTROY;
   rc = ncs_hpl_lib_req(&req_info);

   return rc;
}

