/* include files */
#include "mbcsv_api.h"
#include "tet_api.h"
void tware_mem_ign();
void tware_mem_dump();
/*Non distributed test purposes*/
void tet_mbcsv_initialize(int);
void tet_mbcsv_open_close(int);
void tet_mbcsv_change_role(int);
void tet_mbcsv_op(int);
/*dual process*/
void tet_mbcsv_Notify(int);
void tet_mbcsv_cold_sync(int);
void tet_mbcsv_warm_sync(int);
void tet_get_set_warm_sync(int);
void tet_mbcsv_send_checkpoint(int);
void tet_mbcsv_data_request(int);
void tet_mbcsv_test();
uns32 initsemaphore();
/* tet list definations */
struct tet_testlist api_testlist[]={
  {tet_mbcsv_initialize,1,1},
  {tet_mbcsv_initialize,1,2},
  {tet_mbcsv_initialize,1,3},
  {tet_mbcsv_initialize,1,4},
  {tet_mbcsv_initialize,1,6},
  {tet_mbcsv_initialize,1,7},
  {tet_mbcsv_initialize,1,8},

  {tet_mbcsv_open_close,2,1},
  {tet_mbcsv_open_close,2,2},
  {tet_mbcsv_open_close,2,3},
  {tet_mbcsv_open_close,2,4},
  {tet_mbcsv_open_close,2,5},
  {tet_mbcsv_open_close,2,6},

  {tet_mbcsv_change_role,3,1},
  {tet_mbcsv_change_role,3,2},
  {tet_mbcsv_change_role,3,3},
  {tet_mbcsv_change_role,3,4},
  {tet_mbcsv_change_role,3,5},
  {tet_mbcsv_change_role,3,6},
  {tet_mbcsv_change_role,3,7},
  {tet_mbcsv_change_role,3,8},
  {tet_mbcsv_change_role,3,9},
  {tet_mbcsv_change_role,3,10},
  {tet_mbcsv_change_role,3,11},
  {tet_mbcsv_change_role,3,12},

  {tet_mbcsv_op,4,1},
  {tet_mbcsv_op,4,2},
  {tet_mbcsv_op,4,3},

  {NULL,0},
};

uns32 mbcstm_startup()
{
  int distributed,test_case,single;
  char *tmpprt=NULL;
  tmpprt= (char *) getenv("DISTRIBUTED");
  distributed= atoi(tmpprt);
  tmpprt= (char *) getenv("TET_CASE");
  test_case= atoi(tmpprt);
  tmpprt= (char *) getenv("SINGLE");
  single= atoi(tmpprt);
  if(!distributed)
    {
      if(single)
        {
          tware_mem_ign();  
          tet_test_start(test_case,api_testlist);
          sleep(1);
          tware_mem_dump();
          tet_test_cleanup();
        }    
    }  
  /*change*/
  return 0;
}

uns32 tet_mbcsv_startup()
{
  mbcstm_startup();
  /*change*/
  return 0;
}

uns32 mbcstm_close()
{
  uns32 mbcsv_lib_destroy (void);
    
  mbcsv_lib_destroy();
  /*change*/
  return 0;
}
                                                                                                                       
uns32 mbcstm_wait_to_end()
{
  uns32 n;
  printf("\n Enter Input to End Testing");
  scanf("%d",&n);
  tet_result(TET_PASS);
  /*change*/
  return 0;
}

uns32 mbcstm_final_results()
{
  printf("\n \n");
  printf("*************** FINAL ANALYSIS OF RESULTS *****************");
  printf("\n \t\t TOTAL CASES: %d ",case_num);
  printf("\n \t\t CASES PASSED: %d ",cases_passed);
  printf("\n \t\t CASES FAILED: %d",cases_failed);
  printf("\n*********************** THE END **************************");
  /*change*/
  return 0;
}

uns32 mbcstm_test_print(uns32 case_num,char * case_name, char *disc,
                        TET_MBCSV_RESULT exp_res, TET_MBCSV_RESULT final_res)
{
  char exp_str[15], final_str[15];
    
  if(exp_res == TET_MBCSV_PASS)
    strcpy(exp_str,"PASS");
  else
    strcpy(exp_str,"FAIL");
                                                                                                                       
  if(final_res == TET_MBCSV_PASS)
    {
      tet_result(TET_PASS);    
      strcpy(final_str,"PASSED");
    }
  else
    {
      if(mbcstm_cb.sys == MBCSTM_SVC_INS1)
        {
          tet_result(TET_FAIL);
          strcpy(final_str,"FAILED");
        }
      else
        {
          tet_result(TET_PASS);
          strcpy(final_str,"PASSED");
        }
    }
    
  printf("\n CASE: %d :: CASE NAME: %s :: EXPECTED TO %s :: %s ", case_num, 
         case_name, exp_str, final_str);
    
  tet_printf("\n CASE: %d :: CASE NAME: %s :: EXPECTED TO %s :: %s ", case_num, case_name, exp_str, final_str);
  /*change*/
  return 0;
}
#if 0
static struct tet_testlist mbcstm_list_temp[] = {
  /*      { mbcstm_peer_discover_performance,10 }, */
  { NULL,0}
};

static struct tet_testlist mbcstm_list[] = {
  /* session related cases */
  { mbcstm_se_newbackup_with_active,10},
  { mbcstm_se_newbackup_with_mactive,10},
  { mbcstm_se_xbackup_with_mactive, 10}, 
  { mbcstm_se_active_to_active, 10 },
  { mbcstm_se_active_to_standby, 10 },
  { mbcstm_se_active_to_quiesced, 10 },
  { mbcstm_se_standby_to_standby, 10 },
  { mbcstm_se_standby_to_active, 10 },
  { mbcstm_se_standby_to_quiesced,10},
  { mbcstm_se_quiesced_to_active, 10 },
  { mbcstm_se_quiesced_to_standby, 10 },
  { mbcstm_se_quiesced_to_quiesced, 10 }, 
  /* event related cases */
  { mbcstm_peerup_event_newactive_to_active,10},
  { mbcstm_peerup_event_newactive_to_standby, 10},
  { mbcstm_peerup_event_newstandby_to_active, 10},
  { mbcstm_peerup_event_newstandby_to_standby, 10 }, 
  { mbcstm_peerdown_event_active,10},
  { mbcstm_peerdown_event_standby, 10}, 
  { mbcstm_cngrole_event_newactive_to_active, 10},
  { mbcstm_cngrole_event_newactive_to_standby, 10 },
  { mbcstm_cngrole_event_newstandby_to_standby, 10}, 
  { mbcstm_cngrole_event_newstandby_to_active, 10},
  { mbcstm_active_multiactive_standby, 10 },
  { mbcstm_standby_active_quiesced, 10 }, 
    
  /* call back related cases */
  { mbcstm_coldsync_standby_encode_fail, 10 }, 
  { mbcstm_coldsync_coldrequest_noresponse, 10}, 
  { mbcstm_coldsync_stdby_decode_fail, 10}, 
  { mbcstm_coldsync_active_encode_fail, 10 },  
  { mbcstm_coldsync_timer, 10}, 
  { mbcstm_coldsync_cmp_timer, 10}, 
  { mbcstm_warmsync_timer, 10},
  { mbcstm_warmsync_cmp_timer, 10},  
  { mbcstm_warmsync_complete, 10 }, 
  { mbcstm_data_request_idle, 10}, 
    
  /* ckpt related cases */
  { mbcstm_ckpt_sendsync, 10 },
  { mbcstm_ckpt_send_usrasync, 10}, 
  { mbcstm_ckpt_send_mbcasync, 10}, 
  { mbcstm_ckpt_standby_sendsync, 10 }, 
  { mbcstm_ckpt_idle_sendsync, 10 }, 
    
  /* notify related case */
  { mbcstm_notify_send_all, 10 },  
  { mbcstm_send_notify_idle, 10 }, 
  { mbcstm_notify_send_to_active, 10},
  { mbcstm_notify_send_to_standby, 10},
  { mbcstm_notify_send_active_to_active, 10},
  { mbcstm_notify_send_active_to_multiactive, 10},  
  /* get set related cases */
  { mbcstm_set_warm_sync_off_active, 10 },
  { mbcstm_set_warm_sync_off_standby, 10 }, 
  { mbcstm_set_warm_sync_on_standby, 10 },  
  { mbcstm_set_warm_sync_timer_active, 10 },
  { mbcstm_set_warm_sync_timer_standby, 10 },
  { mbcstm_get_warm_sync_timer, 10 },
  { mbcstm_get_warm_sync_on_off, 10 },
  /* mbcsv mds related cases */
  /* { mbcstm_mbcsv_down_event, 10}, */
  /* mbcsv message verification cases */
  { mbcstm_test_sync_send_messages, 10 },
  { mbcstm_test_usrasync_send_messages, 10 },
  { mbcstm_test_mbcasync_send_messages, 10 },    
  /* final analysis */ 
  { mbcstm_final_results,10},
  { NULL,-1}
};
#endif



