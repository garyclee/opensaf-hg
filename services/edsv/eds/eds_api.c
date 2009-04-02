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
        
This include file contains SE api instrumentation for EDS
          
*******************************************************************************/
#include <configmake.h>
#include "eds.h"

/* global cb handle */
uns32 gl_eds_hdl = 0;
/****************************************************************************
 * Name          : eds_se_lib_init
 *
 * Description   : Invoked to Initialize the EDS
 *                 
 *
 * Arguments     : 
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
eds_se_lib_init (NCS_LIB_REQ_INFO *req_info)
{
   EDS_CB         *eds_cb;
   uns32          rc = NCSCC_RC_SUCCESS;
   SaAisErrorT    error;
   FILE           *fp = NULL;
   char           pidfilename[EDS_PID_FILE_NAME_LEN] = {0};

   /* Register with the Logging subsystem */
   eds_flx_log_reg();


   /* Allocate and initialize the control block*/
   if (NULL == (eds_cb = m_MMGR_ALLOC_EDS_CB))
   {
      m_LOG_EDSV_S(EDS_MEM_ALLOC_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
      return NCSCC_RC_FAILURE;
   }
   memset(eds_cb, '\0',sizeof(EDS_CB));

   /* Obtain the hdl for EDS_CB from hdl-mgr */

   gl_eds_hdl = eds_cb->my_hdl = ncshm_create_hdl(1,
      NCS_SERVICE_ID_EDS, (NCSCONTEXT)eds_cb);

   if(0 == eds_cb->my_hdl)
   {
      m_LOG_EDSV_S(EDS_CB_CREATE_HANDLE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
      ncshm_destroy_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl);
      gl_eds_hdl = 0;
      m_MMGR_FREE_EDS_CB(eds_cb);
      return NCSCC_RC_FAILURE;
   }

   /* initialize the eds cb lock */
   m_NCS_LOCK_INIT(&eds_cb->cb_lock);

   /* Initialize eds control block */
   if (NCSCC_RC_SUCCESS != (rc = eds_cb_init(eds_cb)))
   {
      m_LOG_EDSV_S(EDS_CB_INIT_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
      /* Destroy the hdl for this CB */
      ncshm_destroy_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl);
      gl_eds_hdl = 0;
      /* clean up the CB */
      m_MMGR_FREE_EDS_CB(eds_cb);
      /* log the error */
      printf("eds_se_lib_init: eds_cb_init() FAILED\n");
      return rc;
   }
   
   m_LOG_EDSV_S(EDS_CB_INIT_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,1,__FILE__,__LINE__,1);
   printf("eds_se_lib_init: eds_cb_init()- EDS_CB INIT SUCCESS\n");
   
   if(NCSCC_RC_SUCCESS != (rc = pcs_rda_get_init_role(eds_cb))) 
   {
      /* Destroy the hdl for this CB */
      m_LOG_EDSV_S(EDS_CB_INIT_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      ncshm_destroy_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl);
      gl_eds_hdl = 0;
      /* clean up the CB */
      m_MMGR_FREE_EDS_CB(eds_cb);
      return NCSCC_RC_FAILURE;
   }


   /* Generate the pidfilename. Also assert for string buffer overflow */
   assert(sprintf(pidfilename, "%s", EDS_PID_FILE)
            < sizeof(pidfilename));

   /*Open pidfile for writing the process id */
   fp = fopen(pidfilename, "w");
   if(fp == NULL)
   {
      m_LOG_EDSV_S(EDS_PID_FILE_OPEN_FOR_WRITE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,(long)fp,__FILE__,__LINE__,0);
      printf("eds_se_lib_init : " PIDPATH "eds.pid OPEN FOR WRITE FAILED......\n");
      /* Destroy the hdl for this CB */
      ncshm_destroy_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl);
      gl_eds_hdl = 0;
      /* clean up the CB */
      m_MMGR_FREE_EDS_CB(eds_cb);
      return NCSCC_RC_FAILURE; 
   }

   if(fprintf(fp, "%d", getpid()) < 1)
   {
      fclose(fp);
      m_LOG_EDSV_S(EDS_PID_FILE_WRITE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,(long)fp,__FILE__,__LINE__,0);
      printf("eds_se_lib_init : " PIDPATH "eds.pid FILE WRITE FAILED......\n");
      /* Destroy the hdl for this CB */
      ncshm_destroy_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl);
      gl_eds_hdl = 0;
      /* clean up the CB */
      m_MMGR_FREE_EDS_CB(eds_cb);
      return NCSCC_RC_FAILURE; 
   }
   fclose(fp);

   m_LOG_EDSV_S(EDS_PID_FILE_WRITE_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,1,__FILE__,__LINE__,1);
   printf("eds_se_lib_init : " PIDPATH "eds.pid EDS PID FILE WRITE SUCCESS......\n");

   m_NCS_EDU_HDL_INIT(&eds_cb->edu_hdl);

   /* Create the mbx to communicate with the EDS thread */
   if (NCSCC_RC_SUCCESS != (rc = m_NCS_IPC_CREATE(&eds_cb->mbx)))
   {
      m_LOG_EDSV_S(EDS_IPC_CREATE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      printf("eds_se_lib_init : " PIDPATH "eds.pid FILE WRITE FAILED......\n");
      /* Release EDU handle */
      m_NCS_EDU_HDL_FLUSH(&eds_cb->edu_hdl);
      /* Destroy the hdl for this CB */
      ncshm_destroy_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl);
      gl_eds_hdl = 0;
      /* Free the control block */
      m_MMGR_FREE_EDS_CB(eds_cb);
      return rc;
   }
   /* Attach the IPC to the created thread */ 
   m_NCS_IPC_ATTACH(&eds_cb->mbx);
   m_LOG_EDSV_S(EDS_MAIL_BOX_CREATE_ATTACH_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,1,__FILE__,__LINE__,1);
   printf("eds_se_lib_init : EDS-MAILBOX CREATE & ATTACH SUCCESS\n");

   /* Bind to MDS */
   if (NCSCC_RC_SUCCESS != (rc = eds_mds_init(eds_cb)))
   {
      m_LOG_EDSV_S(EDS_MDS_INIT_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      m_NCS_IPC_RELEASE(&eds_cb->mbx, NULL);
      /* Release EDU handle */
      m_NCS_EDU_HDL_FLUSH(&eds_cb->edu_hdl);
      ncshm_destroy_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl);
      gl_eds_hdl = 0;
      m_MMGR_FREE_EDS_CB(eds_cb);
      printf("eds_se_lib_init: eds_mds_init() EDS MDS INIT FAILED\n");
      return rc;
   }
   m_LOG_EDSV_S(EDS_MDS_INIT_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,1,__FILE__,__LINE__,1);
   printf("eds_se_lib_init: eds_mds_init() EDS MDS INIT SUCCESS\n");
   /* Initialize mbcsv interface */
   if (NCSCC_RC_SUCCESS != (rc = eds_mbcsv_init(eds_cb)))
   {
       m_LOG_EDSV_S(EDS_MBCSV_INIT_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
       printf("eds_se_lib_init : eds_mbcsv_init() EDS MBCSV INIT FAILED\n");
       /* Log it */
   }
   else
     m_LOG_EDSV_S(EDS_MBCSV_INIT_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,1,__FILE__,__LINE__,1);

   printf("eds_se_lib_init : eds_mbcsv_init() EDS MBCSV INIT SUCCESS \n");

   /* initialize the signal handler */
   if ((ncs_app_signal_install(SIGUSR1,eds_amf_sigusr1_handler)) == -1)
   {  
      m_LOG_EDSV_S(EDS_INSTALL_SIGHDLR_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
      eds_mds_finalize(eds_cb);
      printf("eds_se_lib_init : INSTALL SIGNAL HANDLER FAILED\n");
      m_NCS_IPC_RELEASE(&eds_cb->mbx, NULL); 
      /* Release EDU handle */
      m_NCS_EDU_HDL_FLUSH(&eds_cb->edu_hdl);
      ncshm_destroy_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl);
      gl_eds_hdl = 0;
      /* clean up the CB */
      m_MMGR_FREE_EDS_CB(eds_cb);
      return NCSCC_RC_FAILURE; 
   }

   /* Create the selection object associated with USR1 signal handler */
   m_NCS_SEL_OBJ_CREATE(&eds_cb->sighdlr_sel_obj);

   m_LOG_EDSV_S(EDS_INSTALL_SIGHDLR_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,1,__FILE__,__LINE__,1);

   /* Create EDS's thread */
   if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_CREATE((NCS_OS_CB)eds_main_process,
                                          &eds_cb->mbx,
                                          "EDS",
                                          EDS_TASK_PRIORITY,
                                          NCS_STACKSIZE_HUGE,
                                          &eds_cb->task_hdl
                                          )))
   {
      m_LOG_EDSV_S(EDS_TASK_CREATE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      eds_mds_finalize(eds_cb);
      /* release the IPC */ 
      m_NCS_IPC_RELEASE(&eds_cb->mbx, NULL); 
      /* Release EDU handle */
      m_NCS_EDU_HDL_FLUSH(&eds_cb->edu_hdl);
      /* Free the control block */
      ncshm_destroy_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl);
      gl_eds_hdl = 0;
      m_MMGR_FREE_EDS_CB(eds_cb);
      return rc;
   }

   /* Put the EDS thread in the start state */
   if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_START(eds_cb->task_hdl)))
   {
      m_LOG_EDSV_S(EDS_TASK_START_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      eds_mds_finalize(eds_cb);
      /* kill the created task */
      m_NCS_TASK_RELEASE(eds_cb->task_hdl); 
      /* release the IPC */ 
      m_NCS_IPC_RELEASE(&eds_cb->mbx, NULL); 
      /* Release EDU handle */
      m_NCS_EDU_HDL_FLUSH(&eds_cb->edu_hdl);
      /* Destroy the hdl for this CB */
      ncshm_destroy_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl);
      gl_eds_hdl = 0;
      /* clean up the CB */
      m_MMGR_FREE_EDS_CB(eds_cb);
      /* log the error */
      printf("eds_se_lib_init: EDS MAIN PROCESS START FAILED\n");
      return rc; 
   }
   m_LOG_EDSV_S(EDS_MAIN_PROCESS_START_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,1,__FILE__,__LINE__,1);
   printf("eds_se_lib_init: EDS MAIN PROCESS START SUCCESS\n");

   /* Register with MAB */
   rc = edsv_mab_register(eds_cb);
   if ( rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_EDSV_S(EDS_MAB_REGISTER_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      eds_mds_finalize(eds_cb);
      /* stop the already started task */
      m_NCS_TASK_STOP(eds_cb->task_hdl);
      m_NCS_TASK_RELEASE(eds_cb->task_hdl);
      m_NCS_IPC_RELEASE(&eds_cb->mbx, NULL);
      /* Release EDU handle */
      m_NCS_EDU_HDL_FLUSH(&eds_cb->edu_hdl);
      ncshm_destroy_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl);
      gl_eds_hdl = 0;
      m_MMGR_FREE_EDS_CB(eds_cb);
      printf("eds_se_lib_init:edsv_mab_register() EDS MAB REGISTER FAILED\n");
      return NCSCC_RC_FAILURE;
   }
   m_LOG_EDSV_S(EDS_MAB_REGISTER_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,1,__FILE__,__LINE__,1);
   printf("eds_se_lib_init:edsv_mab_register() EDS MAB REGISTER SUCCESS\n");

   if((rc = edsv_table_register()) != NCSCC_RC_SUCCESS)
   { 
      m_LOG_EDSV_S(EDS_TABLE_REGISTER_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      eds_mds_finalize(eds_cb);
      error=edsv_mab_unregister(eds_cb); /*Unregister with MAB */
      /* stop the already started task */
      m_NCS_TASK_STOP(eds_cb->task_hdl);
      m_NCS_TASK_RELEASE(eds_cb->task_hdl);
      m_NCS_IPC_RELEASE(&eds_cb->mbx, NULL);
      /* Release EDU handle */
      m_NCS_EDU_HDL_FLUSH(&eds_cb->edu_hdl);
      ncshm_destroy_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl);
      gl_eds_hdl = 0;
      m_MMGR_FREE_EDS_CB(eds_cb);
      printf("eds_se_lib_init: edsv_table_register() EDS TABLE REGISTER FAILED\n");
      return NCSCC_RC_FAILURE;
   }
   m_LOG_EDSV_S(EDS_TBL_REGISTER_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,1,__FILE__,__LINE__,1);
   printf("eds_se_lib_init: edsv_table_register() EDS TABLE REGISTER SUCCESS\n");

   return (rc);
}

/****************************************************************************
 * Name          : eds_clear_mbx
 *
 * Description   : This is the function which deletes all the messages from 
 *                 the mail box.
 *
 * Arguments     : arg     - argument to be passed.
 *                 msg     - Event start pointer.
 *
 * Return Values : TRUE/FALSE
 *
 * Notes         : None.
 *****************************************************************************/
static NCS_BOOL
eds_clear_mbx (NCSCONTEXT arg, NCSCONTEXT msg)
{   
   EDSV_EDS_EVT  *pEvt = (EDSV_EDS_EVT *)msg;
   EDSV_EDS_EVT  *pnext;
   pnext = pEvt;
   while (pnext)
   {
      pnext = (/* (EDSV_EDS_EVT *)&*/(pEvt->next));
      eds_evt_destroy(pEvt);  
      pEvt = pnext;
   }
   return TRUE;
}

/****************************************************************************
 * Name          : eds_se_lib_destroy
 *
 * Description   : Invoked to destroy the EDS
 *                 
 *
 * Arguments     : 
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
 static uns32
 eds_se_lib_destroy (NCS_LIB_REQ_INFO *req_info)
 {
    /** Code to destroy the EDS **/
    EDS_CB *eds_cb;
    m_INIT_CRITICAL;
    SaAisErrorT status = SA_AIS_OK; 

    if (NULL == (eds_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_EDS, gl_eds_hdl)))
    {
       m_LOG_EDSV_S(EDS_CB_TAKE_HANDLE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
       return (NCSCC_RC_FAILURE);      
    } 
    else
    {
       m_START_CRITICAL;
      /** Lock EDA_CB
       **/
       m_NCS_LOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);
      
       eds_cb->scalar_objects.svc_state=SHUTTING_DOWN; 

       /* deregister from AMF */
       status = saAmfComponentUnregister(eds_cb->amf_hdl, &eds_cb->comp_name, NULL);
       
       /* End association from the AMF lib */
       saAmfFinalize(eds_cb->amf_hdl);

       /* Finalize with CLM */
       saClmFinalize(eds_cb->clm_hdl);

       /* Clean up all internal structures */
       eds_remove_reglist_entry(eds_cb, 0, TRUE);

       /* Destroy the cb */
       eds_cb_destroy(eds_cb);

       /* Give back the handle */
       ncshm_give_hdl(gl_eds_hdl);
       ncshm_destroy_hdl(NCS_SERVICE_ID_GLD, gl_eds_hdl); 

       /* Detach from IPC */
       m_NCS_IPC_DETACH(&eds_cb->mbx , eds_clear_mbx,eds_cb);

       /* Disconnect from MDS */
       eds_mds_finalize(eds_cb);

       if((status=edsv_mab_unregister(eds_cb)) != NCSCC_RC_SUCCESS)
           m_EDSV_DEBUG_CONS_PRINTF(" MAB UNREGISTER FAILED\n");
          /* Log it */; /*Unregister with MAB */
       /* stop & kill the created task */

       m_NCS_TASK_STOP(eds_cb->task_hdl);
       m_NCS_TASK_RELEASE(eds_cb->task_hdl); 

       /* Release the IPC */
       m_NCS_IPC_RELEASE(&eds_cb->mbx, NULL);

      /** UnLock EDA_CB
       **/
       m_NCS_UNLOCK(&eds_cb->cb_lock, NCS_LOCK_WRITE);
       m_NCS_LOCK_DESTROY(&eds_cb->cb_lock);
       m_MMGR_FREE_EDS_CB(eds_cb);

#if (NCS_EDSV_LOG == 1)
         eds_flx_log_dereg();
#endif
       gl_eds_hdl = 0;
       m_END_CRITICAL;
       printf("EDS-CB-LIB DESTROY CALLED...... \n");
    }

    return (NCSCC_RC_SUCCESS);
 }

/****************************************************************************
 * Name          : ncs_edsv_eds_lib_req
 *
 * Description   : This is the NCS SE API which is used to init/destroy EDS
 *                 module
 *                 
 *
 * Arguments     : req_info  - This is the pointer to the input information 
 *                             which SBOM gives.  
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ncs_edsv_eds_lib_req (NCS_LIB_REQ_INFO *req_info)
{
   uns32 rc = NCSCC_RC_FAILURE;

   switch (req_info->i_op)
   {      
      case NCS_LIB_REQ_CREATE:
         rc = eds_se_lib_init(req_info);
         break;
      case NCS_LIB_REQ_DESTROY:
         rc = eds_se_lib_destroy(req_info);
         break;
      default:
         break;
   }
   return (rc);
}


/****************************************************************************
 * Name          : pcs_rda_get_init_role
 *
 * Description   : This is the module which gets the init role by using pcs rda
                   lib requests and get init role methods
 *                 
 *
 * Arguments     : eds_cb  - pinter to control block 
 *                               
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32
pcs_rda_get_init_role(EDS_CB* eds_cb)
{
   PCS_RDA_REQ    pcs_rda_req;
   uns32           rc = NCSCC_RC_SUCCESS;

        /* RDA init */
   memset(&pcs_rda_req, '\0', sizeof(pcs_rda_req));
   pcs_rda_req.req_type = PCS_RDA_LIB_INIT;
   rc = pcs_rda_request(&pcs_rda_req);
   if(rc != PCSRDA_RC_SUCCESS)
   {
      m_LOG_EDSV_S(EDS_PCS_RDA_LIB_INIT_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      printf("eds_se_lib_init : pcs_rda_request() for PCS_RDA_LIB_INIT FAILED\n");
      return NCSCC_RC_FAILURE;
   }
   
   /* Get initial role from RDA */
   memset(&pcs_rda_req, '\0', sizeof(pcs_rda_req));
   pcs_rda_req.req_type = PCS_RDA_GET_ROLE;
   rc = pcs_rda_request(&pcs_rda_req);
   if(rc != PCSRDA_RC_SUCCESS)
   {
      m_LOG_EDSV_S(EDS_PCS_RDA_GET_ROLE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      printf("eds_se_lib_init : pcs_rda_request() for PCS_RDA_GET_ROLE FAILED\n");
      return NCSCC_RC_FAILURE;
   }

   /* Set initial role now */
   switch(pcs_rda_req.info.io_role)
   {
      case PCS_RDA_ACTIVE:
         eds_cb->ha_state = SA_AMF_HA_ACTIVE;
         m_LOG_EDSV_S(EDS_RDA_SET_HA_STATE_ACTIVE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,pcs_rda_req.info.io_role,__FILE__,__LINE__,0);
         printf("eds_se_lib_init : GOT INIT ROLE FROM RDA , I AM HA ACTIVE\n");
         break;
      case PCS_RDA_STANDBY:
         m_LOG_EDSV_S(EDS_RDA_SET_HA_STATE_STANDBY,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,pcs_rda_req.info.io_role,__FILE__,__LINE__,0);
         eds_cb->ha_state = SA_AMF_HA_STANDBY;
         printf("eds_se_lib_init : GOT INIT ROLE FROM RDA , I AM HA STANDBY\n");
         break;
      default:
         m_LOG_EDSV_S(EDS_RDA_SET_HA_STATE_NULL,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,pcs_rda_req.info.io_role,__FILE__,__LINE__,0);
         printf("eds_se_lib_init :GOT INIT ROLE FROM RDA , ROLE = NULL FAILED \n");
         return NCSCC_RC_FAILURE;
   }
   /* RDA finalize */
   memset(&pcs_rda_req, '\0', sizeof(pcs_rda_req));
   pcs_rda_req.req_type = PCS_RDA_LIB_DESTROY;
   rc = pcs_rda_request(&pcs_rda_req);
   if(rc != PCSRDA_RC_SUCCESS)
   {
      m_LOG_EDSV_S(EDS_PCS_RDA_LIB_INIT_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      printf("eds_se_lib_init :PCS RDA LIB DESTROY FAILED \n");
   }

   return NCSCC_RC_SUCCESS;
}         
