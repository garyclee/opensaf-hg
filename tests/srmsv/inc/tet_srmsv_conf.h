#ifndef _TET_SRMSV_CONF_H
#define _TET_SRMSV_CONF_H

#include <stdio.h>
#include "ncs_lib.h"
#include "saAis.h"
#include "mds_papi.h"
#include "srmsv_papi.h"
#include "srma_papi.h"

typedef enum {
  TEST_NONCONFIG_MODE = 0,
  TEST_CONFIG_MODE,
  TEST_USER_DEFINED_MODE,
}CONFIG_FLAG;

typedef enum {
   SRMSV_CLEAN_INIT_SUCCESS_T,
   SRMSV_CLEAN_INIT_SUCCESS2_T,
   SRMSV_CLEAN_INIT_SUCCESS3_T,
}SRMA_CLEAN_HDL;

typedef enum
{
  SRMSV_CPU_UTIL = 1,
  SRMSV_MEM_UTIL,
}NCS_SRMSV_RSRC_UTIL;
                                                                                                                             
typedef struct callback_info {
   NCS_SRMSV_CBK_NOTIF_TYPE not_type;
   NCS_SRMSV_RSRC_HANDLE rsrc_mon_hdl;
   uns32 nodeId;
   int pid;
   NCS_SRMSV_VALUE threshold_val;
   NCS_SRMSV_VALUE low_val;
   NCS_SRMSV_VALUE high_val;
   NCS_BOOL wm_exist;
   NCS_SRMSV_WATERMARK_TYPE wm_type;
   NCS_SRMSV_WATERMARK_TYPE exp_wm_type;
   NCS_SRMSV_RSRC_TYPE rsrc_type;
}SRMSV_CALLBACK_DATA;

struct srmsv_testcase_data {
   NCS_SRMSV_HANDLE client1,client2,client3,client4,client5,client6,client7,client8,dummy_hdl;
   SaSelectionObjectT sel_client1,sel_client2,sel_client3,sel_client4,sel_dummy;
   int childId,grandChildId;
   int disp_flag;
   int num_sys;
   int clbk_count;
   uns32 nodeId;
   uns32 rem_nodeId;

   NCS_SRMSV_CALLBACK srmsv_callback;

   /* THRESHOLD MONITOR */
   NCS_SRMSV_MON_INFO local_monitor_cpu_util,local_monitor_kernel_util,local_monitor_user_util;
   NCS_SRMSV_MON_INFO local_monitor_user_util_one,local_monitor_user_util_five,local_monitor_user_util_fifteen;
   NCS_SRMSV_MON_INFO local_monitor_mem_used,local_monitor_mem_used2,local_monitor_mem_free;
   NCS_SRMSV_MON_INFO local_monitor_swap_used,local_monitor_swap_free;
   NCS_SRMSV_MON_INFO local_monitor_used_buf_mem,local_monitor_proc_exit,local_monitor_proc_child_exit;
   NCS_SRMSV_MON_INFO local_monitor_proc_grandchild_exit,local_monitor_proc_desc_exit,invalid_monitor,invalid_monitor2;
   NCS_SRMSV_MON_INFO remote_monitor_cpu_util,remote_monitor_kernel_util,remote_monitor_user_util;
   NCS_SRMSV_MON_INFO remote_monitor_user_util_one,remote_monitor_user_util_five,remote_monitor_user_util_fifteen;
   NCS_SRMSV_MON_INFO remote_monitor_mem_used,remote_monitor_mem_free,remote_monitor_swap_used,remote_monitor_swap_free;
   NCS_SRMSV_MON_INFO remote_monitor_used_buf_mem;
   NCS_SRMSV_MON_INFO all_monitor_cpu_util,all_monitor_kernel_util,all_monitor_user_util;
   NCS_SRMSV_MON_INFO all_monitor_user_util_one,all_monitor_user_util_five,all_monitor_user_util_fifteen;
   NCS_SRMSV_MON_INFO all_monitor_mem_used,all_monitor_mem_free,all_monitor_swap_used,all_monitor_swap_free;
   NCS_SRMSV_MON_INFO all_monitor_used_buf_mem;

   NCS_SRMSV_RSRC_HANDLE local_cpu_util_rsrc_hdl,local_cpu_util_rsrc_hdl2,local_kernel_util_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE local_user_util_rsrc_hdl,local_user_util_one_rsrc_hdl,local_user_util_five_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE local_user_util_fifteen_rsrc_hdl,local_mem_used_rsrc_hdl,local_mem_free_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE local_swap_used_rsrc_hdl,local_swap_free_rsrc_hdl,local_used_buf_mem_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE local_proc_exit_rsrc_hdl,local_proc_child_exit_rsrc_hdl,local_proc_grandchild_exit_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE local_proc_desc_exit_rsrc_hdl,rsrc_bad_hdl,dummy_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE remote_cpu_util_rsrc_hdl,remote_kernel_util_rsrc_hdl,remote_user_util_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE remote_user_util_one_rsrc_hdl,remote_user_util_five_rsrc_hdl,remote_user_util_fifteen_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE remote_mem_used_rsrc_hdl,remote_mem_free_rsrc_hdl,remote_swap_used_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE remote_swap_free_rsrc_hdl,remote_used_buf_mem_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE all_cpu_util_rsrc_hdl,all_kernel_util_rsrc_hdl,all_user_util_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE all_user_util_one_rsrc_hdl,all_user_util_five_rsrc_hdl,all_user_util_fifteen_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE all_mem_used_rsrc_hdl,all_mem_free_rsrc_hdl,all_swap_used_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE all_swap_free_rsrc_hdl,all_used_buf_mem_rsrc_hdl;

   NCS_SRMSV_SUBSCR_HANDLE local_cpu_util_subscr_hdl,local_cpu_util_subscr_hdl2;
   NCS_SRMSV_SUBSCR_HANDLE local_kernel_util_subscr_hdl,local_kernel_util_subscr_hdl2;
   NCS_SRMSV_SUBSCR_HANDLE local_user_util_subscr_hdl,local_user_util_subscr_hdl2;
   NCS_SRMSV_SUBSCR_HANDLE local_util_one_subscr_hdl,local_util_one_subscr_hdl2;
   NCS_SRMSV_SUBSCR_HANDLE local_util_five_subscr_hdl,local_util_five_subscr_hdl2;
   NCS_SRMSV_SUBSCR_HANDLE local_util_fifteen_subscr_hdl,local_util_fifteen_subscr_hdl2;
   NCS_SRMSV_SUBSCR_HANDLE local_mem_used_subscr_hdl,local_mem_used_subscr_hdl2;
   NCS_SRMSV_SUBSCR_HANDLE local_mem_free_subscr_hdl,local_mem_free_subscr_hdl2;
   NCS_SRMSV_SUBSCR_HANDLE local_swap_used_subscr_hdl,local_swap_used_subscr_hdl2;
   NCS_SRMSV_SUBSCR_HANDLE local_swap_free_subscr_hdl,local_swap_free_subscr_hdl2;
   NCS_SRMSV_SUBSCR_HANDLE local_used_buf_mem_subscr_hdl,local_used_buf_mem_subscr_hdl2;
   NCS_SRMSV_SUBSCR_HANDLE local_proc_exit_subscr_hdl,local_proc_exit_subscr_hdl2;
   NCS_SRMSV_SUBSCR_HANDLE local_proc_child_exit_subscr_hdl,local_proc_child_exit_subscr_hdl2;
   NCS_SRMSV_SUBSCR_HANDLE local_proc_grandchild_exit_subscr_hdl;
   NCS_SRMSV_SUBSCR_HANDLE local_proc_grandchild_exit_subscr_hdl2,local_proc_desc_exit_subscr_hdl;
   NCS_SRMSV_SUBSCR_HANDLE local_proc_desc_exit_subscr_hdl2;
   NCS_SRMSV_SUBSCR_HANDLE bad_subscr_hdl,dummy_subscr_hdl,local_mem_used_subscr_hdl3;
   NCS_SRMSV_SUBSCR_HANDLE remote_cpu_util_subscr_hdl,remote_kernel_util_subscr_hdl;
   NCS_SRMSV_SUBSCR_HANDLE remote_user_util_subscr_hdl,remote_util_one_subscr_hdl;
   NCS_SRMSV_SUBSCR_HANDLE remote_util_five_subscr_hdl,remote_util_fifteen_subscr_hdl;
   NCS_SRMSV_SUBSCR_HANDLE remote_mem_used_subscr_hdl,remote_mem_free_subscr_hdl;
   NCS_SRMSV_SUBSCR_HANDLE remote_swap_used_subscr_hdl,remote_swap_free_subscr_hdl,remote_used_buf_mem_subscr_hdl;
   NCS_SRMSV_SUBSCR_HANDLE all_cpu_util_subscr_hdl,all_kernel_util_subscr_hdl;
   NCS_SRMSV_SUBSCR_HANDLE all_user_util_subscr_hdl,all_util_one_subscr_hdl;
   NCS_SRMSV_SUBSCR_HANDLE all_util_five_subscr_hdl,all_util_fifteen_subscr_hdl;
   NCS_SRMSV_SUBSCR_HANDLE all_mem_used_subscr_hdl,all_mem_free_subscr_hdl;
   NCS_SRMSV_SUBSCR_HANDLE all_swap_used_subscr_hdl,all_swap_free_subscr_hdl,all_used_buf_mem_subscr_hdl;
 
   NCS_SRMSV_RSRC_INFO local_subscr_cpu_util,local_subscr_kernel_util,local_subscr_user_util;
   NCS_SRMSV_RSRC_INFO local_subscr_util_one,local_subscr_util_five;
   NCS_SRMSV_RSRC_INFO local_subscr_util_fifteen,local_subscr_mem_used,local_subscr_mem_free,local_subscr_swap_used;
   NCS_SRMSV_RSRC_INFO local_subscr_swap_free,local_subscr_used_buf_mem,local_subscr_proc_exit,local_subscr_proc_child_exit;
   NCS_SRMSV_RSRC_INFO local_subscr_proc_grandchild_exit,local_subscr_proc_desc_exit,invalid_subscr;
   NCS_SRMSV_RSRC_INFO remote_subscr_cpu_util,remote_subscr_kernel_util,remote_subscr_user_util,remote_subscr_util_one;
   NCS_SRMSV_RSRC_INFO remote_subscr_util_five,remote_subscr_util_fifteen,remote_subscr_mem_used,remote_subscr_mem_free;
   NCS_SRMSV_RSRC_INFO remote_subscr_swap_used,remote_subscr_swap_free,remote_subscr_used_buf_mem;
   NCS_SRMSV_RSRC_INFO all_subscr_cpu_util,all_subscr_kernel_util,all_subscr_user_util,all_subscr_util_one;
   NCS_SRMSV_RSRC_INFO all_subscr_util_five,all_subscr_util_fifteen,all_subscr_mem_used,all_subscr_mem_free;
   NCS_SRMSV_RSRC_INFO all_subscr_swap_used,all_subscr_swap_free,all_subscr_used_buf_mem;

   /* WATERMARK MONITORS */
   NCS_SRMSV_MON_INFO wmk_monitor_cpu_util,wmk_monitor_kernel_util,wmk_monitor_user_util;
   NCS_SRMSV_MON_INFO wmk_monitor_user_util_one,wmk_monitor_user_util_five,wmk_monitor_user_util_fifteen;
   NCS_SRMSV_MON_INFO wmk_monitor_mem_used,wmk_monitor_mem_free,wmk_monitor_swap_used,wmk_monitor_swap_free;
   NCS_SRMSV_MON_INFO wmk_monitor_used_buf_mem;
   NCS_SRMSV_MON_INFO wmk_remote_cpu_util,wmk_remote_kernel_util,wmk_remote_user_util;
   NCS_SRMSV_MON_INFO wmk_remote_user_util_one,wmk_remote_user_util_five,wmk_remote_user_util_fifteen;
   NCS_SRMSV_MON_INFO wmk_remote_mem_used,wmk_remote_mem_free,wmk_remote_swap_used,wmk_remote_swap_free;
   NCS_SRMSV_MON_INFO wmk_remote_used_buf_mem;
   NCS_SRMSV_MON_INFO wmk_all_cpu_util,wmk_all_kernel_util,wmk_all_user_util;
   NCS_SRMSV_MON_INFO wmk_all_user_util_one,wmk_all_user_util_five,wmk_all_user_util_fifteen;
   NCS_SRMSV_MON_INFO wmk_all_mem_used,wmk_all_mem_free,wmk_all_swap_used,wmk_all_swap_free;
   NCS_SRMSV_MON_INFO wmk_all_used_buf_mem;
    
   NCS_SRMSV_RSRC_HANDLE wmk_cpu_util_rsrc_hdl,wmk_kernel_util_rsrc_hdl,wmk_user_util_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE wmk_user_util_one_rsrc_hdl,wmk_user_util_five_rsrc_hdl,wmk_user_util_fifteen_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE wmk_mem_used_rsrc_hdl,wmk_mem_free_rsrc_hdl,wmk_swap_used_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE wmk_swap_free_rsrc_hdl,wmk_used_buf_mem_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE wmk_remote_cpu_util_rsrc_hdl,wmk_remote_kernel_util_rsrc_hdl,wmk_remote_user_util_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE wmk_remote_user_util_one_rsrc_hdl,wmk_remote_user_util_five_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE wmk_remote_user_util_fifteen_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE wmk_remote_mem_used_rsrc_hdl,wmk_remote_mem_free_rsrc_hdl,wmk_remote_swap_used_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE wmk_remote_swap_free_rsrc_hdl,wmk_remote_used_buf_mem_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE wmk_all_cpu_util_rsrc_hdl,wmk_all_kernel_util_rsrc_hdl,wmk_all_user_util_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE wmk_all_user_util_one_rsrc_hdl,wmk_all_user_util_five_rsrc_hdl,wmk_all_user_util_fifteen_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE wmk_all_mem_used_rsrc_hdl,wmk_all_mem_free_rsrc_hdl,wmk_all_swap_used_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE wmk_all_swap_free_rsrc_hdl,wmk_all_used_buf_mem_rsrc_hdl;

   NCS_SRMSV_RSRC_INFO wmk_getval_cpu_util,wmk_getval_kernel_util,wmk_getval_user_util;
   NCS_SRMSV_RSRC_INFO wmk_getval_util_one,wmk_getval_util_five;
   NCS_SRMSV_RSRC_INFO wmk_getval_util_fifteen,wmk_getval_mem_used,wmk_getval_mem_free,wmk_getval_swap_used;
   NCS_SRMSV_RSRC_INFO wmk_getval_swap_free,wmk_getval_used_buf_mem;
   NCS_SRMSV_RSRC_INFO wmk_rem_getval_cpu_util,wmk_rem_getval_kernel_util,wmk_rem_getval_user_util;
   NCS_SRMSV_RSRC_INFO wmk_rem_getval_util_one,wmk_rem_getval_util_five;
   NCS_SRMSV_RSRC_INFO wmk_rem_getval_util_fifteen,wmk_rem_getval_mem_used,wmk_rem_getval_mem_free,wmk_rem_getval_swap_used;
   NCS_SRMSV_RSRC_INFO wmk_rem_getval_swap_free,wmk_rem_getval_used_buf_mem;
   NCS_SRMSV_RSRC_INFO wmk_all_getval_cpu_util,wmk_all_getval_kernel_util,wmk_all_getval_user_util;
   NCS_SRMSV_RSRC_INFO wmk_all_getval_util_one,wmk_all_getval_util_five;
   NCS_SRMSV_RSRC_INFO wmk_all_getval_util_fifteen,wmk_all_getval_mem_used,wmk_all_getval_mem_free,wmk_all_getval_swap_used;
   NCS_SRMSV_RSRC_INFO wmk_all_getval_swap_free,wmk_all_getval_used_buf_mem;

    /* PROC SPECIFIC MONITORS*/
   NCS_SRMSV_MON_INFO local_monitor_threshold_proc_mem,remote_monitor_threshold_proc_mem;
   NCS_SRMSV_MON_INFO local_monitor_threshold_proc_cpu,remote_monitor_threshold_proc_cpu;
   NCS_SRMSV_MON_INFO local_monitor_wmk_proc_mem,remote_monitor_wmk_proc_mem;
   NCS_SRMSV_MON_INFO local_monitor_wmk_proc_cpu,remote_monitor_wmk_proc_cpu;

   NCS_SRMSV_RSRC_HANDLE local_proc_mem_thre_rsrc_hdl,local_proc_mem_wmk_rsrc_hdl;
   NCS_SRMSV_RSRC_HANDLE local_proc_cpu_thre_rsrc_hdl,local_proc_cpu_wmk_rsrc_hdl;
   NCS_SRMSV_RSRC_INFO wmk_getval_proc_mem,wmk_getval_proc_cpu;

   /* CALLBACK parameters */
   SRMSV_CALLBACK_DATA clbk_data;
   NCS_SRMSV_VALUE exp_val;
   NCS_SRMSV_VALUE tolerable_val;
   char *arch;
};

struct srmsv_testcase_data tcd;

extern int tet_test_srmsvInitialize(int i,CONFIG_FLAG flg);
extern int tet_test_srmsvSelection(int i,CONFIG_FLAG flg);
extern int tet_test_srmsvDispatch(int i,CONFIG_FLAG flg);
extern int tet_test_srmsvFinalize(int i,CONFIG_FLAG flg);
extern int tet_test_srmsvStopmon(int i,CONFIG_FLAG flg);
extern int tet_test_srmsvRestartmon(int i,CONFIG_FLAG flg);
extern int tet_test_srmsvStartrsrcmon(int i,CONFIG_FLAG flg);
extern int tet_test_srmsvSubscrrsrcmon(int i,CONFIG_FLAG flg);
extern int tet_test_srmsvStoprsrcmon(int i,CONFIG_FLAG flg);
extern int tet_test_srmsvUnsubscrmon(int i,CONFIG_FLAG flg);
extern int tet_test_srmsvGetWmkVal(int i, CONFIG_FLAG flg);
extern void srmsv_createthread(NCS_SRMSV_HANDLE *cl_hdl);
extern const char *ncs_srmsv_error[];
extern const char *srmsv_notif[];
extern const char *srmsv_scale_type[];
extern const char *srmsv_val_type[];
extern const char *srmsv_wmk_type[];
extern const char *srmsv_rsrc_type[];
extern void printSrmaHead(char *str);
extern void printSrmaResult(int result);

extern int check_condition(NCS_SRMSV_VALUE clbk_val,NCS_SRMSV_VALUE exp_val,
                           NCS_SRMSV_VALUE tolerable_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION condition);

extern int check_threshold_allclbk(SRMSV_CALLBACK_DATA clbk_data,NCS_SRMSV_STAT_SCALE_TYPE scale_type,
                 NCS_SRMSV_THRESHOLD_TEST_CONDITION condition,NCS_SRMSV_VALUE exp_val,
                 NCS_SRMSV_RSRC_HANDLE rsrc_mon_hdl);

extern int check_threshold_clbk(SRMSV_CALLBACK_DATA clbk_data,uns32 node_num,
                 NCS_SRMSV_STAT_SCALE_TYPE scale_type,NCS_SRMSV_THRESHOLD_TEST_CONDITION condition,
                 NCS_SRMSV_VALUE exp_val,NCS_SRMSV_RSRC_HANDLE rsrc_mon_hdl);

extern int check_wmk_clbk(SRMSV_CALLBACK_DATA clbk_data,uns32 node_num,NCS_SRMSV_RSRC_TYPE rsrc_type);
extern int check_wmk_allclbk(SRMSV_CALLBACK_DATA clbk_data,NCS_SRMSV_RSRC_TYPE rsrc_type);
extern void app_srmsv_callback(NCS_SRMSV_RSRC_CBK_INFO *rsrc_info);
extern void fill_exp_value(NCS_SRMSV_VALUE threshold_value,NCS_SRMSV_VALUE tolerable_value);
extern void tet_srma_hdl_clr(void);
extern void tet_srma_cleanup(SRMA_CLEAN_HDL hdl);

extern void fill_mon_info(NCS_SRMSV_MON_INFO *mon_info,NCS_SRMSV_USER_TYPE user_type,
                 NCS_SRMSV_NODE_LOCATION node_loc,NCS_SRMSV_RSRC_TYPE rsrc_type,time_t rate,
                 time_t interval,NCS_SRMSV_MONITOR_TYPE mon_type,uns32 samples,
                 NCS_SRMSV_VAL_TYPE val_type,NCS_SRMSV_STAT_SCALE_TYPE scale_type,
                 int32 val,NCS_SRMSV_VAL_TYPE tol_val_type,NCS_SRMSV_STAT_SCALE_TYPE tol_scale_type,
                 int32 tol_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION test_condition,uns32 severity);

extern void fill_proc_mon_info(NCS_SRMSV_MON_INFO *mon_info,NCS_SRMSV_USER_TYPE user_type,
                 uns32 pid,NCS_SRMSV_RSRC_TYPE rsrc_type,time_t rate,time_t interval,
                 NCS_SRMSV_MONITOR_TYPE mon_type,uns32 severity,uns32 child_level);

extern void fill_subscr_info(NCS_SRMSV_RSRC_INFO *subscr_info,NCS_SRMSV_USER_TYPE user_type,
                 NCS_SRMSV_RSRC_TYPE rsrc_type);

extern void fill_proc_subscr_info(NCS_SRMSV_RSRC_INFO *subscr_info,NCS_SRMSV_USER_TYPE user_type,
                 NCS_SRMSV_RSRC_TYPE rsrc_type,uns32 child_level);

extern void fill_remote_mon_info(NCS_SRMSV_MON_INFO *mon_info,NCS_SRMSV_USER_TYPE user_type,
                 NCS_SRMSV_NODE_LOCATION node_loc,NCS_SRMSV_RSRC_TYPE rsrc_type,time_t rate,
                 time_t interval,NCS_SRMSV_MONITOR_TYPE mon_type,uns32 samples,
                 NCS_SRMSV_VAL_TYPE val_type,NCS_SRMSV_STAT_SCALE_TYPE scale_type,int32 val,
                 NCS_SRMSV_VAL_TYPE tol_val_type,NCS_SRMSV_STAT_SCALE_TYPE tol_scale_type,
                 int32 tol_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION test_condition,uns32 severity );

extern void fill_remote_subscr_info(NCS_SRMSV_RSRC_INFO *subscr_info,NCS_SRMSV_USER_TYPE user_type,
                 NCS_SRMSV_NODE_LOCATION node_loc,NCS_SRMSV_RSRC_TYPE rsrc_type);

extern void fill_wmk_mon_info(NCS_SRMSV_MON_INFO *mon_info,NCS_SRMSV_USER_TYPE user_type,
                 NCS_SRMSV_NODE_LOCATION node_loc,NCS_SRMSV_RSRC_TYPE rsrc_type,time_t rate,
                 time_t interval,NCS_SRMSV_WATERMARK_TYPE wm_type,uns32 severity);

extern void fill_rem_wmk_mon_info(NCS_SRMSV_MON_INFO *mon_info,NCS_SRMSV_USER_TYPE user_type,
                 NCS_SRMSV_RSRC_TYPE rsrc_type,time_t rate,
                 time_t interval,NCS_SRMSV_WATERMARK_TYPE wm_type,uns32 severity);

extern void fill_getval_rsrc_info(NCS_SRMSV_RSRC_INFO *rsrc_info,NCS_SRMSV_NODE_LOCATION node_loc,
                 NCS_SRMSV_RSRC_TYPE rsrc_type);

extern int convert_scaletype_to_scaletype(NCS_SRMSV_MON_INFO mon_data);
extern void srma_it_wmk_02(void);
extern void tware_mem_dump(void);
extern void tware_mem_ign(void);
extern int get_num_sys(void);
extern struct ncsSrmsvGetWmkVal API_SRMSV_Get_Wmk_Val[];
extern void spawn_srmsv_exe(int arg);
extern void kill_srmsv_exe(void);
int check_f_val(double clbk_val,double exp_val,double tolerable_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION condition);
int check_i_val32(int32 clbk_val,int32 exp_val,int32 tolerable_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION condition);
int check_u_val32(uns32 clbk_val,uns32 exp_val,uns32 tolerable_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION condition);
int check_i_val64(int64 clbk_val,int64 exp_val,int64 tolerable_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION condition);
int check_u_val64(uns64 clbk_val,uns64 exp_val,uns64 tolerable_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION condition);
void print_threshold_clbk_values(NCS_SRMSV_VALUE *rsrc_value);
void print_wmk_low_val_values(NCS_SRMSV_VALUE *low_val);
void print_wmk_high_val_values(NCS_SRMSV_VALUE *high_val);
void print_wmk_clbk_values(SRMSV_WATERMARK_VAL *watermarks);
int scale_type_float_check(NCS_SRMSV_STAT_SCALE_TYPE scale_type, double clbk_val);
int scale_type_ival32_check(NCS_SRMSV_STAT_SCALE_TYPE scale_type, int32 clbk_val);
int scale_type_uval32_check(NCS_SRMSV_STAT_SCALE_TYPE scale_type, uns32 clbk_val);
int scale_type_ival64_check(NCS_SRMSV_STAT_SCALE_TYPE scale_type, int64 clbk_val);
int scale_type_uval64_check(NCS_SRMSV_STAT_SCALE_TYPE scale_type, uns64 clbk_val);
int scale_type_check(NCS_SRMSV_STAT_SCALE_TYPE scale_type, NCS_SRMSV_VALUE clbk_val);
int wmk_value_check(NCS_SRMSV_VALUE clbk_value);
void kill_srmsv_exe();
void fill_srma_testcase_data();
void tet_srma_hdl_clr();
void srma_it_threshold_proc_mem();
void srma_it_wmk_proc_mem();
void srma_it_threshold_proc_cpu();
void srma_it_wmk_proc_cpu();
void tet_srmsv_get_inputs(TET_SRMSV_INST *inst);
void tet_srmsv_verify_pid(TET_SRMSV_INST *inst);
void tet_srmsv_verify_numsys(TET_SRMSV_INST *inst);
void tet_srmsv_verify_node_num(TET_SRMSV_INST *inst);
void tet_run_srmsv_instance(TET_SRMSV_INST *inst);
void tet_run_srmsv();

#endif

