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

  MODULE NAME:       vds_main.c   


  DESCRIPTION:       

******************************************************************************
*/
#include "vds.h"

/* global cb handle */
uns32 gl_vds_hdl = 0;

uns32 vds_role_ack_to_state =0;
uns32 vds_destroying = FALSE;

/* global task handle */
NCSCONTEXT gl_vds_task_hdl = 0;

/* Static function prototypes */
static void   vds_mbx_process(SYSF_MBX *mbx);
static uns32  vds_create(NCS_LIB_CREATE *create_info);
static uns32  vds_mbx_create(VDS_CB *cb);
static uns32  vds_task_create(void);
static uns32  vds_cb_destroy(VDS_CB *cb);
static uns32  vds_mbx_destroy(VDS_CB *cb);
static VDS_CB *vds_cb_create(void);


/****************************************************************************
  Name          :  vds_lib_req

  Description   :  This routine does the following

  Arguments     :  req

  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE

  Notes         :  None
******************************************************************************/
uns32 vds_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   VDS_TRACE1_ARG1("vds_lib_req\n");

   switch (req_info->i_op)
   {
   case NCS_LIB_REQ_CREATE:
      rc = vds_create(&req_info->info.create);
      if (rc == NCSCC_RC_SUCCESS)
      {
         m_VDS_LOG_MISC(VDS_LOG_MISC_VDS_CREATE,
                                VDS_LOG_MISC_SUCCESS,
                                        NCSFL_SEV_INFO);
      }
      else
      {
         m_VDS_LOG_MISC(VDS_LOG_MISC_VDS_CREATE,
                                VDS_LOG_MISC_FAILURE,
                                        NCSFL_SEV_CRITICAL);
      }
      break;
       
   case NCS_LIB_REQ_DESTROY:
      rc = vds_destroy(VDS_CANCEL_THREAD);
      if (rc == NCSCC_RC_SUCCESS)
      {
         m_VDS_LOG_MISC(VDS_LOG_MISC_VDS_DESTROY,
                                VDS_LOG_MISC_SUCCESS,
                                        NCSFL_SEV_INFO);
      }
      else
      {
         m_VDS_LOG_MISC(VDS_LOG_MISC_VDS_DESTROY,
                                VDS_LOG_MISC_FAILURE,
                                        NCSFL_SEV_CRITICAL);
      }
      break;
       
   default:
      break;
   }

   return rc;
}


/****************************************************************************
  Name          :  vds_create
 
  Description   :  This routine does the following  
                   a) create & initialize VDS control block.
                   b) create & attach VDS mailbox.
                   d) create & start VDS task.
 
  Arguments     :  create_info - ptr to the NCS_LIB_CREATE
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None
******************************************************************************/
uns32 vds_create(NCS_LIB_CREATE *create_info)
{
   VDS_CB *cb = 0;
   uns32  rc = NCSCC_RC_SUCCESS;
  
   rc = vds_log_reg();
   if(rc == NCSCC_RC_FAILURE)
   {
       VDS_TRACE1_ARG1("ERROR In logging ");
       return NCSCC_RC_FAILURE;  
   }
   VDS_TRACE1_ARG1("vds_create\n");
   
   /* create & initialize VDS cb */
   cb = vds_cb_create();
   if (cb == NULL)
      return NCSCC_RC_FAILURE;      

   /* create & attach VDS mailbox */
   rc = vds_mbx_create(cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_VDS_LOG_TIM(VDS_LOG_MBX_CREATE,
                            VDS_LOG_TIM_FAILURE,
                                    NCSFL_SEV_CRITICAL);
      vds_destroy(VDS_DONT_CANCEL_THREAD);
      return rc;    
   }
   else
   {
      m_VDS_LOG_TIM(VDS_LOG_MBX_CREATE,
                            VDS_LOG_TIM_SUCCESS,
                                    NCSFL_SEV_INFO);
   }
      
   /* create & start VDS task */
   rc = vds_task_create();
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_VDS_LOG_TIM(VDS_LOG_TASK_CREATE,
                            VDS_LOG_TIM_FAILURE,
                                    NCSFL_SEV_CRITICAL);
      vds_destroy(VDS_CANCEL_THREAD);
  
      return rc;    
   }
   else
   {
      m_VDS_LOG_TIM(VDS_LOG_TASK_CREATE,
                            VDS_LOG_TIM_SUCCESS,
                                    NCSFL_SEV_INFO);
   }                
   
   return rc;
}


/****************************************************************************
  Name          :  vds_destroy

  Description   :  This routine does the following
                   a) destroys VDS control block.
                   b) detachs VDS mailbox.
                   c) releases VDS task.

  Arguments     :  vds_thread_cancel - NCS_BOOL
                   Release the thread if it is NON Detachable
                   if it is detachable just leave the thread to 
                   exit it self .

  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE

  Notes         :  None
******************************************************************************/
uns32 vds_destroy(NCS_BOOL vds_thread_cancel)
{
   VDS_CB *cb = 0;
   uns32 rc = NCSCC_RC_SUCCESS;

     
   VDS_TRACE1_ARG1("vds_destroy\n");
   

   if (!gl_vds_hdl)
   {     
      return NCSCC_RC_FAILURE;     
   }   
 
   vds_destroying = TRUE;

   if(vds_thread_cancel == VDS_DONT_CANCEL_THREAD && gl_vds_task_hdl!=0)
   {
      m_NCS_TASK_DETACH(gl_vds_task_hdl);
      gl_vds_task_hdl = 0;
   }
   /* retrieve VDS cb */
   if ((cb = (VDS_CB *)ncshm_take_hdl(NCS_SERVICE_ID_VDS,
                                      (uns32)gl_vds_hdl)) == VDS_CB_NULL)
   {
      m_VDS_LOG_HDL(VDS_LOG_HDL_RETRIEVE_CB,
                            VDS_LOG_HDL_FAILURE,
                                    NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;      
   }

   if (cb->mds_init_done)
      vds_mds_unreg(cb);

   rc = vds_amf_finalize(cb); 
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_VDS_LOG_AMF(VDS_LOG_AMF_FINALIZE,
                            VDS_LOG_AMF_FAILURE,
                                    NCSFL_SEV_CRITICAL, rc);
      /* continue destory */
   } 
   else
   {
      m_VDS_LOG_AMF(VDS_LOG_AMF_FINALIZE,
                            VDS_LOG_AMF_SUCCESS,
                                    NCSFL_SEV_INFO, 0);
   }

   
   rc = vds_ckpt_finalize(cb); 
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_VDS_LOG_CKPT(VDS_LOG_CKPT_FINALIZE,
                            VDS_LOG_CKPT_FAILURE,
                                    NCSFL_SEV_CRITICAL, rc);
   } 
   else
   {
     m_VDS_LOG_CKPT(VDS_LOG_CKPT_FINALIZE,
                           VDS_LOG_CKPT_SUCCESS,
                                   NCSFL_SEV_INFO, rc );
   }
   
   rc = vds_mbx_destroy(cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_VDS_LOG_TIM(VDS_LOG_MBX_DESTROY,
                            VDS_LOG_TIM_FAILURE,
                                    NCSFL_SEV_CRITICAL);
   }
   else
   {
      m_VDS_LOG_TIM(VDS_LOG_MBX_DESTROY,
                            VDS_LOG_TIM_SUCCESS,
                                    NCSFL_SEV_INFO);
   }                

   /* Clean VDS database */
   vds_db_scavanger(cb);

   /* destroy VDS control block */
   vds_cb_destroy(cb);
   
   vds_log_unreg();
   
   /* release the task */
   if(vds_thread_cancel != VDS_DONT_CANCEL_THREAD && gl_vds_task_hdl !=0 )
   {
     m_NCS_TASK_RELEASE(gl_vds_task_hdl);
     gl_vds_task_hdl = 0;
   }

   return rc;
}


/****************************************************************************
  Name          : vds_cb_create
 
  Description   : This routine creates & initializes VDS control block.
 
  Arguments     : None.
 
  Return Values : if successfull, ptr to VDS control block
                  else, VDS_CB_NULL
 
  Notes         : None
******************************************************************************/
VDS_CB *vds_cb_create()
{
   VDS_CB *cb = VDS_CB_NULL;
   NCS_PATRICIA_PARAMS pat_param;
   uns32 rc = NCSCC_RC_SUCCESS;

   VDS_TRACE1_ARG1("vds_cb_create\n");

   /* allocate VDS cb */
   if (((cb = m_MMGR_ALLOC_VDS_CB)) == NULL)
   {
      m_VDS_LOG_MEM(VDS_LOG_MEM_VDS_CB,
                            VDS_LOG_MEM_ALLOC_FAILURE,
                                        NCSFL_SEV_CRITICAL);    
      return VDS_CB_NULL;
   }
   
   m_VDS_LOG_MEM(VDS_LOG_MEM_VDS_CB,
                         VDS_LOG_MEM_ALLOC_SUCCESS,
                                     NCSFL_SEV_INFO);
   
   m_NCS_OS_MEMSET((char *)cb, 0, sizeof(VDS_CB));

   /* assign the VDS pool-id (used by hdl-mngr) */
   cb->pool_id = NCS_HM_POOL_ID_COMMON;

   /* initialize the VDS cb lock */
   m_NCS_LOCK_INIT(&cb->cb_lock);
   m_VDS_LOG_LOCK(VDS_LOG_LOCK_INIT,
                          VDS_LOG_LOCK_SUCCESS,
                                  NCSFL_SEV_INFO);
   
 
   /* create the association with hdl-mngr */
   if (((cb->cb_hdl = ncshm_create_hdl(cb->pool_id,
                                       NCS_SERVICE_ID_VDS, 
                                       (NCSCONTEXT)cb))) == 0)
   {
       m_VDS_LOG_HDL(VDS_LOG_HDL_CREATE_CB,
                             VDS_LOG_HDL_FAILURE,
                                     NCSFL_SEV_CRITICAL); 

      vds_cb_destroy(cb);
      return VDS_CB_NULL;      
   }

    m_VDS_LOG_HDL(VDS_LOG_HDL_CREATE_CB,
                          VDS_LOG_HDL_SUCCESS,
                                  NCSFL_SEV_INFO);
   
   m_NCS_OS_MEMSET((void *) &pat_param, 0, sizeof(NCS_PATRICIA_PARAMS));

   /* STEP : Init VDEST-NAME DB */
   pat_param.key_size = sizeof(SaNameT);
   pat_param.info_size = 0;

   rc = ncs_patricia_tree_init(&cb->vdest_name_db,&pat_param);
   if( rc != NCSCC_RC_SUCCESS)
   {
      m_VDS_LOG_TREE(VDS_LOG_PAT_INIT,
                             VDS_LOG_PAT_FAILURE,
                                     NCSFL_SEV_CRITICAL, rc);

      vds_cb_destroy(cb);
      return  VDS_CB_NULL;
   }
   m_VDS_LOG_TREE(VDS_LOG_PAT_INIT,
                          VDS_LOG_PAT_SUCCESS,
                                  NCSFL_SEV_INFO, rc);

   /* STEP : Init VDEST DB */
   pat_param.key_size = sizeof(MDS_DEST);
   pat_param.info_size = 0;
 
   rc = ncs_patricia_tree_init(&cb->vdest_id_db,&pat_param);
     
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_VDS_LOG_TREE(VDS_LOG_PAT_INIT,
                             VDS_LOG_PAT_FAILURE,
                                     NCSFL_SEV_CRITICAL, rc);
      vds_cb_destroy(cb);
      return  VDS_CB_NULL;
   }

   m_VDS_LOG_TREE(VDS_LOG_PAT_INIT,
                          VDS_LOG_PAT_SUCCESS,
                                  NCSFL_SEV_INFO ,rc); 
   /* STEP : Set the latest_allocated_vdest */
   cb->latest_allocated_vdest = NCSVDA_UNNAMED_MAX;

   gl_vds_hdl = cb->cb_hdl; 

   return cb;
}


/****************************************************************************
  Name          :  vds_mbx_create
 
  Description   :  This routine creates & attaches VDS mailbox.
 
  Arguments     :  cb - ptr to VDS control block
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None
******************************************************************************/
uns32 vds_mbx_create (VDS_CB *cb)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   VDS_TRACE1_ARG1("vds_mbx_create\n");

   /* create the mail box */
   rc = m_NCS_IPC_CREATE(&cb->mbx);
   if (rc != NCSCC_RC_SUCCESS)
   {
      /* LOG the message */
      m_VDS_LOG_TIM(VDS_LOG_IPC_CREATE,
                            VDS_LOG_TIM_FAILURE,
                                    NCSFL_SEV_CRITICAL); 
      return rc;
   }
   
    m_VDS_LOG_TIM(VDS_LOG_IPC_CREATE,
                          VDS_LOG_TIM_SUCCESS,
                                  NCSFL_SEV_INFO); 
   
   /* attach the mail box */
   rc = m_NCS_IPC_ATTACH(&cb->mbx);
   if (rc != NCSCC_RC_SUCCESS)
   {
       m_VDS_LOG_TIM(VDS_LOG_IPC_ATTACH,
                             VDS_LOG_TIM_FAILURE,
                                     NCSFL_SEV_CRITICAL);
      /* destroy the mailbox */
      vds_mbx_destroy(cb);
      return rc;
   }
   
    m_VDS_LOG_TIM(VDS_LOG_IPC_ATTACH,
                          VDS_LOG_TIM_SUCCESS,
                                  NCSFL_SEV_INFO); 
   return rc;
}


/****************************************************************************
  Name          : vds_task_create
 
  Description   : This routine creates & starts VDS task.
 
  Arguments     : None.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 vds_task_create ()
{
   uns32 rc = NCSCC_RC_SUCCESS;

   
   VDS_TRACE1_ARG1("vds_task_create\n");

   /* create vds task */
   rc = m_NCS_TASK_CREATE((NCS_OS_CB)vds_main_process,
                          (void *)&gl_vds_hdl,
                          "VDS",
                          m_VDS_TASK_PRIORITY,
                          m_VDS_STACKSIZE,
                          &gl_vds_task_hdl);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_VDS_LOG_TIM(VDS_LOG_TASK_CREATE,
                            VDS_LOG_TIM_FAILURE,
                                    NCSFL_SEV_CRITICAL); 
      return rc;
   }
   else
   {
      m_VDS_LOG_TIM(VDS_LOG_TASK_CREATE,
                            VDS_LOG_TIM_SUCCESS,
                                    NCSFL_SEV_INFO); 
   }

   /* now start the task */
   rc = m_NCS_TASK_START(gl_vds_task_hdl);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_VDS_LOG_TIM(VDS_LOG_TASK_START,
                            VDS_LOG_TIM_FAILURE,
                                    NCSFL_SEV_CRITICAL);
      /* release the task */
      m_NCS_TASK_RELEASE(gl_vds_task_hdl);      
   }
   else
   {
      m_VDS_LOG_TIM(VDS_LOG_TASK_START,
                            VDS_LOG_TIM_SUCCESS,
                                    NCSFL_SEV_INFO); 
   }

   return rc;
}


/****************************************************************************
  Name          :  vds_db_scavanger
 
  Description   :  This routine cleans the VDS database.
 
  Arguments     :  cb - ptr to VDS control block
 
  Return Values :  Nothing
 
  Notes         :  None
******************************************************************************/
void vds_db_scavanger(VDS_CB *cb)
{
   VDS_VDEST_DB_INFO *vdest_db_node = NULL;
   VDS_VDEST_ADEST_INFO *adest_node = NULL, *tmp_adest_node = NULL;
   uns32 rc = NCSCC_RC_SUCCESS;


   VDS_TRACE1_ARG1("vds_db_scavanger\n");

   m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE);

   /* Clearoff VDEST info from the VDS database */
   while((vdest_db_node = (VDS_VDEST_DB_INFO *)ncs_patricia_tree_getnext(&cb->vdest_name_db, 
                         (uns8 *)0)) != NULL)
   {
      /* Delete the adest list corresponding to VDEST */
      adest_node = vdest_db_node->adest_list;
      while (adest_node)
      {
         tmp_adest_node = adest_node->next;

         /* FREE the adest node */
         m_MMGR_FREE_VDS_ADEST_INFO(adest_node);

         /* Go for the next adest node */
         adest_node = tmp_adest_node;
      }

      /* Make ANC_LIST ptr of VDEST node to NULL */
      vdest_db_node->adest_list = NULL;

      /* Delete the VDEST from the vdest-named PAT tree */
      rc = ncs_patricia_tree_del(&cb->vdest_name_db,
                            &vdest_db_node->name_pat_node);
       
      if(rc != NCSCC_RC_SUCCESS)
      {
          m_VDS_LOG_TREE(VDS_LOG_PAT_DEL_NAME,
                               VDS_LOG_PAT_FAILURE,
                                       NCSFL_SEV_CRITICAL, rc);
      }
      else
       m_VDS_LOG_TREE(VDS_LOG_PAT_DEL_NAME,
                               VDS_LOG_PAT_SUCCESS,
                                       NCSFL_SEV_INFO, rc);

      /* Delete the VDEST from the vdest-id PAT tree */
      rc = ncs_patricia_tree_del(&cb->vdest_id_db,
                            &vdest_db_node->id_pat_node);
      if( rc  != NCSCC_RC_SUCCESS)
      {
         m_VDS_LOG_TREE(VDS_LOG_PAT_DEL_ID,
                                VDS_LOG_PAT_FAILURE,
                                        NCSFL_SEV_CRITICAL, rc);
      }
      else
       m_VDS_LOG_TREE(VDS_LOG_PAT_DEL_ID,
                               VDS_LOG_PAT_SUCCESS,
                                       NCSFL_SEV_INFO, rc);

      /* Free the VDEST Node */
      m_MMGR_FREE_VDS_DB_INFO(vdest_db_node);
   }
 
   m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);

   return;
}


/****************************************************************************
  Name          : vds_cb_destroy
 
  Description   : This routine destroys VDS control block.
 
  Arguments     : cb - ptr to VDS control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 vds_cb_destroy(VDS_CB *cb)
{
 
   VDS_TRACE1_ARG1("vds_cb_destroy\n");

   ncs_patricia_tree_destroy(&cb->vdest_name_db);
   ncs_patricia_tree_destroy(&cb->vdest_id_db);

   /* destroy the lock */
   m_NCS_LOCK_DESTROY(&cb->cb_lock);
   m_VDS_LOG_LOCK(VDS_LOG_LOCK_DESTROY,
                    VDS_LOG_LOCK_SUCCESS,
                    NCSFL_SEV_INFO);
   if (gl_vds_hdl)
   {
      ncshm_give_hdl(gl_vds_hdl);

      /* remove the association with hdl-mngr */
      ncshm_destroy_hdl(NCS_SERVICE_ID_VDS, gl_vds_hdl);
      m_VDS_LOG_HDL(VDS_LOG_HDL_DESTROY_CB,
                           VDS_LOG_HDL_SUCCESS,
                                 NCSFL_SEV_INFO);
   
      /* reset the global cb handle */
      gl_vds_hdl = 0;
   }

   /* free the control block */
   m_MMGR_FREE_VDS_CB(cb);
   
   m_VDS_LOG_CB(VDS_LOG_CB_DESTROY,
                        VDS_LOG_MISC_SUCCESS,
                                NCSFL_SEV_INFO);  
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          : vds_mbx_destroy
 
  Description   : This routine destroys & detaches VDS mailbox.
 
  Arguments     : cb - ptr to VDS control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 vds_mbx_destroy (VDS_CB *cb)
{
   uns32  rc = NCSCC_RC_SUCCESS;

   VDS_TRACE1_ARG1("vds_mbx_destroy\n");

   if (!(cb->mbx))
      return rc;

   /* detach the mail box */
   rc = m_NCS_IPC_DETACH(&cb->mbx, vds_mbx_clean, cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
      /* LOG the message */
      m_VDS_LOG_TIM(VDS_LOG_IPC_DETACH,
                            VDS_LOG_TIM_FAILURE,
                                NCSFL_SEV_CRITICAL);
      return rc;
   }

   /* delete the mail box */
   rc = m_NCS_IPC_RELEASE(&cb->mbx, 0);
   if (rc != NCSCC_RC_SUCCESS)
   { 
      m_VDS_LOG_TIM(VDS_LOG_IPC_RELEASE,
                        VDS_LOG_TIM_FAILURE,
                            NCSFL_SEV_CRITICAL);
      return rc;
   }  

   cb->mbx = 0;
   m_VDS_LOG_TIM(VDS_LOG_IPC_RELEASE,
                     VDS_LOG_TIM_SUCCESS,
                          NCSFL_SEV_INFO);
   return rc;
}


/****************************************************************************
   Name          :  vds_mbx_clean
  
   Description   :  This routine dequeues & deletes all the events from the 
                    mailbox. It is invoked when mailbox is detached.
  
   Arguments     :  arg - argument to be passed
                    msg - ptr to the 1st event in the mailbox
  
   Return Values :  TRUE/FALSE
  
   Notes         :  None.
*****************************************************************************/
NCS_BOOL vds_mbx_clean (NCSCONTEXT arg, NCSCONTEXT msg)
{
    VDS_EVT *curr = (VDS_EVT *)msg;

   if(curr != NULL)
   {
      vds_evt_destroy(curr);
   }
   else
    return FALSE;

   return TRUE;
}

/****************************************************************************
  Name          :  vds_main_process
 
  Description   :  This routine is an entry point for the VDS task.
 
  Arguments     :  arg - ptr to the cb handle
 
  Return Values :  None.
 
  Notes         :  None
******************************************************************************/
void vds_main_process(void *arg)
{
   VDS_CB       *cb = NULL;
   SYSF_MBX     *mbx = NULL;
   uns32        cb_hdl = 0;
   NCS_SEL_OBJ  mbx_sel_obj;
   NCS_SEL_OBJ  highest_sel_obj;
   SaAisErrorT  saf_status = SA_AIS_OK; 
   NCS_SEL_OBJ  amf_sel_obj;
   NCS_SEL_OBJ_SET wait_sel_objs;
   int rc = NCSCC_RC_FAILURE;
 
   VDS_TRACE1_ARG1("vds_main_process\n");

   /* get cb-hdl */
   cb_hdl = *((uns32 *)arg);

   /* retrieve VDS cb */
   if ((cb = (VDS_CB *)ncshm_take_hdl(NCS_SERVICE_ID_VDS, cb_hdl)) == NULL)
   {
      m_VDS_LOG_HDL(VDS_LOG_HDL_RETRIEVE_CB,
                            VDS_LOG_HDL_FAILURE,
                                    NCSFL_SEV_CRITICAL);
      vds_destroy(VDS_DONT_CANCEL_THREAD);
      return;
   }
  
   /* Initialize the interface with AMF */
   rc = vds_amf_initialize(cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_VDS_LOG_AMF(VDS_LOG_AMF_INITIALIZE,
                            VDS_LOG_AMF_FAILURE,
                                    NCSFL_SEV_CRITICAL, rc);

      ncshm_give_hdl(cb_hdl);
      vds_destroy(VDS_DONT_CANCEL_THREAD);
      return; 
   }
   else
   {
      m_VDS_LOG_AMF(VDS_LOG_AMF_INITIALIZE,
                            VDS_LOG_AMF_SUCCESS,
                                    NCSFL_SEV_INFO, 0);
   }
                                                                             
   /* get the mbx */
   mbx = &cb->mbx;

   /* reset the wait select objects */
   m_NCS_SEL_OBJ_ZERO(&wait_sel_objs);

   /* get the mbx select object */
   mbx_sel_obj = m_NCS_IPC_GET_SEL_OBJ(mbx);

   /* set all the select objects on which vds waits */
   m_NCS_SEL_OBJ_SET(mbx_sel_obj, &wait_sel_objs);  
   m_SET_FD_IN_SEL_OBJ((uns32)cb->amf.amf_sel_obj,
                       amf_sel_obj);

   m_NCS_SEL_OBJ_SET(amf_sel_obj, &wait_sel_objs);

   /* get the highest select object in the set */
   highest_sel_obj = m_GET_HIGHER_SEL_OBJ(amf_sel_obj, mbx_sel_obj);
   
   ncshm_give_hdl(cb_hdl);

   /* now wait forever */
   while (m_NCS_SEL_OBJ_SELECT(highest_sel_obj, 
                               &wait_sel_objs,
                               NULL, NULL, NULL) != -1)
   {
      /* vds mbx processing */
      if (m_NCS_SEL_OBJ_ISSET(amf_sel_obj, &wait_sel_objs))
      {
         /* dispatch all the AMF pending function */
         saf_status = saAmfDispatch(cb->amf.amf_handle, SA_DISPATCH_ALL);

         if(vds_destroying == TRUE)
           break;

         if (saf_status != SA_AIS_OK)
         {
            m_VDS_LOG_AMF(VDS_LOG_AMF_DISPATCH,
                                  VDS_LOG_AMF_FAILURE,
                                          NCSFL_SEV_NOTICE, saf_status);
         }
      }
 
      /* vds mbx processing */
      if (m_NCS_SEL_OBJ_ISSET(mbx_sel_obj, &wait_sel_objs))
         vds_mbx_process(mbx);

      /* reset the wait select objects */
      m_NCS_SEL_OBJ_ZERO(&wait_sel_objs);

      /* again set all the wait select objs */
      m_NCS_SEL_OBJ_SET(mbx_sel_obj, &wait_sel_objs);

      m_NCS_SEL_OBJ_SET(amf_sel_obj, &wait_sel_objs);
   }

   return;
}


/****************************************************************************
  Name          :  vds_mbx_process
 
  Description   :  This routine dequeues & processes the events from the 
                   vds mailbox.
 
  Arguments     :  mbx - ptr to the mailbox
 
  Return Values :  None.
 
  Notes         :  None
******************************************************************************/
void vds_mbx_process(SYSF_MBX *mbx)
{
   VDS_EVT *evt = NULL;

   VDS_TRACE1_ARG1("vds_mbx_process\n");

   /* process each event */
   while ((evt = (VDS_EVT *)m_NCS_IPC_NON_BLK_RECEIVE(mbx, evt)) != NULL)
   {
      m_VDS_LOG_EVT(VDS_LOG_EVT_RECEIVE,
                            VDS_LOG_EVT_NOTHING,
                                    NCSFL_SEV_INFO);
      vds_evt_process(evt);
   }
   return;
}


/*****************************************************************************
  Name          :  vds_debug_dump

  Description   :  This routine dumps the VDS database.

  Arguments     :  Nothing

  Return Values :  Nothing

  Notes         :  None
******************************************************************************/
void vds_debug_dump(void)
{
   VDS_CB *cb;
   VDS_VDEST_DB_INFO *vdest_db_node = NULL;
   VDS_VDEST_ADEST_INFO *adest_node = NULL;
   SaNameT *vdest_name = NULL;

   if ((cb = (VDS_CB *)ncshm_take_hdl(NCS_SERVICE_ID_VDS, gl_vds_hdl)) == NULL)
   {
      m_VDS_LOG_HDL(VDS_LOG_HDL_RETRIEVE_CB,
                            VDS_LOG_HDL_FAILURE,
                                    NCSFL_SEV_CRITICAL);
      return;
   }

   m_NCS_LOCK(&cb->cb_lock, NCS_LOCK_WRITE);

   while((vdest_db_node = (VDS_VDEST_DB_INFO *)ncs_patricia_tree_getnext(&cb->vdest_name_db,
                         (uns8 *)vdest_name)) != NULL)
   {
      adest_node = vdest_db_node->adest_list;
      m_NCS_CONS_PRINTF("VNAME - %s ::  ID -  %lld :: PERSISTENT %ld :: ADESTS ",
                                               vdest_db_node->vdest_name.value,vdest_db_node->vdest_id,vdest_db_node->persistent);
      while (adest_node)
      {
         m_NCS_CONS_PRINTF(" %lld ",adest_node->adest);

         adest_node = adest_node->next;
      }
      vdest_name = &vdest_db_node->vdest_name;

      m_NCS_CONS_PRINTF("\n");
   }


   m_NCS_UNLOCK(&cb->cb_lock, NCS_LOCK_WRITE);
   ncshm_give_hdl(gl_vds_hdl);

   return;
}

