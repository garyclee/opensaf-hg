/***************************************************************************************************
#
# modification:
# 2008-11   Jony <jiangnan_hw@huawei.com>  Huawei Technologies CO., LTD.
#  1.  Provide test case tet_startup and tet_cleanup functions. The tet_startup function is defined
#      as tet_srmsv_startup(), as well as the tet_cleanup function is defined as tet_srmsv_end().
#  2.  If tetware-patch is not used, change the name of test case array from 
#      tet_srmsv_single_node_testlist[] to tet_testlist[]. Delete the tet_testlist tet_testlist[] 
#      array, because it is not used now.
#  3.  If tetware-patch is not used, delete the third parameter of each item in this array. And add 
#      some functions to implement the function of those parameters.
#  4.  If tetware-patch is not used, delete parts of tet_run_srmsv() function, which invoke the 
#      functions in the patch.
#  5.  If tetware-patch is required, one can add a macro definition TET_PATCH=1 in compilation process.
#
*****************************************************************************************************/



#include "tet_startup.h"
#include "ncs_main_papi.h"
#include "tet_srmsv.h"
#include "tet_srmsv_conf.h"

void tet_run_srmsv(void);

void tet_srmsv_startup(void);
void tet_srmsv_cleanup(void);

void (*tet_startup)()=tet_srmsv_startup;
void (*tet_cleanup)()=tet_srmsv_cleanup;

#if (TET_PATCH==1)

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


#else

struct tet_testlist tet_testlist[]={

#if (TET_MANUAL==1)
  {srma_it_threshold_proc_mem,1},
  {srma_it_wmk_proc_mem,2},
  {srma_it_threshold_proc_cpu,3},
  {srma_it_wmk_proc_cpu,4},
  {srma_it_startmon_13,5},
  {srma_it_onenode_21,6},
  {srma_it_onenode_22,7},
  {srma_it_onenode_23,8},
  {srma_it_onenode_24,9},

#else

  {srma_it_init_01, 1},
  {srma_it_init_02, 2},
  {srma_it_init_03, 3},
  {srma_it_sel_obj_01, 4},
  {srma_it_sel_obj_02, 5},
  {srma_it_sel_obj_03, 6},
  {srma_it_sel_obj_04, 7},
  {srma_it_sel_obj_05, 8},
  {srma_it_sel_obj_06, 9},
  {srma_it_sel_obj_07, 10},
  {srma_it_stop_mon_01, 11},
  {srma_it_stop_mon_02, 12},
  {srma_it_stop_mon_03, 13},
  {srma_it_stop_mon_04, 14},
  {srma_it_stop_mon_05, 15},
  {srma_it_stop_mon_06, 16},
  {srma_it_restartmon_01, 17},
  {srma_it_restartmon_02, 18},
  {srma_it_restartmon_03, 19},
  {srma_it_restartmon_04, 20},
  {srma_it_restartmon_05, 21},
  {srma_it_startmon_01, 22},
  {srma_it_startmon_02, 23},
  {srma_it_startmon_03, 24},
  {srma_it_startmon_04, 25},
  {srma_it_startmon_05, 26},
  {srma_it_startmon_06, 27},
  {srma_it_startmon_07, 28},
  {srma_it_startmon_08, 29},
  {srma_it_startmon_09, 30},
  {srma_it_startmon_10, 31},
  {srma_it_startmon_11, 32},
  {srma_it_stop_rsrcmon_01, 33},
  {srma_it_stop_rsrcmon_02, 34},
  {srma_it_stop_rsrcmon_03, 35},
  {srma_it_stop_rsrcmon_04, 36},
  {srma_it_subscr_rsrcmon_01, 37},
  {srma_it_subscr_rsrcmon_02, 38},
  {srma_it_subscr_rsrcmon_03, 39},
  {srma_it_subscr_rsrcmon_04, 40},
  {srma_it_subscr_rsrcmon_05, 41},
  {srma_it_subscr_rsrcmon_06, 42},
  {srma_it_subscr_rsrcmon_07, 43},
  {srma_it_subscr_rsrcmon_08, 44},
  {srma_it_subscr_rsrcmon_09, 45},
  {srma_it_subscr_rsrcmon_10, 46},
  {srma_it_subscr_rsrcmon_11, 47},
  {srma_it_unsubscr_rsrcmon_01, 48},
  {srma_it_unsubscr_rsrcmon_02, 49},
  {srma_it_unsubscr_rsrcmon_03, 50},
  {srma_it_unsubscr_rsrcmon_04, 51},
  {srma_it_disp_01, 52},
  {srma_it_disp_02, 53},
  {srma_it_disp_04, 54},
  {srma_it_disp_05, 55},
  {srma_it_disp_06, 56},
  {srma_it_disp_07, 57},
  {srma_it_disp_08, 58},
  {srma_it_disp_09, 59},
  {srma_it_fin_01, 60},
  {srma_it_fin_02, 61},
  {srma_it_fin_03, 62},
  {srma_it_fin_04, 63},
  {srma_it_clbk_01, 64},
  {srma_it_clbk_02, 65},
  {srma_it_onenode_02, 67},
  {srma_it_onenode_03, 68},
  {srma_it_onenode_04, 69},
  {srma_it_onenode_05, 70},
  {srma_it_onenode_06, 71},
  {srma_it_onenode_07, 72},
  {srma_it_onenode_08, 73},
  {srma_it_onenode_09, 74},
  {srma_it_onenode_10, 75},
  {srma_it_onenode_11, 76},
  {srma_it_onenode_12, 77},
  {srma_it_onenode_13, 78},
  {srma_it_onenode_14, 79},
  {srma_it_onenode_15, 80},
  {srma_it_onenode_17, 82},
  {srma_it_onenode_18, 83},
  {srma_it_onenode_19, 84},
  {srma_it_onenode_20, 85},
  {srma_it_wmk_01, 86},
  {srma_it_wmk_02, 87},
  {srma_it_wmk_03, 88},
  {srma_it_wmk_04, 89},
  {srma_it_wmk_05, 90},
  {srma_it_wmk_06, 91},
  {srma_it_wmk_07, 92},
  {srma_it_wmk_08, 93},
  {srma_it_wmk_09, 94},
  {srma_it_wmk_10, 95},
  {srma_it_wmk_11, 96},
  {srma_it_wmk_12, 97},
  {srma_it_wmk_13, 98},
  {srma_it_wmk_14, 99},
  {srma_it_wmk_15, 100},
  {srma_it_wmk_16, 101},
  {srma_it_wmk_17, 102},
  {srma_it_wmk_18, 103},
  {srma_it_stop_rsrcmon_05, 104},
  {srma_it_stop_rsrcmon_06, 105},
  {srma_it_startmon_12, 106},
  {srma_it_disp_03, 107},

#endif

  {NULL, -1}
};

#endif


void tet_run_srmsv()
{
   TET_SRMSV_INST inst;
   tet_printf(" ===================================");
   
   tet_srmsv_get_inputs(&inst);
   fill_srma_testcase_data();

#if (TET_PATCH==1)
   tware_mem_ign();
   tet_run_srmsv_instance(&inst);
#if 0
   m_TET_SRMSV_PRINTF("\n *** ENTER TO C MEM DUMP ***\n");
   getchar();
#endif
   tware_mem_dump();

   sleep(5);
   tet_test_cleanup();

#else

#if (TET_MANUAL==1)
   tet_srmsv_verify_node_num(&inst);

#else
   tet_srmsv_verify_pid(&inst);
#endif 

#endif
}
                                                                                  

                                                                                                                                                                      
void tet_srmsv_startup(void)
{
/* Using the default mode for startup */
    ncs_agents_startup(0,NULL); 

    tet_run_srmsv(); 
}

void tet_srmsv_cleanup()
{
  tet_infoline(" Ending the agony.. ");
}

