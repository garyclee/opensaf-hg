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
  FILE NAME: GLD_API.C

  DESCRIPTION: Contain functions to create and destroy the GLD

  FUNCTIONS INCLUDED in this module:
  
******************************************************************************/

#include "gld.h"
GLDDLL_API uns32 gl_gld_hdl;

static void gld_main_process(SYSF_MBX *mbx);
/****************************************************************************
 * Name          : ncs_glsv_gld_lib_req
 *
 * Description   : This is the NCS SE API which is used to init/destroy GLD
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
gld_lib_req (NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;

   switch (req_info->i_op)
   {      
      case NCS_LIB_REQ_CREATE:
         /* need to fetch the information from the "NCS_LIB_REQ_INFO" struct - TBD*/
         res = gld_se_lib_init(req_info);
         if (res == NCSCC_RC_SUCCESS)
            m_LOG_GLD_API(GLD_SE_API_CREATE_SUCCESS, NCSFL_SEV_INFO);
         else
            m_LOG_GLD_API(GLD_SE_API_CREATE_SUCCESS, NCSFL_SEV_ERROR);
         break;

      case NCS_LIB_REQ_DESTROY:
         /* need to fetch the information from the "NCS_LIB_REQ_INFO" struct - TBD*/
         res = gld_se_lib_destroy(req_info);
         if (res == NCSCC_RC_SUCCESS)
            m_LOG_GLD_API(GLD_SE_API_DESTROY_SUCCESS, NCSFL_SEV_INFO);
         else
            m_LOG_GLD_API(GLD_SE_API_DESTROY_FAILED, NCSFL_SEV_ERROR);
#if (NCS_GLSV_LOG == 1)
         gld_flx_log_dereg();
#endif
         break;

      default:
         m_LOG_GLD_API(GLD_SE_API_UNKNOWN, NCSFL_SEV_ERROR);
         break;
   }
   return (res);
}

/****************************************************************************
 * Name          : gld_se_lib_init
 *
 * Description   : Invoked to Initialize the GLD
 *                 
 *
 * Arguments     : 
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32
gld_se_lib_init (NCS_LIB_REQ_INFO *req_info)
{
   GLSV_GLD_CB    *gld_cb;
   SaAisErrorT       amf_error;
   uns32          res = NCSCC_RC_SUCCESS;
   SaAmfHealthcheckKeyT  Healthy;
   int8*          health_key;



   /* Register with Logging subsystem */
  if(NCS_GLSV_LOG == 1)
    gld_flx_log_reg();


   /* Allocate and initialize the control block*/
   gld_cb = m_MMGR_ALLOC_GLSV_GLD_CB;

   if (gld_cb == NULL)
   {
      m_LOG_GLD_MEMFAIL(GLD_CB_ALLOC_FAILED);
      return NCSCC_RC_FAILURE;
   }
   memset(gld_cb, 0,sizeof(GLSV_GLD_CB));

   /* TBD- Pool id is to be set */
   gl_gld_hdl = gld_cb->my_hdl = ncshm_create_hdl(gld_cb->hm_poolid,
      NCS_SERVICE_ID_GLD, (NCSCONTEXT)gld_cb);
   if(0 == gld_cb->my_hdl)
   {
      m_LOG_GLD_HEADLINE(GLD_CREATE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      m_MMGR_FREE_GLSV_GLD_CB(gld_cb);
      return NCSCC_RC_FAILURE;
   }

   /* Initialize the cb parameters */
   if (gld_cb_init(gld_cb) != NCSCC_RC_SUCCESS)
   {
      m_MMGR_FREE_GLSV_GLD_CB(gld_cb);
      return NCSCC_RC_FAILURE;
   }


  /* Initialize amf framework */
   if (gld_amf_init(gld_cb) != NCSCC_RC_SUCCESS)
   {
      m_LOG_GLD_SVC_PRVDR(GLD_AMF_INIT_ERROR, NCSFL_SEV_ERROR);
      m_MMGR_FREE_GLSV_GLD_CB(gld_cb);
      return NCSCC_RC_FAILURE;
   }
      m_LOG_GLD_SVC_PRVDR(GLD_AMF_INIT_SUCCESS, NCSFL_SEV_INFO);

   /* Bind to MDS */
   if (gld_mds_init(gld_cb) != NCSCC_RC_SUCCESS)
   {
      m_NCS_TASK_RELEASE(gld_cb->task_hdl);
      m_NCS_IPC_RELEASE(&gld_cb->mbx, NULL);
      m_NCS_EDU_HDL_FLUSH(&gld_cb->edu_hdl);
      m_MMGR_FREE_GLSV_GLD_CB(gld_cb);
      m_LOG_GLD_SVC_PRVDR(GLD_MDS_INSTALL_FAIL, NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }
   else
      m_LOG_GLD_SVC_PRVDR(GLD_MDS_INSTALL_SUCCESS, NCSFL_SEV_INFO);

   /* Register with MIBLIB */
   if((gld_reg_with_miblib( ))!= NCSCC_RC_SUCCESS)
   {
      m_LOG_GLD_SVC_PRVDR(GLD_MIBLIB_REGISTER_FAIL,NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }
   else
      m_LOG_GLD_SVC_PRVDR(GLD_MIBLIB_REGISTER_SUCCESS,NCSFL_SEV_INFO);

   strcpy(gld_cb->saf_spec_ver.value,"B.01.01");
   gld_cb->saf_spec_ver.length   = strlen("B.01.01");
   strcpy(gld_cb->saf_agent_vend.value,"OpenSAF");
   gld_cb->saf_agent_vend.length = strlen("OpenSAF");
   gld_cb->saf_agent_vend_prod   = 2;
   gld_cb->saf_serv_state_enabled= FALSE;
   gld_cb->saf_serv_state        = 1;

   /*   Initialise with the MBCSV service  */
   if(glsv_gld_mbcsv_register(gld_cb)!=NCSCC_RC_SUCCESS)
   {   
      m_LOG_GLD_MBCSV(GLD_MBCSV_INIT_FAILED,NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
        
   }
   else
   {
      m_LOG_GLD_MBCSV(GLD_MBCSV_INIT_SUCCESS,NCSFL_SEV_INFO);

   }
   

   /* TASK CREATION AND INITIALIZING THE MAILBOX*/
   if ((m_NCS_IPC_CREATE(&gld_cb->mbx) != NCSCC_RC_SUCCESS) ||
       (m_NCS_IPC_ATTACH(&gld_cb->mbx) != NCSCC_RC_SUCCESS) ||
       (m_NCS_TASK_CREATE ((NCS_OS_CB)gld_main_process,
                           &gld_cb->mbx,"GLD",m_GLD_TASK_PRIORITY,
                           m_GLD_STACKSIZE,
                           &gld_cb->task_hdl) != NCSCC_RC_SUCCESS) ||
       (m_NCS_TASK_START (gld_cb->task_hdl) != NCSCC_RC_SUCCESS))
        
   {
      m_LOG_GLD_HEADLINE(GLD_IPC_TASK_INIT, NCSFL_SEV_ERROR);
      m_NCS_TASK_RELEASE(gld_cb->task_hdl);
      m_NCS_IPC_RELEASE(&gld_cb->mbx, NULL);
      m_MMGR_FREE_GLSV_GLD_CB(gld_cb);
      return (NCSCC_RC_FAILURE);
   }

   m_NCS_EDU_HDL_INIT(&gld_cb->edu_hdl);


    /* register GLD component with AvSv */
   amf_error = saAmfComponentRegister(gld_cb->amf_hdl, 
               &gld_cb->comp_name, (SaNameT*)NULL);
   if (amf_error != SA_AIS_OK)
   {            
      m_LOG_GLD_SVC_PRVDR(GLD_AMF_REG_ERROR,NCSFL_SEV_ERROR);
      saAmfFinalize(gld_cb->amf_hdl);
      gld_mds_shut(gld_cb);
      m_NCS_TASK_RELEASE(gld_cb->task_hdl);
      m_NCS_IPC_RELEASE(&gld_cb->mbx, NULL);
      m_NCS_EDU_HDL_FLUSH(&gld_cb->edu_hdl);
      m_MMGR_FREE_GLSV_GLD_CB(gld_cb);
      return NCSCC_RC_FAILURE;
   }
   else
      m_LOG_GLD_SVC_PRVDR(GLD_AMF_REG_SUCCESS,NCSFL_SEV_INFO);

   /** start the AMF health check **/   
   memset(&Healthy,0,sizeof(Healthy));
   health_key = getenv("GLSV_ENV_HEALTHCHECK_KEY");
   if(health_key == NULL)
   {
      strcpy(Healthy.key,"A1B2");
      m_LOG_GLD_HEADLINE(GLD_HEALTH_KEY_DEFAULT_SET, NCSFL_SEV_INFO);
   }
   else
   {
      strcpy(Healthy.key,health_key);
   }
   Healthy.keyLen=strlen(Healthy.key);

   amf_error = saAmfHealthcheckStart(gld_cb->amf_hdl,&gld_cb->comp_name,&Healthy,
      SA_AMF_HEALTHCHECK_AMF_INVOKED,SA_AMF_COMPONENT_FAILOVER);
   if (amf_error != SA_AIS_OK)
   {
      m_LOG_GLD_SVC_PRVDR(GLD_AMF_HLTH_CHK_START_FAIL,NCSFL_SEV_ERROR);
   }
   else
      m_LOG_GLD_SVC_PRVDR(GLD_AMF_HLTH_CHK_START_DONE,NCSFL_SEV_INFO);
   
   return (res);
}


/****************************************************************************
 * Name          : gld_se_lib_destroy
 *
 * Description   : Invoked to destroy the GLD
 *                 
 *
 * Arguments     : 
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32
gld_se_lib_destroy (NCS_LIB_REQ_INFO *req_info)
{
   GLSV_GLD_CB *gld_cb;

   if ((gld_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_GLD, gl_gld_hdl))
                  == NULL)
   {
      m_LOG_GLD_HEADLINE(GLD_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      return (NCSCC_RC_FAILURE);      
   } else
   {
      /* Disconnect from MDS */
      gld_mds_shut(gld_cb);

      saAmfComponentUnregister(gld_cb->amf_hdl, &gld_cb->comp_name,
         (SaNameT*)NULL);
      saAmfFinalize(gld_cb->amf_hdl);

      ncshm_give_hdl(gl_gld_hdl);
      ncshm_destroy_hdl(NCS_SERVICE_ID_GLD, gl_gld_hdl);   
      m_NCS_IPC_DETACH(&gld_cb->mbx ,gld_clear_mbx,gld_cb);
    
      gld_cb_destroy(gld_cb);
      m_MMGR_FREE_GLSV_GLD_CB(gld_cb);
   }
   return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : gld_cb_init
 *
 * Description   : This function is invoked at init time. Initiaziles all the
 *                 parameters in the CB
 *
 * Arguments     : gld_cb  - GLD control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32
gld_cb_init (GLSV_GLD_CB *gld_cb)
{
   NCS_PATRICIA_PARAMS      params;

   memset(&params,0,sizeof(NCS_PATRICIA_PARAMS));
     
   /* Intialize all the patrica trees */
   params.key_size = sizeof(uns32);
   params.info_size = 0;
   if ((ncs_patricia_tree_init(&gld_cb->glnd_details, &params))
      != NCSCC_RC_SUCCESS)
   {         
      m_LOG_GLD_HEADLINE(GLD_PATRICIA_TREE_INIT_FAILED, NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }
   gld_cb->glnd_details_tree_up = TRUE;

   params.key_size = sizeof(uns32);
   params.info_size = 0;
   if ((ncs_patricia_tree_init(&gld_cb->rsc_info_id, &params))
      != NCSCC_RC_SUCCESS)
   {         
      m_LOG_GLD_HEADLINE(GLD_PATRICIA_TREE_INIT_FAILED, NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }
   gld_cb->rsc_info_id_tree_up = TRUE;

   params.key_size = sizeof(SaNameT);
   params.info_size = 0;
   if ((ncs_patricia_tree_init(&gld_cb->rsc_map_info, &params))
      != NCSCC_RC_SUCCESS)
   {
      m_LOG_GLD_HEADLINE(GLD_PATRICIA_TREE_INIT_FAILED, NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }


   /* Initialize the next resource id */
   gld_cb->nxt_rsc_id = 1;

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : gld_cb_destroy
 *
 * Description   : This function is invoked at destroy time. This function will
 *                 free all the dynamically allocated memory
 *
 * Arguments     : gld_cb  - GLD control block pointer.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32
gld_cb_destroy (GLSV_GLD_CB *gld_cb)
{
   GLSV_GLD_GLND_DETAILS *node_details;
   GLSV_GLD_RSC_INFO     *rsc_info;
   GLSV_NODE_LIST        *node_list;
   GLSV_GLD_GLND_RSC_REF *glnd_rsc;  

   /* destroy the patricia trees */
   while((node_details = (GLSV_GLD_GLND_DETAILS*) ncs_patricia_tree_getnext(&gld_cb->glnd_details, (uns8*)0)))
   {
      while((glnd_rsc = (GLSV_GLD_GLND_RSC_REF*) ncs_patricia_tree_getnext(&node_details->rsc_info_tree, (uns8*)0)))
      {
         ncs_patricia_tree_del(&node_details->rsc_info_tree, (NCS_PATRICIA_NODE*)glnd_rsc);
         m_MMGR_FREE_GLSV_GLD_GLND_RSC_REF(glnd_rsc);
      }
      ncs_patricia_tree_destroy(&node_details->rsc_info_tree);
      ncs_patricia_tree_del(&gld_cb->glnd_details, (NCS_PATRICIA_NODE*)node_details);
      m_MMGR_FREE_GLSV_GLD_GLND_DETAILS(node_details);
   }

   while((rsc_info = (GLSV_GLD_RSC_INFO*) ncs_patricia_tree_getnext(&gld_cb->rsc_info_id, (uns8*)0)))
   {
      /* Free the node list */
      while(rsc_info->node_list != NULL)
      {
         node_list = rsc_info->node_list;
         rsc_info->node_list = node_list->next;
         m_MMGR_FREE_GLSV_NODE_LIST(node_list);
      }

      gld_free_rsc_info(gld_cb, rsc_info);
   }
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : gld_clear_mbx
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
NCS_BOOL
gld_clear_mbx (NCSCONTEXT arg, NCSCONTEXT msg)
{   
   GLSV_GLD_EVT  *pEvt = (GLSV_GLD_EVT *)msg;
   GLSV_GLD_EVT  *pnext;
   pnext = pEvt;
   while (pnext)
   {
      pnext = pEvt->next;
      gld_evt_destroy(pEvt);  
      pEvt = pnext;
   }
   return TRUE;
}



/****************************************************************************
 * Name          : gld_process_mbx
 *
 * Description   : This is the function which process the IPC mail box of 
 *                 GLD 
 *
 * Arguments     : mbx  - This is the mail box pointer on which IfD/IfND is 
 *                        going to block.  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void
gld_process_mbx(SYSF_MBX *mbx)
{
   GLSV_GLD_EVT *evt = GLSV_GLD_EVT_NULL;

   while(GLSV_GLD_EVT_NULL != (evt = (GLSV_GLD_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(mbx,evt)))
   {
      if ((evt->evt_type >= GLSV_GLD_EVT_BASE) && (evt->evt_type < GLSV_GLD_EVT_MAX))
      {        
         /* This event belongs to GLD */
         gld_process_evt(evt);
      } else
      {
         m_LOG_GLD_HEADLINE(GLD_UNKNOWN_EVT_RCVD, NCSFL_SEV_ERROR);
         m_MMGR_FREE_GLSV_GLD_EVT(evt); 
      }
   }
   return;
}

/****************************************************************************
 * Name          : gld_main_process
 *
 * Description   : This is the function which is given as a input to the 
 *                 GLD task.
 *                 This function will be select of both the FD's (AMF FD and
 *                 Mail Box FD), depending on which FD has been selected, it
 *                 will call the corresponding routines.
 *
 * Arguments     : mbx  - This is the mail box pointer on which GLD is 
 *                        going to block.  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
static void
gld_main_process(SYSF_MBX *mbx)
{   
   NCS_SEL_OBJ          mbx_fd  = m_NCS_IPC_GET_SEL_OBJ(mbx);
   GLSV_GLD_CB          *gld_cb = NULL;

   NCS_SEL_OBJ_SET     all_sel_obj;
   NCS_SEL_OBJ         amf_ncs_sel_obj;
   NCS_SEL_OBJ         mbcsv_ncs_sel_obj;
   NCS_MBCSV_ARG       mbcsv_arg;
   NCS_SEL_OBJ         high_sel_obj;
   SaSelectionObjectT  amf_sel_obj;
   SaAmfHandleT        amf_hdl;
   SaAisErrorT            amf_error;
   
   
   
   if ((gld_cb = (GLSV_GLD_CB*) ncshm_take_hdl(NCS_SERVICE_ID_GLD,gl_gld_hdl))
      == NULL)
   {
      m_LOG_GLD_HEADLINE(GLD_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      return;
   }

   amf_hdl = gld_cb->amf_hdl;
   ncshm_give_hdl(gl_gld_hdl);

   m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
   m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);

   amf_error = saAmfSelectionObjectGet(amf_hdl, &amf_sel_obj);

   if (amf_error != SA_AIS_OK)
   {     
      m_LOG_GLD_SVC_PRVDR(GLD_AMF_SEL_OBJ_GET_ERROR,NCSFL_SEV_ERROR);
      return;
   }
   m_SET_FD_IN_SEL_OBJ((uns32) amf_sel_obj,amf_ncs_sel_obj);
   m_NCS_SEL_OBJ_SET(amf_ncs_sel_obj, &all_sel_obj);

   m_SET_FD_IN_SEL_OBJ((uns32)gld_cb->mbcsv_sel_obj,mbcsv_ncs_sel_obj);
   m_NCS_SEL_OBJ_SET(mbcsv_ncs_sel_obj,&all_sel_obj);


   high_sel_obj = m_GET_HIGHER_SEL_OBJ(amf_ncs_sel_obj,mbx_fd);
   high_sel_obj = m_GET_HIGHER_SEL_OBJ(high_sel_obj,mbcsv_ncs_sel_obj);

   while (m_NCS_SEL_OBJ_SELECT(high_sel_obj,&all_sel_obj,0,0,0) != -1)
   {
      /* process all the AMF messages */
      if (m_NCS_SEL_OBJ_ISSET(amf_ncs_sel_obj,&all_sel_obj))
      {
         /* dispatch all the AMF pending function */
         amf_error = saAmfDispatch(amf_hdl, SA_DISPATCH_ALL);
         if (amf_error != SA_AIS_OK)
         {
            m_LOG_GLD_SVC_PRVDR(GLD_AMF_DISPATCH_ERROR,NCSFL_SEV_ERROR);
         }
      }

       /* Process all the MBCSV messages  */
      if(m_NCS_SEL_OBJ_ISSET(mbcsv_ncs_sel_obj,&all_sel_obj))
      {
          /* dispatch all the MBCSV pending callbacks */
         mbcsv_arg.i_op        = NCS_MBCSV_OP_DISPATCH;
         mbcsv_arg.i_mbcsv_hdl = gld_cb->mbcsv_handle;
         mbcsv_arg.info.dispatch.i_disp_flags = SA_DISPATCH_ALL;
         if(ncs_mbcsv_svc(&mbcsv_arg)!=SA_AIS_OK)
         {
            m_LOG_GLD_HEADLINE(GLD_MBCSV_DISPATCH_FAILURE,NCSFL_SEV_ERROR);
         }
      }

      /* process the GLSv Mail box */
      if (m_NCS_SEL_OBJ_ISSET(mbx_fd,&all_sel_obj))
      {
         /* now got the IPC mail box event */
         gld_process_mbx(mbx);
      }

      /*** set the fd's again ***/
      m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
      m_NCS_SEL_OBJ_SET(amf_ncs_sel_obj, &all_sel_obj);
      m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);
      m_NCS_SEL_OBJ_SET(mbcsv_ncs_sel_obj,&all_sel_obj);


   }    
   return;
}

/****************************************************************************
 * Name          : gld_dump_cb
 *
 * Description   : This is the function dumps the contents of the control block.
 *
 * Arguments     : gld_cb  -  Pointer to the control block
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void gld_dump_cb()
{
   GLSV_GLD_CB          *gld_cb = NULL;
   GLSV_GLD_GLND_DETAILS   *node_details;
   MDS_DEST                mds_dest_id;
   GLSV_GLD_RSC_INFO       *rsc_info;
   SaLckResourceIdT        rsc_id = 0;
   uns32                   node_id = 0;

   gld_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_GLD, gl_gld_hdl);
   
   memset(&mds_dest_id,0,sizeof(MDS_DEST));
   
   m_NCS_OS_PRINTF("************ GLD CB info *************** \n");
   /* print Amf Info */
   m_NCS_OS_PRINTF("AMF HA state : %d \n",gld_cb->ha_state);
   /* print the Node details */
   m_NCS_OS_PRINTF("GLND info :\n");
   while((node_details = (GLSV_GLD_GLND_DETAILS *)ncs_patricia_tree_getnext(&gld_cb->glnd_details, 
                                            (uns8*)&node_id)))
   {
      node_id = node_details->node_id;
      m_NCS_OS_PRINTF("Node Id - :%d \n",node_details->node_id);
   }

   /* print the Resource details */
   while((rsc_info = (GLSV_GLD_RSC_INFO*) ncs_patricia_tree_getnext(&gld_cb->rsc_info_id, (uns8*)&rsc_id)))
   {
      GLSV_NODE_LIST      *list;
      rsc_id = rsc_info->rsc_id;
      m_NCS_OS_PRINTF("\nResource Id - : %d  Resource Name - %.10s \n",
         (uns32)rsc_info->rsc_id, rsc_info->lck_name.value);
      m_NCS_OS_PRINTF("Can Orphan - %d Mode - %d \n",rsc_info->can_orphan,
         (uns32)rsc_info->orphan_lck_mode);
      list = rsc_info->node_list;
      m_NCS_OS_PRINTF("List of Nodes :");   
      while(list != NULL)
      {
         m_NCS_OS_PRINTF("%d    ",m_NCS_NODE_ID_FROM_MDS_DEST(list->dest_id));
         list = list->next;
      }
      m_NCS_OS_PRINTF("\n");
   }
   ncshm_give_hdl(gl_gld_hdl);
   m_NCS_OS_PRINTF("************************************************** \n");

}
