#if ( (NCS_SRMA == 1)&& (TET_A==1) )

#include "tet_startup.h"
#include "tet_srmsv.h"
#include "tet_srmsv_conf.h"

#define m_TET_SRMSV_PRINTF printf

const char *ncs_srmsv_error[] = {
    "SA_AIS_NOT_VALID",
    "SA_AIS_OK",
    "SA_AIS_ERR_LIBRARY",
    "SA_AIS_ERR_VERSION",
    "SA_AIS_ERR_INIT",
    "SA_AIS_ERR_TIMEOUT",
    "SA_AIS_ERR_TRY_AGAIN",
    "SA_AIS_ERR_INVALID_PARAM",
    "SA_AIS_ERR_NO_MEMORY",
    "SA_AIS_ERR_BAD_HANDLE",
    "SA_AIS_ERR_BUSY",
    "SA_AIS_ERR_ACCESS",
    "SA_AIS_ERR_NOT_EXIST",
    "SA_AIS_ERR_NAME_TOO_LONG",
    "SA_AIS_ERR_EXIST",
    "SA_AIS_ERR_NO_SPACE",
    "SA_AIS_ERR_INTERRUPT",
    "SA_AIS_ERR_NAME_NOT_FOUND",
    "SA_AIS_ERR_NO_RESOURCES",
    "SA_AIS_ERR_NOT_SUPPORTED",
    "SA_AIS_ERR_BAD_OPERATION",
    "SA_AIS_ERR_FAILED_OPERATION",
    "SA_AIS_ERR_MESSAGE_ERROR",
    "SA_AIS_ERR_QUEUE_FULL",
    "SA_AIS_ERR_QUEUE_NOT_AVAILABLE",
    "SA_AIS_ERR_BAD_FLAGS",
    "SA_AIS_ERR_TOO_BIG",
    "SA_AIS_ERR_NO_SECTIONS",
};

int srmsv_test_result(SaAisErrorT rc,SaAisErrorT exp_out,char *test_case,CONFIG_FLAG flg);
int srmsv_test_result(SaAisErrorT rc,SaAisErrorT exp_out,char *test_case,CONFIG_FLAG flg)
{
   int result = 0;

   if(rc == exp_out)
   {
      m_TET_SRMSV_PRINTF("\n SUCCESS       : %s  \n",test_case);
      tet_printf("\n SUCCESS       : %s  \n",test_case);
      result = TET_PASS;
   }
   else
   {
      m_TET_SRMSV_PRINTF("\n FAILED        : %s  \n",test_case);
      tet_printf("\n FAILED        : %s  \n",test_case);
      result = TET_FAIL;
      if(flg == TEST_CONFIG_MODE)
         result = TET_UNRESOLVED;
   }

   m_TET_SRMSV_PRINTF(" Return Value  : %s\n",ncs_srmsv_error[rc]);
   tet_printf(" Return Value  : %s\n",ncs_srmsv_error[rc]);
   return(result);
}

void event_thread_blocking (NCSCONTEXT arg)
{
   SaSelectionObjectT     selection_object;
   NCS_SRMSV_HANDLE srmsvHandle = *(NCS_SRMSV_HANDLE *)arg;
   NCS_SEL_OBJ_SET io_readfds;
   NCS_SEL_OBJ sel_obj;
   SaAisErrorT rc;

   m_TET_SRMSV_PRINTF(" Executing Thread selection\n");
   rc = ncs_srmsv_selection_object_get(srmsvHandle, &selection_object);
   if (rc != SA_AIS_OK)
   {
        m_TET_SRMSV_PRINTF(" Selection Object Get failed  - %d", rc);
        return;
   }

   m_SET_FD_IN_SEL_OBJ(selection_object, sel_obj);
   m_NCS_SEL_OBJ_ZERO(&io_readfds);
   m_NCS_SEL_OBJ_SET(sel_obj, &io_readfds);

   rc = m_NCS_SEL_OBJ_SELECT(sel_obj, &io_readfds, NULL, NULL, NULL);
   if (rc == -1)
   {
      m_NCS_CONS_PRINTF(" Select failed\n");
      return;
   }

   if (m_NCS_SEL_OBJ_ISSET(sel_obj, &io_readfds) )
   {
      rc = ncs_srmsv_dispatch(srmsvHandle, SA_DISPATCH_BLOCKING);
      if (rc != SA_AIS_OK)
         m_NCS_CONS_PRINTF(" Dispatching failed %d \n", rc);
      else
         m_NCS_CONS_PRINTF(" Thread selected \n");
   }
}

void srmsv_createthread(NCS_SRMSV_HANDLE *cl_hdl)
{
   SaAisErrorT  rc;
   NCSCONTEXT thread_handle;

   rc = m_NCS_TASK_CREATE((NCS_OS_CB)event_thread_blocking, (NCSCONTEXT)cl_hdl,
                           "srmsv_block_test", 5, 8000, &thread_handle);
   if (rc != NCSCC_RC_SUCCESS)
   {
       m_TET_SRMSV_PRINTF(" Failed to create thread\n");
       return ;
   }

   rc = m_NCS_TASK_START(thread_handle);
   if (rc != NCSCC_RC_SUCCESS)
   {
       m_TET_SRMSV_PRINTF(" Failed to start thread\n");
       return ;
   }
}

/*********************** INITIALIZE API ***********************/

struct ncsSrmsvInitialize API_SRMSV_Initialize[] = {

  [SRMSV_INIT_SUCCESS_T]    = {&tcd.srmsv_callback,&tcd.client1,SA_AIS_OK,"Init successful for client"},

  [SRMSV_INIT_NULL_CLBK_T]  = {NULL,&tcd.client2,SA_AIS_OK,"Init successful for client2 with NULL callbk"},

  [SRMSV_INIT_BAD_HANDLE_T] = {NULL,NULL,SA_AIS_ERR_INVALID_PARAM,"NULL client handle"},

  [SRMSV_INIT_SUCCESS2_T]   = {&tcd.srmsv_callback,&tcd.client3,SA_AIS_OK,"Init successful for client3"},

  [SRMSV_INIT_SUCCESS3_T]   = {&tcd.srmsv_callback,&tcd.client4,SA_AIS_OK,"Init successful for client4"},

  [SRMSV_INIT_SUCCESS4_T]   = {&tcd.srmsv_callback,&tcd.client5,SA_AIS_OK,"Init successful for client5"},
};

int tet_test_srmsvInitialize(int i,CONFIG_FLAG flg)
{
   NCS_SRMSV_ERR rc;
   NCS_SRMSV_CALLBACK clbk;

   if (API_SRMSV_Initialize[i].callback == NULL)
       clbk=NULL;
   else
       clbk=*(API_SRMSV_Initialize[i].callback);

   rc = ncs_srmsv_initialize(clbk,API_SRMSV_Initialize[i].client_hdl);
   return(srmsv_test_result(rc,API_SRMSV_Initialize[i].exp_output,API_SRMSV_Initialize[i].result_string,flg));
}


/********************** SELOBJ *************************/


struct ncsSrmsvSelectionObject API_SRMSV_Selection[] = {

    [SRMSV_SEL_SUCCESS_T]     = {&tcd.client1,&tcd.sel_client1,SA_AIS_OK,"Selection Obj Get success for client"},

    [SRMSV_SEL_SUCCESS2_T]     = {&tcd.client3,&tcd.sel_client3,SA_AIS_OK,"Selection Obj Get success for client"},

    [SRMSV_SEL_SUCCESS3_T]     = {&tcd.client2,&tcd.sel_client2,SA_AIS_OK,"Selection Obj Get success for client"},

    [SRMSV_SEL_NULL_HDL_T]    = {NULL,&tcd.sel_client2,SA_AIS_ERR_BAD_HANDLE,"NULL hdl"},

    [SRMSV_SEL_BAD_HANDLE_T]  = {NULL,NULL,SA_AIS_ERR_BAD_HANDLE," NULL client handle and NULL selobj"},

    [SRMSV_SEL_BAD_HANDLE2_T] = {&tcd.dummy_hdl,&tcd.sel_dummy,SA_AIS_ERR_BAD_HANDLE," uninitialized handle"},

    [SRMSV_SEL_INVALID_PARAM_T] = {&tcd.client1,NULL,SA_AIS_ERR_INVALID_PARAM,"NULL sel obj"},

    [SRMSV_SEL_BAD_HANDLE3_T] = {&tcd.client3,&tcd.sel_dummy,SA_AIS_ERR_BAD_HANDLE,
                                      "Bad handle after finalising handle"},

};

int tet_test_srmsvSelection(int i,CONFIG_FLAG flg)
{
   SaAisErrorT rc;
   NCS_SRMSV_HANDLE cl_hdl;

   if (API_SRMSV_Selection[i].client_hdl == NULL)
       cl_hdl = 0;
   else
       cl_hdl = *(API_SRMSV_Selection[i].client_hdl);

   rc = ncs_srmsv_selection_object_get(cl_hdl,API_SRMSV_Selection[i].sel_obj);
   return(srmsv_test_result(rc,API_SRMSV_Selection[i].exp_output,API_SRMSV_Selection[i].result_string,flg));
}


/*DISPATCH*/


struct ncsSrmsvDispatch API_SRMSV_Dispatch[] = {

    [SRMSV_DISPATCH_ONE_SUCCESS_T]   = {&tcd.client1,SA_DISPATCH_ONE,SA_AIS_OK,"Dispatch for client ONE"},

    [SRMSV_DISPATCH_ALL_SUCCESS_T]   = {&tcd.client1,SA_DISPATCH_ALL,SA_AIS_OK,"Dispatch for client ALL"},

    [SRMSV_DISPATCH_ALL_SUCCESS2_T]   = {&tcd.client3,SA_DISPATCH_ALL,SA_AIS_OK,"Dispatch for client ALL"},

    [SRMSV_DISPATCH_ALL_SUCCESS3_T]   = {&tcd.client2,SA_DISPATCH_ALL,SA_AIS_OK,"Dispatch for client ALL"},

    [SRMSV_DISPATCH_INVALID_FLAG_T]  = {&tcd.client1,-1,SA_AIS_ERR_BAD_FLAGS,"NULL flag"},

    [SRMSV_DISPATCH_NULL_HDL_T]      = {NULL,SA_DISPATCH_ONE,SA_AIS_ERR_BAD_HANDLE,"NULL hdl dispatch_one"},

    [SRMSV_DISPATCH_NULL_HDL2_T]     = {NULL,SA_DISPATCH_ALL,SA_AIS_ERR_BAD_HANDLE,"NULL hdl dispatch_all"},

    [SRMSV_DISPATCH_NULL_HDL3_T]     = {NULL,SA_DISPATCH_BLOCKING,SA_AIS_ERR_BAD_HANDLE,"NULL hdl dispatch_block"},

    [SRMSV_DISPATCH_BAD_FLAGS_T]     = {&tcd.client2,4,SA_AIS_ERR_BAD_FLAGS,"Dispatch unidentified flags"},

    [SRMSV_DISPATCH_BAD_HANDLE_T]    = {&tcd.dummy_hdl,SA_DISPATCH_ONE,SA_AIS_ERR_BAD_HANDLE,"uninitialized handle"},

    [SRMSV_DISPATCH_BAD_HANDLE2_T]   = {NULL,-1,SA_AIS_ERR_BAD_HANDLE," NULL hdl and flags"},

    [SRMSV_DISPATCH_BAD_HANDLE3_T]   = {&tcd.client3,SA_DISPATCH_ONE,SA_AIS_ERR_BAD_HANDLE,"finalized handle"},

    [SRMSV_DISPATCH_ONE_SUCCESS2_T]   = {&tcd.client3,SA_DISPATCH_ONE,SA_AIS_OK,"Dispatch single clbk for client "},

};

int tet_test_srmsvDispatch(int i,CONFIG_FLAG flg)
{
   SaAisErrorT rc;
   NCS_SRMSV_HANDLE cl_hdl;

   if (API_SRMSV_Dispatch[i].client_hdl == NULL)
       cl_hdl = 0;
   else
       cl_hdl = *(API_SRMSV_Dispatch[i].client_hdl);

   rc = ncs_srmsv_dispatch(cl_hdl,API_SRMSV_Dispatch[i].flags);
   return(srmsv_test_result(rc,API_SRMSV_Dispatch[i].exp_output,API_SRMSV_Dispatch[i].result_string,flg));
}


/**************** FINALIZE *****************/

struct ncsSrmsvFinalize API_SRMSV_Finalize[] = {

    [SRMSV_FIN_SUCCESS_T]     = {&tcd.client1,SA_AIS_OK,"client Finalized"},

    [SRMSV_FIN_SUCCESS2_T]    = {&tcd.client2,SA_AIS_OK,"client Finalized"},

    [SRMSV_FIN_SUCCESS3_T]    = {&tcd.client3,SA_AIS_OK,"client Finalized"},

    [SRMSV_FIN_SUCCESS4_T]    = {&tcd.client4,SA_AIS_OK,"client Finalized"},

    [SRMSV_FIN_BAD_HANDLE_T]  = {&tcd.dummy_hdl,SA_AIS_ERR_BAD_HANDLE,"bad handle"},

    [SRMSV_FIN_BAD_HANDLE2_T] = {NULL,SA_AIS_ERR_BAD_HANDLE," NULL handle"},

};

int tet_test_srmsvFinalize(int i,CONFIG_FLAG flg)
{
   SaAisErrorT rc;
   NCS_SRMSV_HANDLE cl_hdl;

   if (API_SRMSV_Finalize[i].client_hdl == NULL)
        cl_hdl = 0;
   else
        cl_hdl = *(API_SRMSV_Finalize[i].client_hdl);

   rc = ncs_srmsv_finalize(cl_hdl);
   return(srmsv_test_result(rc,API_SRMSV_Finalize[i].exp_output,API_SRMSV_Finalize[i].result_string,flg));
}


/******************* STOP MON ********************/


struct ncsSrmsvStopMon API_SRMSV_Stop_Mon[] = {

    [SRMSV_STOP_MON_BAD_HANDLE_T]  = {&tcd.dummy_hdl,SA_AIS_ERR_BAD_HANDLE,"Stop monitor bad handle"},

    [SRMSV_STOP_MON_BAD_HANDLE2_T] = {NULL,SA_AIS_ERR_BAD_HANDLE,"Stop monitor bad handle"},

    [SRMSV_STOP_MON_SUCCESS_T]     = {&tcd.client1,SA_AIS_OK,"Stop monitor for client"},

    [SRMSV_STOP_MON_BAD_OPERATION_T] = {&tcd.client1,SA_AIS_ERR_BAD_OPERATION,"Stop monitor bad operation"},

};

int tet_test_srmsvStopmon(int i,CONFIG_FLAG flg)
{
   SaAisErrorT rc;
   NCS_SRMSV_HANDLE cl_hdl;

   if (API_SRMSV_Stop_Mon[i].client_hdl == NULL)
      cl_hdl = 0;
   else
      cl_hdl = *(API_SRMSV_Stop_Mon[i].client_hdl);

   rc = ncs_srmsv_stop_monitor(cl_hdl);
   return(srmsv_test_result(rc,API_SRMSV_Stop_Mon[i].exp_output,API_SRMSV_Stop_Mon[i].result_string,flg));

}


/******************* START MON ********************/


struct ncsSrmsvRestartMon API_SRMSV_Restart_Mon[] = {

    [SRMSV_RESTART_MON_BAD_HANDLE_T]    = {&tcd.dummy_hdl,SA_AIS_ERR_BAD_HANDLE,"Restart monitor bad handle"},

    [SRMSV_RESTART_MON_BAD_HANDLE2_T]   = {NULL,SA_AIS_ERR_BAD_HANDLE,"Restart monitor bad handle"},

    [SRMSV_RESTART_MON_BAD_OPERATION_T] = {&tcd.client1,SA_AIS_ERR_BAD_OPERATION,"Restart monitor bad operation"},

    [SRMSV_RESTART_MON_SUCCESS_T]       = {&tcd.client1,SA_AIS_OK,"Restart monitor after stop monitor"},

    [SRMSV_RESTART_MON_FIN_HDL_T]       = {&tcd.client1,SA_AIS_ERR_BAD_HANDLE,"Restart monitor with finalized hdl"},

};

int tet_test_srmsvRestartmon(int i,CONFIG_FLAG flg)
{
   SaAisErrorT rc;
   NCS_SRMSV_HANDLE cl_hdl;

   if (API_SRMSV_Restart_Mon[i].client_hdl == NULL)
      cl_hdl = 0;
   else
      cl_hdl = *(API_SRMSV_Restart_Mon[i].client_hdl);

   rc = ncs_srmsv_restart_monitor(cl_hdl);
   return(srmsv_test_result(rc, API_SRMSV_Restart_Mon[i].exp_output,API_SRMSV_Restart_Mon[i].result_string,flg));
}


/******************* RSRC MON *********************/

struct ncsSrmsvStartrsrcmon API_SRMSV_Start_Rsrc_Mon[] = {

    [SRMSV_RSRC_MON_BAD_HANDLE_T]    = {&tcd.dummy_hdl,&tcd.local_monitor_cpu_util,&tcd.dummy_rsrc_hdl,
                                        SA_AIS_ERR_BAD_HANDLE,"Uninit hdl"},

    [SRMSV_RSRC_MON_BAD_HANDLE2_T]   = {NULL,&tcd.local_monitor_cpu_util,&tcd.dummy_rsrc_hdl,
                                        SA_AIS_ERR_BAD_HANDLE,"NULL hdl"},

    [SRMSV_RSRC_MON_BAD_HANDLE3_T]   = {&tcd.client1,&tcd.local_monitor_cpu_util,&tcd.dummy_rsrc_hdl,
                                        SA_AIS_ERR_BAD_HANDLE,"Start monitor Finalized hdl"},

    [SRMSV_RSRC_MON_INVALID_PARAM_T] = {&tcd.client4,&tcd.local_monitor_cpu_util,NULL,
                                        SA_AIS_ERR_BAD_HANDLE,"NULL hdl"},

    [SRMSV_RSRC_MON_INVALID_PARAM2_T]= {&tcd.client1,&tcd.invalid_monitor,&tcd.dummy_rsrc_hdl,
                                        SA_AIS_ERR_INVALID_PARAM,"Subscri from start rsrc"},

    [SRMSV_RSRC_MON_PID_ZERO_T]      = {&tcd.client1,&tcd.invalid_monitor2,&tcd.dummy_rsrc_hdl,
                                        SA_AIS_ERR_INVALID_PARAM,"pid zero"},

    [SRMSV_RSRC_MON_CPU_UTIL_T]      = {&tcd.client1,&tcd.local_monitor_cpu_util,&tcd.local_cpu_util_rsrc_hdl,
                                        SA_AIS_OK,"Mon for cpuutil client"},

    [SRMSV_RSRC_MON_CPU_KERNEL_T]    = {&tcd.client1,&tcd.local_monitor_kernel_util,&tcd.local_kernel_util_rsrc_hdl,
                                        SA_AIS_OK,"Monitor for kernel util"},

    [SRMSV_RSRC_MON_CPU_USER_T]      = {&tcd.client1,&tcd.local_monitor_user_util,&tcd.local_user_util_rsrc_hdl,
                                        SA_AIS_OK,"Monitor for cpu user util"},
   
    [SRMSV_RSRC_MON_UTIL_ONE_T]      = {&tcd.client1,&tcd.local_monitor_user_util_one,
                                        &tcd.local_user_util_one_rsrc_hdl,SA_AIS_OK,
                                        "Monitor for cpu user util one"},

    [SRMSV_RSRC_MON_UTIL_FIVE_T]     = {&tcd.client1,&tcd.local_monitor_user_util_five,
                                        &tcd.local_user_util_five_rsrc_hdl,SA_AIS_OK,
                                        "Monitor for cpu user util five"},

    [SRMSV_RSRC_MON_UTIL_FIFTEEN_T]  = {&tcd.client1,&tcd.local_monitor_user_util_fifteen,
                                        &tcd.local_user_util_fifteen_rsrc_hdl,SA_AIS_OK,
                                        "Monitor for cpu user util fifteen"},

    [SRMSV_RSRC_MON_MEM_USED_T]      = {&tcd.client1,&tcd.local_monitor_mem_used,&tcd.local_mem_used_rsrc_hdl,
                                        SA_AIS_OK,"Monitor for mem used"},

    [SRMSV_RSRC_MON_MEM_USED2_T]     = {&tcd.client1,&tcd.local_monitor_mem_used2,&tcd.local_mem_used_rsrc_hdl,
                                        SA_AIS_OK,"Monitor after changing req to subscr also"},

    [SRMSV_RSRC_MON_MEM_FREE_T]      = {&tcd.client1,&tcd.local_monitor_mem_free,&tcd.local_mem_free_rsrc_hdl,
                                        SA_AIS_OK, "Monitor for mem free"},

    [SRMSV_RSRC_MON_SWAP_USED_T]     = {&tcd.client1,&tcd.local_monitor_swap_used,&tcd.local_swap_used_rsrc_hdl,
                                        SA_AIS_OK,"Monitor for swap used"},

    [SRMSV_RSRC_MON_SWAP_FREE_T]     = {&tcd.client1,&tcd.local_monitor_swap_free,&tcd.local_swap_free_rsrc_hdl,
                                        SA_AIS_OK,"Monitor for swap free"},

    [SRMSV_RSRC_MON_USED_BUF_MEM_T]  = {&tcd.client1,&tcd.local_monitor_used_buf_mem,&tcd.local_used_buf_mem_rsrc_hdl,
                                        SA_AIS_OK,"Monitor for used buf mem"},

    [SRMSV_RSRC_MON_PROC_EXIT_T]     = {&tcd.client1,&tcd.local_monitor_proc_exit,&tcd.local_proc_exit_rsrc_hdl,
                                        SA_AIS_OK,"Monitor for proc exit"},

    [SRMSV_RSRC_MON_PROC_CHILD_EXIT_T]= {&tcd.client1,&tcd.local_monitor_proc_child_exit,
                                         &tcd.local_proc_child_exit_rsrc_hdl,SA_AIS_OK,
                                         "Monitor for proc child exit"},

    [SRMSV_RSRC_MON_PROC_GRANDCHILD_EXIT_T]= {&tcd.client1,&tcd.local_monitor_proc_grandchild_exit,
                                              &tcd.local_proc_grandchild_exit_rsrc_hdl,SA_AIS_OK,
                                              "Monitor for proc grand child exit"},

    [SRMSV_RSRC_MON_PROC_DESC_EXIT_T]  = {&tcd.client1,&tcd.local_monitor_proc_desc_exit,
                                          &tcd.local_proc_desc_exit_rsrc_hdl,SA_AIS_OK,
                                          "Monitor for proc desc exit"},

    [SRMSV_RSRC_REMOTE_MON_CPU_UTIL_T] = {&tcd.client3,&tcd.remote_monitor_cpu_util,&tcd.remote_cpu_util_rsrc_hdl,
                                          SA_AIS_OK,"Mon cpuutil on remote node"},

    [SRMSV_RSRC_REMOTE_MON_CPU_KERNEL_T]   = {&tcd.client3,&tcd.remote_monitor_kernel_util,
                                              &tcd.remote_kernel_util_rsrc_hdl,SA_AIS_OK,
                                              "Monitor kernel util on remote node"},

    [SRMSV_RSRC_REMOTE_MON_CPU_USER_T] = {&tcd.client3,&tcd.remote_monitor_user_util,&tcd.remote_user_util_rsrc_hdl,
                                          SA_AIS_OK,"Monitor cpu user util on remote node"},

    [SRMSV_RSRC_REMOTE_MON_UTIL_ONE_T]     = {&tcd.client3,&tcd.remote_monitor_user_util_one,
                                              &tcd.remote_user_util_one_rsrc_hdl,SA_AIS_OK,
                                              "Monitor cpu user util on remote node"},

    [SRMSV_RSRC_REMOTE_MON_UTIL_FIVE_T]    = {&tcd.client3,&tcd.remote_monitor_user_util_five,
                                              &tcd.remote_user_util_five_rsrc_hdl,SA_AIS_OK,
                                              "Monitor cpu user util five on remote node"},

    [SRMSV_RSRC_REMOTE_MON_UTIL_FIFTEEN_T] = {&tcd.client3,&tcd.remote_monitor_user_util_fifteen,
                                              &tcd.remote_user_util_fifteen_rsrc_hdl,SA_AIS_OK,
                                              "Monitor cpu user util fifteen on remote node"},

    [SRMSV_RSRC_REMOTE_MON_MEM_USED_T]  = {&tcd.client3,&tcd.remote_monitor_mem_used,&tcd.remote_mem_used_rsrc_hdl,
                                           SA_AIS_OK,"Monitor mem used on remote node"},

    [SRMSV_RSRC_REMOTE_MON_MEM_FREE_T]     = {&tcd.client3,&tcd.remote_monitor_mem_free,&tcd.remote_mem_free_rsrc_hdl,
                                              SA_AIS_OK, "Monitor mem free on remote node"},

    [SRMSV_RSRC_REMOTE_MON_SWAP_USED_T]    = {&tcd.client3,&tcd.remote_monitor_swap_used,
                                              &tcd.remote_swap_used_rsrc_hdl,SA_AIS_OK,
                                              "Monitor swap used remote node"},

    [SRMSV_RSRC_REMOTE_MON_SWAP_FREE_T]    = {&tcd.client3,&tcd.remote_monitor_swap_free,
                                              &tcd.remote_swap_free_rsrc_hdl,SA_AIS_OK,
                                              "Monitor swap free remote node"},

    [SRMSV_RSRC_REMOTE_MON_USED_BUF_MEM_T] = {&tcd.client3,&tcd.remote_monitor_used_buf_mem,
                                              &tcd.remote_used_buf_mem_rsrc_hdl,SA_AIS_OK,
                                              "Monitor used buf mem remote node"},

    [SRMSV_RSRC_ALL_MON_CPU_UTIL_T]     = {&tcd.client3,&tcd.all_monitor_cpu_util,&tcd.all_cpu_util_rsrc_hdl,
                                           SA_AIS_OK,"Mon cpuutil on all node"},

    [SRMSV_RSRC_ALL_MON_CPU_KERNEL_T]   = {&tcd.client3,&tcd.all_monitor_kernel_util,&tcd.all_kernel_util_rsrc_hdl,
                                           SA_AIS_OK,"Monitor kernel util on all node"},

    [SRMSV_RSRC_ALL_MON_CPU_USER_T]     = {&tcd.client3,&tcd.all_monitor_user_util,&tcd.all_user_util_rsrc_hdl,
                                           SA_AIS_OK,"Monitor cpu user util on all node"},

    [SRMSV_RSRC_ALL_MON_UTIL_ONE_T]     = {&tcd.client3,&tcd.all_monitor_user_util_one,
                                           &tcd.all_user_util_one_rsrc_hdl,SA_AIS_OK,
                                           "Monitor cpu user util on all node"},

    [SRMSV_RSRC_ALL_MON_UTIL_FIVE_T]    = {&tcd.client3,&tcd.all_monitor_user_util_five,
                                           &tcd.all_user_util_five_rsrc_hdl,SA_AIS_OK,
                                           "Monitor cpu user util five on all node"},

    [SRMSV_RSRC_ALL_MON_UTIL_FIFTEEN_T] = {&tcd.client3,&tcd.all_monitor_user_util_fifteen,
                                           &tcd.all_user_util_fifteen_rsrc_hdl,SA_AIS_OK,
                                           "Monitor cpu user util fifteen on all node"},

    [SRMSV_RSRC_ALL_MON_MEM_USED_T]     = {&tcd.client3,&tcd.all_monitor_mem_used,&tcd.all_mem_used_rsrc_hdl,
                                           SA_AIS_OK,"Monitor mem used on all node"},

    [SRMSV_RSRC_ALL_MON_MEM_FREE_T]     = {&tcd.client3,&tcd.all_monitor_mem_free,&tcd.all_mem_free_rsrc_hdl,
                                           SA_AIS_OK,"Monitor mem free on all node"},

    [SRMSV_RSRC_ALL_MON_SWAP_USED_T]    = {&tcd.client3,&tcd.all_monitor_swap_used,&tcd.all_swap_used_rsrc_hdl,
                                           SA_AIS_OK,"Monitor swap used all node"},

    [SRMSV_RSRC_ALL_MON_SWAP_FREE_T]    = {&tcd.client3,&tcd.all_monitor_swap_free,&tcd.all_swap_free_rsrc_hdl,
                                           SA_AIS_OK,"Monitor swap free all node"},

    [SRMSV_RSRC_ALL_MON_USED_BUF_MEM_T] = {&tcd.client3,&tcd.all_monitor_used_buf_mem,
                                           &tcd.all_used_buf_mem_rsrc_hdl,SA_AIS_OK,
                                           "Monitor used buf mem all node"},

    [SRMSV_RSRC_WMK_MON_CPU_UTIL_T]     = {&tcd.client3,&tcd.wmk_monitor_cpu_util,&tcd.wmk_cpu_util_rsrc_hdl,
                                           SA_AIS_OK,"Mon cpuutil on wmk node"},

    [SRMSV_RSRC_WMK_MON_CPU_KERNEL_T]   = {&tcd.client3,&tcd.wmk_monitor_kernel_util,&tcd.wmk_kernel_util_rsrc_hdl,
                                           SA_AIS_OK,"Monitor kernel util on wmk node"},

    [SRMSV_RSRC_WMK_MON_CPU_USER_T]     = {&tcd.client3,&tcd.wmk_monitor_user_util,&tcd.wmk_user_util_rsrc_hdl,
                                           SA_AIS_OK,"Monitor cpu user util on wmk node"},

    [SRMSV_RSRC_WMK_MON_UTIL_ONE_T]     = {&tcd.client3,&tcd.wmk_monitor_user_util_one,
                                           &tcd.wmk_user_util_one_rsrc_hdl,SA_AIS_OK,
                                           "Monitor cpu user util on wmk node"},

    [SRMSV_RSRC_WMK_MON_UTIL_FIVE_T]    = {&tcd.client3,&tcd.wmk_monitor_user_util_five,
                                           &tcd.wmk_user_util_five_rsrc_hdl,SA_AIS_OK,
                                           "Monitor cpu user util five on wmk node"},

    [SRMSV_RSRC_WMK_MON_UTIL_FIFTEEN_T] = {&tcd.client3,&tcd.wmk_monitor_user_util_fifteen,
                                           &tcd.wmk_user_util_fifteen_rsrc_hdl,SA_AIS_OK,
                                           "Monitor cpu user util fifteen on wmk node"},

    [SRMSV_RSRC_WMK_MON_MEM_USED_T]     = {&tcd.client3,&tcd.wmk_monitor_mem_used,&tcd.wmk_mem_used_rsrc_hdl,
                                           SA_AIS_OK,"Monitor mem used on wmk node"},

    [SRMSV_RSRC_WMK_MON_MEM_FREE_T]     = {&tcd.client3,&tcd.wmk_monitor_mem_free,&tcd.wmk_mem_free_rsrc_hdl,
                                           SA_AIS_OK,"Monitor mem free on wmk node"},

    [SRMSV_RSRC_WMK_MON_SWAP_USED_T]    = {&tcd.client3,&tcd.wmk_monitor_swap_used,&tcd.wmk_swap_used_rsrc_hdl,
                                           SA_AIS_OK,"Monitor swap used wmk node"},

    [SRMSV_RSRC_WMK_MON_SWAP_FREE_T]    = {&tcd.client3,&tcd.wmk_monitor_swap_free,&tcd.wmk_swap_free_rsrc_hdl,
                                           SA_AIS_OK,"Monitor swap free wmk node"},

    [SRMSV_RSRC_WMK_MON_USED_BUF_MEM_T] = {&tcd.client3,&tcd.wmk_monitor_used_buf_mem,
                                           &tcd.wmk_used_buf_mem_rsrc_hdl,SA_AIS_OK,
                                           "Monitor used buf mem wmk node"},

    [SRMSV_RSRC_WMK_REMOTE_CPU_UTIL_T]  = {&tcd.client3,&tcd.wmk_remote_cpu_util,&tcd.wmk_remote_cpu_util_rsrc_hdl,
                                           SA_AIS_OK,"Mon cpuutil wmk_remote"},

    [SRMSV_RSRC_WMK_REMOTE_CPU_KERNEL_T] = {&tcd.client3,&tcd.wmk_remote_kernel_util,
                                            &tcd.wmk_remote_kernel_util_rsrc_hdl,SA_AIS_OK,
                                            "Monitor kernel util on wmk_remote node"},

    [SRMSV_RSRC_WMK_REMOTE_CPU_USER_T]  = {&tcd.client3,&tcd.wmk_remote_user_util,
                                           &tcd.wmk_remote_user_util_rsrc_hdl,SA_AIS_OK,
                                           "Monitor cpu user util on wmk_remote node"},

    [SRMSV_RSRC_WMK_REMOTE_UTIL_ONE_T]  = {&tcd.client3,&tcd.wmk_remote_user_util_one,
                                           &tcd.wmk_remote_user_util_one_rsrc_hdl,SA_AIS_OK,
                                           "Monitor cpu user util on wmk_remote node"},

    [SRMSV_RSRC_WMK_REMOTE_UTIL_FIVE_T] = {&tcd.client3,&tcd.wmk_remote_user_util_five,
                                           &tcd.wmk_remote_user_util_five_rsrc_hdl,SA_AIS_OK,
                                           "Monitor cpu user util five on wmk_remote node"},

    [SRMSV_RSRC_WMK_REMOTE_UTIL_FIFTEEN_T] = {&tcd.client3,&tcd.wmk_remote_user_util_fifteen,
                                              &tcd.wmk_remote_user_util_fifteen_rsrc_hdl,SA_AIS_OK,
                                              "Monitor cpu user util fifteen on wmk_remote node"},

    [SRMSV_RSRC_WMK_REMOTE_MEM_USED_T]  = {&tcd.client3,&tcd.wmk_remote_mem_used,&tcd.wmk_remote_mem_used_rsrc_hdl,
                                           SA_AIS_OK,"Monitor mem used on wmk_remote node"},

    [SRMSV_RSRC_WMK_REMOTE_MEM_FREE_T]  = {&tcd.client3,&tcd.wmk_remote_mem_free,&tcd.wmk_remote_mem_free_rsrc_hdl,
                                           SA_AIS_OK,"Monitor mem free on wmk_remote node"},

    [SRMSV_RSRC_WMK_REMOTE_SWAP_USED_T] = {&tcd.client3,&tcd.wmk_remote_swap_used,
                                           &tcd.wmk_remote_swap_used_rsrc_hdl,SA_AIS_OK,
                                           "Monitor swap used wmk_remote node"},

    [SRMSV_RSRC_WMK_REMOTE_SWAP_FREE_T] = {&tcd.client3,&tcd.wmk_remote_swap_free,
                                           &tcd.wmk_remote_swap_free_rsrc_hdl,SA_AIS_OK,
                                           "Monitor swap free wmk_remote node"},

    [SRMSV_RSRC_WMK_REMOTE_USED_BUF_MEM_T] = {&tcd.client3,&tcd.wmk_remote_used_buf_mem,
                                              &tcd.wmk_remote_used_buf_mem_rsrc_hdl,SA_AIS_OK,
                                              "Monitor used buf mem wmk_remote node"},

    [SRMSV_RSRC_WMK_ALL_CPU_UTIL_T]  = {&tcd.client3,&tcd.wmk_all_cpu_util,&tcd.wmk_all_cpu_util_rsrc_hdl,
                                        SA_AIS_OK,"Mon cpuutil wmk_all"},

    [SRMSV_RSRC_WMK_ALL_CPU_KERNEL_T]   = {&tcd.client3,&tcd.wmk_all_kernel_util,&tcd.wmk_all_kernel_util_rsrc_hdl,
                                           SA_AIS_OK,"Monitor kernel util on wmk_all node"},

    [SRMSV_RSRC_WMK_ALL_CPU_USER_T]     = {&tcd.client3,&tcd.wmk_all_user_util,&tcd.wmk_all_user_util_rsrc_hdl,
                                           SA_AIS_OK,"Monitor cpu user util on wmk_all node"},

    [SRMSV_RSRC_WMK_ALL_UTIL_ONE_T]     = {&tcd.client3,&tcd.wmk_all_user_util_one,
                                           &tcd.wmk_all_user_util_one_rsrc_hdl,SA_AIS_OK,
                                           "Monitor cpu user util on wmk_all node"},

    [SRMSV_RSRC_WMK_ALL_UTIL_FIVE_T]    = {&tcd.client3,&tcd.wmk_all_user_util_five,
                                           &tcd.wmk_all_user_util_five_rsrc_hdl,SA_AIS_OK,
                                           "Monitor cpu user util five on wmk_all node"},

    [SRMSV_RSRC_WMK_ALL_UTIL_FIFTEEN_T] = {&tcd.client3,&tcd.wmk_all_user_util_fifteen,
                                           &tcd.wmk_all_user_util_fifteen_rsrc_hdl,SA_AIS_OK,
                                           "Monitor cpu user util fifteen on wmk_all node"},

    [SRMSV_RSRC_WMK_ALL_MEM_USED_T]     = {&tcd.client3,&tcd.wmk_all_mem_used,&tcd.wmk_all_mem_used_rsrc_hdl,
                                           SA_AIS_OK,"Monitor mem used on wmk_all node"},

    [SRMSV_RSRC_WMK_ALL_MEM_FREE_T]     = {&tcd.client3,&tcd.wmk_all_mem_free,&tcd.wmk_all_mem_free_rsrc_hdl,
                                           SA_AIS_OK,"Monitor mem free on wmk_all node"},

    [SRMSV_RSRC_WMK_ALL_SWAP_USED_T]    = {&tcd.client3,&tcd.wmk_all_swap_used,&tcd.wmk_all_swap_used_rsrc_hdl,
                                           SA_AIS_OK,"Monitor swap used wmk_all node"},

    [SRMSV_RSRC_WMK_ALL_SWAP_FREE_T]    = {&tcd.client3,&tcd.wmk_all_swap_free,&tcd.wmk_all_swap_free_rsrc_hdl,
                                           SA_AIS_OK,"Monitor swap free wmk_all node"},

    [SRMSV_RSRC_WMK_ALL_USED_BUF_MEM_T] = {&tcd.client3,&tcd.wmk_all_used_buf_mem,
                                           &tcd.wmk_all_used_buf_mem_rsrc_hdl,SA_AIS_OK,
                                           "Monitor used buf mem wmk_all node"},

    [SRMSV_RSRC_ERR_NOT_EXIST_T]   = {&tcd.client3,&tcd.remote_monitor_user_util_one,
                                        &tcd.remote_user_util_one_rsrc_hdl,SA_AIS_ERR_NOT_EXIST,
                                        "ND on remote node doesnot exist"},

    [SRMSV_RSRC_MON_PROC_MEM_T]    = {&tcd.client1,&tcd.local_monitor_threshold_proc_mem,
                                      &tcd.local_proc_mem_thre_rsrc_hdl,SA_AIS_OK,
                                      "Monitor for proc mem "},

    [SRMSV_RSRC_WMK_PROC_MEM_T]    = {&tcd.client1,&tcd.local_monitor_wmk_proc_mem,
                                      &tcd.local_proc_mem_wmk_rsrc_hdl,SA_AIS_OK,
                                      "Monitor for wmk proc mem "},

    [SRMSV_RSRC_MON_PROC_CPU_T]    = {&tcd.client1,&tcd.local_monitor_threshold_proc_cpu,
                                      &tcd.local_proc_cpu_thre_rsrc_hdl,SA_AIS_OK,
                                      "Monitor for proc cpu "},

    [SRMSV_RSRC_WMK_PROC_CPU_T]    = {&tcd.client1,&tcd.local_monitor_wmk_proc_cpu,
                                      &tcd.local_proc_cpu_wmk_rsrc_hdl,SA_AIS_OK,
                                      "Monitor for wmk proc cpu "},

    [SRMSV_RSRC_WMK_ERR_NOT_EXIST_T] = {&tcd.client3,&tcd.wmk_monitor_mem_free,&tcd.wmk_mem_free_rsrc_hdl,
                                           SA_AIS_ERR_NOT_EXIST,"Monitor mem free on wmk node"},

    [SRMSV_RSRC_INVALID_PARAM_T]   = {&tcd.client1,&tcd.local_monitor_mem_used,&tcd.local_mem_used_rsrc_hdl,
                                       SA_AIS_ERR_INVALID_PARAM,"Diff scale type for threshold and tolerable values"},

};


int tet_test_srmsvStartrsrcmon(int i,CONFIG_FLAG flg)
{
   SaAisErrorT rc;
   NCS_SRMSV_HANDLE cl_hdl;

   if (API_SRMSV_Start_Rsrc_Mon[i].client_hdl == NULL)
   {
      cl_hdl = 0;
   }
   else
   {
      cl_hdl = *(API_SRMSV_Start_Rsrc_Mon[i].client_hdl);
   }

   rc = ncs_srmsv_start_rsrc_mon(cl_hdl,API_SRMSV_Start_Rsrc_Mon[i].mon_info,API_SRMSV_Start_Rsrc_Mon[i].rsrc_hdl);
   if(rc == SA_AIS_OK)
   {
      m_TET_SRMSV_PRINTF("\n Mon_Handle    : %u",*(API_SRMSV_Start_Rsrc_Mon[i].rsrc_hdl));
      tet_printf("\n Mon_Handle    : %u",*(API_SRMSV_Start_Rsrc_Mon[i].rsrc_hdl));
   }
   return(srmsv_test_result(rc,API_SRMSV_Start_Rsrc_Mon[i].exp_output,API_SRMSV_Start_Rsrc_Mon[i].result_string,flg));

}


/*************************** SUBSCR START *******************/


struct ncsSrmsvSubscrrsrcmon API_SRMSV_Subscr_Rsrc_Mon[] = {

    [SRMSV_RSRC_SUBSCR_BAD_HANDLE_T]     = {&tcd.dummy_hdl,&tcd.local_subscr_cpu_util,&tcd.dummy_subscr_hdl,
                                            SA_AIS_ERR_BAD_HANDLE,"Uninit hdl"},

    [SRMSV_RSRC_SUBSCR_BAD_HANDLE2_T]    = {NULL,&tcd.local_subscr_cpu_util,&tcd.dummy_subscr_hdl,
                                            SA_AIS_ERR_BAD_HANDLE,"NULL hdl"},

    [SRMSV_RSRC_SUBSCR_BAD_HANDLE3_T]    = {&tcd.client1,&tcd.local_subscr_cpu_util,NULL,SA_AIS_ERR_INVALID_PARAM,
                                            "NULL subscr hdl"},

    [SRMSV_RSRC_SUBSCR_BAD_HANDLE4_T]    = {&tcd.client1,&tcd.local_subscr_cpu_util,&tcd.dummy_subscr_hdl,
                                            SA_AIS_ERR_BAD_HANDLE,"Subscr Finalized hdl"},

    [SRMSV_RSRC_SUBSCR_REQ_T]            = {&tcd.client1,&tcd.invalid_subscr,&tcd.dummy_subscr_hdl,SA_AIS_OK,
                                            "Request for monitor"},
   
    [SRMSV_RSRC_SUBSCR_INVALID_PARAM_T]  = {&tcd.client1,NULL,&tcd.dummy_subscr_hdl,SA_AIS_ERR_INVALID_PARAM,
                                            "NULL subscr info"},

    [SRMSV_RSRC_SUBSCR_CPU_UTIL_T]       = {&tcd.client1,&tcd.local_subscr_cpu_util,&tcd.local_cpu_util_subscr_hdl,
                                            SA_AIS_OK,"Subscribe for cpu_util"},

    [SRMSV_RSRC_SUBSCR_USER_UTIL_T]      = {&tcd.client1,&tcd.local_subscr_user_util,&tcd.local_user_util_subscr_hdl,
                                            SA_AIS_OK,"Subscribe for user_util"},

    [SRMSV_RSRC_SUBSCR_UTIL_ONE_T]       = {&tcd.client1,&tcd.local_subscr_util_one,&tcd.local_util_one_subscr_hdl,
                                            SA_AIS_OK,"Subscribe for util one"},

    [SRMSV_RSRC_SUBSCR_UTIL_FIVE_T]      = {&tcd.client1,&tcd.local_subscr_util_five,&tcd.local_util_five_subscr_hdl,
                                            SA_AIS_OK,"Subscribe for util five"},

    [SRMSV_RSRC_SUBSCR_UTIL_FIFTEEN_T]   = {&tcd.client1,&tcd.local_subscr_util_fifteen,
                                            &tcd.local_util_fifteen_subscr_hdl,SA_AIS_OK,
                                            "Subscribe for util fifteen"},

    [SRMSV_RSRC_SUBSCR_MEM_USED_T]       = {&tcd.client1,&tcd.local_subscr_mem_used,&tcd.local_mem_used_subscr_hdl,
                                            SA_AIS_OK,"Subscribe for mem used "},

    [SRMSV_RSRC_SUBSCR_MEM_USED2_T]      = {&tcd.client3,&tcd.local_subscr_mem_used,&tcd.local_mem_used_subscr_hdl2,
                                            SA_AIS_OK,"Subscribe for mem used from another client"},

    [SRMSV_RSRC_SUBSCR_MEM_USED3_T]      = {&tcd.client2,&tcd.local_subscr_mem_used,&tcd.local_mem_used_subscr_hdl3,
                                            SA_AIS_OK,"Subscribe for mem used from another client"},

    [SRMSV_RSRC_SUBSCR_AFTER_STOP_T]      = {&tcd.client3,&tcd.local_subscr_mem_used,&tcd.local_mem_used_subscr_hdl2,
                                             SA_AIS_OK,"Subscribe for mem used after monitor stop"},

    [SRMSV_RSRC_SUBSCR_SWAP_USED_T]      = {&tcd.client1,&tcd.local_subscr_swap_used,&tcd.local_swap_used_subscr_hdl,
                                            SA_AIS_OK,"Subscribe for swap used"},

    [SRMSV_RSRC_SUBSCR_SWAP_FREE_T]      = {&tcd.client1,&tcd.local_subscr_swap_free,&tcd.local_swap_free_subscr_hdl,
                                            SA_AIS_OK,"Subscribe for swap free"},

    [SRMSV_RSRC_SUBSCR_USED_BUF_MEM_T]   = {&tcd.client1,&tcd.local_subscr_used_buf_mem,
                                            &tcd.local_used_buf_mem_subscr_hdl,SA_AIS_OK,
                                            "Subscribe for used_buf_mem"},

    [SRMSV_RSRC_SUBSCR_PROC_EXIT_T]      = {&tcd.client1,&tcd.local_subscr_proc_exit,
                                            &tcd.local_proc_exit_subscr_hdl,SA_AIS_OK,
                                            "Subscribe for proc_exit"},

    [SRMSV_RSRC_SUBSCR_PROC_CHILD_EXIT_T] = {&tcd.client1,&tcd.local_subscr_proc_child_exit,
                                             &tcd.local_proc_child_exit_subscr_hdl,SA_AIS_OK,
                                             "Subscribe for proc child exit"},

    [SRMSV_RSRC_SUBSCR_PROC_GRANDCHILD_EXIT_T] = {&tcd.client1,&tcd.local_subscr_proc_grandchild_exit,
                                                  &tcd.local_proc_grandchild_exit_subscr_hdl,SA_AIS_OK,
                                                  "Subscribe for proc grand child exit"},

    [SRMSV_RSRC_SUBSCR_PROC_DESC_EXIT_T]   = {&tcd.client1,&tcd.local_subscr_proc_desc_exit,
                                              &tcd.local_proc_desc_exit_subscr_hdl,SA_AIS_OK,
                                              "Subscribe for proc desc exit"},

    [SRMSV_RSRC_SUBSCR_REMOTE_CPU_UTIL_T]  = {&tcd.client3,&tcd.remote_subscr_cpu_util, 
                                              &tcd.remote_cpu_util_subscr_hdl,SA_AIS_OK,
                                              "Subscribe for remote cpu_util"},

    [SRMSV_RSRC_SUBSCR_REMOTE_KERNEL_UTIL_T]  = {&tcd.client3,&tcd.remote_subscr_kernel_util,
                                                 &tcd.remote_kernel_util_subscr_hdl,SA_AIS_OK,
                                                 "Subscribe for remote kernel_util"},

    [SRMSV_RSRC_SUBSCR_REMOTE_USER_UTIL_T]  = {&tcd.client3,&tcd.remote_subscr_user_util,
                                               &tcd.remote_user_util_subscr_hdl,SA_AIS_OK,
                                               "Subscribe for remote user_util"},

    [SRMSV_RSRC_SUBSCR_REMOTE_UTIL_ONE_T]   = {&tcd.client3,&tcd.remote_subscr_util_one,
                                               &tcd.remote_util_one_subscr_hdl,SA_AIS_OK,
                                               "Subscribe for remote util one"},

    [SRMSV_RSRC_SUBSCR_REMOTE_UTIL_FIVE_T]  = {&tcd.client3,&tcd.remote_subscr_util_five,
                                               &tcd.remote_util_five_subscr_hdl,SA_AIS_OK,
                                               "Subscribe for remote util five"},

    [SRMSV_RSRC_SUBSCR_REMOTE_UTIL_FIFTEEN_T]  = {&tcd.client3,&tcd.remote_subscr_util_fifteen,
                                                  &tcd.remote_util_fifteen_subscr_hdl,SA_AIS_OK,
                                                  "Subscribe for remote util fifteen"},

    [SRMSV_RSRC_SUBSCR_REMOTE_MEM_USED_T]  = {&tcd.client3,&tcd.remote_subscr_mem_used,
                                              &tcd.remote_mem_used_subscr_hdl,SA_AIS_OK,
                                              "Subscribe for remote mem used"},

    [SRMSV_RSRC_SUBSCR_REMOTE_MEM_FREE_T]  = {&tcd.client3,&tcd.remote_subscr_mem_free,
                                              &tcd.remote_mem_free_subscr_hdl,SA_AIS_OK,
                                              "Subscribe for remote mem free"},

    [SRMSV_RSRC_SUBSCR_REMOTE_SWAP_USED_T] = {&tcd.client3,&tcd.remote_subscr_swap_used,
                                              &tcd.remote_swap_used_subscr_hdl,SA_AIS_OK,
                                              "Subscribe for remote swap used"},

    [SRMSV_RSRC_SUBSCR_REMOTE_SWAP_FREE_T] = {&tcd.client3,&tcd.remote_subscr_swap_free,
                                              &tcd.remote_swap_free_subscr_hdl,SA_AIS_OK,
                                              "Subscribe for remote swap free"},

    [SRMSV_RSRC_SUBSCR_REMOTE_USED_BUF_MEM_T] = {&tcd.client3,&tcd.remote_subscr_used_buf_mem,
                                                 &tcd.remote_used_buf_mem_subscr_hdl,SA_AIS_OK,
                                                 "Subscribe for remote used_buf_mem"},

    [SRMSV_RSRC_SUBSCR_ALL_CPU_UTIL_T]  = {&tcd.client3,&tcd.all_subscr_cpu_util,&tcd.all_cpu_util_subscr_hdl,
                                           SA_AIS_OK,"Subscribe for all cpu_util"},

    [SRMSV_RSRC_SUBSCR_ALL_KERNEL_UTIL_T]  = {&tcd.client3,&tcd.all_subscr_kernel_util,
                                              &tcd.all_kernel_util_subscr_hdl,SA_AIS_OK,
                                              "Subscribe for all kernel_util"},

    [SRMSV_RSRC_SUBSCR_ALL_USER_UTIL_T]  = {&tcd.client3,&tcd.all_subscr_user_util,&tcd.all_user_util_subscr_hdl,
                                            SA_AIS_OK,"Subscribe for all user_util"},

    [SRMSV_RSRC_SUBSCR_ALL_UTIL_ONE_T]   = {&tcd.client3,&tcd.all_subscr_util_one,&tcd.all_util_one_subscr_hdl,
                                            SA_AIS_OK,"Subscribe for all util one"},

    [SRMSV_RSRC_SUBSCR_ALL_UTIL_FIVE_T]  = {&tcd.client3,&tcd.all_subscr_util_five,&tcd.all_util_five_subscr_hdl,
                                            SA_AIS_OK,"Subscribe for all util five"},

    [SRMSV_RSRC_SUBSCR_ALL_UTIL_FIFTEEN_T]   = {&tcd.client3,&tcd.all_subscr_util_fifteen,
                                                &tcd.all_util_fifteen_subscr_hdl,SA_AIS_OK,
                                                "Subscribe for all util fifteen"},

    [SRMSV_RSRC_SUBSCR_ALL_MEM_FREE_T]  = {&tcd.client3,&tcd.all_subscr_mem_free,&tcd.all_mem_free_subscr_hdl,
                                           SA_AIS_OK,"Subscribe for all mem free"},

    [SRMSV_RSRC_SUBSCR_ALL_SWAP_USED_T] = {&tcd.client3,&tcd.all_subscr_swap_used,&tcd.all_swap_used_subscr_hdl,
                                           SA_AIS_OK,"Subscribe for all swap used"},

    [SRMSV_RSRC_SUBSCR_ALL_SWAP_FREE_T] = {&tcd.client3,&tcd.all_subscr_swap_free,&tcd.all_swap_free_subscr_hdl,
                                           SA_AIS_OK,"Subscribe for all swap free"},

    [SRMSV_RSRC_SUBSCR_ALL_USED_BUF_MEM_T]   = {&tcd.client3,&tcd.all_subscr_used_buf_mem,
                                                &tcd.all_used_buf_mem_subscr_hdl,SA_AIS_OK,
                                                "Subscribe for all used_buf_mem"},

    [SRMSV_RSRC_SUBSCR_REM_NOT_EXIST_T]  = {&tcd.client3,&tcd.remote_subscr_mem_used,&tcd.remote_mem_used_subscr_hdl,
                                            SA_AIS_ERR_NOT_EXIST,"Subscribe remote node doesnot exist"},
};


int tet_test_srmsvSubscrrsrcmon(int i,CONFIG_FLAG flg)
{
   SaAisErrorT rc;
   NCS_SRMSV_HANDLE cl_hdl;

   if (API_SRMSV_Subscr_Rsrc_Mon[i].client_hdl == NULL)
   {
      cl_hdl = 0;
   }
   else
   {
      cl_hdl = *(API_SRMSV_Subscr_Rsrc_Mon[i].client_hdl);
   }

   rc = ncs_srmsv_subscr_rsrc_mon(cl_hdl,API_SRMSV_Subscr_Rsrc_Mon[i].rsrc_info,API_SRMSV_Subscr_Rsrc_Mon[i].subscr_hdl);
   if (rc == SA_AIS_OK)
      m_TET_SRMSV_PRINTF("\n Subscr_hdl    : %u",*(API_SRMSV_Subscr_Rsrc_Mon[i].subscr_hdl));
   return(srmsv_test_result(rc,API_SRMSV_Subscr_Rsrc_Mon[i].exp_output,API_SRMSV_Subscr_Rsrc_Mon[i].result_string,flg));

}



/*************************** STOP_RSRC_MON *******************/


struct ncsSrmsvStoprsrcmon API_SRMSV_Stop_Rsrc_Mon[] = {

    [SRMSV_RSRC_STOP_BAD_HANDLE_T] = {&tcd.rsrc_bad_hdl,SA_AIS_ERR_BAD_HANDLE,"Bad handle rsrc_stop"},

    [SRMSV_RSRC_STOP_CPU_UTIL_T]= {&tcd.local_cpu_util_rsrc_hdl,SA_AIS_OK,"CPU UTIL mon stopped"},

    [SRMSV_RSRC_STOP_KERNEL_UTIL_T]= {&tcd.local_kernel_util_rsrc_hdl,SA_AIS_OK,"KERNEL UTIL mon stopped"},

    [SRMSV_RSRC_STOP_CPU_USER_T]= {&tcd.local_user_util_rsrc_hdl,SA_AIS_OK,"USER UTIL mon stopped"},

    [SRMSV_RSRC_STOP_UTIL_ONE_T]= {&tcd.local_user_util_one_rsrc_hdl,SA_AIS_OK,"UTIL ONE mon stopped"},

    [SRMSV_RSRC_STOP_UTIL_FIVE_T]= {&tcd.local_user_util_five_rsrc_hdl,SA_AIS_OK,"UTIL FIVE mon stopped"},

    [SRMSV_RSRC_STOP_UTIL_FIFTEEN_T]= {&tcd.local_user_util_fifteen_rsrc_hdl,SA_AIS_OK,"UTIL FIFTEEN mon stopped"},

    [SRMSV_RSRC_STOP_MEM_USED_T]= {&tcd.local_mem_used_rsrc_hdl,SA_AIS_OK,"MEM USED mon stopped"},

    [SRMSV_RSRC_STOP_MEM_FREE_T]= {&tcd.local_mem_free_rsrc_hdl,SA_AIS_OK,"MEM FREE mon stopped"},

    [SRMSV_RSRC_STOP_SWAP_USED_T]= {&tcd.local_swap_used_rsrc_hdl,SA_AIS_OK,"SWAP USED mon stopped"},

    [SRMSV_RSRC_STOP_SWAP_FREE_T]= {&tcd.local_swap_free_rsrc_hdl,SA_AIS_OK,"SWAP FREE mon stopped"},

    [SRMSV_RSRC_STOP_USED_BUF_MEM_T]= {&tcd.local_used_buf_mem_rsrc_hdl,SA_AIS_OK,"USED BUF MEM mon stopped"},

    [SRMSV_RSRC_STOP_PROC_EXIT_T]= {&tcd.local_proc_exit_rsrc_hdl,SA_AIS_OK,"PROC EXIT mon stopped"},

    [SRMSV_RSRC_STOP_CHILD_EXIT_T]= {&tcd.local_proc_child_exit_rsrc_hdl,SA_AIS_OK,"CHILD EXIT mon stopped"},

    [SRMSV_RSRC_STOP_GRANDCHILD_EXIT_T]= {&tcd.local_proc_grandchild_exit_rsrc_hdl,SA_AIS_OK,"GRANDCHILD EXIT mon stopped"},

    [SRMSV_RSRC_STOP_DESC_EXIT_T]= {&tcd.local_proc_desc_exit_rsrc_hdl,SA_AIS_OK,"DESC EXIT mon stopped"},

    [SRMSV_RSRC_STOP_WITHOUT_START_T]= {&tcd.local_user_util_rsrc_hdl,SA_AIS_ERR_BAD_HANDLE,
                                                                 "stop rsrcmon without start rsrcmon"},

};


int tet_test_srmsvStoprsrcmon(int i,CONFIG_FLAG flg)
{
   SaAisErrorT rc;

   rc = ncs_srmsv_stop_rsrc_mon(*(API_SRMSV_Stop_Rsrc_Mon[i].rsrc_hdl));
   return(srmsv_test_result(rc,API_SRMSV_Stop_Rsrc_Mon[i].exp_output,API_SRMSV_Stop_Rsrc_Mon[i].result_string,flg));
}



/*************************** UNSUBSCR MON *******************/



struct ncsSrmsvUnsubscrmon API_SRMSV_Unsubscr_Rsrc_Mon[] = {

    [SRMSV_RSRC_UNSUBSCR_INVALID_PARAM_T] = {&tcd.bad_subscr_hdl,SA_AIS_ERR_BAD_HANDLE,"Uninit hdl"},

    [SRMSV_RSRC_UNSUBSCR_INVALID_PARAM2_T]= {NULL,SA_AIS_ERR_BAD_HANDLE,"NULL hdl"},

    [SRMSV_RSRC_UNSUBSCR_CPU_UTIL_T] = {&tcd.local_cpu_util_subscr_hdl,SA_AIS_OK,"Unsubscribe for cpu_util"},

    [SRMSV_RSRC_UNSUBSCR_KERNEL_UTIL_T] = {&tcd.local_kernel_util_subscr_hdl,SA_AIS_OK,"Unsubscribe for kernel_util"},

    [SRMSV_RSRC_UNSUBSCR_USER_UTIL_T] = {&tcd.local_user_util_subscr_hdl,SA_AIS_OK,"Unsubscribe for user_util"},

    [SRMSV_RSRC_UNSUBSCR_UTIL_ONE_T] = {&tcd.local_util_one_subscr_hdl,SA_AIS_OK,"Unsubscribe for util one"},

    [SRMSV_RSRC_UNSUBSCR_UTIL_FIVE_T]= {&tcd.local_util_five_subscr_hdl,SA_AIS_OK,"Unsubscribe for util five"},

    [SRMSV_RSRC_UNSUBSCR_UTIL_FIFTEEN_T]= {&tcd.local_util_fifteen_subscr_hdl,SA_AIS_OK,
                                           "Unsubscribe for util fifteen client1"},

    [SRMSV_RSRC_UNSUBSCR_MEM_USED_T]= {&tcd.local_mem_used_subscr_hdl,SA_AIS_OK,"Unsubscribe for mem used"},

    [SRMSV_RSRC_UNSUBSCR_MEM_USED2_T]= {&tcd.local_mem_used_subscr_hdl2,SA_AIS_OK,"Unsubscribe for mem used"},

    [SRMSV_RSRC_UNSUBSCR_MEM_FREE_T]= {&tcd.local_mem_free_subscr_hdl,SA_AIS_OK,"Unsubscribe for mem free"},

    [SRMSV_RSRC_UNSUBSCR_SWAP_USED_T]= {&tcd.local_swap_used_subscr_hdl,SA_AIS_OK,"Unsubscribe for swap used"},

    [SRMSV_RSRC_UNSUBSCR_SWAP_FREE_T]= {&tcd.local_swap_free_subscr_hdl,SA_AIS_OK,"Unsubscribe for swap free"},

    [SRMSV_RSRC_UNSUBSCR_USED_BUF_MEM_T]= {&tcd.local_used_buf_mem_subscr_hdl,SA_AIS_OK,"Unsubscribe for used_buf_mem"},

    [SRMSV_RSRC_UNSUBSCR_PROC_EXIT_T] = {&tcd.local_proc_exit_subscr_hdl,SA_AIS_ERR_BAD_HANDLE,"Unsubscribe for proc_exit"},

    [SRMSV_RSRC_UNSUBSCR_PROC_CHILD_EXIT_T]= {&tcd.local_proc_child_exit_subscr_hdl,SA_AIS_ERR_BAD_HANDLE,
                                              "Unsubscribe for proc child exit"},

    [SRMSV_RSRC_UNSUBSCR_PROC_GRANDCHILD_EXIT_T]= {&tcd.local_proc_grandchild_exit_subscr_hdl,SA_AIS_ERR_BAD_HANDLE,
                                                   "Unsubscribe for proc grand child exit"},

    [SRMSV_RSRC_UNSUBSCR_PROC_DESC_EXIT_T] = {&tcd.local_proc_desc_exit_subscr_hdl,SA_AIS_ERR_BAD_HANDLE,
                                              "Unsubscribe for proc desc exit"},

    [SRMSV_RSRC_UNSUBSCR_AFTER_UNSUBSCRIBE_T]= {&tcd.local_mem_used_subscr_hdl,SA_AIS_ERR_BAD_HANDLE,
                                                 "Unsubscribe hdl which was unsubscribed"},

};


int tet_test_srmsvUnsubscrmon(int i,CONFIG_FLAG flg)
{
   SaAisErrorT rc;
   NCS_SRMSV_SUBSCR_HANDLE scr_hdl;

   if (API_SRMSV_Unsubscr_Rsrc_Mon[i].subscr_hdl == NULL)
   {
      scr_hdl = 0;
   }
   else
   {
      scr_hdl = *(API_SRMSV_Unsubscr_Rsrc_Mon[i].subscr_hdl);
   }

   rc = ncs_srmsv_unsubscr_rsrc_mon(scr_hdl);
   return(srmsv_test_result(rc,API_SRMSV_Unsubscr_Rsrc_Mon[i].exp_output,API_SRMSV_Unsubscr_Rsrc_Mon[i].result_string,flg));
}


struct ncsSrmsvGetWmkVal API_SRMSV_Get_Wmk_Val[] = {

    [SRMSV_RSRC_GET_WMK_VAL_CPU_UTIL_T]     = {&tcd.client3,&tcd.wmk_getval_cpu_util,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val cpu util dual"},

    [SRMSV_RSRC_GET_WMK_VAL_KERNEL_UTIL_T]  = {&tcd.client3,&tcd.wmk_getval_kernel_util,NCS_SRMSV_WATERMARK_LOW,
                                                     SA_AIS_OK,"Get wmk val kernel util low"},

    [SRMSV_RSRC_GET_WMK_VAL_USER_UTIL_T]    = {&tcd.client3,&tcd.wmk_getval_user_util,NCS_SRMSV_WATERMARK_HIGH,
                                                     SA_AIS_OK,"Get wmk val user util high"},

    [SRMSV_RSRC_GET_WMK_VAL_UTIL_ONE_T]     = {&tcd.client3,&tcd.wmk_getval_util_one,NCS_SRMSV_WATERMARK_LOW,
                                                     SA_AIS_OK,"Get wmk val util one low"},

    [SRMSV_RSRC_GET_WMK_VAL_UTIL_FIVE_T]    = {&tcd.client3,&tcd.wmk_getval_util_five,NCS_SRMSV_WATERMARK_HIGH,
                                                     SA_AIS_OK,"Get wmk val util five high"},

    [SRMSV_RSRC_GET_WMK_VAL_UTIL_FIFTEEN_T] = {&tcd.client3,&tcd.wmk_getval_util_fifteen,NCS_SRMSV_WATERMARK_LOW,
                                                     SA_AIS_OK,"Get wmk val util fifteen low"},

    [SRMSV_RSRC_GET_WMK_VAL_MEM_USED_T]     = {&tcd.client3,&tcd.wmk_getval_mem_used,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val mem used dual"},

    [SRMSV_RSRC_GET_WMK_VAL_MEM_FREE_T]     = {&tcd.client3,&tcd.wmk_getval_mem_free,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val mem free dual"},

    [SRMSV_RSRC_GET_WMK_VAL_SWAP_USED_T]    = {&tcd.client3,&tcd.wmk_getval_swap_used,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val swap used dual"},

    [SRMSV_RSRC_GET_WMK_VAL_SWAP_FREE_T]    = {&tcd.client3,&tcd.wmk_getval_swap_free,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val swap free dual"},

    [SRMSV_RSRC_GET_WMK_VAL_USED_BUF_MEM_T] = {&tcd.client3,&tcd.wmk_getval_used_buf_mem,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val buff used dual"},

    [SRMSV_RSRC_GET_WMK_REM_CPU_UTIL_T]     = {&tcd.client3,&tcd.wmk_rem_getval_cpu_util,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val remote cpu util dual"},

    [SRMSV_RSRC_GET_WMK_REM_KERNEL_UTIL_T]  = {&tcd.client3,&tcd.wmk_rem_getval_kernel_util,NCS_SRMSV_WATERMARK_LOW,
                                                     SA_AIS_OK,"Get wmk val remote kernel util low"},

    [SRMSV_RSRC_GET_WMK_REM_USER_UTIL_T]    = {&tcd.client3,&tcd.wmk_rem_getval_user_util,NCS_SRMSV_WATERMARK_HIGH,
                                                     SA_AIS_OK,"Get wmk val remote user util high"},

    [SRMSV_RSRC_GET_WMK_REM_UTIL_ONE_T]     = {&tcd.client3,&tcd.wmk_rem_getval_util_one,NCS_SRMSV_WATERMARK_LOW,
                                                     SA_AIS_OK,"Get wmk val remote util one low"},

    [SRMSV_RSRC_GET_WMK_REM_UTIL_FIVE_T]    = {&tcd.client3,&tcd.wmk_rem_getval_util_five,NCS_SRMSV_WATERMARK_HIGH,
                                                     SA_AIS_OK,"Get wmk val remote util five high"},

    [SRMSV_RSRC_GET_WMK_REM_UTIL_FIFTEEN_T] = {&tcd.client3,&tcd.wmk_rem_getval_util_fifteen,NCS_SRMSV_WATERMARK_LOW,
                                                     SA_AIS_OK,"Get wmk val remote util fifteen low"},

    [SRMSV_RSRC_GET_WMK_REM_MEM_USED_T]     = {&tcd.client3,&tcd.wmk_rem_getval_mem_used,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val remote mem used dual"},

    [SRMSV_RSRC_GET_WMK_REM_MEM_FREE_T]     = {&tcd.client3,&tcd.wmk_rem_getval_mem_free,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val remote mem free dual"},

    [SRMSV_RSRC_GET_WMK_REM_SWAP_USED_T]    = {&tcd.client3,&tcd.wmk_rem_getval_swap_used,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val remote swap used dual"},

    [SRMSV_RSRC_GET_WMK_REM_SWAP_FREE_T]    = {&tcd.client3,&tcd.wmk_rem_getval_swap_free,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val remote swap free dual"},

    [SRMSV_RSRC_GET_WMK_REM_USED_BUF_MEM_T] = {&tcd.client3,&tcd.wmk_rem_getval_used_buf_mem,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val remote buff used dual"},

    [SRMSV_RSRC_GET_WMK_ALL_CPU_UTIL_T]     = {&tcd.client3,&tcd.wmk_all_getval_cpu_util,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val all cpu util dual"},

    [SRMSV_RSRC_GET_WMK_ALL_KERNEL_UTIL_T]  = {&tcd.client3,&tcd.wmk_all_getval_kernel_util,NCS_SRMSV_WATERMARK_LOW,
                                                     SA_AIS_OK,"Get wmk val all kernel util low"},

    [SRMSV_RSRC_GET_WMK_ALL_USER_UTIL_T]    = {&tcd.client3,&tcd.wmk_all_getval_user_util,NCS_SRMSV_WATERMARK_HIGH,
                                                     SA_AIS_OK,"Get wmk val all user util high"},

    [SRMSV_RSRC_GET_WMK_ALL_UTIL_ONE_T]     = {&tcd.client3,&tcd.wmk_all_getval_util_one,NCS_SRMSV_WATERMARK_LOW,
                                                     SA_AIS_OK,"Get wmk val all util one low"},

    [SRMSV_RSRC_GET_WMK_ALL_UTIL_FIVE_T]    = {&tcd.client3,&tcd.wmk_all_getval_util_five,NCS_SRMSV_WATERMARK_HIGH,
                                                     SA_AIS_OK,"Get wmk val all util five high"},

    [SRMSV_RSRC_GET_WMK_ALL_UTIL_FIFTEEN_T] = {&tcd.client3,&tcd.wmk_all_getval_util_fifteen,NCS_SRMSV_WATERMARK_LOW,
                                                     SA_AIS_OK,"Get wmk val all util fifteen low"},

    [SRMSV_RSRC_GET_WMK_ALL_MEM_USED_T]     = {&tcd.client3,&tcd.wmk_all_getval_mem_used,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val all mem used dual"},

    [SRMSV_RSRC_GET_WMK_ALL_MEM_FREE_T]     = {&tcd.client3,&tcd.wmk_all_getval_mem_free,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val all mem free dual"},

    [SRMSV_RSRC_GET_WMK_ALL_SWAP_USED_T]    = {&tcd.client3,&tcd.wmk_all_getval_swap_used,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val all swap used dual"},

    [SRMSV_RSRC_GET_WMK_ALL_SWAP_FREE_T]    = {&tcd.client3,&tcd.wmk_all_getval_swap_free,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val all swap free dual"},

    [SRMSV_RSRC_GET_WMK_ALL_USED_BUF_MEM_T] = {&tcd.client3,&tcd.wmk_all_getval_used_buf_mem,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val all buff used dual"},

    [SRMSV_RSRC_GET_WMK_VAL_BAD_HANDLE_T]   = {NULL,&tcd.wmk_getval_util_one,NCS_SRMSV_WATERMARK_LOW,
                                                     SA_AIS_ERR_BAD_HANDLE,"Get wmk val NULL handle"},

    [SRMSV_RSRC_GET_WMK_VAL_BAD_HANDLE2_T]  = {&tcd.client3,&tcd.wmk_getval_util_one,NCS_SRMSV_WATERMARK_LOW,
                                                     SA_AIS_ERR_BAD_HANDLE,"Get wmk val finalized hdl"},

    [SRMSV_RSRC_GET_WMK_VAL_MEM_FREE2_T]    = {&tcd.client1,&tcd.wmk_getval_mem_free,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val mem free dual"},

    [SRMSV_RSRC_GET_WMK_REM_NOT_EXIST_T] = {&tcd.client3,&tcd.wmk_rem_getval_used_buf_mem,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_ERR_NOT_EXIST,"Get wmk val remote ND doesnot exist"},

    [SRMSV_RSRC_GET_WMK_VAL_PROC_MEM_T]    = {&tcd.client1,&tcd.wmk_getval_proc_mem,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val proc mem dual"},

    [SRMSV_RSRC_GET_WMK_VAL_PROC_CPU_T]    = {&tcd.client1,&tcd.wmk_getval_proc_cpu,NCS_SRMSV_WATERMARK_DUAL,
                                                     SA_AIS_OK,"Get wmk val proc mem dual"},


};


int tet_test_srmsvGetWmkVal(int i,CONFIG_FLAG flg)
{
   SaAisErrorT rc;
   NCS_SRMSV_HANDLE cl_hdl;

   if (API_SRMSV_Get_Wmk_Val[i].srmsv_hdl == NULL)
   {
      cl_hdl = 0;
   }
   else
   {
      cl_hdl = *(API_SRMSV_Get_Wmk_Val[i].srmsv_hdl);
   }

   rc = ncs_srmsv_get_watermark_val(cl_hdl,API_SRMSV_Get_Wmk_Val[i].rsrc_info,API_SRMSV_Get_Wmk_Val[i].wmk_type);
   return(srmsv_test_result(rc,API_SRMSV_Get_Wmk_Val[i].exp_output,API_SRMSV_Get_Wmk_Val[i].result_string,flg));
}



#endif





