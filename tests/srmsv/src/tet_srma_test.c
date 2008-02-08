#if ( (NCS_SRMA == 1)&& (TET_A==1) )

#include "tet_startup.h"
#include "tet_srmsv.h"
#include "tet_srmsv_conf.h"
#include "ncs_main_papi.h"

#define m_TET_SRMSV_PRINTF printf
#define TRUE 1
#define FALSE 0


void spawn_srmsv_exe(int arg)
{
   char *tmp_ptr=NULL;
   char arch[4]="xxxx";
   int result=0;
   int rc;
   tmp_ptr = (char *) getenv("TARGET_ARCH");
   if(tmp_ptr)
   {
      strcpy(arch,tmp_ptr);
      tmp_ptr = NULL;
   }
  
   if(result == (strcmp(arch,"ppc")))
   {
     switch(arg)
        { 
           case SRMSV_CPU_UTIL:
                           rc = system("./srmsv_ex_ppc.exe CPU_UTIL &");
                           break;
           case SRMSV_MEM_UTIL:
                           rc = system("./srmsv_ex_ppc.exe MEM &");
                           break;
           default:
                   m_TET_SRMSV_PRINTF("\nSpawning nothing\n");
        }
    }

   else if(result == (strcmp(arch,"64T")))
   {
       switch(arg)
       { 
          case SRMSV_CPU_UTIL:
                          rc = system("./srmsv_ex_64t.exe CPU_UTIL &");
                          break;
          case SRMSV_MEM_UTIL:
                          rc = system("./srmsv_ex_64t.exe MEM &");
                          break;
          default:
                  m_TET_SRMSV_PRINTF("\nSpawning nothing\n");
       }
   }

   else if(result == (strcmp(arch,"pen3")))
   {
       switch(arg)
       { 
          case SRMSV_CPU_UTIL:
                          rc = system("./srmsv_ex_pen3.exe CPU_UTIL &");
                          break;
          case SRMSV_MEM_UTIL:
                          rc = system("./srmsv_ex_pen3.exe MEM &");
                          break;
          default:
                  m_TET_SRMSV_PRINTF("\nSpawning nothing\n");
       }
    }

   else if(result == (strcmp(arch,"i386")))
   {
       switch(arg)
       { 
          case SRMSV_CPU_UTIL:
                          rc = system("./srmsv_ex_i386.exe CPU_UTIL &");
                          break;
          case SRMSV_MEM_UTIL:
                          rc = system("./srmsv_ex_i386.exe MEM &");
                          break;
          default:
                  m_TET_SRMSV_PRINTF("\nSpawning nothing\n");
       }
    }

    else
    {
       m_TET_SRMSV_PRINTF(" Couldnot spawn executable \n");
       tet_printf(" Couldnot spawn executable \n");
    }

    if(rc == -1)
    {
       m_TET_SRMSV_PRINTF("system() call failed with -1 - couldnot spawn exe\n");
       tet_printf("system() call failed with -1 - couldnot spawn exe\n");
    }
}

void kill_srmsv_exe()
{
   char *tmp_ptr=NULL;
   char arch[4]="xxxx";
   int result=0;
   int rc;
   tmp_ptr = (char *) getenv("TARGET_ARCH");
   if(tmp_ptr)
   {
      strcpy(arch,tmp_ptr);
      tmp_ptr = NULL;
   }

   if(result == (strcmp(arch,"ppc")))
      rc = system("sudo killall -9 srmsv_ex_ppc.exe");

   else if(result == (strcmp(arch,"64T")))
      rc = system("sudo killall -9 srmsv_ex_64t.exe");

   else if(result == (strcmp(arch,"pen3")))
      rc = system("sudo killall -9 srmsv_ex_pen3.exe");

   else if(result == (strcmp(arch,"i386")))
      rc = system("sudo killall -9 srmsv_ex_i386.exe");

    else
    {
       m_TET_SRMSV_PRINTF(" Couldnot kill executable \n");
       tet_printf(" Couldnot kill executable \n");
    }

    if(rc == -1)
    {
       m_TET_SRMSV_PRINTF("system() call failed with -1 - couldnot sudo kill exe\n");
       tet_printf("system() call failed with -1 - couldnot sudo kill exe\n");
    }
}

void printSrmaHead(char *str)
{
  m_TET_SRMSV_PRINTF("\n************************************************************* \n");
  m_TET_SRMSV_PRINTF("%s",str);
  m_TET_SRMSV_PRINTF("\n************************************************************* \n");

  tet_printf("\n************************************************************* \n");
  tet_printf("%s",str);
  tet_printf("\n************************************************************* \n");
}

void printSrmaResult(int result)
{
  if (result == TET_PASS)
  {
     m_TET_SRMSV_PRINTF("\n ##### TEST CASE SUCCEEDED #####\n");
     tet_printf("\n ##### TEST CASE SUCCEEDED #####\n");
  }
  else if(result == TET_FAIL)
  {
     m_TET_SRMSV_PRINTF("\n ##### TEST CASE FAILED #####\n");
     tet_printf("\n ##### TEST CASE FAILED #####\n");
  }
  else if(result == TET_UNRESOLVED)
  {
     m_TET_SRMSV_PRINTF("\n ##### TEST CASE UNRESOLVED #####\n");
     tet_printf("\n ##### TEST CASE UNRESOLVED #####\n");
  }
  tet_result(result);
}

void fill_mon_info(NCS_SRMSV_MON_INFO *mon_info,NCS_SRMSV_USER_TYPE user_type,
                   NCS_SRMSV_NODE_LOCATION node_loc,NCS_SRMSV_RSRC_TYPE rsrc_type,time_t rate,
                   time_t interval,NCS_SRMSV_MONITOR_TYPE mon_type,uns32 samples,
                   NCS_SRMSV_VAL_TYPE val_type,NCS_SRMSV_STAT_SCALE_TYPE scale_type,
                   int32 val,NCS_SRMSV_VAL_TYPE tol_val_type,NCS_SRMSV_STAT_SCALE_TYPE tol_scale_type,
                   int32 tol_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION test_condition,uns32 severity)
{
   mon_info->rsrc_info.usr_type = user_type;
   mon_info->rsrc_info.srmsv_loc = node_loc;
   mon_info->rsrc_info.rsrc_type = rsrc_type;
   mon_info->monitor_data.monitoring_rate = rate;
   mon_info->monitor_data.monitoring_interval = interval;
   mon_info->monitor_data.monitor_type = mon_type;
   mon_info->monitor_data.mon_cfg.threshold.samples = samples;
   mon_info->monitor_data.mon_cfg.threshold.threshold_val.val_type = val_type;
   mon_info->monitor_data.mon_cfg.threshold.threshold_val.scale_type = scale_type;
   mon_info->monitor_data.mon_cfg.threshold.threshold_val.val.i_val32 = val;
   mon_info->monitor_data.mon_cfg.threshold.tolerable_val.val_type = tol_val_type;
   mon_info->monitor_data.mon_cfg.threshold.tolerable_val.scale_type = tol_scale_type;
   mon_info->monitor_data.mon_cfg.threshold.tolerable_val.val.i_val32 = tol_val;
   mon_info->monitor_data.mon_cfg.threshold.condition = test_condition;
   mon_info->monitor_data.evt_severity = severity ;
}

void fill_proc_mon_info(NCS_SRMSV_MON_INFO *mon_info,NCS_SRMSV_USER_TYPE user_type,
                        uns32 pid,NCS_SRMSV_RSRC_TYPE rsrc_type,time_t rate,time_t interval,
                        NCS_SRMSV_MONITOR_TYPE mon_type,uns32 severity,uns32 child_level)
{
   mon_info->rsrc_info.usr_type = user_type;
   mon_info->rsrc_info.srmsv_loc = NCS_SRMSV_NODE_LOCAL;
   mon_info->rsrc_info.pid = pid;
   mon_info->rsrc_info.child_level = child_level;
   mon_info->rsrc_info.rsrc_type = rsrc_type;
   mon_info->monitor_data.monitoring_rate = rate;
   mon_info->monitor_data.monitoring_interval = interval;
   mon_info->monitor_data.monitor_type = mon_type;
   mon_info->monitor_data.evt_severity = severity ;
}

void fill_subscr_info(NCS_SRMSV_RSRC_INFO *subscr_info,NCS_SRMSV_USER_TYPE user_type,
                      NCS_SRMSV_RSRC_TYPE rsrc_type)
{
   subscr_info->usr_type = user_type;
   subscr_info->srmsv_loc = NCS_SRMSV_NODE_LOCAL;
   subscr_info->rsrc_type = rsrc_type;
}

void fill_proc_subscr_info(NCS_SRMSV_RSRC_INFO *subscr_info,NCS_SRMSV_USER_TYPE user_type,
                           NCS_SRMSV_RSRC_TYPE rsrc_type,uns32 child_level)
{
   subscr_info->usr_type = user_type;
   subscr_info->srmsv_loc = NCS_SRMSV_NODE_LOCAL;
   subscr_info->child_level = child_level;
   subscr_info->rsrc_type = rsrc_type;
}

void fill_remote_mon_info(NCS_SRMSV_MON_INFO *mon_info,NCS_SRMSV_USER_TYPE user_type,
                          NCS_SRMSV_NODE_LOCATION node_loc,NCS_SRMSV_RSRC_TYPE rsrc_type,time_t rate,
                          time_t interval,NCS_SRMSV_MONITOR_TYPE mon_type,uns32 samples,
                          NCS_SRMSV_VAL_TYPE val_type,NCS_SRMSV_STAT_SCALE_TYPE scale_type,int32 val,
                          NCS_SRMSV_VAL_TYPE tol_val_type,NCS_SRMSV_STAT_SCALE_TYPE tol_scale_type,
                          int32 tol_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION test_condition,uns32 severity)
{
   mon_info->rsrc_info.usr_type = user_type;
   mon_info->rsrc_info.srmsv_loc = node_loc;
   mon_info->rsrc_info.rsrc_type = rsrc_type;
   mon_info->monitor_data.monitoring_rate = rate;
   mon_info->monitor_data.monitoring_interval = interval;
   mon_info->monitor_data.monitor_type = mon_type;
   mon_info->monitor_data.mon_cfg.threshold.samples = samples;
   mon_info->monitor_data.mon_cfg.threshold.threshold_val.val_type = val_type;
   mon_info->monitor_data.mon_cfg.threshold.threshold_val.scale_type = scale_type;
   mon_info->monitor_data.mon_cfg.threshold.threshold_val.val.i_val32 = val;
   mon_info->monitor_data.mon_cfg.threshold.tolerable_val.val_type = tol_val_type;
   mon_info->monitor_data.mon_cfg.threshold.tolerable_val.scale_type = tol_scale_type;
   mon_info->monitor_data.mon_cfg.threshold.tolerable_val.val.i_val32 = tol_val;
   mon_info->monitor_data.mon_cfg.threshold.condition = test_condition;
   mon_info->monitor_data.evt_severity = severity ;
}

void fill_remote_subscr_info(NCS_SRMSV_RSRC_INFO *subscr_info,NCS_SRMSV_USER_TYPE user_type,
                             NCS_SRMSV_NODE_LOCATION node_loc,NCS_SRMSV_RSRC_TYPE rsrc_type)
{
   subscr_info->usr_type = user_type;
   subscr_info->srmsv_loc = node_loc;
   subscr_info->rsrc_type = rsrc_type;
}

void fill_wmk_mon_info(NCS_SRMSV_MON_INFO *mon_info,NCS_SRMSV_USER_TYPE user_type,
                       NCS_SRMSV_NODE_LOCATION node_loc,NCS_SRMSV_RSRC_TYPE rsrc_type,time_t rate,
                       time_t interval,NCS_SRMSV_WATERMARK_TYPE wm_type,uns32 severity)
{
   mon_info->rsrc_info.usr_type = user_type;
   mon_info->rsrc_info.srmsv_loc = node_loc;
   mon_info->rsrc_info.rsrc_type = rsrc_type;
   mon_info->monitor_data.monitoring_rate = rate;
   mon_info->monitor_data.monitoring_interval = interval;
   mon_info->monitor_data.monitor_type = NCS_SRMSV_MON_TYPE_WATERMARK;
   mon_info->monitor_data.mon_cfg.watermark_type = wm_type;
   mon_info->monitor_data.evt_severity = severity ;
}

void fill_rem_wmk_mon_info(NCS_SRMSV_MON_INFO *mon_info,NCS_SRMSV_USER_TYPE user_type,
                           NCS_SRMSV_RSRC_TYPE rsrc_type,time_t rate,
                           time_t interval,NCS_SRMSV_WATERMARK_TYPE wm_type,uns32 severity)
{
   mon_info->rsrc_info.usr_type = user_type;
   mon_info->rsrc_info.srmsv_loc = NCS_SRMSV_NODE_REMOTE;
   mon_info->rsrc_info.rsrc_type = rsrc_type;
   mon_info->monitor_data.monitoring_rate = rate;
   mon_info->monitor_data.monitoring_interval = interval;
   mon_info->monitor_data.monitor_type = NCS_SRMSV_MON_TYPE_WATERMARK;
   mon_info->monitor_data.mon_cfg.watermark_type = wm_type;
   mon_info->monitor_data.evt_severity = severity ;
}

void fill_getval_rsrc_info(NCS_SRMSV_RSRC_INFO *rsrc_info,NCS_SRMSV_NODE_LOCATION node_loc,
                           NCS_SRMSV_RSRC_TYPE rsrc_type)
{
   rsrc_info->srmsv_loc = node_loc;
   rsrc_info->rsrc_type = rsrc_type;
}

void fill_srma_testcase_data()
{
   tcd.srmsv_callback = app_srmsv_callback;

/* local monitor for cpu utilization threshold set equal 70% - runs for 3min */
   fill_mon_info(&tcd.local_monitor_cpu_util,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                  NCS_SRMSV_RSRC_CPU_UTIL,2,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,70,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                  NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW,5);

/* local monitor for cpu kernel utilization threshold set equal 1% - runs for 3min */
   fill_mon_info(&tcd.local_monitor_kernel_util,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,NCS_SRMSV_NODE_LOCAL,
                  NCS_SRMSV_RSRC_CPU_KERNEL,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,50,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                  NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW,5);

/* local monitor for cpu user utilization threshold set equal 1% - runs for 3min */
   fill_mon_info(&tcd.local_monitor_user_util,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                  NCS_SRMSV_RSRC_CPU_USER,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,1,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                  NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* local monitor for cpu user utilization for one min threshold set below 100% - runs for 3min */
   fill_mon_info(&tcd.local_monitor_user_util_one,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                  NCS_SRMSV_RSRC_CPU_UTIL_ONE,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,100,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,5,
                  NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW,1);

/* local monitor for cpu user utilization for last five min threshold set below 10% - runs for 3min */
   fill_mon_info(&tcd.local_monitor_user_util_five,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                  NCS_SRMSV_RSRC_CPU_UTIL_FIVE,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,100,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,5,
                  NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW,1);

/* local monitor for cpu user utilization for last fifteen min threshold set below 10% - runs for 3min */
   fill_mon_info(&tcd.local_monitor_user_util_fifteen,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                  NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,100,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,5,
                  NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW,1);

/* local monitor for physical memory used threshold set equal 700KB - runs for 3min */
   fill_mon_info(&tcd.local_monitor_mem_used,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                  NCS_SRMSV_RSRC_MEM_PHYSICAL_USED,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,700,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,5,
                  NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* local monitor for physical memory free threshold set equal 70KB - runs for 3min */
   fill_mon_info(&tcd.local_monitor_mem_free,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,NCS_SRMSV_NODE_LOCAL,
                  NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,70,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,5,
                  NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* local monitor for swap space used threshold set equal 0 - runs for 3min */
   fill_mon_info(&tcd.local_monitor_swap_used,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                  NCS_SRMSV_RSRC_SWAP_SPACE_USED,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,0,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,0,
                  NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* local monitor for swap space free threshold set equal 700 - runs for 3min */
   fill_mon_info(&tcd.local_monitor_swap_free,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                  NCS_SRMSV_RSRC_SWAP_SPACE_FREE,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,0,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,5,
                  NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* local monitor for swap space free threshold set equal 700 - runs for 3min */
   fill_mon_info(&tcd.local_monitor_used_buf_mem,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                  NCS_SRMSV_RSRC_USED_BUFFER_MEM,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,700,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,5,
                  NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/*Monitors for process exits */
   fill_proc_mon_info(&tcd.local_monitor_proc_exit,NCS_SRMSV_USER_REQUESTOR,9140,
                       NCS_SRMSV_RSRC_PROC_EXIT,1,300,NCS_SRMSV_MON_TYPE_THRESHOLD,5,0);
    
   fill_proc_mon_info(&tcd.local_monitor_proc_child_exit,NCS_SRMSV_USER_REQUESTOR,0,
                       NCS_SRMSV_RSRC_PROC_EXIT,1,600,NCS_SRMSV_MON_TYPE_THRESHOLD,5,1);

   fill_proc_mon_info(&tcd.local_monitor_proc_grandchild_exit,NCS_SRMSV_USER_REQUESTOR,0,
                       NCS_SRMSV_RSRC_PROC_EXIT,1,600,NCS_SRMSV_MON_TYPE_THRESHOLD,5,2);

   fill_proc_mon_info(&tcd.local_monitor_proc_desc_exit,NCS_SRMSV_USER_REQUESTOR,0,
                       NCS_SRMSV_RSRC_PROC_EXIT,1,600,NCS_SRMSV_MON_TYPE_THRESHOLD,5,4);

/* local monitor for physical memory used threshold set equal 10% - runs for 3min */
   fill_mon_info(&tcd.local_monitor_mem_used2,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,NCS_SRMSV_NODE_LOCAL,
                  NCS_SRMSV_RSRC_MEM_PHYSICAL_USED,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,10,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                  NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/*invalid monitors */
    /*specifying subscriber while req monitor */
   fill_mon_info(&tcd.invalid_monitor,NCS_SRMSV_USER_SUBSCRIBER,NCS_SRMSV_NODE_LOCAL,
                  NCS_SRMSV_RSRC_CPU_UTIL,5,600,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,70,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,5,
                  NCS_SRMSV_THRESHOLD_VAL_IS_AT,5);

    /* process id cannot be zero*/
   fill_proc_mon_info(&tcd.invalid_monitor2,NCS_SRMSV_USER_REQUESTOR,0,
                       NCS_SRMSV_RSRC_PROC_EXIT,0,600,NCS_SRMSV_MON_TYPE_THRESHOLD,5,1);

   fill_subscr_info(&tcd.local_subscr_cpu_util,NCS_SRMSV_USER_SUBSCRIBER,NCS_SRMSV_RSRC_CPU_UTIL);
   fill_subscr_info(&tcd.local_subscr_kernel_util,NCS_SRMSV_USER_SUBSCRIBER,NCS_SRMSV_RSRC_CPU_KERNEL);
   fill_subscr_info(&tcd.local_subscr_user_util,NCS_SRMSV_USER_SUBSCRIBER,NCS_SRMSV_RSRC_CPU_USER);
   fill_subscr_info(&tcd.local_subscr_util_one,NCS_SRMSV_USER_SUBSCRIBER,NCS_SRMSV_RSRC_CPU_UTIL_ONE);
   fill_subscr_info(&tcd.local_subscr_util_five,NCS_SRMSV_USER_SUBSCRIBER,NCS_SRMSV_RSRC_CPU_UTIL_FIVE);
   fill_subscr_info(&tcd.local_subscr_util_fifteen,NCS_SRMSV_USER_SUBSCRIBER,NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN);
   fill_subscr_info(&tcd.local_subscr_mem_used,NCS_SRMSV_USER_SUBSCRIBER,NCS_SRMSV_RSRC_MEM_PHYSICAL_USED);
   fill_subscr_info(&tcd.local_subscr_mem_free,NCS_SRMSV_USER_SUBSCRIBER,NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE);
   fill_subscr_info(&tcd.local_subscr_swap_used,NCS_SRMSV_USER_SUBSCRIBER,NCS_SRMSV_RSRC_SWAP_SPACE_USED);
   fill_subscr_info(&tcd.local_subscr_swap_free,NCS_SRMSV_USER_SUBSCRIBER,NCS_SRMSV_RSRC_SWAP_SPACE_FREE);
   fill_subscr_info(&tcd.local_subscr_used_buf_mem,NCS_SRMSV_USER_SUBSCRIBER,NCS_SRMSV_RSRC_USED_BUFFER_MEM);
   fill_subscr_info(&tcd.invalid_subscr,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_RSRC_MEM_PHYSICAL_USED);

   fill_proc_subscr_info(&tcd.local_subscr_proc_exit,NCS_SRMSV_USER_SUBSCRIBER,NCS_SRMSV_RSRC_PROC_EXIT,0);
   fill_proc_subscr_info(&tcd.local_subscr_proc_child_exit,NCS_SRMSV_USER_SUBSCRIBER, NCS_SRMSV_RSRC_PROC_EXIT,1);
   fill_proc_subscr_info(&tcd.local_subscr_proc_grandchild_exit,NCS_SRMSV_USER_SUBSCRIBER,NCS_SRMSV_RSRC_PROC_EXIT,2);
   fill_proc_subscr_info(&tcd.local_subscr_proc_desc_exit,NCS_SRMSV_USER_SUBSCRIBER,NCS_SRMSV_RSRC_PROC_EXIT,3);

/* remote monitor for cpu utilization threshold set equal 1% - runs for 3min */
   fill_remote_mon_info(&tcd.remote_monitor_cpu_util,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_REMOTE,
                        NCS_SRMSV_RSRC_CPU_UTIL,2,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                        NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,1,
                        NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                        NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* remote monitor for cpu kernel utilization threshold set equal 1% - runs for 3min */
   fill_remote_mon_info(&tcd.remote_monitor_kernel_util,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,NCS_SRMSV_NODE_REMOTE,
                         NCS_SRMSV_RSRC_CPU_KERNEL,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,1,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* remote monitor for cpu user utilization threshold set equal 1% - runs for 3min */
   fill_remote_mon_info(&tcd.remote_monitor_user_util,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_REMOTE,
                         NCS_SRMSV_RSRC_CPU_USER,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,1,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* remote monitor for cpu user utilization for one min threshold set below 100% - runs for 3min */
   fill_remote_mon_info(&tcd.remote_monitor_user_util_one,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_REMOTE,
                         NCS_SRMSV_RSRC_CPU_UTIL_ONE,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,100,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW,5);

/* remote monitor for cpu user utilization for last five min threshold set below 100% - runs for 3min */
   fill_remote_mon_info(&tcd.remote_monitor_user_util_five,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_REMOTE,
                         NCS_SRMSV_RSRC_CPU_UTIL_FIVE,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,0,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,100,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW,5);

/* remote monitor for cpu user utilization for last fifteen min threshold set below 100% - runs for 3min */
   fill_remote_mon_info(&tcd.remote_monitor_user_util_fifteen,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_REMOTE,
                         NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,100,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,1,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW,5);

/* remote monitor for physical memory used threshold set equal 700KB - runs for 3min */
   fill_remote_mon_info(&tcd.remote_monitor_mem_used,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,NCS_SRMSV_NODE_REMOTE,
                         NCS_SRMSV_RSRC_MEM_PHYSICAL_USED,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,700,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,5,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* remote monitor for physical memory free threshold set equal 70KB - runs for 3min */
   fill_remote_mon_info(&tcd.remote_monitor_mem_free,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_REMOTE,
                         NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,70,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,5,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* remote monitor for swap space used threshold set above 0 - runs for 3min */
   fill_remote_mon_info(&tcd.remote_monitor_swap_used,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_REMOTE,
                         NCS_SRMSV_RSRC_SWAP_SPACE_USED,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,0,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,1,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* remote monitor for swap space free threshold set above 0 - runs for 3min */
   fill_remote_mon_info(&tcd.remote_monitor_swap_free,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_REMOTE,
                         NCS_SRMSV_RSRC_SWAP_SPACE_FREE,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,0,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,5,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* remote monitor for used buf memory threshold set equal 700 - runs for 3min */
   fill_remote_mon_info(&tcd.remote_monitor_used_buf_mem,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_REMOTE,
                         NCS_SRMSV_RSRC_USED_BUFFER_MEM,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,700,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,5,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

   fill_remote_subscr_info(&tcd.remote_subscr_cpu_util,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_CPU_UTIL);

   fill_remote_subscr_info(&tcd.remote_subscr_kernel_util,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_CPU_KERNEL);

   fill_remote_subscr_info(&tcd.remote_subscr_user_util,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_CPU_USER);

   fill_remote_subscr_info(&tcd.remote_subscr_util_one,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_CPU_UTIL_ONE);

   fill_remote_subscr_info(&tcd.remote_subscr_util_five,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_CPU_UTIL_FIVE);

   fill_remote_subscr_info(&tcd.remote_subscr_util_fifteen,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN);

   fill_remote_subscr_info(&tcd.remote_subscr_mem_used,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_MEM_PHYSICAL_USED);

   fill_remote_subscr_info(&tcd.remote_subscr_mem_free,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE);

   fill_remote_subscr_info(&tcd.remote_subscr_swap_used,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_SWAP_SPACE_USED);

   fill_remote_subscr_info(&tcd.remote_subscr_swap_free,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_SWAP_SPACE_FREE);

   fill_remote_subscr_info(&tcd.remote_subscr_used_buf_mem,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_USED_BUFFER_MEM);

/* all monitor for cpu utilization threshold set equal 1% - runs for 3min */
   fill_remote_mon_info(&tcd.all_monitor_cpu_util,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                        NCS_SRMSV_RSRC_CPU_UTIL,2,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                        NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,1,
                        NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                        NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* all monitor for cpu kernel utilization threshold set equal 1% - runs for 3min */
   fill_remote_mon_info(&tcd.all_monitor_kernel_util,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,NCS_SRMSV_NODE_ALL,
                         NCS_SRMSV_RSRC_CPU_KERNEL,2,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,1,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,2,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* all monitor for cpu user utilization threshold set equal 1% - runs for 3min */
   fill_remote_mon_info(&tcd.all_monitor_user_util,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                         NCS_SRMSV_RSRC_CPU_USER,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,1,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* all monitor for cpu user utilization for one min threshold set above 0% - runs for 3min */
   fill_remote_mon_info(&tcd.all_monitor_user_util_one,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                         NCS_SRMSV_RSRC_CPU_UTIL_ONE,5,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,5,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* all monitor for cpu user utilization for last five min threshold set above 0% - runs for 3min */
   fill_remote_mon_info(&tcd.all_monitor_user_util_five,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                         NCS_SRMSV_RSRC_CPU_UTIL_FIVE,5,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,5,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* all monitor for cpu user utilization for last fifteen min threshold set above 0% - runs for 3min */
   fill_remote_mon_info(&tcd.all_monitor_user_util_fifteen,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                         NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN,5,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,5,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* all monitor for physical memory used threshold set equal 700KB - runs for 3min */
   fill_remote_mon_info(&tcd.all_monitor_mem_used,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,NCS_SRMSV_NODE_ALL,
                         NCS_SRMSV_RSRC_MEM_PHYSICAL_USED,5,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,700,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,5,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* all monitor for physical memory free threshold set equal 70KB - runs for 3min */
   fill_remote_mon_info(&tcd.all_monitor_mem_free,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                         NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE,5,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                         NCS_SRMSV_VAL_TYPE_INT32, NCS_SRMSV_STAT_SCALE_KBYTE,70,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,5,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

/* all monitor for swap space used threshold set below 100 - runs for 3min */
   fill_remote_mon_info(&tcd.all_monitor_swap_used,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                         NCS_SRMSV_RSRC_SWAP_SPACE_USED,1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,100,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,0,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW,5);

/* all monitor for swap space free threshold set below 0 - runs for 3min */
   fill_remote_mon_info(&tcd.all_monitor_swap_free,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                         NCS_SRMSV_RSRC_SWAP_SPACE_FREE,5,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,100,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,0,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW,5);

/* all monitor for swap space free threshold set equal 700 - runs for 3min */
   fill_remote_mon_info(&tcd.all_monitor_used_buf_mem,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                         NCS_SRMSV_RSRC_USED_BUFFER_MEM,5,180,NCS_SRMSV_MON_TYPE_THRESHOLD,3,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,700,
                         NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,5,
                         NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

   fill_remote_subscr_info(&tcd.all_subscr_cpu_util,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_CPU_UTIL);

   fill_remote_subscr_info(&tcd.all_subscr_kernel_util,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_CPU_KERNEL);

   fill_remote_subscr_info(&tcd.all_subscr_user_util,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_CPU_USER);

   fill_remote_subscr_info(&tcd.all_subscr_util_one,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_CPU_UTIL_ONE);

   fill_remote_subscr_info(&tcd.all_subscr_util_five,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_CPU_UTIL_FIVE);

   fill_remote_subscr_info(&tcd.all_subscr_util_fifteen,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN);

   fill_remote_subscr_info(&tcd.all_subscr_mem_used,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_MEM_PHYSICAL_USED);

   fill_remote_subscr_info(&tcd.all_subscr_mem_free,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE);

   fill_remote_subscr_info(&tcd.all_subscr_swap_used,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_SWAP_SPACE_USED);

   fill_remote_subscr_info(&tcd.all_subscr_swap_free,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_SWAP_SPACE_FREE);

   fill_remote_subscr_info(&tcd.all_subscr_used_buf_mem,NCS_SRMSV_USER_SUBSCRIBER,
                            NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_USED_BUFFER_MEM);

   /************* WATER MARK MONITORS ***************/

/* local wmk monitor for cpu utilization - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_monitor_cpu_util,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                      NCS_SRMSV_RSRC_CPU_UTIL,2,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* local wmk monitor for cpu kernel utilization  - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_monitor_kernel_util,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,NCS_SRMSV_NODE_LOCAL,
                      NCS_SRMSV_RSRC_CPU_KERNEL,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* local wmk monitor for cpu user utilization  - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_monitor_user_util,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                      NCS_SRMSV_RSRC_CPU_USER,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* local wmk monitor for cpu user utilization for one min - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_monitor_user_util_one,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                      NCS_SRMSV_RSRC_CPU_UTIL_ONE,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* local wmk monitor for cpu user utilization for last five min - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_monitor_user_util_five,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                      NCS_SRMSV_RSRC_CPU_UTIL_FIVE,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* local wmk monitor for cpu user utilization for last fifteen min - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_monitor_user_util_fifteen,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                      NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* local wmk monitor for physical memory used - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_monitor_mem_used,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                      NCS_SRMSV_RSRC_MEM_PHYSICAL_USED,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* local wmk monitor for physical memory free - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_monitor_mem_free,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,NCS_SRMSV_NODE_LOCAL,
                      NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* local wmk monitor for swap space used - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_monitor_swap_used,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                      NCS_SRMSV_RSRC_SWAP_SPACE_USED,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* local wmk monitor for swap space free - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_monitor_swap_free,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                      NCS_SRMSV_RSRC_SWAP_SPACE_FREE,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* local wmk monitor for swap space free - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_monitor_used_buf_mem,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                      NCS_SRMSV_RSRC_USED_BUFFER_MEM,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* remote wmk monitor for cpu utilization - runs for 3min */
   fill_rem_wmk_mon_info(&tcd.wmk_remote_cpu_util,NCS_SRMSV_USER_REQUESTOR,
                          NCS_SRMSV_RSRC_CPU_UTIL,2,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* remote wmk monitor for cpu kernel utilization - runs for 3min */
   fill_rem_wmk_mon_info(&tcd.wmk_remote_kernel_util,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,
                          NCS_SRMSV_RSRC_CPU_KERNEL,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* remote wmk monitor for cpu user utilization - runs for 3min */
   fill_rem_wmk_mon_info(&tcd.wmk_remote_user_util,NCS_SRMSV_USER_REQUESTOR,
                          NCS_SRMSV_RSRC_CPU_USER,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* remote wmk monitor for cpu user utilization for one min - runs for 3min */
   fill_rem_wmk_mon_info(&tcd.wmk_remote_user_util_one,NCS_SRMSV_USER_REQUESTOR,
                          NCS_SRMSV_RSRC_CPU_UTIL_ONE,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* remote wmk monitor for cpu user utilization for last five min - runs for 3min */
   fill_rem_wmk_mon_info(&tcd.wmk_remote_user_util_five,NCS_SRMSV_USER_REQUESTOR,
                          NCS_SRMSV_RSRC_CPU_UTIL_FIVE,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* remote wmk monitor for cpu user utilization for last fifteen min - runs for 3min */
   fill_rem_wmk_mon_info(&tcd.wmk_remote_user_util_fifteen,NCS_SRMSV_USER_REQUESTOR,
                          NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* remote wmk monitor for physical memory used - runs for 3min */
   fill_rem_wmk_mon_info(&tcd.wmk_remote_mem_used,NCS_SRMSV_USER_REQUESTOR,
                          NCS_SRMSV_RSRC_MEM_PHYSICAL_USED,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* remote wmk monitor for physical memory free - runs for 3min */
   fill_rem_wmk_mon_info(&tcd.wmk_remote_mem_free,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,
                          NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* remote wmk monitor for swap space used - runs for 3min */
   fill_rem_wmk_mon_info(&tcd.wmk_remote_swap_used,NCS_SRMSV_USER_REQUESTOR,
                          NCS_SRMSV_RSRC_SWAP_SPACE_USED,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* remote wmk monitor for swap space free - runs for 3min */
   fill_rem_wmk_mon_info(&tcd.wmk_remote_swap_free,NCS_SRMSV_USER_REQUESTOR,
                          NCS_SRMSV_RSRC_SWAP_SPACE_FREE,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* remote wmk monitor for swap space free - runs for 3min */
   fill_rem_wmk_mon_info(&tcd.wmk_remote_used_buf_mem,NCS_SRMSV_USER_REQUESTOR,
                          NCS_SRMSV_RSRC_USED_BUFFER_MEM,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* all wmk monitor for cpu utilization - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_all_cpu_util,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                      NCS_SRMSV_RSRC_CPU_UTIL,2,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* all wmk monitor for cpu kernel utilization - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_all_kernel_util,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,NCS_SRMSV_NODE_ALL,
                      NCS_SRMSV_RSRC_CPU_KERNEL,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* all wmk monitor for cpu user utilization - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_all_user_util,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                      NCS_SRMSV_RSRC_CPU_USER,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* all wmk monitor for cpu user utilization for one min - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_all_user_util_one,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                      NCS_SRMSV_RSRC_CPU_UTIL_ONE,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* all wmk monitor for cpu user utilization for last five min - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_all_user_util_five,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                      NCS_SRMSV_RSRC_CPU_UTIL_FIVE,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* all wmk monitor for cpu user utilization for last fifteen min - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_all_user_util_fifteen,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                      NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* all wmk monitor for physical memory used - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_all_mem_used,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                      NCS_SRMSV_RSRC_MEM_PHYSICAL_USED,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* all wmk monitor for physical memory free - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_all_mem_free,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,NCS_SRMSV_NODE_ALL,
                      NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* all wmk monitor for swap space used - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_all_swap_used,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                      NCS_SRMSV_RSRC_SWAP_SPACE_USED,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* all wmk monitor for swap space free - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_all_swap_free,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                      NCS_SRMSV_RSRC_SWAP_SPACE_FREE,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

/* all wmk monitor for swap space free - runs for 3min */
   fill_wmk_mon_info(&tcd.wmk_all_used_buf_mem,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_ALL,
                      NCS_SRMSV_RSRC_USED_BUFFER_MEM,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

  fill_getval_rsrc_info(&tcd.wmk_getval_cpu_util,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_CPU_UTIL);
  fill_getval_rsrc_info(&tcd.wmk_getval_kernel_util,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_CPU_KERNEL);
  fill_getval_rsrc_info(&tcd.wmk_getval_user_util,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_CPU_USER);
  fill_getval_rsrc_info(&tcd.wmk_getval_util_one,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_CPU_UTIL_ONE);
  fill_getval_rsrc_info(&tcd.wmk_getval_util_five,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_CPU_UTIL_FIVE);
  fill_getval_rsrc_info(&tcd.wmk_getval_util_fifteen,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN);
  fill_getval_rsrc_info(&tcd.wmk_getval_mem_used,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_MEM_PHYSICAL_USED);
  fill_getval_rsrc_info(&tcd.wmk_getval_mem_free,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE);
  fill_getval_rsrc_info(&tcd.wmk_getval_swap_used,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_SWAP_SPACE_USED);
  fill_getval_rsrc_info(&tcd.wmk_getval_swap_free,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_SWAP_SPACE_FREE);
  fill_getval_rsrc_info(&tcd.wmk_getval_used_buf_mem,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_USED_BUFFER_MEM);

  fill_getval_rsrc_info(&tcd.wmk_rem_getval_cpu_util,NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_CPU_UTIL);
  fill_getval_rsrc_info(&tcd.wmk_rem_getval_kernel_util,NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_CPU_KERNEL);
  fill_getval_rsrc_info(&tcd.wmk_rem_getval_user_util,NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_CPU_USER);
  fill_getval_rsrc_info(&tcd.wmk_rem_getval_util_one,NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_CPU_UTIL_ONE);
  fill_getval_rsrc_info(&tcd.wmk_rem_getval_util_five,NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_CPU_UTIL_FIVE);
  fill_getval_rsrc_info(&tcd.wmk_rem_getval_util_fifteen,NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN);
  fill_getval_rsrc_info(&tcd.wmk_rem_getval_mem_used,NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_MEM_PHYSICAL_USED);
  fill_getval_rsrc_info(&tcd.wmk_rem_getval_mem_free,NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE);
  fill_getval_rsrc_info(&tcd.wmk_rem_getval_swap_used,NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_SWAP_SPACE_USED);
  fill_getval_rsrc_info(&tcd.wmk_rem_getval_swap_free,NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_SWAP_SPACE_FREE);
  fill_getval_rsrc_info(&tcd.wmk_rem_getval_used_buf_mem,NCS_SRMSV_NODE_REMOTE,NCS_SRMSV_RSRC_USED_BUFFER_MEM);

  fill_getval_rsrc_info(&tcd.wmk_all_getval_cpu_util,NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_CPU_UTIL);
  fill_getval_rsrc_info(&tcd.wmk_all_getval_kernel_util,NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_CPU_KERNEL);
  fill_getval_rsrc_info(&tcd.wmk_all_getval_user_util,NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_CPU_USER);
  fill_getval_rsrc_info(&tcd.wmk_all_getval_util_one,NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_CPU_UTIL_ONE);
  fill_getval_rsrc_info(&tcd.wmk_all_getval_util_five,NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_CPU_UTIL_FIVE);
  fill_getval_rsrc_info(&tcd.wmk_all_getval_util_fifteen,NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN);
  fill_getval_rsrc_info(&tcd.wmk_all_getval_mem_used,NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_MEM_PHYSICAL_USED);
  fill_getval_rsrc_info(&tcd.wmk_all_getval_mem_free,NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE);
  fill_getval_rsrc_info(&tcd.wmk_all_getval_swap_used,NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_SWAP_SPACE_USED);
  fill_getval_rsrc_info(&tcd.wmk_all_getval_swap_free,NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_SWAP_SPACE_FREE);
  fill_getval_rsrc_info(&tcd.wmk_all_getval_used_buf_mem,NCS_SRMSV_NODE_ALL,NCS_SRMSV_RSRC_USED_BUFFER_MEM);

  fill_mon_info(&tcd.local_monitor_threshold_proc_mem,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,NCS_SRMSV_NODE_LOCAL,
                 NCS_SRMSV_RSRC_PROC_MEM,2,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                 NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,10,
                 NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                 NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

  fill_mon_info(&tcd.local_monitor_threshold_proc_cpu,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,NCS_SRMSV_NODE_LOCAL,
                 NCS_SRMSV_RSRC_PROC_CPU,2,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                 NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,10,
                 NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                 NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

  fill_wmk_mon_info(&tcd.local_monitor_wmk_proc_mem,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,NCS_SRMSV_NODE_LOCAL,
                     NCS_SRMSV_RSRC_PROC_MEM,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

  fill_wmk_mon_info(&tcd.local_monitor_wmk_proc_cpu,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,NCS_SRMSV_NODE_LOCAL,
                     NCS_SRMSV_RSRC_PROC_CPU,1,180,NCS_SRMSV_WATERMARK_DUAL,5);

  fill_getval_rsrc_info(&tcd.wmk_getval_proc_mem,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_PROC_MEM);
  fill_getval_rsrc_info(&tcd.wmk_getval_proc_cpu,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_PROC_CPU);

}


void tet_srma_hdl_clr()
{
    tcd.local_cpu_util_rsrc_hdl=tcd.local_cpu_util_rsrc_hdl2=tcd.local_kernel_util_rsrc_hdl=0;
    tcd.local_user_util_rsrc_hdl=tcd.local_user_util_one_rsrc_hdl=tcd.local_user_util_five_rsrc_hdl=0;
    tcd.local_user_util_fifteen_rsrc_hdl=tcd.local_mem_used_rsrc_hdl=tcd.local_mem_free_rsrc_hdl=0;
    tcd.local_swap_used_rsrc_hdl=tcd.local_swap_free_rsrc_hdl=tcd.local_used_buf_mem_rsrc_hdl=0;
    tcd.local_proc_exit_rsrc_hdl=tcd.local_proc_child_exit_rsrc_hdl=tcd.local_proc_grandchild_exit_rsrc_hdl=0;
    tcd.local_proc_desc_exit_rsrc_hdl=tcd.rsrc_bad_hdl,tcd.dummy_rsrc_hdl=0;
    tcd.remote_cpu_util_rsrc_hdl=tcd.remote_kernel_util_rsrc_hdl=tcd.remote_user_util_rsrc_hdl=0;
    tcd.remote_user_util_one_rsrc_hdl=tcd.remote_user_util_five_rsrc_hdl=tcd.remote_user_util_fifteen_rsrc_hdl=0;
    tcd.remote_mem_used_rsrc_hdl=tcd.remote_mem_free_rsrc_hdl=tcd.remote_swap_used_rsrc_hdl=0;
    tcd.remote_swap_free_rsrc_hdl=tcd.remote_used_buf_mem_rsrc_hdl=0;
    tcd.all_cpu_util_rsrc_hdl=tcd.all_kernel_util_rsrc_hdl=tcd.all_user_util_rsrc_hdl=0;
    tcd.all_user_util_one_rsrc_hdl=tcd.all_user_util_five_rsrc_hdl=tcd.all_user_util_fifteen_rsrc_hdl=0;
    tcd.all_mem_used_rsrc_hdl=tcd.all_mem_free_rsrc_hdl=tcd.all_swap_used_rsrc_hdl=0;
    tcd.all_swap_free_rsrc_hdl=tcd.all_used_buf_mem_rsrc_hdl=0;

    tcd.wmk_cpu_util_rsrc_hdl=tcd.wmk_kernel_util_rsrc_hdl=tcd.wmk_user_util_rsrc_hdl=0;
    tcd.wmk_user_util_one_rsrc_hdl=tcd.wmk_user_util_five_rsrc_hdl=tcd.wmk_user_util_fifteen_rsrc_hdl=0;
    tcd.wmk_mem_used_rsrc_hdl=tcd.wmk_mem_free_rsrc_hdl=tcd.wmk_swap_used_rsrc_hdl=0;
    tcd.wmk_swap_free_rsrc_hdl=tcd.wmk_used_buf_mem_rsrc_hdl=0;
    tcd.wmk_remote_cpu_util_rsrc_hdl=tcd.wmk_remote_kernel_util_rsrc_hdl=tcd.wmk_remote_user_util_rsrc_hdl=0;
    tcd.wmk_remote_user_util_one_rsrc_hdl=tcd.wmk_remote_user_util_five_rsrc_hdl=tcd.wmk_remote_user_util_fifteen_rsrc_hdl=0;
    tcd.wmk_remote_mem_used_rsrc_hdl=tcd.wmk_remote_mem_free_rsrc_hdl=tcd.wmk_remote_swap_used_rsrc_hdl=0;
    tcd.wmk_remote_swap_free_rsrc_hdl=tcd.wmk_remote_used_buf_mem_rsrc_hdl=0;
    tcd.wmk_all_cpu_util_rsrc_hdl=tcd.wmk_all_kernel_util_rsrc_hdl=tcd.wmk_all_user_util_rsrc_hdl=0;
    tcd.wmk_all_user_util_one_rsrc_hdl=tcd.wmk_all_user_util_five_rsrc_hdl=tcd.wmk_all_user_util_fifteen_rsrc_hdl=0;
    tcd.wmk_all_mem_used_rsrc_hdl=tcd.wmk_all_mem_free_rsrc_hdl=tcd.wmk_all_swap_used_rsrc_hdl=0;
    tcd.wmk_all_swap_free_rsrc_hdl=tcd.wmk_all_used_buf_mem_rsrc_hdl=0;
    tcd.local_proc_mem_thre_rsrc_hdl=tcd.local_proc_mem_wmk_rsrc_hdl=0;
    tcd.local_proc_cpu_thre_rsrc_hdl=tcd.local_proc_cpu_wmk_rsrc_hdl=0;


    tcd.disp_flag=0;
    tcd.clbk_count=0;
    tcd.clbk_data.not_type = 10;
    tcd.clbk_data.nodeId = 0;
    tcd.clbk_data.pid = 0;
    tcd.exp_val.val_type = -1;
    tcd.exp_val.scale_type = -1;
    tcd.exp_val.val.f_val = -1;
    tcd.exp_val.val.i_val32 = -1;
    tcd.exp_val.val.u_val32 = -1;
    tcd.exp_val.val.i_val64 = -1;
    tcd.exp_val.val.u_val64 = -1;
    tcd.clbk_data.threshold_val.val_type = -1;
    tcd.clbk_data.threshold_val.scale_type = -1;
    tcd.clbk_data.threshold_val.val.f_val = -1;
    tcd.clbk_data.threshold_val.val.i_val32 = -1;
    tcd.clbk_data.threshold_val.val.u_val32 = -1;
    tcd.clbk_data.threshold_val.val.i_val64 = -1;
    tcd.clbk_data.threshold_val.val.u_val64 = -1;
    tcd.clbk_data.low_val.val_type = -1;
    tcd.clbk_data.low_val.scale_type = -1;
    tcd.clbk_data.low_val.val.f_val = -1;
    tcd.clbk_data.low_val.val.i_val32 = -1;
    tcd.clbk_data.low_val.val.u_val32 = -1;
    tcd.clbk_data.low_val.val.i_val64 = -1;
    tcd.clbk_data.low_val.val.u_val64 = -1;
    tcd.clbk_data.high_val.val_type = -1;
    tcd.clbk_data.high_val.scale_type = -1;
    tcd.clbk_data.high_val.val.f_val = -1;
    tcd.clbk_data.high_val.val.i_val32 = -1;
    tcd.clbk_data.high_val.val.u_val32 = -1;
    tcd.clbk_data.high_val.val.i_val64 = -1;
    tcd.clbk_data.high_val.val.u_val64 = -1;
    tcd.tolerable_val.val_type = -1;
    tcd.tolerable_val.scale_type = -1;
    tcd.tolerable_val.val.f_val = -1;
    tcd.tolerable_val.val.i_val32 = -1;
    tcd.tolerable_val.val.u_val32 = -1;
    tcd.tolerable_val.val.i_val64 = -1;
    tcd.tolerable_val.val.u_val64 = -1;
    tcd.clbk_data.wm_exist = FALSE;
    tcd.clbk_data.wm_type = 100;
    tcd.clbk_data.rsrc_type = 100;
    tcd.clbk_data.rsrc_mon_hdl = 0;
}


void tet_srma_cleanup(SRMA_CLEAN_HDL hdl)
{
    int result;

    switch(hdl)
    {
        case SRMSV_CLEAN_INIT_SUCCESS_T:
             result = tet_test_srmsvFinalize(SRMSV_FIN_SUCCESS_T,TEST_CONFIG_MODE);
             break;

        case SRMSV_CLEAN_INIT_SUCCESS2_T:
             result = tet_test_srmsvFinalize(SRMSV_FIN_SUCCESS3_T,TEST_CONFIG_MODE);
             break;

        case SRMSV_CLEAN_INIT_SUCCESS3_T:
             result = tet_test_srmsvFinalize(SRMSV_FIN_SUCCESS2_T,TEST_CONFIG_MODE);
             break;

        default:
            m_TET_SRMSV_PRINTF("\nINVALID ARGUMENT DEAR\n");
    }
}

                                                                                                                            
/********** ncs_srmsv_initialize ***********/


void srma_it_init_01()
{
  int result;
  printSrmaHead("To verify initialize that the api initializes SRM service");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_NONCONFIG_MODE);
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
  printSrmaResult(result);
}

void srma_it_init_02()
{
  int result;
  printSrmaHead("To verify initialize api when NULL clbk is provided");
  result = tet_test_srmsvInitialize(SRMSV_INIT_NULL_CLBK_T,TEST_NONCONFIG_MODE);
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS3_T);
  printSrmaResult(result);
}

void srma_it_init_03()
{
  int result;
  printSrmaHead("To verify initialize api when both parameters provided are NULL");
  result = tet_test_srmsvInitialize(SRMSV_INIT_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printSrmaResult(result);
}

/********** ncs_srmsv_selection_object_get ***********/

void srma_it_sel_obj_01()
{
  int result;
  printSrmaHead("To verify selection object returns operating system handle");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);

  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final:
  printSrmaResult(result);
}

void srma_it_sel_obj_02()
{
  int result;
  fd_set read_fd;
  struct timeval tv;

  printSrmaHead("To verify selection object returned can be used to detect pending callbacks");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_CPU_KERNEL_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_CPU_UTIL);
/*  system("./srmsv_ex.exe CPU_UTIL &");*/

  result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
  if(result != -1)
     result = TET_PASS;
  else
     result = TET_FAIL;

  kill_srmsv_exe();

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  printSrmaResult(result);
}

void srma_it_sel_obj_03()
{
  int result;
  fd_set read_fd;
  struct timeval tv;

  printSrmaHead("To verify selection object is valid till finalize");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvFinalize(SRMSV_FIN_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
  if(result == -1)
     result = TET_PASS;
  else
     result = TET_FAIL;
  goto final1;

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  printSrmaResult(result);
}

void srma_it_sel_obj_04()
{
  int result;
  printSrmaHead("To verify selection object with uninitilaized handle");
  result = tet_test_srmsvSelection(SRMSV_SEL_BAD_HANDLE2_T,TEST_NONCONFIG_MODE);
  printSrmaResult(result);
}

void srma_it_sel_obj_05()
{
  int result;
  printSrmaHead("To verify selection object with NULL handle");
  result = tet_test_srmsvSelection(SRMSV_SEL_NULL_HDL_T,TEST_NONCONFIG_MODE);
  printSrmaResult(result);
}

void srma_it_sel_obj_06()
{
  int result;
  printSrmaHead("To verify selection object with NULL selection object");
  result = tet_test_srmsvSelection(SRMSV_SEL_INVALID_PARAM_T,TEST_NONCONFIG_MODE);
  printSrmaResult(result);
}

void srma_it_sel_obj_07()
{
  int result;
  printSrmaHead("To verify selection object with NULL parameters");
  result = tet_test_srmsvSelection(SRMSV_SEL_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printSrmaResult(result);
}



/********* ncs_srmsv_stop_monitor ********/


void srma_it_stop_mon_01()
{
  int result;
  int i=0; 
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that stop monitor stops monitoring the resources");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  while(!tcd.disp_flag)
  {
    if(i == 2)
    {
      result = TET_UNRESOLVED;
      break;
    } 
    result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
    result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_CONFIG_MODE);
    tv.tv_sec = 30;
    i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl);
  }

  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvStopmon(SRMSV_STOP_MON_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tcd.disp_flag = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
  if(result == 0)
     result = TET_PASS;
  else
  {
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_CONFIG_MODE);
     result = TET_FAIL;
     if(tcd.disp_flag == 0)
        result = TET_PASS;
  }
 
  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final3:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}
   
void srma_it_stop_mon_02()
{
  int result;
  int i=0; 
  struct timeval tv;
  fd_set read_fd;
  printSrmaHead("To verify that stop monitor stores the data acquired before");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_UNRESOLVED;
       break;
     } 
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_CONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl);
  }
  if(result != TET_PASS)
     goto final2;
 
  tcd.disp_flag = i = 0;
  result = tet_test_srmsvStopmon(SRMSV_STOP_MON_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvRestartmon(SRMSV_RESTART_MON_SUCCESS_T,TEST_NONCONFIG_MODE); 
  if(result != TET_PASS)
     goto final2;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     } 
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_CONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl);
  }

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}
   
void srma_it_stop_mon_03()
{
  int result;
  printSrmaHead("To verify stop monitor api when NULL handle is passed ");
  result = tet_test_srmsvStopmon(SRMSV_STOP_MON_BAD_HANDLE2_T,TEST_NONCONFIG_MODE);
  tet_srma_hdl_clr();
  printSrmaResult(result);
}
   
void srma_it_stop_mon_04()
{
  int result;
  int i=0; 
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify stop monitor is invoked without configuring a monitor ");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStopmon(SRMSV_STOP_MON_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv); /*no callback present because the monitor not started*/
  if(result == 0)
     result = TET_PASS;
  else
  {
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     if(tcd.disp_flag == 0)
        result = TET_PASS;
     else
     {
       result = TET_FAIL;
       goto final2;
     }
  }
 
  result = tet_test_srmsvRestartmon(SRMSV_RESTART_MON_SUCCESS_T,TEST_NONCONFIG_MODE); 
  if(result != TET_PASS)
     goto final2;

  tcd.disp_flag = i = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl);
  }
 
  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}
   
void srma_it_stop_mon_05()
{
  int result;
  printSrmaHead("To verify when stop monitor is invoked twice");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStopmon(SRMSV_STOP_MON_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvStopmon(SRMSV_STOP_MON_BAD_OPERATION_T,TEST_NONCONFIG_MODE);
 
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}


void srma_it_stop_mon_06()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that after invoking stop monitor all pending callbacks are canceled");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;
                                                                                                                           
  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                           
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                           
  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                           
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 60;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
  if(result != 1)
     goto final2;

  result = tet_test_srmsvStopmon(SRMSV_STOP_MON_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  tv.tv_sec = 60;
  result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
  if(result == 0)
     result = TET_PASS;
  else
  {
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     result = TET_FAIL;
     if(tcd.disp_flag == 0)
        result = TET_PASS;
  }
                                                                                                                           
  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}
   

/********* ncs_srmsv_restart_monitor ********/


void srma_it_restartmon_01()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that restart monitor restarts monitoring ");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl);
  }
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvStopmon(SRMSV_STOP_MON_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tcd.disp_flag = i = 0;
  result = tet_test_srmsvRestartmon(SRMSV_RESTART_MON_SUCCESS_T,TEST_NONCONFIG_MODE); 
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl2);
  }
 
  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final3:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_restartmon_02()
{
  int result;
  printSrmaHead("To verify that restart monitor cannot be invoked without stopping monitor");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_CPU_UTIL_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvRestartmon(SRMSV_RESTART_MON_BAD_OPERATION_T,TEST_NONCONFIG_MODE); 
 
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}
   
void srma_it_restartmon_03()
{
  int result;
  printSrmaHead("To verify restart monitor with NULL parameter");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvStopmon(SRMSV_STOP_MON_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_CPU_UTIL_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvRestartmon(SRMSV_RESTART_MON_BAD_HANDLE2_T,TEST_NONCONFIG_MODE); 
 
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}    

void srma_it_restartmon_04()
{
  int result;
  printSrmaHead("To verify restart monitor with uninitialized handle");
  result = tet_test_srmsvRestartmon(SRMSV_RESTART_MON_BAD_HANDLE_T,TEST_NONCONFIG_MODE); 
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_restartmon_05()
{
  int result;
  printSrmaHead("To verify restart monitor with finalized handle");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvStopmon(SRMSV_STOP_MON_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvFinalize(SRMSV_FIN_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvRestartmon(SRMSV_RESTART_MON_FIN_HDL_T,TEST_NONCONFIG_MODE); 
  goto final1;
 
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}


/******* ncs_srmsv_start_rsrc_mon *******/


void srma_it_startmon_01()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify start monitor monitors for the test condition");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl);
  }

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_startmon_02() /*To verify that user can request for monitoring */
{
   srma_it_startmon_01();
}

void srma_it_startmon_03() 
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that same client can request and subscribe for monitor");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_FREE_T,TEST_NONCONFIG_MODE);

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_free.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_free.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_free.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_free_rsrc_hdl);
  }
     
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_startmon_04() 
{
  int result;
  printSrmaHead("To verify that client cannot only subscribe for monitor from start_monitor api");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_INVALID_PARAM2_T,TEST_NONCONFIG_MODE);

  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_startmon_05()
{
  int result;
  printSrmaHead("To verify start_monitor api with uninitialized hdl");
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_startmon_06()
{
  int result;
  printSrmaHead("To verify start monitor with finalized handle");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvFinalize(SRMSV_FIN_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_BAD_HANDLE3_T,TEST_NONCONFIG_MODE);
  goto final1;
 
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_startmon_07()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify start monitor can modify resource and starts monitoring");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 60;
  tv.tv_usec = 0;
                                                                       
  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
  if(result == 0)
     result = TET_PASS;
  else
  {
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     if(tcd.disp_flag == 1)
     {
         result = TET_FAIL;
         goto final2;
     }
  }

  tcd.disp_flag = i = 0;
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED2_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used2.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used2.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_PERCENT,
                                    tcd.local_monitor_mem_used2.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_rsrc_hdl);
  }

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_startmon_08()
{
   srma_it_startmon_07();
}

void srma_it_startmon_09()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify start monitor when monitoring rate is zero");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  fill_mon_info(&tcd.local_monitor_mem_used,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,
                 NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_MEM_PHYSICAL_USED,
                 0,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,NCS_SRMSV_VAL_TYPE_INT32,
                 NCS_SRMSV_STAT_SCALE_KBYTE,700,NCS_SRMSV_VAL_TYPE_INT32,
                 NCS_SRMSV_STAT_SCALE_KBYTE,5,NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_rsrc_hdl);
  }
 
  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
  fill_mon_info(&tcd.local_monitor_mem_used,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_MEM_PHYSICAL_USED,
                 1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,700,
                 NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,5,NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);
}


void srma_it_startmon_10()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify start monitor when monitoring interval is zero");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  fill_mon_info(&tcd.local_monitor_mem_used,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,
                 NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_MEM_PHYSICAL_USED,
                 1,0,NCS_SRMSV_MON_TYPE_THRESHOLD,1,NCS_SRMSV_VAL_TYPE_INT32,
                 NCS_SRMSV_STAT_SCALE_KBYTE,700,NCS_SRMSV_VAL_TYPE_INT32,
                 NCS_SRMSV_STAT_SCALE_KBYTE,5,NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_rsrc_hdl);
  }
 
  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
  fill_mon_info(&tcd.local_monitor_mem_used,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_MEM_PHYSICAL_USED,
                 1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,700,
                 NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,5,NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);
}

void srma_it_startmon_11()
{
  int result;
  printSrmaHead("To verify start rsrc mon when SRMND is not up on remote node");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  tcd.remote_monitor_user_util_one.rsrc_info.node_num = 134927;
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_ERR_NOT_EXIST_T,TEST_CONFIG_MODE);

  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);

final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_startmon_12()
{
  int result;
  printSrmaHead("To verify when tolerable value is on different scale than the threshold value");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_mon_info(&tcd.local_monitor_mem_used,NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR,
                 NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_MEM_PHYSICAL_USED,
                 1,0,NCS_SRMSV_MON_TYPE_THRESHOLD,1,NCS_SRMSV_VAL_TYPE_INT32,
                 NCS_SRMSV_STAT_SCALE_PERCENT,1,NCS_SRMSV_VAL_TYPE_INT32,
                 NCS_SRMSV_STAT_SCALE_KBYTE,200,NCS_SRMSV_THRESHOLD_VAL_IS_NOT_AT,5);

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_INVALID_PARAM_T,TEST_CONFIG_MODE);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
  fill_mon_info(&tcd.local_monitor_mem_used,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,NCS_SRMSV_RSRC_MEM_PHYSICAL_USED,
                 1,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,700,
                 NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_KBYTE,5,NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE,5);
}

void srma_it_startmon_13()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify start rsrc mon when SRMND is not up on remote node");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  tcd.wmk_monitor_mem_free.rsrc_info.node_num = 133135;
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_ERR_NOT_EXIST_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  m_TET_SRMSV_PRINTF(" Bring the ND UP \n");
  getchar();

  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_REM_MEM_FREE_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;
                                                                                                                             
  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
                                                                                                                             
  if(tcd.disp_flag == 1)
  {
     tcd.clbk_data.exp_wm_type = API_SRMSV_Get_Wmk_Val[SRMSV_RSRC_GET_WMK_REM_MEM_FREE_T].wmk_type;
     result = check_wmk_clbk(tcd.clbk_data,tcd.wmk_monitor_mem_free.rsrc_info.node_num,
                             NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE);
  }
                                                                                                                             

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}



/******** ncs_srmsv_stop_rsrc_monitor ******/


void srma_it_stop_rsrcmon_01()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify stop rsrc monitor can stop monitoring and deletes resource data");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl);
  }
  if(result != TET_PASS)
     goto final3;

  tcd.disp_flag = i = 0;
  result = tet_test_srmsvStoprsrcmon(SRMSV_RSRC_STOP_MEM_USED_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_AFTER_STOP_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 60;

  result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
  if(result == 0)
     result = TET_PASS;
  else
  {
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     result = TET_FAIL;
     if(tcd.disp_flag == 0)
         result = TET_PASS;
  }
 
  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final3:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_stop_rsrcmon_02()
{
   srma_it_stop_rsrcmon_01();
}

void srma_it_stop_rsrcmon_03()
{
  int result;
  printSrmaHead("To verify when stop rsrc monitor is invoked without start_rsrcmon");
  result = tet_test_srmsvStoprsrcmon(SRMSV_RSRC_STOP_WITHOUT_START_T,TEST_NONCONFIG_MODE);
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_stop_rsrcmon_04()
{
  int result;
  printSrmaHead("To verify when stop rsrc monitor is invoked with bad handle");
  result = tet_test_srmsvStoprsrcmon(SRMSV_RSRC_STOP_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_stop_rsrcmon_05()
{
  int result;
  struct timeval tv;
  fd_set read_fd;
  printSrmaHead("To verify that stoprsrcmon cancels all pending callbacks");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;
                                                                                                                             
  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 60;
  tv.tv_usec = 0;
                                                                                                                             
  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/
                                                                                                                             
  result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
                                                                                                                             
  result = tet_test_srmsvStoprsrcmon(SRMSV_RSRC_STOP_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
  result = tet_test_srmsvUnsubscrmon(SRMSV_RSRC_UNSUBSCR_MEM_USED_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;  
                                                                                                                             
  sleep(2);
  result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_CONFIG_MODE);
  if(tcd.disp_flag == 1)
    result = TET_FAIL;
                                                                                                                             
  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_stop_rsrcmon_06()
{
  int result;
  struct timeval tv;
  fd_set read_fd;
  int i =0;
  printSrmaHead("To verify stop_rsrc_mon deletes only that particular monitor when multiple monitors exist");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;
                                                                                                                             
  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_FREE_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
   fill_mon_info(&tcd.local_monitor_cpu_util,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                  NCS_SRMSV_RSRC_CPU_UTIL,2,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,100,
                  NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                  NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW,5);

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_CPU_UTIL_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_CPU_UTIL_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 60;
  tv.tv_usec = 0;
                                                                                                                             
  spawn_srmsv_exe(SRMSV_CPU_UTIL);
/*  system("./srmsv_ex.exe CPU_UTIL &");*/
  sleep(5);

  result = tet_test_srmsvStopmon(SRMSV_STOP_MON_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvStoprsrcmon(SRMSV_RSRC_STOP_CPU_UTIL_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_srmsvRestartmon(SRMSV_RESTART_MON_SUCCESS_T,TEST_NONCONFIG_MODE); 
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       if(tcd.clbk_count == 2)
       {
          result = TET_PASS;
          break;
       }
       else
       {
          result = TET_FAIL;
          break;
       }
     }

     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ONE_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;

     if(tcd.disp_flag == 1)
     {
        i =0;
        tcd.disp_flag = 0;
        if(tcd.clbk_data.rsrc_mon_hdl == tcd.local_mem_used_subscr_hdl)
           result = TET_FAIL;
     }

     if(result == TET_FAIL)
       break;
  }

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  fill_mon_info(&tcd.local_monitor_cpu_util,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                 NCS_SRMSV_RSRC_CPU_UTIL,2,180,NCS_SRMSV_MON_TYPE_THRESHOLD,1,
                 NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,70,
                 NCS_SRMSV_VAL_TYPE_INT32,NCS_SRMSV_STAT_SCALE_PERCENT,0,
                 NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW,5);

  tet_srma_hdl_clr();
  printSrmaResult(result);
}


/********** ncs_srmsv_subscr_rsrc_mon ********/


void srma_it_subscr_rsrcmon_01() /*To verify that user can subscribe for the monitor*/
{
   srma_it_startmon_01();
}

void srma_it_subscr_rsrcmon_02()
{
  int result;
  printSrmaHead("To verify when NULL handle is passed to the api");
  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_BAD_HANDLE2_T,TEST_NONCONFIG_MODE);
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_subscr_rsrcmon_03()
{
  int result;
  printSrmaHead("To verify when NULL rsrc_info is passed to the api");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_INVALID_PARAM_T,TEST_NONCONFIG_MODE);

  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_subscr_rsrcmon_04()
{
  int result;
  printSrmaHead("To verify when NULL subscr handle is passed to the api");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_BAD_HANDLE3_T,TEST_NONCONFIG_MODE);

  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_subscr_rsrcmon_05()
{
  int result;
  printSrmaHead("To verify when NULL subscr handle is passed to the api");
  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_subscr_rsrcmon_06()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify when subscr api is called before start rsrcmon api");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 60;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
  if(result == 0)
     result = TET_PASS;
  else
  {
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     result = TET_FAIL;
     if(tcd.disp_flag == 0)
       result = TET_PASS;
  }
  if(result != TET_PASS)
     goto final2;
 
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  tcd.disp_flag = i = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl);
  }
 
  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_subscr_rsrcmon_07()
{
  int result;
  printSrmaHead("To verify when finalized srmsv handle is passed to the api");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_srmsvFinalize(SRMSV_FIN_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_BAD_HANDLE4_T,TEST_NONCONFIG_MODE);

final:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_subscr_rsrcmon_08()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify when user type is requestor in susbcr_rsrc_mon api");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_REQ_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
  if(result == 0)
     result = TET_PASS;
  else
  {
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     result = TET_FAIL;
     if(tcd.disp_flag == 0)
        result = TET_PASS;
  }
  if(result != TET_PASS)
     goto final2;
 
  tcd.disp_flag = i = 0;
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.dummy_subscr_hdl);
  }
 
  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_subscr_rsrcmon_09()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that the subscribers that join after the event has been sent gets the event");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl);
  }
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED2_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tcd.disp_flag = i = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl2);
  }
 
  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final3:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}


void srma_it_subscr_rsrcmon_10()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that each subscriber gets an event when the threshold condition is met");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED2_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl);
  }
  if(result != TET_PASS)
     goto final3;

  tcd.disp_flag = i = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl2);
  }

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final3:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_subscr_rsrcmon_11()
{
  int result;
  printSrmaHead("To verify subscr rsrc mon when SRMND is not up on remote node");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;
                                                                                                                             
  tcd.remote_subscr_mem_used.node_num = 111111;
  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_REM_NOT_EXIST_T,TEST_NONCONFIG_MODE);
                                                                                                                             
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
                                                                                                                             
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}
                                                                                                                             


/******** ncs_srmsv_unsubscr_rsrc_mon *****/


void srma_it_unsubscr_rsrcmon_01() 
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that unsubscribed client doesnot receive any events");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED2_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
       result = TET_FAIL;
       break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl);
  }
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvUnsubscrmon(SRMSV_RSRC_UNSUBSCR_MEM_USED2_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tcd.disp_flag = i = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 60;
  tv.tv_usec = 0;

  result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
  if(result == 0)
     result = TET_PASS;
  else
  {
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     if(tcd.disp_flag == 1)
        result = TET_FAIL;
  }

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final3:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_unsubscr_rsrcmon_02()
{
  int result;
  printSrmaHead("To verify that after invoking unsubscribe clienthandle is no longer valid");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvUnsubscrmon(SRMSV_RSRC_UNSUBSCR_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvUnsubscrmon(SRMSV_RSRC_UNSUBSCR_AFTER_UNSUBSCRIBE_T,TEST_NONCONFIG_MODE);

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);

}

void srma_it_unsubscr_rsrcmon_03()
{
  int result;
  printSrmaHead("To verify unsubscribe when NULL is passed as srmsv_handle");
  result = tet_test_srmsvUnsubscrmon(SRMSV_RSRC_UNSUBSCR_INVALID_PARAM2_T,TEST_NONCONFIG_MODE);
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_unsubscr_rsrcmon_04()
{
  int result;
  printSrmaHead("To verify subscr_rsrc_mon is not invoked before unsubscribe");
  result = tet_test_srmsvUnsubscrmon(SRMSV_RSRC_UNSUBSCR_INVALID_PARAM_T,TEST_NONCONFIG_MODE);
  tet_srma_hdl_clr();
  printSrmaResult(result);
}


/********* ncs_srmsv_dispatch **********/


void srma_it_disp_01()
{
  int result;
  printSrmaHead("To verify dispatch when dispatchFlag is SA_DISPATCH_ONE");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  sleep(5);
  result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ONE_SUCCESS_T,TEST_NONCONFIG_MODE);

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_disp_02()
{
  int result;
  printSrmaHead("To verify dispatch when dispatchFlag is SA_DISPATCH_ALL");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  sleep(5);
  result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_disp_03()
{
  int result;
  printSrmaHead("To verify dispatch when dispatchFlag is SA_DISPATCH_BLOCKING");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  srmsv_createthread(&tcd.client1);

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_NONCONFIG_MODE);
  sleep(10);

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_disp_04()
{
  int result;
  printSrmaHead("To verify dispatch when dispatchFlag is invalid");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_srmsvDispatch(SRMSV_DISPATCH_INVALID_FLAG_T,TEST_NONCONFIG_MODE);

  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_disp_05()
{
  int result;
  printSrmaHead("To verify dispatch with NULL handle and SA_DISPATCH_ONE");
  result = tet_test_srmsvDispatch(SRMSV_DISPATCH_NULL_HDL_T,TEST_NONCONFIG_MODE);
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_disp_06()
{
  int result;
  printSrmaHead("To verify dispatch with NULL handle and SA_DISPATCH_ALL");
  result = tet_test_srmsvDispatch(SRMSV_DISPATCH_NULL_HDL2_T,TEST_NONCONFIG_MODE);
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_disp_07()
{
  int result;
  printSrmaHead("To verify dispatch with NULL handle and SA_DISPATCH_BLOCKING");
  result = tet_test_srmsvDispatch(SRMSV_DISPATCH_NULL_HDL3_T,TEST_NONCONFIG_MODE);
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_disp_08()
{
  int result;
  printSrmaHead("To verify dispatch with uninitialized handle");
  result = tet_test_srmsvDispatch(SRMSV_DISPATCH_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_disp_09()
{
  int result;
  printSrmaHead("To verify dispatch when srmsv_handle has been finalized");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_srmsvFinalize(SRMSV_FIN_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_srmsvDispatch(SRMSV_DISPATCH_BAD_HANDLE3_T,TEST_NONCONFIG_MODE);

final:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}


/****** ncs_srmsv_finalize *********/


void srma_it_fin_01()
{
  int result;
  printSrmaHead("To verify that finalize closes the association between process and service");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_srmsvFinalize(SRMSV_FIN_SUCCESS3_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_srmsvDispatch(SRMSV_DISPATCH_BAD_HANDLE3_T,TEST_NONCONFIG_MODE);

final:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}


void srma_it_fin_02()
{
  int result;
  printSrmaHead("To verify finalize when uninitialized handle is given to the api");
  result = tet_test_srmsvFinalize(SRMSV_FIN_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  tet_srma_hdl_clr();
  printSrmaResult(result);
}
  
void srma_it_fin_03()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that selObj gets invalid after finalize");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvFinalize(SRMSV_FIN_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
  if(result == -1)
     result = TET_PASS;
  else
     result = TET_FAIL;
  goto final1;

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}
  
void srma_it_fin_04()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that monitor is deleted after invoking finalize");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl);
  }
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvFinalize(SRMSV_FIN_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED2_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  tcd.disp_flag = i = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 60;
  tv.tv_usec = 0;

  result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
  if(result == 0)
     result = TET_PASS;
  else
  {
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     result = TET_FAIL;
     if(tcd.disp_flag == 0)
        result = TET_PASS;
  }
  goto final2;

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final3:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}
  
/******* NCS_SRMSV_CALLBACK *******/

void srma_it_clbk_01()
{
  srma_it_startmon_01();
}
  
void srma_it_clbk_02()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that event doesnot arrive when NULL callback");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvInitialize(SRMSV_INIT_NULL_CLBK_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED3_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag  == 1)
  {
      fill_exp_value(tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.threshold_val,
                     tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.tolerable_val); 

      result = check_threshold_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_STAT_SCALE_KBYTE,
                                    tcd.local_monitor_mem_used.monitor_data.mon_cfg.threshold.condition,
                                    tcd.exp_val,tcd.local_mem_used_subscr_hdl);
  }
  if(result != TET_PASS)
     goto final3;

  tcd.disp_flag = i = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client2, &read_fd);
  tv.tv_sec = 60;

  result = select(tcd.sel_client2 + 1, &read_fd, NULL, NULL, &tv);
  if(result == 0)
     result = TET_PASS;
  else
  {
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS3_T,TEST_NONCONFIG_MODE);
     if(tcd.disp_flag == 1)
        result = TET_FAIL;
  }

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final3:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS3_T);
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}
  

/******* single node test cases ********/


void srma_it_onenode_02()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that event doesnot arrive when the client is REQUESTOR");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_MEM_USED2_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;
                                                                                                                            
  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(result != TET_PASS)
     goto final3;

  tcd.disp_flag = i = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
  if(result == 0)
    result = TET_PASS;
  else
  {
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     result = TET_FAIL;
     if(tcd.disp_flag == 0)
        result = TET_PASS;
  }

final3:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_onenode_03()
{
   srma_it_startmon_03();
}

void srma_it_onenode_04()
{
   srma_it_onenode_02();
}

void srma_it_onenode_05()
{
   srma_it_onenode_02();
}

void srma_it_onenode_06()
{
  srma_it_wmk_02();
}

void srma_it_onenode_07()
{
  int result=TET_UNRESOLVED;
  printSrmaHead("To verify that user can request/subscribe from local/remote node with MON_TYPE_LEAKY_BUCKET **** NOT SUPPORTED ****");
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_onenode_08()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that node id is ignored when location is NODE_LOCAL");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;
                                                                                                                            
  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                            
  tcd.local_monitor_mem_free.rsrc_info.node_num = 20;
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_FREE_T,TEST_NONCONFIG_MODE);
                                                                                                                            
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;
                                                                                                                            
  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_onenode_09()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that pid is ignored when rsrc_type is other than PROC_EXIT");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;
                                                                                                                            
  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                            
  tcd.local_monitor_mem_free.rsrc_info.pid = 8;
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_MEM_FREE_T,TEST_NONCONFIG_MODE);
                                                                                                                            
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;
                                                                                                                            
  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
                                                                                                                            
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);

}

void srma_it_onenode_10()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that user can monitor CPU_KERNEL utilization");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_CPU_KERNEL_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  spawn_srmsv_exe(SRMSV_CPU_UTIL);

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

  kill_srmsv_exe();
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_onenode_11()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that user can monitor CPU_USER utilization");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_CPU_USER_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_USER_UTIL_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_CPU_UTIL);
/*  system("./srmsv_ex.exe CPU_UTIL &");*/

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_onenode_12()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that user can monitor CPU_UTIL utilization");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_CPU_UTIL_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_CPU_UTIL_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_onenode_13()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that user can monitor load avg for the last one min");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_UTIL_ONE_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_UTIL_ONE_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_CPU_UTIL);
/*  system("./srmsv_ex.exe CPU_UTIL &");*/

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_onenode_14()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that user can monitor load avg for the last five min");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_UTIL_FIVE_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_UTIL_FIVE_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_CPU_UTIL);
/*  system("./srmsv_ex.exe CPU_UTIL &");*/

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_onenode_15()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that user can monitor load avg for the last fifteen min");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_UTIL_FIFTEEN_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_UTIL_FIFTEEN_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  spawn_srmsv_exe(SRMSV_CPU_UTIL);
/*  system("./srmsv_ex.exe CPU_UTIL &");*/

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}


void srma_it_onenode_17()
{
   srma_it_startmon_03();
}

void srma_it_onenode_18()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that user can monitor for the swap space used");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_SWAP_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_SWAP_USED_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_onenode_19()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that user can monitor for the free swap space");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_SWAP_FREE_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_SWAP_FREE_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_onenode_20()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that user can monitor for the used buf memory");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_USED_BUF_MEM_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_USED_BUF_MEM_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_onenode_21()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that user can monitor for process exit");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_PROC_EXIT_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_PROC_EXIT_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag ==1)
  {
     if(tcd.clbk_data.not_type != SRMSV_CBK_NOTIF_PROCESS_EXPIRED || tcd.clbk_data.pid != tcd.local_monitor_proc_exit.rsrc_info.pid) 
        result = TET_FAIL;
  }

  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvUnsubscrmon(SRMSV_RSRC_UNSUBSCR_PROC_EXIT_T,TEST_CONFIG_MODE);

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void  srma_it_onenode_22()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify for process and its child exit");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  tcd.childId = fork();
  if (tcd.childId == 0)
  {
     printf("\t\tI am %d , child of %d \n",getpid(),getppid());
     sleep(100);
     printf("not seen\n");
  }
  else if (tcd.childId < 0)
  {
     printf("\t\tChild not created\n");
  }
  else
  {
     printf("In parent\n");
     printf("I am %d \n",getpid());
     printf("pid to kill is %d \n",tcd.childId);
     tcd.local_monitor_proc_child_exit.rsrc_info.pid = getpid();
     tcd.local_subscr_proc_child_exit.pid = getpid();
     result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_PROC_CHILD_EXIT_T,TEST_CONFIG_MODE);
     if(result != TET_PASS)
         goto final2;

     result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_PROC_CHILD_EXIT_T,TEST_CONFIG_MODE);
     if(result != TET_PASS)
         goto final2;

     FD_ZERO(&read_fd);
     FD_SET(tcd.sel_client1, &read_fd);
     tv.tv_sec = 30;
     tv.tv_usec = 0;

     while(!tcd.disp_flag)
     {
        if(i == 2)
        {
           result = TET_FAIL;
           break;
        }
        result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
        kill(tcd.childId,SIGKILL);
        printf("child killed\n");
        result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
        tv.tv_sec = 30;
        i++;
     }

     if(tcd.disp_flag ==1)
     {
        if(tcd.clbk_data.not_type != SRMSV_CBK_NOTIF_PROCESS_EXPIRED || tcd.clbk_data.pid != tcd.childId) 
           result = TET_FAIL;
     }

     if(result != TET_PASS)
        goto final2;

     result = tet_test_srmsvUnsubscrmon(SRMSV_RSRC_UNSUBSCR_PROC_CHILD_EXIT_T,TEST_CONFIG_MODE);
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}
                                                                                                                                                                      
void srma_it_onenode_23()
{
  int result;
  int i=0;
  int grandChildId=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify for process and its grandchild exit");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  tcd.childId = fork();
  if (tcd.childId == 0)
  {
     printf("\t\tI am %d , child of %d \n",getpid(),getppid());
     grandChildId = fork();
     if (grandChildId == 0)
     {
         printf("\t\t\t\tI am %d , child of %d \n",getpid(),getppid());
         sleep(100);
         printf("not seen\n");
     }
     else if (grandChildId < 0)
     {
         printf("\t\t\t\t grandchild not created\n");
     }
     else
     {
         printf("\t\tpid to kill is %d \n",grandChildId);
         sleep(40);
         kill(grandChildId,SIGKILL);
         printf("\t\tGrand child killed\n");
         printf("\t\tGrand child execution finished\n");
         sleep(100);
     }
   }
   else if (tcd.childId < 0)
   {
      printf("\t\tChild not created\n");
   }
   else
   {
     printf("In parent\n");
     printf("I am %d \n",getpid());
     tcd.local_monitor_proc_grandchild_exit.rsrc_info.pid = getpid();
     tcd.local_subscr_proc_grandchild_exit.pid = getpid();
     result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_PROC_GRANDCHILD_EXIT_T,TEST_CONFIG_MODE);
     if(result != TET_PASS)
        goto final2;

     result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_PROC_GRANDCHILD_EXIT_T,TEST_CONFIG_MODE);
     if(result != TET_PASS)
        goto final2;

     FD_ZERO(&read_fd);
     FD_SET(tcd.sel_client1, &read_fd);
     tv.tv_sec = 30;
     tv.tv_usec = 0;

     while(!tcd.disp_flag)
     {
        if(i == 2)
        {
           result = TET_FAIL;
           break;
        }
        select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
        result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
        tv.tv_sec = 30;
        i++;
     }

     if(tcd.disp_flag ==1)
     {
        if(tcd.clbk_data.not_type != SRMSV_CBK_NOTIF_PROCESS_EXPIRED || tcd.clbk_data.pid != grandChildId) 
           result = TET_FAIL;
     }

     if(result != TET_PASS)
        goto final2;

     final2:
        tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
     final1:
        tet_srma_hdl_clr();
        printSrmaResult(result);
   }

  sleep(100);
}

void srma_it_onenode_24()
{
  int result;
  int i=0;
  int grandChildId=0;
  int desc1;
  int desc2;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify for process and its descendants exit");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;
                                                                                                                           
  tcd.childId = fork();
  if (tcd.childId == 0)
  {
     printf("\tI am %d , child of %d \n",getpid(),getppid());
     grandChildId = fork();
     if (grandChildId == 0)
     {
        printf("\t\tI am %d , child of %d \n",getpid(),getppid());
        desc1 = fork();
        if (desc1 == 0)
        {
           printf("\t\t\tI am %d , child of %d \n",getpid(),getppid());
           desc2 = fork();
           if (desc2 == 0)
           {
              printf("\t\t\tI am %d , child of %d \n",getpid(),getppid());
              sleep(100);
           }
           else if (desc2 < 0)
           {
              printf("\t\t\t desc2 not created\n");
           }
           else
           {
              printf("\t\t\tpid to kill is %d \n",desc2);
              sleep(10);
              kill(desc2,SIGKILL);
              printf("\t\t\tdesc2 killed\n");
              sleep(100);
           }
                                                                                                                           
                                                                                                                           
        }
        else if (desc1 < 0)
        {
           printf("\t\t desc1 not created\n");
        }
        else
        {
           sleep(200);
           printf("\t\t Grand child exit \n");
        }
     }
     else if (grandChildId < 0)
     {
        printf("\t\t\t\t grandchild not created\n");
     }
     else
     {
       sleep(200);
     }
  }
  else if (tcd.childId < 0)
  {
     printf("\t\tChild not created\n");
  }
  else
  {
     printf("In parent\n");
     printf("I am %d \n",getpid());
     tcd.local_monitor_proc_desc_exit.rsrc_info.pid = getpid();
     tcd.local_subscr_proc_desc_exit.pid = getpid();
     result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_PROC_DESC_EXIT_T,TEST_CONFIG_MODE);
     if(result != TET_PASS)
        goto final2;
                                                                                                                           
     result = tet_test_srmsvSubscrrsrcmon(SRMSV_RSRC_SUBSCR_PROC_DESC_EXIT_T,TEST_CONFIG_MODE);
     if(result != TET_PASS)
        goto final2;
                                                                                                                           
     FD_ZERO(&read_fd);
     FD_SET(tcd.sel_client1, &read_fd);
     tv.tv_sec = 30;
     tv.tv_usec = 0;

     while(!tcd.disp_flag)
     {
        if(i == 2)
        {
           result = TET_FAIL;
           break;
        }
        result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, NULL);
        result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
        tv.tv_sec = 30;
        i++;
     }

     if(tcd.disp_flag ==1)
     {
        if(tcd.clbk_data.not_type != SRMSV_CBK_NOTIF_PROCESS_EXPIRED || tcd.clbk_data.pid != desc2) 
           result = TET_FAIL;
     }

     if(result != TET_PASS)
        goto final2;
                                                                                                                           
     result = tet_test_srmsvUnsubscrmon(SRMSV_RSRC_UNSUBSCR_PROC_DESC_EXIT_T,TEST_CONFIG_MODE);
                                                                                                                           
     final2:
       tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
     final1:
       tet_srma_hdl_clr();
       printSrmaResult(result);
  }
  sleep(100);
}


/******* WATERMARK MONITOR TESTS ********/


void srma_it_wmk_01()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that only one configuration for watermark monitor is accepted");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_ALL_CPU_UTIL_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_CPU_UTIL_T,TEST_CONFIG_MODE);

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag == 1)
  {
     if(tcd.clbk_data.not_type != SRMSV_CBK_NOTIF_WM_CFG_ALREADY_EXIST || tcd.clbk_data.nodeId != tcd.nodeId || 
                             tcd.clbk_data.rsrc_type != NCS_SRMSV_RSRC_CPU_UTIL)
        result = TET_FAIL;
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_02()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify obtaining wmk values for cpu utilization");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_CPU_UTIL_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  sleep(5);
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_CPU_UTIL_T,TEST_NONCONFIG_MODE);

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag == 1)
  {
     tcd.clbk_data.exp_wm_type = NCS_SRMSV_WATERMARK_DUAL;
     result = check_wmk_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_RSRC_CPU_UTIL);
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_03()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify obtaining wmk values for kernel utilization");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_CPU_KERNEL_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  sleep(5);
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_KERNEL_UTIL_T,TEST_NONCONFIG_MODE);

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag == 1)
  {
     tcd.clbk_data.exp_wm_type = NCS_SRMSV_WATERMARK_LOW;
     result = check_wmk_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_RSRC_CPU_KERNEL);
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_04()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify obtaining wmk values for user utilization");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_CPU_USER_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  sleep(5);
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_USER_UTIL_T,TEST_NONCONFIG_MODE);

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag == 1)
  {
     tcd.clbk_data.exp_wm_type = NCS_SRMSV_WATERMARK_HIGH;
     result = check_wmk_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_RSRC_CPU_USER);
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_05()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify obtaining wmk values for last one min cpu utilization");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_UTIL_ONE_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  sleep(5);
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_UTIL_ONE_T,TEST_NONCONFIG_MODE);

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag == 1)
  {
     tcd.clbk_data.exp_wm_type = NCS_SRMSV_WATERMARK_LOW;
     result = check_wmk_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_RSRC_CPU_UTIL_ONE);
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_06()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify obtaining wmk values for last five min cpu utilization");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_UTIL_FIVE_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  sleep(5);
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_UTIL_FIVE_T,TEST_NONCONFIG_MODE);

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag == 1)
  {
     tcd.clbk_data.exp_wm_type = NCS_SRMSV_WATERMARK_HIGH;
     result = check_wmk_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_RSRC_CPU_UTIL_FIVE);
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_07()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify obtaining wmk values for last fifteen min cpu utilization");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_UTIL_FIFTEEN_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  sleep(5);
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_UTIL_FIFTEEN_T,TEST_NONCONFIG_MODE);

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag == 1)
  {
     tcd.clbk_data.exp_wm_type = NCS_SRMSV_WATERMARK_LOW;
     result = check_wmk_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN);
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_08()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify obtaining wmk values for physical mem used");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_MEM_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  sleep(5);
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_MEM_USED_T,TEST_NONCONFIG_MODE);

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag == 1)
  {
     tcd.clbk_data.exp_wm_type = NCS_SRMSV_WATERMARK_DUAL;
     result = check_wmk_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_RSRC_MEM_PHYSICAL_USED);
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_09()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify obtaining wmk values for physical mem free");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_MEM_FREE_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  sleep(5);
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_MEM_FREE_T,TEST_NONCONFIG_MODE);

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag == 1)
  {
     tcd.clbk_data.exp_wm_type = NCS_SRMSV_WATERMARK_DUAL;
     result = check_wmk_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE);
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_10()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify obtaining wmk values for swap space used");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_SWAP_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  sleep(5);
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_SWAP_USED_T,TEST_NONCONFIG_MODE);

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag == 1)
  {
     tcd.clbk_data.exp_wm_type = NCS_SRMSV_WATERMARK_DUAL;
     result = check_wmk_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_RSRC_SWAP_SPACE_USED);
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_11()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify obtaining wmk values for swap free");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_SWAP_FREE_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  sleep(5);
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_SWAP_FREE_T,TEST_NONCONFIG_MODE);

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag == 1)
  {
     tcd.clbk_data.exp_wm_type = NCS_SRMSV_WATERMARK_DUAL;
     result = check_wmk_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_RSRC_SWAP_SPACE_FREE);
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_12()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify obtaining wmk values for used buff memory");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_USED_BUF_MEM_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  sleep(5);
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_USED_BUF_MEM_T,TEST_NONCONFIG_MODE);

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag == 1)
  {
     tcd.clbk_data.exp_wm_type = NCS_SRMSV_WATERMARK_DUAL;
     result = check_wmk_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_RSRC_USED_BUFFER_MEM);
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_13()
{
  int result;
  printSrmaHead("To verify get wmk value api with NULL handle");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_UTIL_ONE_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_BAD_HANDLE_T,TEST_NONCONFIG_MODE);

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_14()
{
  int result;
  printSrmaHead("To verify get wmk value api with finalized handle");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_UTIL_ONE_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvFinalize(SRMSV_FIN_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_BAD_HANDLE2_T,TEST_NONCONFIG_MODE);
  goto final1;

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_15()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that pending wmk callbacks are cleaned after finalize");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
   
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_MEM_FREE_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  sleep(5);
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_MEM_FREE_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_MEM_FREE2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }
  if(tcd.disp_flag == 1)
  {
     tcd.clbk_data.exp_wm_type = NCS_SRMSV_WATERMARK_DUAL;
     result = check_wmk_clbk(tcd.clbk_data,tcd.nodeId,NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE);
  }
  if(result != TET_PASS)
     goto final3;

  result = tet_test_srmsvFinalize(SRMSV_FIN_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  tcd.disp_flag = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
  if(result != -1)
      result = TET_FAIL;
  else 
      result = TET_PASS;

  goto final2;

final3:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_16()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify obtaining wmk values without configuring watermark monitor");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  sleep(5);
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_MEM_FREE_T,TEST_NONCONFIG_MODE);

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_PASS;
        break; 
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

  if(tcd.disp_flag == 1)
  {
     if(tcd.clbk_data.not_type != SRMSV_CBK_NOTIF_WATERMARK_VAL||tcd.clbk_data.rsrc_type!=NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE ||
        tcd.clbk_data.nodeId != tcd.nodeId || tcd.clbk_data.wm_exist != 0) 
        result = TET_FAIL;
  }
  else
     result = TET_FAIL;

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_17()
{
  int result;
  printSrmaHead("To verify when remote ND is not up");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  tcd.wmk_rem_getval_used_buf_mem.node_num = 134927;
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_REM_NOT_EXIST_T,TEST_NONCONFIG_MODE);

  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_18()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  printSrmaHead("To verify that expired event arrives after monitoring interval");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
   
  fill_wmk_mon_info(&tcd.wmk_monitor_swap_used,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                     NCS_SRMSV_RSRC_SWAP_SPACE_USED,1,10,NCS_SRMSV_WATERMARK_DUAL,5);
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_MON_SWAP_USED_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client3, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client3 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS2_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

  if(tcd.disp_flag == 1)
  {
     if(tcd.clbk_data.not_type != SRMSV_CBK_NOTIF_RSRC_MON_EXPIRED)
       result = TET_FAIL;
  }

final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS2_T);
final1:
  fill_wmk_mon_info(&tcd.wmk_monitor_swap_used,NCS_SRMSV_USER_REQUESTOR,NCS_SRMSV_NODE_LOCAL,
                     NCS_SRMSV_RSRC_SWAP_SPACE_USED,1,180,NCS_SRMSV_WATERMARK_DUAL,5);
  tet_srma_hdl_clr();
  printSrmaResult(result);
}


void srma_it_threshold_proc_mem()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  FILE *fp;
  char buf[2000];
  int pid=-1;
  char *spawned_process[]= {"srmsv_ex.exe"};
  printSrmaHead("To verify that user can monitor process memory threshold condition");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  system("./srmsv_ex.exe MEM &");*/
  if(result == -1)
  {
     m_TET_SRMSV_PRINTF("system() call failed with -1 -- exiting test\n");
     result = TET_UNRESOLVED;
     goto final2;
  }

  /*sprintf(buf, "ps -e -o pid -o args | grep %s | grep -v grep | grep -v xterm | grep -v gdb | grep -v ddd | awk '{print $1}'", spawned_process[0]);*/
  sprintf(buf, "pgrep %s", spawned_process[0]);
  fp = popen(buf, "r");
  if (fp == NULL)
  {
     fprintf(stderr, "Dieing");
     exit(-1);
  }
  memset(buf,'\0',1255);
  fread(buf,1,6,fp);

  pid = atoi(buf);
  pclose(fp);
  m_TET_SRMSV_PRINTF("process id %d \n",pid);

  tcd.local_monitor_threshold_proc_mem.rsrc_info.pid = pid;
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_PROC_MEM_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_proc_mem()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  FILE *fp;
  char buf[2000];
  int pid=-1;
  char *spawned_process[]= {"srmsv_ex.exe"};
  printSrmaHead("To verify that user can monitor process memory wmk condition");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  spawn_srmsv_exe(SRMSV_MEM_UTIL);
/*  result = system("./srmsv_ex.exe MEM &");*/
  if(result == -1)
  {
     m_TET_SRMSV_PRINTF("system() call failed with -1 -- exiting test\n");
     result = TET_UNRESOLVED;
     goto final2;
  }

  /*sprintf(buf, "ps -e -o pid -o args | grep %s | grep -v grep | grep -v xterm | grep -v gdb | grep -v ddd | awk '{print $1}'", spawned_process[0]);*/
  sprintf(buf, "pgrep %s", spawned_process[0]);
  fp = popen(buf, "r");
  if (fp == NULL)
  {
     fprintf(stderr, "Dieing");
     exit(-1);
  }
  memset(buf,'\0',1255);
  fread(buf,1,6,fp);

  pid = atoi(buf);
  pclose(fp);
  m_TET_SRMSV_PRINTF("process id %d \n",pid);
  tcd.local_monitor_wmk_proc_mem.rsrc_info.pid = pid;
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_PROC_MEM_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  sleep(5);
  tcd.wmk_getval_proc_mem.pid = pid;
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_PROC_MEM_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
  {
     kill_srmsv_exe();
     /*system("sudo killall -9 srmsv_ex.exe");*/
     goto final2;
  }

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_threshold_proc_cpu()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  FILE *fp;
  char buf[2000];
  int pid=-1;
  char *spawned_process[]= {"srmsv_ex.exe"};
  printSrmaHead("To verify that user can monitor process cpu utilization threshold condition");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  spawn_srmsv_exe(SRMSV_CPU_UTIL);
/*  system("./srmsv_ex.exe CPU_UTIL &");*/
  if(result == -1)
  {
     m_TET_SRMSV_PRINTF("system() call failed with -1 -- exiting test\n");
     result = TET_UNRESOLVED;
     goto final2;
  }

  /*sprintf(buf, "ps -e -o pid -o args | grep %s | grep -v grep | grep -v xterm | grep -v gdb | grep -v ddd | awk '{print $1}'", spawned_process[0]);*/
  sprintf(buf, "pgrep %s", spawned_process[0]);
  fp = popen(buf, "r");
  if (fp == NULL)
  {
     fprintf(stderr, "Dieing");
     exit(-1);
  }
  memset(buf,'\0',1255);
  fread(buf,1,6,fp);

  pid = atoi(buf);
  pclose(fp);
  m_TET_SRMSV_PRINTF("process id %d \n",pid);

  tcd.local_monitor_threshold_proc_cpu.rsrc_info.pid = pid;
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_MON_PROC_CPU_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
  {
     kill_srmsv_exe();
    /* system("sudo killall -9 srmsv_ex.exe");*/
     goto final2;
  }

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }  

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

void srma_it_wmk_proc_cpu()
{
  int result;
  int i=0;
  fd_set read_fd;
  struct timeval tv;
  FILE *fp;
  char buf[2000];
  int pid=-1;
  char *spawned_process[]= {"srmsv_ex.exe"};
  printSrmaHead("To verify that user can monitor process cpu utilization wmk condition");
  result = tet_test_srmsvInitialize(SRMSV_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_srmsvSelection(SRMSV_SEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  spawn_srmsv_exe(SRMSV_CPU_UTIL);
/*  system("./srmsv_ex.exe CPU_UTIL &");*/
  if(result == -1)
  {
     m_TET_SRMSV_PRINTF("system() call failed with -1 -- exiting test\n");
     result = TET_UNRESOLVED;
     goto final2;
  }

  /*sprintf(buf, "ps -e -o pid -o args | grep %s | grep -v grep | grep -v xterm | grep -v gdb | grep -v ddd | awk '{print $1}'", spawned_process[0]);*/
  sprintf(buf, "pgrep %s", spawned_process[0]);
  fp = popen(buf, "r");
  if (fp == NULL)
  {
     fprintf(stderr, "Dieing");
     exit(-1);
  }
  memset(buf,'\0',1255);
  fread(buf,1,6,fp);

  pid = atoi(buf);
  pclose(fp);
  m_TET_SRMSV_PRINTF("process id %d \n",pid);

  tcd.local_monitor_wmk_proc_cpu.rsrc_info.pid = pid;
  result = tet_test_srmsvStartrsrcmon(SRMSV_RSRC_WMK_PROC_CPU_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
  {
     kill_srmsv_exe();
    /* system("sudo killall -9 srmsv_ex.exe");*/
     goto final2;
  }

  tcd.wmk_getval_proc_cpu.pid = pid;
  result = tet_test_srmsvGetWmkVal(SRMSV_RSRC_GET_WMK_VAL_PROC_CPU_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  FD_ZERO(&read_fd);
  FD_SET(tcd.sel_client1, &read_fd);
  tv.tv_sec = 30;
  tv.tv_usec = 0;

  while(!tcd.disp_flag)
  {
     if(i == 2)
     {
        result = TET_FAIL;
        break;
     }
     result = select(tcd.sel_client1 + 1, &read_fd, NULL, NULL, &tv);
     result = tet_test_srmsvDispatch(SRMSV_DISPATCH_ALL_SUCCESS_T,TEST_NONCONFIG_MODE);
     tv.tv_sec = 30;
     i++;
  }

  kill_srmsv_exe();
/*  system("sudo killall -9 srmsv_ex.exe");*/
final2:
  tet_srma_cleanup(SRMSV_CLEAN_INIT_SUCCESS_T);
final1:
  tet_srma_hdl_clr();
  printSrmaResult(result);
}

/*********** INPUTS AND CONFIG *********/

void tet_srmsv_get_inputs(TET_SRMSV_INST *inst)
{
   char *tmp_ptr=NULL;
   memset(inst,'\0',sizeof(TET_SRMSV_INST));

   tmp_ptr = (char *) getenv("TET_SRMSV_TEST_CASE_NUM");
   if(tmp_ptr)
   {
      inst->test_case_num = atoi(tmp_ptr);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_SRMSV_TEST_LIST");
   if(tmp_ptr)
   {
      inst->test_list = atoi(tmp_ptr);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_SRMSV_REM_NODE_ID");
   if(tmp_ptr)
   {
      inst->rem_node_num = atoi(tmp_ptr);
      tcd.rem_nodeId = inst->rem_node_num; 
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_SRMSV_NUM_ITER");
   if(tmp_ptr)
   {
      inst->num_of_iter = atoi(tmp_ptr);
      tmp_ptr = NULL;
   }


   inst->node_id = m_NCS_GET_NODE_ID;
   tcd.nodeId = inst->node_id;
   
   tmp_ptr = (char *) getenv("TET_SRMSV_PID");
   if(tmp_ptr)
   {
      inst->pid = atoi(tmp_ptr);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_SRMSV_NUM_SYS");
   if(tmp_ptr)
   {
      inst->num_sys = atoi(tmp_ptr);
      tmp_ptr = NULL;
      tcd.num_sys = inst->num_sys;
   }
}

void tet_srmsv_verify_pid(TET_SRMSV_INST *inst)
{
   if(inst->pid)
   {
      tcd.local_monitor_proc_exit.rsrc_info.pid = inst->pid;
      tcd.local_subscr_proc_exit.pid = inst->pid;
   }
   else
   {
      m_TET_SRMSV_PRINTF("\n Needs a process id to run the test case\n");
      exit(0);
   }
}

void tet_srmsv_verify_numsys(TET_SRMSV_INST *inst)
{
   if(!inst->num_sys)
   {
      m_TET_SRMSV_PRINTF("\n Please enter the number of systems in the cluster\n");
      exit(0);
   }
}

void tet_srmsv_verify_node_num(TET_SRMSV_INST *inst)
{
   if(inst->rem_node_num)
   {
      tcd.remote_monitor_cpu_util.rsrc_info.node_num = inst->rem_node_num;
      tcd.remote_monitor_kernel_util.rsrc_info.node_num = inst->rem_node_num;
      tcd.remote_monitor_user_util.rsrc_info.node_num = inst->rem_node_num;
      tcd.remote_monitor_user_util_one.rsrc_info.node_num = inst->rem_node_num;
      tcd.remote_monitor_user_util_five.rsrc_info.node_num = inst->rem_node_num;
      tcd.remote_monitor_user_util_fifteen.rsrc_info.node_num = inst->rem_node_num;
      tcd.remote_monitor_mem_used.rsrc_info.node_num = inst->rem_node_num;
      tcd.remote_monitor_mem_free.rsrc_info.node_num = inst->rem_node_num;
      tcd.remote_monitor_swap_used.rsrc_info.node_num = inst->rem_node_num;
      tcd.remote_monitor_swap_free.rsrc_info.node_num = inst->rem_node_num;
      tcd.remote_monitor_used_buf_mem.rsrc_info.node_num = inst->rem_node_num;

      tcd.remote_subscr_cpu_util.node_num = inst->rem_node_num;
      tcd.remote_subscr_kernel_util.node_num = inst->rem_node_num;
      tcd.remote_subscr_user_util.node_num = inst->rem_node_num;
      tcd.remote_subscr_util_one.node_num = inst->rem_node_num;
      tcd.remote_subscr_util_five.node_num = inst->rem_node_num;
      tcd.remote_subscr_util_fifteen.node_num = inst->rem_node_num;
      tcd.remote_subscr_mem_used.node_num = inst->rem_node_num;
      tcd.remote_subscr_mem_free.node_num = inst->rem_node_num;
      tcd.remote_subscr_swap_used.node_num = inst->rem_node_num;
      tcd.remote_subscr_swap_free.node_num = inst->rem_node_num;
      tcd.remote_subscr_used_buf_mem.node_num = inst->rem_node_num;

      tcd.wmk_remote_cpu_util.rsrc_info.node_num = inst->rem_node_num;
      tcd.wmk_remote_kernel_util.rsrc_info.node_num = inst->rem_node_num;
      tcd.wmk_remote_user_util.rsrc_info.node_num = inst->rem_node_num;
      tcd.wmk_remote_user_util_one.rsrc_info.node_num = inst->rem_node_num;
      tcd.wmk_remote_user_util_five.rsrc_info.node_num = inst->rem_node_num;
      tcd.wmk_remote_user_util_fifteen.rsrc_info.node_num = inst->rem_node_num;
      tcd.wmk_remote_mem_used.rsrc_info.node_num = inst->rem_node_num;
      tcd.wmk_remote_mem_free.rsrc_info.node_num = inst->rem_node_num;
      tcd.wmk_remote_swap_used.rsrc_info.node_num = inst->rem_node_num;
      tcd.wmk_remote_swap_free.rsrc_info.node_num = inst->rem_node_num;
      tcd.wmk_remote_used_buf_mem.rsrc_info.node_num = inst->rem_node_num;

      tcd.wmk_rem_getval_cpu_util.node_num = inst->rem_node_num;
      tcd.wmk_rem_getval_kernel_util.node_num = inst->rem_node_num;
      tcd.wmk_rem_getval_user_util.node_num = inst->rem_node_num;
      tcd.wmk_rem_getval_util_one.node_num = inst->rem_node_num;
      tcd.wmk_rem_getval_util_five.node_num = inst->rem_node_num;
      tcd.wmk_rem_getval_util_fifteen.node_num = inst->rem_node_num;
      tcd.wmk_rem_getval_mem_used.node_num = inst->rem_node_num;
      tcd.wmk_rem_getval_mem_free.node_num = inst->rem_node_num;
      tcd.wmk_rem_getval_swap_used.node_num = inst->rem_node_num;
      tcd.wmk_rem_getval_swap_free.node_num = inst->rem_node_num;
      tcd.wmk_rem_getval_used_buf_mem.node_num = inst->rem_node_num;
  }
  else
  {
      m_TET_SRMSV_PRINTF("\nNeeds a node num to run the test case\n");
      exit(0);
  }
}


struct tet_testlist tet_srmsv_single_node_testlist[]= {
   {srma_it_init_01,1,0},
   {srma_it_init_02,2,0},
   {srma_it_init_03,3,0},

   {srma_it_sel_obj_01,4,0},
   {srma_it_sel_obj_02,5,0},
   {srma_it_sel_obj_03,6,0},
   {srma_it_sel_obj_04,7,0},
   {srma_it_sel_obj_05,8,0},
   {srma_it_sel_obj_06,9,0},
   {srma_it_sel_obj_07,10,0},

   {srma_it_stop_mon_01,11,0},
   {srma_it_stop_mon_02,12,0},
   {srma_it_stop_mon_03,13,0},
   {srma_it_stop_mon_04,14,0},
   {srma_it_stop_mon_05,15,0},
   {srma_it_stop_mon_06,16,0},

   {srma_it_restartmon_01,17,0},
   {srma_it_restartmon_02,18,0},
   {srma_it_restartmon_03,19,0},
   {srma_it_restartmon_04,20,0},
   {srma_it_restartmon_05,21,0},

   {srma_it_startmon_01,22,0},
   {srma_it_startmon_02,23,0},
   {srma_it_startmon_03,24,0},
   {srma_it_startmon_04,25,0},
   {srma_it_startmon_05,26,0},
   {srma_it_startmon_06,27,0},
   {srma_it_startmon_07,28,0},
   {srma_it_startmon_08,29,0},
   {srma_it_startmon_09,30,0},
   {srma_it_startmon_10,31,0},
   {srma_it_startmon_11,32,0},

   {srma_it_stop_rsrcmon_01,33,0},
   {srma_it_stop_rsrcmon_02,34,0},
   {srma_it_stop_rsrcmon_03,35,0},
   {srma_it_stop_rsrcmon_04,36,0},

   {srma_it_subscr_rsrcmon_01,37,0},
   {srma_it_subscr_rsrcmon_02,38,0},
   {srma_it_subscr_rsrcmon_03,39,0},
   {srma_it_subscr_rsrcmon_04,40,0},
   {srma_it_subscr_rsrcmon_05,41,0},
   {srma_it_subscr_rsrcmon_06,42,0},
   {srma_it_subscr_rsrcmon_07,43,0},
   {srma_it_subscr_rsrcmon_08,44,0},
   {srma_it_subscr_rsrcmon_09,45,0},
   {srma_it_subscr_rsrcmon_10,46,0},
   {srma_it_subscr_rsrcmon_11,47,0},

   {srma_it_unsubscr_rsrcmon_01,48,0},
   {srma_it_unsubscr_rsrcmon_02,49,0},
   {srma_it_unsubscr_rsrcmon_03,50,0},
   {srma_it_unsubscr_rsrcmon_04,51,0},

   {srma_it_disp_01,52,0},
   {srma_it_disp_02,53,0},
   {srma_it_disp_04,54,0},
   {srma_it_disp_05,55,0},
   {srma_it_disp_06,56,0},
   {srma_it_disp_07,57,0},
   {srma_it_disp_08,58,0},
   {srma_it_disp_09,59,0},

   {srma_it_fin_01,60,0},
   {srma_it_fin_02,61,0},
   {srma_it_fin_03,62,0},
   {srma_it_fin_04,63,0},

   {srma_it_clbk_01,64,0},
   {srma_it_clbk_02,65,0}, /*NULL cbk*/
   {srma_it_onenode_02,67,0},
   {srma_it_onenode_03,68,0},
   {srma_it_onenode_04,69,0},
   {srma_it_onenode_05,70,0},
   {srma_it_onenode_06,71,0},
   {srma_it_onenode_07,72,0},
   {srma_it_onenode_08,73,0},
   {srma_it_onenode_09,74,0},
   {srma_it_onenode_10,75,0},
   {srma_it_onenode_11,76,0},
   {srma_it_onenode_12,77,0},
   {srma_it_onenode_13,78,0},
   {srma_it_onenode_14,79,0},
   {srma_it_onenode_15,80,0},
   {srma_it_onenode_17,82,0},
   {srma_it_onenode_18,83,0},
   {srma_it_onenode_19,84,0},
   {srma_it_onenode_20,85,0},
   {srma_it_wmk_01,86,0},
   {srma_it_wmk_02,87,0},
   {srma_it_wmk_03,88,0},
   {srma_it_wmk_04,89,0},
   {srma_it_wmk_05,90,0},
   {srma_it_wmk_06,91,0},
   {srma_it_wmk_07,92,0},
   {srma_it_wmk_08,93,0},
   {srma_it_wmk_09,94,0},
   {srma_it_wmk_10,95,0},
   {srma_it_wmk_11,96,0},
   {srma_it_wmk_12,97,0},
   {srma_it_wmk_13,98,0},
   {srma_it_wmk_14,99,0},
   {srma_it_wmk_15,100,0},
   {srma_it_wmk_16,101,0},
   {srma_it_wmk_17,102,0},
   {srma_it_wmk_18,103,0},
   {srma_it_stop_rsrcmon_05,104,0},
   {srma_it_stop_rsrcmon_06,105,0},
   {srma_it_startmon_12,106,0},
   {srma_it_disp_03,107,0},
   {NULL,-1,0},
}; 

struct tet_testlist tet_srmsv_single_manual_testlist[]= {
   {srma_it_threshold_proc_mem,1,0},
   {srma_it_wmk_proc_mem,2,0},
   {srma_it_threshold_proc_cpu,3,0},
   {srma_it_wmk_proc_cpu,4,0},
   {srma_it_startmon_13,5,0},
   {srma_it_onenode_21,6,0},
   {srma_it_onenode_22,7,0},
   {srma_it_onenode_23,8,0},
   {srma_it_onenode_24,9,0},
   {NULL,-1,0},
}; 


void tet_run_srmsv_instance(TET_SRMSV_INST *inst)
{
   int test_case_num=1; 
   int iter;
#if 0
   int sysid;
   int *sysnames;
#endif
   struct tet_testlist *srmsv_testlist = tet_srmsv_single_node_testlist;

   if(inst->test_case_num)
       test_case_num = inst->test_case_num;

   if(inst->test_list == 1)
   {
       tet_srmsv_verify_pid(inst);
       srmsv_testlist = tet_srmsv_single_node_testlist;
   }


   if(inst->test_list == 6) /* manual tests */
   {
       tet_srmsv_verify_node_num(inst);
       srmsv_testlist = tet_srmsv_single_manual_testlist;
   }

   for(iter=0; iter < (inst->num_of_iter); iter++)
      tet_test_start(test_case_num,srmsv_testlist);
}

void tet_run_srmsv()
{
   TET_SRMSV_INST inst;
   tet_srmsv_get_inputs(&inst);
   fill_srma_testcase_data();

   tware_mem_ign();
   tet_run_srmsv_instance(&inst);
#if 0
   m_TET_SRMSV_PRINTF("\n *** ENTER TO C MEM DUMP ***\n");
   getchar();
#endif
   tware_mem_dump();

   sleep(5);
   tet_test_cleanup();
}
                                                                                                                                                                      
#endif
