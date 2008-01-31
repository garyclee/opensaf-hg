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
  FILE NAME: cpd_init.c

  DESCRIPTION: APIs used to initialize & Destroy the cpd Service Part.

  FUNCTIONS INCLUDED in this module:
  cpd_lib_req ............ SE API to init and create PWE for CPD
  cpd_lib_init ........... Function used to init CPD.  
  cpd_lib_destroy ........ Function used to destroy CPD.
  cpd_main_process ........Process all the events posted to CPD.
******************************************************************************/

#include "cpd.h"

uns32 gl_cpd_cb_hdl = 0;

/* Static Function Declerations */
static uns32 
cpd_extract_create_info(int argc, char *argv[], CPD_CREATE_INFO *create_info);

static uns32 
cpd_extract_destroy_info(int argc, char *argv[], CPD_DESTROY_INFO *destroy_info);

static uns32 cpd_lib_init(CPD_CREATE_INFO *info);

static uns32 cpd_lib_destroy (CPD_DESTROY_INFO *info);

static NCS_BOOL cpd_clear_mbx (NCSCONTEXT arg, NCSCONTEXT msg);

static void cpd_main_process(NCSCONTEXT info);


/****************************************************************************
 * Name          : cpd_lib_req
 *
 * Description   : This is the SE API which is used to init/destroy or 
 *                 Create/destroy PWE's of CPD. This will be called by SBOM.
 *
 * Arguments     : req_info  - This is the pointer to the input information 
 *                             which SBOM gives.  
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32 cpd_lib_req (NCS_LIB_REQ_INFO *req_info)
{
   uns32 rc = NCSCC_RC_FAILURE;
   CPD_CREATE_INFO   create_info;
   CPD_DESTROY_INFO  destroy_info;

   switch (req_info->i_op)
   {      
      case NCS_LIB_REQ_CREATE:
         if (cpd_extract_create_info(req_info->info.create.argc, 
            req_info->info.create.argv, &create_info) == NCSCC_RC_FAILURE)
         {
            break;
         }
         rc = cpd_lib_init(&create_info);
         break;
      case NCS_LIB_REQ_DESTROY:         
         if (cpd_extract_destroy_info(req_info->info.create.argc, 
            req_info->info.create.argv, &destroy_info) == NCSCC_RC_FAILURE)
         {
            break;
         }
         rc = cpd_lib_destroy(&destroy_info);
         break;
      default:
         break;
   }
   return (rc);
}


/****************************************************************************
 * Name          : cpd_extract_create_info
 *
 * Description   : Used to extract the create information from argc & agrv
 *
 * Arguments     : argc  - This is the Number of arguments received.
 *                 argv  - string received.
 *                 create_info - Structure to be filled for initing the service.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
cpd_extract_create_info(int argc, char *argv[], CPD_CREATE_INFO *create_info)
{

   m_NCS_MEMSET(create_info,0,sizeof(CPD_CREATE_INFO));

   /* Need to change this once we get these parameters in the argv */
   create_info->pool_id   = NCS_HM_POOL_ID_COMMON;

   return(NCSCC_RC_SUCCESS);
}


/****************************************************************************
 * Name          : cpd_extract_destroy_info
 *
 * Description   : Used to extract the destroy information from argc & agrv
 *
 * Arguments     : argc  - This is the Number of arguments received.
 *                 argv  - string received.
 *                 destroy_info - Structure to be filled for initing the service.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
cpd_extract_destroy_info(int argc, char *argv[], CPD_DESTROY_INFO *destroy_info)
{

   m_NCS_MEMSET(destroy_info,0,sizeof(CPD_DESTROY_INFO));

   /* Need to fill this once we get these parameters in the argv */

   return(NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : cpd_lib_init
 *
 * Description   : This is the function which initalize the CPD libarary.
 *                 This function creates an IPC mail Box and spawns CPD
 *                 thread.
 *                 This function initializes the CB, handle manager, MDS, CPD 
 *                 and Registers with AMF with respect to the component Type 
 *                 (CPD).
 *
 * Arguments     : create_info: pointer to struct CPD_CREATE_INFO.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 cpd_lib_init (CPD_CREATE_INFO *info)
{
   CPD_CB        *cb;
   uns32          rc = NCSCC_RC_SUCCESS;
   SaAisErrorT   amf_error;
   SaAmfHealthcheckKeyT healthy;
   int8*     health_key;
   SaVersionT      clm_version;   

   m_CPSV_GET_AMF_VER(clm_version);
   SaClmCallbacksT cpd_clm_cbk;


   /* register with the Flex log service */
   cpd_flx_log_reg( );
  
   /* allocate a CB  */
   cb = m_MMGR_ALLOC_CPD_CB;

   if (cb == NULL)
   {
      m_LOG_CPD_CL(CPD_CB_ALLOC_FAILED,CPD_FC_MEMFAIL,NCSFL_SEV_ERROR,__FILE__,__LINE__);
      rc = NCSCC_RC_OUT_OF_MEM;
      goto cpd_cb_alloc_fail;
   }
   m_NCS_MEMSET(cb, 0, sizeof(CPD_CB));
   cb->hm_poolid       = info->pool_id;
   
   /* Init the EDU Handle */
   m_NCS_EDU_HDL_INIT(&cb->edu_hdl);
   
   if ((rc = cpd_cb_db_init(cb)) == NCSCC_RC_FAILURE)
   {
      m_NCS_CONS_PRINTF("CPD_CB_DB_INIT FAILED\n");
      goto cpd_cb_init_fail;         
   } 
   
   if((cb->cpd_hdl = ncshm_create_hdl(cb->hm_poolid, 
      NCS_SERVICE_ID_CPD, (NCSCONTEXT)cb)) == 0)
   {
      m_LOG_CPD_CL(CPD_CB_HDL_CREATE_FAILED,CPD_FC_HDLN, NCSFL_SEV_ERROR,__FILE__,__LINE__);
      m_NCS_CONS_PRINTF("CPD_CB_HDL_CREATE_FAILED\n");
      rc = NCSCC_RC_FAILURE;
      goto cpd_hdl_fail;            
   }                        
   
   /* Store the handle in some global location */
   m_CPD_STORE_CB_HDL(cb->cpd_hdl);

   /* create a mail box */
   if ((rc = m_NCS_IPC_CREATE(&cb->cpd_mbx)) != NCSCC_RC_SUCCESS)         
   {
      m_LOG_CPD_CL(CPD_IPC_CREATE_FAIL,CPD_FC_HDLN, NCSFL_SEV_ERROR,__FILE__,__LINE__);
      m_NCS_CONS_PRINTF("CPD_IPC_CREATE_FAIL\n");
      goto cpd_ipc_create_fail;
   }
   
   /* Attach the IPC to mail box */
   if ((rc = m_NCS_IPC_ATTACH(&cb->cpd_mbx)) != NCSCC_RC_SUCCESS)
   {
      m_LOG_CPD_CL(CPD_IPC_ATTACH_FAIL,CPD_FC_HDLN, NCSFL_SEV_ERROR,__FILE__,__LINE__);
      m_NCS_CONS_PRINTF("CPD_IPC_ATTACH_FAILED\n");
      goto cpd_ipc_att_fail;
   }
   
   if ((rc = cpd_mds_register(cb)) != NCSCC_RC_SUCCESS)
   {
      m_LOG_CPD_CL(CPD_MDS_REGISTER_FAILED,CPD_FC_HDLN, NCSFL_SEV_ERROR,__FILE__,__LINE__);
      m_NCS_CONS_PRINTF("CPD_MDS_REGISTER_FAILED %d\n",rc);
      goto cpd_mds_fail;
   }

   /* Initialise with the AMF service */
   if(cpd_amf_init(cb) != NCSCC_RC_SUCCESS)
   {
      m_LOG_CPD_CL(CPD_AMF_INIT_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__);
      m_NCS_CONS_PRINTF("CPD_AMF_INIT_FAILED %d\n",rc);
      goto amf_init_err;
   }
                                                                                                                             
   /* register with the AMF service */
   if(cpd_amf_register(cb) != NCSCC_RC_SUCCESS)
   {
      m_LOG_CPD_CL(CPD_AMF_REGISTER_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__);
      m_NCS_CONS_PRINTF("CPD_AMF_REGISTER_FAILED\n");
      goto amf_reg_err;
   }
   
 
   /*   Initialise with the MBCSV service  */
   if(cpd_mbcsv_register(cb)!=NCSCC_RC_SUCCESS)
   {
        m_LOG_CPD_CL(CPD_MBCSV_INIT_FAILED,CPD_FC_MBCSV,NCSFL_SEV_ERROR,__FILE__,__LINE__); 
        m_NCS_CONS_PRINTF("CPD_MBCSV_INIT_FAILED\n");
        goto mbcsv_reg_err;
   }

   cpd_clm_cbk.saClmClusterNodeGetCallback = NULL;
   cpd_clm_cbk.saClmClusterTrackCallback   = cpd_clm_cluster_track_cb;

   if ( saClmInitialize(&cb->clm_hdl,&cpd_clm_cbk,&clm_version) != SA_AIS_OK)
   {
       m_LOG_CPD_CL(CPD_CLM_REGISTER_FAIL,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__);
       m_NCS_CONS_PRINTF("CPD_CLM_REGISTER_FAIL\n");
       goto cpd_clm_fail;
   }

   if ( saClmClusterTrack(cb->clm_hdl,SA_TRACK_CHANGES_ONLY,NULL)!= SA_AIS_OK)
   {
       m_NCS_CONS_PRINTF("CPD_CLM_CLUSTER_TRACK FAIL\n");
       goto cpd_clm_fail;
   }

   m_NCS_OS_STRCPY(cb->safSpecVer.value,"B.01.01");
   cb->safSpecVer.length = m_NCS_STRLEN("B.01.01");
   m_NCS_OS_STRCPY(cb->safAgtVen.value,"OpenSAF");
   cb->safAgtVen.length = m_NCS_STRLEN("OpenSAF");
   cb->safAgtVenPro = 2;
   cb->serv_enabled = 2;
   cb->serv_state = 1; 

   /* Create the task for CPD */
   if ((rc = m_NCS_TASK_CREATE ((NCS_OS_CB)cpd_main_process,
      (NCSCONTEXT)cb, m_CPD_TASKNAME, m_CPD_TASK_PRI,
      m_CPD_STACKSIZE, &cb->task_hdl)) != NCSCC_RC_SUCCESS)
   {
      m_LOG_CPD_CL(CPD_TASK_CREATE_FAIL,CPD_FC_HDLN, NCSFL_SEV_ERROR,__FILE__,__LINE__);
      m_NCS_CONS_PRINTF("CPD_TASK_CREATE_FAIL\n");
      goto cpd_task_create_fail;
   }
   
   if ((rc = m_NCS_TASK_START(cb->task_hdl))
      != NCSCC_RC_SUCCESS)         
   {
      m_LOG_CPD_CL(CPD_TASK_START_FAIL,CPD_FC_HDLN, NCSFL_SEV_ERROR,__FILE__,__LINE__);
      m_NCS_CONS_PRINTF("CPD_TASK_START_FAIL\n");
      goto cpd_task_start_fail;            
   }
  
   /* Register with MIBLIB */
   if((rc = cpd_reg_with_miblib())!= NCSCC_RC_SUCCESS)
   {
      m_LOG_CPD_CL(CPD_MIBLIB_REGISTER_FAIL,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__);
      m_NCS_CONS_PRINTF("CPD_MIBLIB_REGISTER_FAILED\n");
      goto cpd_mab_fail;
   }
  /* Register with MASv */
   if((rc = cpd_reg_with_mab(cb))!= NCSCC_RC_SUCCESS)
   {
      m_LOG_CPD_CL(CPD_MASV_REGISTER_FAIL,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__); 
      m_NCS_CONS_PRINTF("CPD_MASV_REGISTER_FAILED\n"); 
      goto cpd_mab_fail;
   }

#if 0
  /* Register with CLM */
   cpd_clm_cbk.saClmClusterNodeGetCallback = NULL;
   cpd_clm_cbk.saClmClusterTrackCallback   = cpd_clm_cluster_track_cb;

   if ( saClmInitialize(&cb->clm_hdl,&cpd_clm_cbk,&clm_version) != SA_AIS_OK)
   {
       m_LOG_CPD_HEADLINE(CPD_CLM_REGISTER_FAIL,NCSFL_SEV_ERROR);
       m_NCS_CONS_PRINTF("CPD_CLM_REGISTER_FAIL\n");
       goto cpd_clm_fail;
   }

   if ( saClmClusterTrack(cb->clm_hdl,SA_TRACK_CHANGES_ONLY,NULL)!= SA_AIS_OK)
   {
       m_NCS_CONS_PRINTF("CPD_CLM_CLUSTER_TRACK FAIL\n");
       goto cpd_clm_fail;
   }
#endif

 
   m_NCS_MEMSET(&healthy,0,sizeof(healthy));
   health_key = m_NCS_OS_PROCESS_GET_ENV_VAR("CPSV_ENV_HEALTHCHECK_KEY");
   if(health_key == NULL)
   {
      m_NCS_STRCPY(healthy.key,"A1B2");
      /* TBD Log */
   }
   else
   {
      m_NCS_STRCPY(healthy.key,health_key);
   }
   healthy.keyLen=strlen(healthy.key);

   amf_error = saAmfHealthcheckStart(cb->amf_hdl,&cb->comp_name,&healthy,
      SA_AMF_HEALTHCHECK_AMF_INVOKED,SA_AMF_COMPONENT_FAILOVER);
  
   if (amf_error != SA_AIS_OK)
   {
      m_LOG_CPD_CL(CPD_HEALTHCHECK_START_FAIL,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__); 
   }

   
   m_LOG_CPD_HEADLINE(CPD_INIT_SUCCESS, NCSFL_SEV_INFO);
   return NCSCC_RC_SUCCESS;
   
cpd_clm_fail:
     saClmFinalize(cb->clm_hdl);

cpd_mab_fail:
     cpd_mds_unregister(cb);

cpd_task_start_fail:      
   m_NCS_TASK_RELEASE(cb->task_hdl);
   
cpd_task_create_fail:
   cpd_mbcsv_finalize(cb);

mbcsv_reg_err:
   cpd_amf_deregister(cb);

amf_reg_err:
   cpd_amf_de_init(cb);

amf_init_err:
   cpd_mds_unregister(cb);

cpd_mds_fail:
   m_NCS_IPC_DETACH(&cb->cpd_mbx, cpd_clear_mbx, cb);   
   
cpd_ipc_att_fail:
   m_NCS_IPC_RELEASE(&cb->cpd_mbx, NULL);
   
cpd_ipc_create_fail:
   ncshm_destroy_hdl(NCS_SERVICE_ID_CPD, cb->cpd_hdl);
   
cpd_hdl_fail:            
   cpd_cb_db_destroy(cb);
   
cpd_cb_init_fail:
  m_NCS_EDU_HDL_FLUSH(&cb->edu_hdl);
  m_MMGR_FREE_CPD_CB(cb);
  
cpd_cb_alloc_fail:
   cpd_flx_log_dereg( );

   m_LOG_CPD_CL(CPD_INIT_FAIL,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__);
   
   return (rc);
}

/****************************************************************************
 * Name          : cpd_lib_destroy
 *
 * Description   : This is the function which destroy the cpd libarary.
 *                 This function releases the Task and the IPX mail Box.
 *                 This function unregisters with AMF, destroies handle 
 *                 manager, CB and clean up all the component specific 
 *                 databases.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 cpd_lib_destroy (CPD_DESTROY_INFO *info)
{ 
   CPD_CB *cb;
   uns32 rc = NCSCC_RC_SUCCESS;

   m_CPD_RETRIEVE_CB(cb);
   if(cb == NULL)
   {
      m_LOG_CPD_CL(CPD_DESTROY_FAIL,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__);
      return (NCSCC_RC_FAILURE);
   }      

   m_NCS_TASK_STOP(cb->task_hdl);
   
   m_NCS_TASK_RELEASE(cb->task_hdl);
   
   m_NCS_IPC_DETACH(&cb->cpd_mbx, cpd_clear_mbx, cb);      
   
   m_NCS_IPC_RELEASE(&cb->cpd_mbx, NULL);
   
   ncshm_destroy_hdl(NCS_SERVICE_ID_CPD, cb->cpd_hdl);
   
   cpd_flx_log_dereg( );

   saClmFinalize(cb->clm_hdl);

   cpd_mds_unregister(cb);

   /* deregister with the AMF */
   if(cpd_amf_deregister(cb) != NCSCC_RC_SUCCESS)
   {
      rc = NCSCC_RC_FAILURE;
   }
                                                                                                                             
   /* uninitialise with AMF */
   cpd_amf_de_init(cb);

   /* deregister with MBCSV */
   cpd_mbcsv_finalize(cb);

   ncshm_give_hdl(cb->cpd_hdl);

   cpd_cb_db_destroy(cb);

   m_MMGR_FREE_CPD_CB(cb);

   if(rc == NCSCC_RC_SUCCESS)
   {
      m_LOG_CPD_HEADLINE(CPD_DESTROY_SUCCESS, NCSFL_SEV_INFO);
   }
   else
   {
      m_LOG_CPD_CL(CPD_DESTROY_FAIL,CPD_FC_HDLN, NCSFL_SEV_ERROR,__FILE__,__LINE__);
   }
   return rc;
}


/****************************************************************************
 * Name          : cpd_clear_mbx
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
static NCS_BOOL cpd_clear_mbx (NCSCONTEXT arg, NCSCONTEXT msg)
{   
   CPSV_EVT  *pEvt = (CPSV_EVT *)msg;
   CPSV_EVT  *pnext;
   pnext = pEvt;
   while (pnext)
   {
      pnext = pEvt->next;
      m_MMGR_FREE_CPSV_EVT(pEvt, NCS_SERVICE_ID_CPD);  
      pEvt = pnext;
   }
   
   return TRUE;
}

/****************************************************************************
 * Name          : cpd_main_process
 *
 * Description   : This is the function which is given as a input to the 
 *                 CPD task.
 *                 This function will be select of both the FD's (AMF FD and
 *                 Mail Box FD), depending on which FD has been selected, it
 *                 will call the corresponding routines.
 *
 * Arguments     : mbx  - This is the mail box pointer on which IfD/IfND is 
 *                        going to block.  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
static void cpd_main_process(NCSCONTEXT info)
{   
   CPD_CB             *cb = (CPD_CB*)info;      
   NCS_SEL_OBJ_SET     all_sel_obj;
   NCS_SEL_OBJ         mbx_fd;
   SYSF_MBX            mbx = cb->cpd_mbx;
   CPSV_EVT            *evt = 0;
   SaSelectionObjectT  amf_sel_obj,clm_sel_obj;
   SaAmfHandleT        amf_hdl;
   SaAisErrorT         amf_error;
   NCS_SEL_OBJ         mbcsv_ncs_sel_obj,amf_ncs_sel_obj, highest_sel_obj, clm_ncs_sel_obj;
   NCS_MBCSV_ARG       mbcsv_arg;

   mbx_fd = m_NCS_IPC_GET_SEL_OBJ(&cb->cpd_mbx);
   m_NCS_SEL_OBJ_ZERO(&all_sel_obj);
   m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj); 

   amf_hdl = cb->amf_hdl;
   amf_error = saAmfSelectionObjectGet(amf_hdl, &amf_sel_obj);
                                                                                                                             
   if (amf_error != SA_AIS_OK)
   {
      m_LOG_CPD_CL(CPD_AMF_GET_SEL_OBJ_FAILURE,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__);
      return;
   }
   m_SET_FD_IN_SEL_OBJ((uns32)amf_sel_obj,amf_ncs_sel_obj);
   m_NCS_SEL_OBJ_SET(amf_ncs_sel_obj, &all_sel_obj);
   
   m_SET_FD_IN_SEL_OBJ((uns32)cb->mbcsv_sel_obj,mbcsv_ncs_sel_obj);
   m_NCS_SEL_OBJ_SET(mbcsv_ncs_sel_obj,&all_sel_obj);

   if(saClmSelectionObjectGet(cb->clm_hdl,&clm_sel_obj) != SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("CLM Selection Object Get failed\n");
      return;
   }

   m_SET_FD_IN_SEL_OBJ((uns32)clm_sel_obj,clm_ncs_sel_obj);
   m_NCS_SEL_OBJ_SET(clm_ncs_sel_obj,&all_sel_obj);


                                                                                                                          
   highest_sel_obj  = m_GET_HIGHER_SEL_OBJ(amf_ncs_sel_obj,mbx_fd);
   highest_sel_obj  = m_GET_HIGHER_SEL_OBJ(highest_sel_obj,mbcsv_ncs_sel_obj);
   highest_sel_obj  = m_GET_HIGHER_SEL_OBJ(highest_sel_obj,clm_ncs_sel_obj);
   while (m_NCS_SEL_OBJ_SELECT(highest_sel_obj,&all_sel_obj,0,0,0) != -1)
   {

      /* process all the AMF messages */
      if (m_NCS_SEL_OBJ_ISSET(amf_ncs_sel_obj,&all_sel_obj))
      {
         /* dispatch all the AMF pending function */
         amf_error = saAmfDispatch(amf_hdl, SA_DISPATCH_ALL);
         if (amf_error != SA_AIS_OK)
         {
            m_LOG_CPD_CL(CPD_AMF_DISPATCH_FAILURE,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__);
         }
      }

      /* Process all Clm Messages */
      if (m_NCS_SEL_OBJ_ISSET(clm_ncs_sel_obj,&all_sel_obj))
      {
         /* dispatch all the CLM pending function */
         if(saClmDispatch(cb->clm_hdl, SA_DISPATCH_ALL) != SA_AIS_OK)
         {
            m_LOG_CPD_HEADLINE(CPD_AMF_DISPATCH_FAILURE,NCSFL_SEV_ERROR);
         }
      }

     /* Process all the MBCSV messages  */
      if(m_NCS_SEL_OBJ_ISSET(mbcsv_ncs_sel_obj,&all_sel_obj))
      {
          /* dispatch all the MBCSV pending callbacks */
         mbcsv_arg.i_op        = NCS_MBCSV_OP_DISPATCH;
         mbcsv_arg.i_mbcsv_hdl = cb->mbcsv_handle;
         mbcsv_arg.info.dispatch.i_disp_flags = SA_DISPATCH_ALL;
         if(ncs_mbcsv_svc(&mbcsv_arg)!=SA_AIS_OK)
         {
            m_LOG_CPD_CL(CPD_MBCSV_DISPATCH_FAILURE,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__); 
         }
      }

      /* process the CPD Mail box */
      if (m_NCS_SEL_OBJ_ISSET(mbx_fd,&all_sel_obj))
      {        
         
         if (NULL != (evt = (CPSV_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(&mbx,evt)))
         {
            /* now got the IPC mail box event */
            cpd_process_evt(evt);
         }
      }

      m_NCS_SEL_OBJ_SET(amf_ncs_sel_obj, &all_sel_obj);
      m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);
      m_NCS_SEL_OBJ_SET(mbcsv_ncs_sel_obj,&all_sel_obj);
      m_NCS_SEL_OBJ_SET(clm_ncs_sel_obj,&all_sel_obj);
   }     
   return;
}
