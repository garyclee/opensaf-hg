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

  This file contains SRMND initialization & destroy routines.

..............................................................................

  FUNCTIONS INCLUDED in this module:
  
******************************************************************************
*/

#include "srmnd.h"
#include "ncs_main_pvt.h"

/* global cb handle */
uns32 gl_srmnd_hdl = 0;

/* global task handle */
NCSCONTEXT gl_srmnd_task_hdl = 0;

/* static function declarations */
static uns32 srmnd_create (NCS_LIB_CREATE *create_info);
static uns32 srmnd_mbx_create (SRMND_CB *);
static uns32 srmnd_task_create (void);
static uns32 srmnd_cb_destroy (SRMND_CB *);
static uns32 srmnd_mbx_destroy (SRMND_CB *);
static NCS_BOOL srmnd_mbx_clean (NCSCONTEXT, NCSCONTEXT);
static SRMND_CB *srmnd_cb_create (void);

/****************************************************************************
  Name          : srmnd_lib_req
 
  Description   : This routine is exported to the external entities & is used
                  to create & destroy SRMND.
 
  Arguments     : req_info - ptr to the request info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 srmnd_lib_req (NCS_LIB_REQ_INFO *req_info)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   switch (req_info->i_op)
   {
   case NCS_LIB_REQ_CREATE:
      rc =  srmnd_create(&req_info->info.create);
      if (rc == NCSCC_RC_SUCCESS)
      {
         m_SRMND_LOG_MISC(SRMND_LOG_MISC_ND_CREATE,
                          SRMND_LOG_MISC_SUCCESS,
                          NCSFL_SEV_INFO);
      }
      else
      {
         m_SRMND_LOG_MISC(SRMND_LOG_MISC_ND_CREATE,
                          SRMND_LOG_MISC_FAILED,
                          NCSFL_SEV_CRITICAL);
      }
      break;
       
   case NCS_LIB_REQ_DESTROY:
      rc = srmnd_destroy(TRUE);
      
      /* release the task */
      if (gl_srmnd_task_hdl)
         m_SRMND_TASK_RELEASE(gl_srmnd_task_hdl);

      if (rc == NCSCC_RC_SUCCESS)
      {
         m_SRMND_LOG_MISC(SRMND_LOG_MISC_ND_DESTROY,
                          SRMND_LOG_MISC_SUCCESS,
                          NCSFL_SEV_INFO);
      }
      else
      {
         m_SRMND_LOG_MISC(SRMND_LOG_MISC_ND_DESTROY,
                          SRMND_LOG_MISC_FAILED,
                          NCSFL_SEV_CRITICAL);
      }
      break;
       
   default:
      break;
   }

   return rc;
}


/****************************************************************************
  Name          :  srmnd_create
 
  Description   :  This routine creates & initializes the PWE of SRMND. It does
                   the following:
                   a) create & initialize SRMND control block.
                   b) create & attach SRMND mailbox.
                   c) initialize external interfaces (logging service being the
                      exception).
                   d) create & start SRMND task.
 
  Arguments     :  create_info - ptr to the create info
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  None
******************************************************************************/
uns32 srmnd_create (NCS_LIB_CREATE *create_info)
{
   SRMND_CB *cb = 0;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* register with dtsv */
#if (NCS_SRMND_LOG == 1)
   rc = srmnd_log_reg();
#endif

   /* create & initialize SRMND cb */
   cb = srmnd_cb_create();
   if (cb == NULL)
      return NCSCC_RC_FAILURE;      
    
   /* create & attach SRMND mailbox */
   rc = srmnd_mbx_create(cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
      srmnd_destroy(FALSE);
      return rc;    
   }

   /* create & start SRMND task */
   rc = srmnd_task_create();
   if (rc != NCSCC_RC_SUCCESS)
   {
      srmnd_destroy(FALSE);
      return rc;    
   }

#if 0
   /* Don't call if SRMND is going to activate through AvSv */
   rc = srmnd_active(cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
      srmnd_destroy(FALSE);
      return rc;    
   }
#endif
   return rc;
}


/****************************************************************************
  Name          :  srmnd_destroy
 
  Description   :  This routine destroys the PWE of SRMND. It does the following:
                   a) destroy external interfaces (logging service being the
                      exception).
                   b) detach & destroy SRMND mailbox.
                   c) destroy SRMND control block.
                   d) destroy SRMND task.
 
  Arguments     :  None.
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 srmnd_destroy (NCS_BOOL deactivate_flg)
{
   SRMND_CB *cb = 0;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* unregister with DTSv */
#if (NCS_SRMND_LOG == 1)
   rc = srmnd_log_unreg();
#endif

   /* retrieve srmnd cb */
   if ((cb = (SRMND_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SRMND,
                                       (uns32)gl_srmnd_hdl)) == SRMND_CB_NULL)
   {
      /* No association record?? :-( ok log it & return error */
      m_SRMND_LOG_HDL(SRMSV_LOG_HDL_RETRIEVE_CB,
                      SRMSV_LOG_HDL_FAILURE,
                      NCSFL_SEV_CRITICAL);
      return NCSCC_RC_FAILURE;      
   }

   /* If the deactivate flag is set only, call the srmnd deactivate function */
   if (deactivate_flg)
      srmnd_deactive(cb);

   /* de-nitialize the interface with Availability Management Framework */
   rc = srmnd_amf_finalize(cb); 
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_SRMND_LOG_AMF(SRMND_LOG_AMF_FINALIZE,
                      SRMND_LOG_AMF_FAILED,
                      NCSFL_SEV_CRITICAL);
      /* continue destory */
   } 
   else
   {
      m_SRMND_LOG_AMF(SRMND_LOG_AMF_FINALIZE,
                      SRMND_LOG_AMF_SUCCESS,
                      NCSFL_SEV_INFO);
   }

   /* detach & destroy SRMND mailbox */
   rc = srmnd_mbx_destroy(cb);
   if (rc != NCSCC_RC_SUCCESS) {
      ncshm_give_hdl((uns32)gl_srmnd_hdl);
      return rc;
   }

   /* destroy SRMND control block */
   rc = srmnd_cb_destroy(cb);
   if (rc != NCSCC_RC_SUCCESS) {
      return rc;
   } 

   return rc;
}


/****************************************************************************
  Name          : srmnd_cb_create
 
  Description   : This routine creates & initializes SRMND control block.
 
  Arguments     : None.
 
  Return Values : if successfull, ptr to SRMND control block
                  else, 0
 
  Notes         : None
******************************************************************************/
SRMND_CB *srmnd_cb_create()
{
   SRMND_CB *cb = 0;
   NCS_PATRICIA_PARAMS params;

   /* allocate SRMND cb */
   if (((cb = m_MMGR_ALLOC_SRMND_CB)) == SRMND_CB_NULL)
   {
      m_SRMND_LOG_MEM(SRMND_MEM_SRMND_CB,
                      SRMND_MEM_ALLOC_FAILED,
                      NCSFL_SEV_CRITICAL);

      return SRMND_CB_NULL;
   }
   
   m_SRMND_LOG_MEM(SRMND_MEM_EVENT,
                   SRMND_MEM_ALLOC_SUCCESS,
                   NCSFL_SEV_INFO);

   m_NCS_OS_MEMSET((char *)cb, 0, sizeof(SRMND_CB));

   /* make srmnd operational status to DOWN state */ 
   cb->oper_status = SRMSV_ND_OPER_STATUS_DOWN;   

   /* assign the SRMND pool-id (used by hdl-mngr) */
   cb->pool_id = NCS_HM_POOL_ID_COMMON;

   /* initialize the SRMND cb lock */
   m_NCS_LOCK_INIT(&cb->cb_lock);
   m_SRMND_LOG_LOCK(SRMSV_LOG_LOCK_INIT,
                    SRMSV_LOG_LOCK_SUCCESS,
                    NCSFL_SEV_INFO);

   /* create the association with hdl-mngr */
   if (((cb->cb_hdl = ncshm_create_hdl(cb->pool_id,
                                       NCS_SERVICE_ID_SRMND, 
                                       (NCSCONTEXT)cb))) == 0)
   {
      m_SRMND_LOG_HDL(SRMSV_LOG_HDL_CREATE_CB,
                      SRMSV_LOG_HDL_FAILURE,
                      NCSFL_SEV_CRITICAL); 

      srmnd_cb_destroy(cb);
      return SRMND_CB_NULL;      
   }

   m_SRMND_LOG_HDL(SRMSV_LOG_HDL_CREATE_CB,
                   SRMSV_LOG_HDL_SUCCESS,
                   NCSFL_SEV_INFO);
   
   /* Initialize Resource Mon Tree */
   m_NCS_OS_MEMSET(&params, 0, sizeof(NCS_PATRICIA_PARAMS));
   params.key_size = sizeof(uns32);
   params.info_size = 0;
   if (ncs_patricia_tree_init(&cb->rsrc_mon_tree, &params)
       != NCSCC_RC_SUCCESS)
   {   
      m_SRMND_LOG_TREE(SRMSV_LOG_PAT_INIT,
                       SRMSV_LOG_PAT_FAILURE,
                       NCSFL_SEV_CRITICAL);

      /* Delete SRMA Control Block */
      srmnd_cb_destroy(cb);

      return SRMND_CB_NULL;      
   }
   m_SRMND_LOG_TREE(SRMSV_LOG_PAT_INIT,
                    SRMSV_LOG_PAT_SUCCESS,
                    NCSFL_SEV_INFO);

   /* everything went off well.. store the cb hdl in the glb variable */
   gl_srmnd_hdl = cb->cb_hdl;

   return cb;
}


/****************************************************************************
  Name          :  srmnd_mbx_create
 
  Description   :  This routine creates & attaches SRMND mailbox.
 
  Arguments     :  cb - ptr to SRMND control block
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None
******************************************************************************/
uns32 srmnd_mbx_create (SRMND_CB *cb)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* create the mail box */
   rc = m_NCS_IPC_CREATE(&cb->mbx);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_SRMND_LOG_TIM(SRMSV_LOG_IPC_CREATE,
                      SRMSV_LOG_TIM_FAILURE,
                      NCSFL_SEV_CRITICAL); 
      return rc;
   }

   m_SRMND_LOG_TIM(SRMSV_LOG_IPC_CREATE,
                   SRMSV_LOG_TIM_SUCCESS,
                   NCSFL_SEV_INFO); 
   
   /* attach the mail box */
   rc = m_NCS_IPC_ATTACH(&cb->mbx);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_SRMND_LOG_TIM(SRMSV_LOG_IPC_CREATE,
                      SRMSV_LOG_TIM_FAILURE,
                      NCSFL_SEV_CRITICAL); 

      /* destroy the mailbox */
      srmnd_mbx_destroy(cb);

      return rc;
   }
   
   m_SRMND_LOG_TIM(SRMSV_LOG_IPC_ATTACH,
                   SRMSV_LOG_TIM_SUCCESS,
                   NCSFL_SEV_INFO); 
   return rc;
}



/****************************************************************************
  Name          : srmnd_task_create
 
  Description   : This routine creates & starts SRMND task.
 
  Arguments     : None.
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 srmnd_task_create ()
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* create srmnd task */
   rc = m_NCS_TASK_CREATE ((NCS_OS_CB)srmnd_main_process,
                           (void *)&gl_srmnd_hdl,
                           "SRMND",
                           m_SRMND_TASK_PRIORITY,
                           m_SRMND_STACKSIZE,
                           &gl_srmnd_task_hdl);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_SRMND_LOG_TIM(SRMSV_LOG_TASK_CREATE,
                      SRMSV_LOG_TIM_FAILURE,
                      NCSFL_SEV_CRITICAL); 
      gl_srmnd_task_hdl = 0;
      return rc;
   }
   else
   {
      m_SRMND_LOG_TIM(SRMSV_LOG_TASK_CREATE,
                      SRMSV_LOG_TIM_SUCCESS,
                      NCSFL_SEV_INFO); 
   }

   /* now start the task */
   rc = m_NCS_TASK_START (gl_srmnd_task_hdl);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_SRMND_LOG_TIM(SRMSV_LOG_TASK_START,
                      SRMSV_LOG_TIM_FAILURE,
                      NCSFL_SEV_CRITICAL); 

      /* release the task */
      if (gl_srmnd_task_hdl)
         m_SRMND_TASK_RELEASE(gl_srmnd_task_hdl);      
   }
   else
   {
      m_SRMND_LOG_TIM(SRMSV_LOG_TASK_START,
                      SRMSV_LOG_TIM_SUCCESS,
                      NCSFL_SEV_INFO); 
   }

   return rc;
}


/****************************************************************************
  Name          : srmnd_cb_destroy
 
  Description   : This routine destroys SRMND control block.
 
  Arguments     : cb - ptr to SRMND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 srmnd_cb_destroy (SRMND_CB *cb)
{
   uns32  rc = NCSCC_RC_SUCCESS;

   /* Cleanup of the SRMND database */
   srmnd_del_srma(cb, NULL); 

   /* Destroy the initiates PAT resource-mon tree */
   ncs_patricia_tree_destroy(&cb->rsrc_mon_tree);
   m_SRMND_LOG_TREE(SRMSV_LOG_PAT_DESTROY,
                    SRMSV_LOG_PAT_SUCCESS,
                    NCSFL_SEV_INFO);

   /* destroy the lock */
   m_NCS_LOCK_DESTROY(&cb->cb_lock);
   m_SRMND_LOG_LOCK(SRMSV_LOG_LOCK_DESTROY,
                    SRMSV_LOG_LOCK_SUCCESS,
                    NCSFL_SEV_INFO);
   
   /* return SRMND CB */
   if (cb->cb_hdl)
      ncshm_give_hdl(cb->cb_hdl);

   /* remove the association with hdl-mngr */
   ncshm_destroy_hdl(NCS_SERVICE_ID_SRMND, cb->cb_hdl);   
   m_SRMND_LOG_HDL(SRMSV_LOG_HDL_DESTROY_CB,
                   SRMSV_LOG_HDL_SUCCESS,
                   NCSFL_SEV_INFO);
 
   /* reset the global cb handle */
   gl_srmnd_hdl = 0;

   /* free the control block */
   m_MMGR_FREE_SRMND_CB(cb);
   m_SRMND_LOG_CB(SRMSV_LOG_CB_DESTROY,
                  SRMSV_LOG_HDL_SUCCESS,
                  NCSFL_SEV_INFO);  
   return rc;
}


/****************************************************************************
  Name          : srmnd_mbx_destroy
 
  Description   : This routine destroys & detaches SRMND mailbox.
 
  Arguments     : cb - ptr to SRMND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None
******************************************************************************/
uns32 srmnd_mbx_destroy (SRMND_CB *cb)
{
   uns32  rc = NCSCC_RC_SUCCESS;

   if (!(cb->mbx))
      return rc;

   /* detach the mail box */
   rc = m_NCS_IPC_DETACH(&cb->mbx, srmnd_mbx_clean, cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_SRMND_LOG_TIM(SRMSV_LOG_IPC_DETACH,
                      SRMSV_LOG_TIM_FAILURE,
                      NCSFL_SEV_CRITICAL);
      return rc;
   }
   m_SRMND_LOG_TIM(SRMSV_LOG_IPC_DETACH,
                   SRMSV_LOG_TIM_SUCCESS,
                   NCSFL_SEV_INFO);

   /* delete the mail box */
   rc = m_NCS_IPC_RELEASE(&cb->mbx, 0);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_SRMND_LOG_TIM(SRMSV_LOG_IPC_RELEASE,
                      SRMSV_LOG_TIM_FAILURE,
                      NCSFL_SEV_CRITICAL);
      return rc;
   }  

   cb->mbx = 0;

   m_SRMND_LOG_TIM(SRMSV_LOG_IPC_RELEASE,
                   SRMSV_LOG_TIM_SUCCESS,
                   NCSFL_SEV_INFO);

   return rc;
}


/****************************************************************************
   Name          :  srmnd_mbx_clean
  
   Description   :  This routine dequeues & deletes all the events from the 
                    mailbox. It is invoked when mailbox is detached.
  
   Arguments     :  arg - argument to be passed
                    msg - ptr to the 1st event in the mailbox
  
   Return Values :  TRUE/FALSE
  
   Notes         :  None.
*****************************************************************************/
NCS_BOOL srmnd_mbx_clean (NCSCONTEXT arg, NCSCONTEXT msg)
{
#if 0
   SRMND_EVT *curr;

   /* clean the entire mailbox */
   for (curr = (SRMND_EVT *)msg; curr; curr = curr->next)
      srmnd_evt_destroy(curr);
#endif

   return TRUE;
}


/****************************************************************************
   Name          :  srmnd_active
  
   Description   :  This function activates SRMND, registers with RP timer, 
                    MDS, EDU and makes SRMND oper status to DOWN.
  
   Arguments     :  srmnd - ptr to the SRMND CB
  
   Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
  
   Notes         :  None.
*****************************************************************************/
uns32 srmnd_active(SRMND_CB *srmnd)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   if (srmnd->oper_status == SRMSV_ND_OPER_STATUS_UP)
      return rc;

   /* make srmnd operational status to UP state */ 
   srmnd->oper_status = SRMSV_ND_OPER_STATUS_UP;   

   /* EDU initialisation */
   rc = m_NCS_EDU_HDL_INIT(&srmnd->edu_hdl);
   if (rc != NCSCC_RC_SUCCESS)
      return rc;    

   /* Registering with timer */
   rc = srmnd_timer_init(srmnd);
   if (rc != NCSCC_RC_SUCCESS)
   {
      /* EDU cleanup */
      m_NCS_EDU_HDL_FLUSH(&srmnd->edu_hdl);
      return rc;    
   }

   /* Versioning changes - Set SRMND's MDS sub-part version */
   srmnd->srmnd_mds_ver = SRMND_SVC_PVT_VER ;

   /* MDS registration */
   rc = srmnd_mds_reg(srmnd);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_SRMND_LOG_MDS(SRMSV_LOG_MDS_REG,
                      SRMSV_LOG_MDS_FAILURE,
                      NCSFL_SEV_CRITICAL);

      /* EDU cleanup */
      m_NCS_EDU_HDL_FLUSH(&srmnd->edu_hdl);

      /* De-register from NCS RP timer */
      srmnd_timer_destroy(srmnd);   

      return rc;
   }

   return rc;
}


/****************************************************************************
   Name          :  srmnd_deactive
  
   Description   :  This function deactivates SRMND, de-registers from 
                    RP timer, MDS, EDU and makes SRMND oper status to DOWN.
  
   Arguments     :  srmnd - ptr to the SRMND CB
  
   Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
  
   Notes         :  None.
*****************************************************************************/
uns32 srmnd_deactive(SRMND_CB *srmnd)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Check whether SRMND oper status is already DOWN */
   if (srmnd->oper_status == SRMSV_ND_OPER_STATUS_DOWN)
      return rc;

   /* make srmnd operational status to DOWN state */ 
   srmnd->oper_status = SRMSV_ND_OPER_STATUS_DOWN;   

   /* MDS unregistration */
   rc = srmnd_mds_unreg(srmnd);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_SRMND_LOG_MDS(SRMSV_LOG_MDS_UNREG,
                      SRMSV_LOG_MDS_FAILURE,
                      NCSFL_SEV_CRITICAL);  
   }

   /* De-register from NCS RP timer */
   srmnd_timer_destroy(srmnd);   
 
   /* EDU cleanup */
   if (srmnd->edu_hdl.is_inited)
      m_NCS_EDU_HDL_FLUSH(&srmnd->edu_hdl);
   
   return rc;
}

