/*******************************************************************************************
#
# modification:
# 2008-11   Jony <jiangnan_hw@huawei.com>  Huawei Technologies CO., LTD.
#   1.  Provide test case tet_startup and tet_cleanup functions. The tet_startup function 
#       is defined as tet_cpsv_startup(), as well as the tet_cleanup function is defined 
#       as tet_cpsv_end().
#   2.  If "tetware-patch" is not used, change the name of test case array from 
#       cpsv_single_node_testlist[] to tet_testlist[]. 
#   3.  If "tetware-patch" is not used, delete the third parameter of each item in test casts 
#       array.
#   4.  If "tetware-patch" is not used, delete tet_run_cpsv_intance() function and parts of 
#       tet_run_cpsv_app() function, which invoke the functions in the patch.  
#   5.  If "tetware-patch" is required, one can add a macro definition "TET_PATCH=1" in 
#       compilation process.
#
**********************************************************************************************/

#include "tet_startup.h"
#include "ncs_main_papi.h"
#include "tet_cpsv.h"

void tet_run_cpsv_app(void);

void tet_cpsv_startup(void);
void tet_cpsv_end(void);

void (*tet_startup)()=tet_cpsv_startup;
void (*tet_cleanup)()=tet_cpsv_end;

#if (TET_PATCH==1)
struct tet_testlist cpsv_single_node_testlist[]={
  {cpsv_it_init_01,1,0},
  {cpsv_it_init_02,2,0},
  {cpsv_it_init_03,3,0},
  {cpsv_it_init_04,4,0},
  {cpsv_it_init_05,5,0},
  {cpsv_it_init_06,6,0},
  {cpsv_it_init_07,7,0},
  {cpsv_it_init_08,8,0},
  {cpsv_it_init_09,9,0},
  {cpsv_it_init_10,10,0},

  {cpsv_it_disp_01,11,0},
  {cpsv_it_disp_02,12,0},
/*dispatch blocking */
  {cpsv_it_disp_03,13,0},
  {cpsv_it_disp_04,14,0},
  {cpsv_it_disp_05,15,0},
  {cpsv_it_disp_06,16,0},
  {cpsv_it_disp_07,17,0},
  {cpsv_it_disp_08,18,0},
  {cpsv_it_disp_09,19,0},

  {cpsv_it_fin_01,20,0},
  {cpsv_it_fin_02,21,0},
  {cpsv_it_fin_03,22,0},
  {cpsv_it_fin_04,23,0},

  {cpsv_it_sel_01,24,0},
  {cpsv_it_sel_02,25,0},
  {cpsv_it_sel_03,26,0},
  {cpsv_it_sel_04,27,0},
  {cpsv_it_sel_05,28,0},
  {cpsv_it_sel_06,29,0},

  {cpsv_it_open_01,30,0},
  {cpsv_it_open_02,31,0},
  {cpsv_it_open_03,32,0},
  {cpsv_it_open_04,33,0},
  {cpsv_it_open_05,34,0},
  {cpsv_it_open_06,35,0},
  {cpsv_it_open_07,36,0},
  {cpsv_it_open_08,37,0},
  {cpsv_it_open_10,38,0},
  {cpsv_it_open_11,39,0},
  {cpsv_it_open_12,40,0},
  {cpsv_it_open_13,41,0},
  {cpsv_it_open_14,42,0},
  {cpsv_it_open_15,43,0},
  {cpsv_it_open_16,44,0},
  {cpsv_it_open_17,45,0},
  {cpsv_it_open_18,46,0},
  {cpsv_it_open_19,47,0},
  {cpsv_it_open_20,48,0},
  {cpsv_it_open_21,49,0},
  {cpsv_it_open_22,50,0},
  {cpsv_it_open_23,51,0},
  {cpsv_it_open_24,52,0},
  {cpsv_it_open_25,53,0},
  {cpsv_it_open_26,54,0},
  {cpsv_it_open_27,55,0},
  {cpsv_it_open_28,56,0},
  {cpsv_it_open_29,57,0},
  {cpsv_it_open_30,58,0},
  {cpsv_it_open_31,59,0},
  {cpsv_it_open_32,60,0},
  {cpsv_it_open_33,61,0},
  {cpsv_it_open_34,62,0},
  {cpsv_it_open_35,63,0},
  {cpsv_it_open_36,64,0},
  {cpsv_it_open_37,65,0},
  {cpsv_it_open_38,66,0},
  {cpsv_it_open_39,67,0},
  {cpsv_it_open_40,68,0},
  {cpsv_it_open_41,69,0},
  {cpsv_it_open_42,70,0},
  {cpsv_it_open_43,71,0},
  {cpsv_it_open_44,72,0},
  {cpsv_it_open_45,73,0},
  {cpsv_it_open_46,74,0},
  {cpsv_it_open_47,75,0},
  {cpsv_it_open_48,76,0},
  {cpsv_it_open_49,77,0},
  {cpsv_it_open_50,78,0},
  {cpsv_it_open_51,79,0},

  {cpsv_it_close_01,80,0},
  {cpsv_it_close_02,81,0},
  {cpsv_it_close_03,82,0},
  {cpsv_it_close_04,83,0},
  {cpsv_it_close_05,84,0},
  {cpsv_it_close_06,85,0},

  {cpsv_it_unlink_01,86,0},
  {cpsv_it_unlink_02,87,0},
  {cpsv_it_unlink_03,88,0},
  {cpsv_it_unlink_04,89,0},
  {cpsv_it_unlink_05,90,0},
  {cpsv_it_unlink_06,91,0},
  {cpsv_it_unlink_07,92,0},
  {cpsv_it_unlink_08,93,0},
  {cpsv_it_unlink_09,94,0},
#if 0
  {cpsv_it_unlink_10,95,0},
#endif
  {cpsv_it_unlink_11,96,0},

  {cpsv_it_rdset_01,97,0},
  {cpsv_it_rdset_02,98,0},
  {cpsv_it_rdset_03,99,0},
  {cpsv_it_rdset_04,100,0},
  {cpsv_it_rdset_05,101,0},
#if 0
  {cpsv_it_rdset_06,102,0},
#endif

  {cpsv_it_repset_01,103,0},
  {cpsv_it_repset_02,104,0},
  {cpsv_it_repset_03,105,0},
  {cpsv_it_repset_04,106,0},
  {cpsv_it_repset_05,107,0},

  {cpsv_it_status_01,108,0},
  {cpsv_it_status_02,109,0},
  {cpsv_it_status_03,110,0},
  {cpsv_it_status_05,111,0},
  {cpsv_it_status_06,112,0},

  {cpsv_it_seccreate_01,113,0},
  {cpsv_it_seccreate_02,114,0},
  {cpsv_it_seccreate_03,115,0},
  {cpsv_it_seccreate_04,116,0},
  {cpsv_it_seccreate_05,117,0},
  {cpsv_it_seccreate_07,118,0},
  {cpsv_it_seccreate_09,119,0},
  {cpsv_it_seccreate_10,120,0},
  {cpsv_it_seccreate_11,121,0},
  {cpsv_it_seccreate_12,122,0},
  {cpsv_it_seccreate_13,123,0},
  {cpsv_it_seccreate_14,124,0},
  {cpsv_it_seccreate_15,125,0},

  {cpsv_it_secdel_01,126,0},
  {cpsv_it_secdel_02,127,0},
  {cpsv_it_secdel_03,128,0},
  {cpsv_it_secdel_04,129,0},
  {cpsv_it_secdel_08,130,0},
  {cpsv_it_secdel_09,131,0},

  {cpsv_it_expset_01,132,0},
  {cpsv_it_expset_02,133,0},
  {cpsv_it_expset_03,134,0},
#if 0
  {cpsv_it_expset_04,135,0},
#endif
  {cpsv_it_expset_05,136,0},
  {cpsv_it_expset_06,137,0},
  {cpsv_it_expset_07,138,0},

  {cpsv_it_iterinit_01,139,0},
  {cpsv_it_iterinit_02,140,0},
  {cpsv_it_iterinit_03,141,0},
  {cpsv_it_iterinit_04,142,0},
  {cpsv_it_iterinit_05,143,0},
  {cpsv_it_iterinit_06,144,0},
#if 0 
  {cpsv_it_iterinit_07,145,0},
#endif
  {cpsv_it_iterinit_08,146,0},
  {cpsv_it_iterinit_09,147,0},
  {cpsv_it_iterinit_10,148,0},
  {cpsv_it_iterinit_12,149,0},

  {cpsv_it_iternext_01,150,0},
  {cpsv_it_iternext_02,151,0},
  {cpsv_it_iternext_03,152,0},
  {cpsv_it_iternext_04,153,0},
  {cpsv_it_iternext_05,154,0},
  {cpsv_it_iternext_06,155,0},
  {cpsv_it_iternext_08,156,0},
  {cpsv_it_iternext_09,157,0},

  {cpsv_it_iterfin_01,158,0},
  {cpsv_it_iterfin_02,159,0},
  {cpsv_it_iterfin_04,160,0},

  {cpsv_it_write_01,161,0},
  {cpsv_it_write_02,162,0},
  {cpsv_it_write_03,163,0},
  {cpsv_it_write_04,164,0},
  {cpsv_it_write_05,165,0},
  {cpsv_it_write_06,166,0},
  {cpsv_it_write_07,167,0},
  {cpsv_it_write_08,168,0},
  {cpsv_it_write_09,169,0},
  {cpsv_it_write_10,170,0},
  {cpsv_it_write_11,171,0},
  {cpsv_it_write_12,172,0},
  {cpsv_it_write_14,173,0},
  {cpsv_it_write_16,174,0},

  {cpsv_it_read_01,175,0},
  {cpsv_it_read_02,176,0},
  {cpsv_it_read_03,177,0},
  {cpsv_it_read_04,178,0},
  {cpsv_it_read_05,179,0},
  {cpsv_it_read_06,180,0},
  {cpsv_it_read_07,181,0},
  {cpsv_it_read_08,182,0},
  {cpsv_it_read_10,183,0},
  {cpsv_it_read_11,184,0},
  {cpsv_it_read_13,185,0},

  {cpsv_it_sync_01,186,0},
  {cpsv_it_sync_02,187,0},
  {cpsv_it_sync_03,188,0},
  {cpsv_it_sync_04,189,0},
  {cpsv_it_sync_05,190,0},
  {cpsv_it_sync_06,191,0},
  {cpsv_it_sync_07,192,0},
  {cpsv_it_sync_08,193,0},
  {cpsv_it_sync_09,194,0},
  {cpsv_it_sync_10,195,0},
  {cpsv_it_sync_11,196,0},
  {cpsv_it_sync_12,197,0},
  {cpsv_it_sync_13,198,0},
  {cpsv_it_sync_14,199,0},

  {cpsv_it_overwrite_01,200,0},
  {cpsv_it_overwrite_02,201,0},
  {cpsv_it_overwrite_03,202,0},
  {cpsv_it_overwrite_04,203,0},
  {cpsv_it_overwrite_05,204,0},
  {cpsv_it_overwrite_06,205,0},
  {cpsv_it_overwrite_07,206,0},
  {cpsv_it_overwrite_08,207,0},
  {cpsv_it_overwrite_09,208,0},
  {cpsv_it_overwrite_10,209,0},
  {cpsv_it_overwrite_11,210,0},
  {cpsv_it_overwrite_12,211,0},

  {cpsv_it_openclbk_01,212,0},
  {cpsv_it_openclbk_02,213,0},
  {cpsv_it_syncclbk_01,214,0},

  {cpsv_it_onenode_01,215,0},
  {cpsv_it_onenode_02,216,0},
  {cpsv_it_onenode_03,217,0},
  {cpsv_it_onenode_04,218,0},
  {cpsv_it_onenode_05,219,0},
  {cpsv_it_onenode_07,220,0},
  {cpsv_it_onenode_08,221,0},
  {cpsv_it_onenode_13,222,0},
  {cpsv_it_onenode_17,223,0},
  {cpsv_it_onenode_18,224,0},
  {cpsv_it_onenode_19,225,0},
  {cpsv_it_open_52,226,0},
  {cpsv_it_open_53,227,0},
  {cpsv_it_open_54,228,0},
  {cpsv_it_close_07,229,0},
  {cpsv_it_arr_default_sec,230,0},
  {cpsv_it_read_withoutwrite,231,0},
  {cpsv_it_read_withnullbuf,232,0},
  {cpsv_it_close_08,233,0},
  {cpsv_it_arr_invalid_param,234,0},
  {cpsv_it_seccreate_16,235,0},
  {cpsv_it_seccreate_17,236,0},
  {cpsv_it_seccreate_18,237,0},
#if 0
  /* Test procedure unknown */
  {cpsv_it_onenode_10,222,0},
  {cpsv_it_onenode_11,223,0},
  {cpsv_it_onenode_12,224,0},
  {cpsv_it_noncolloc_01,235,0},
  {cpsv_it_unlinktest,236,0},
#endif
  {NULL,-1,0}
};

void tet_run_cpsv_instance(TET_CPSV_INST *inst)
{
  int iter;
  int test_case_num=-1; /* DEFAULT - all test cases run */
  struct tet_testlist *cpsv_testlist = cpsv_single_node_testlist; 

  if(inst->test_case_num)
      test_case_num = inst->test_case_num;

  if(inst->test_list ==1)
      cpsv_testlist = cpsv_single_node_testlist;


  for(iter=0; iter < (inst->num_of_iter); iter++)
      tet_test_start(test_case_num,cpsv_testlist);
}

#else

struct tet_testlist tet_testlist[]={
  {cpsv_it_init_01,1},
  {cpsv_it_init_02,2},
  {cpsv_it_init_03,3},
  {cpsv_it_init_04,4},
  {cpsv_it_init_05,5},
  {cpsv_it_init_06,6},
  {cpsv_it_init_07,7},
  {cpsv_it_init_08,8},
  {cpsv_it_init_09,9},
  {cpsv_it_init_10,10},

  {cpsv_it_disp_01,11},
  {cpsv_it_disp_02,12},
/*dispatch blocking */
  {cpsv_it_disp_03,13},
  {cpsv_it_disp_04,14},
  {cpsv_it_disp_05,15},
  {cpsv_it_disp_06,16},
  {cpsv_it_disp_07,17},
  {cpsv_it_disp_08,18},
  {cpsv_it_disp_09,19},

  {cpsv_it_fin_01,20},
  {cpsv_it_fin_02,21},
  {cpsv_it_fin_03,22},
  {cpsv_it_fin_04,23},

  {cpsv_it_sel_01,24},
  {cpsv_it_sel_02,25},
  {cpsv_it_sel_03,26},
  {cpsv_it_sel_04,27},
  {cpsv_it_sel_05,28},
  {cpsv_it_sel_06,29},

  {cpsv_it_open_01,30},
  {cpsv_it_open_02,31},
  {cpsv_it_open_03,32},
  {cpsv_it_open_04,33},
  {cpsv_it_open_05,34},
  {cpsv_it_open_06,35},
  {cpsv_it_open_07,36},
  {cpsv_it_open_08,37},
  {cpsv_it_open_10,38},
  {cpsv_it_open_11,39},
  {cpsv_it_open_12,40},
  {cpsv_it_open_13,41},
  {cpsv_it_open_14,42},
  {cpsv_it_open_15,43},
  {cpsv_it_open_16,44},
  {cpsv_it_open_17,45},
  {cpsv_it_open_18,46},
  {cpsv_it_open_19,47},
  {cpsv_it_open_20,48},
  {cpsv_it_open_21,49},
  {cpsv_it_open_22,50},
  {cpsv_it_open_23,51},
  {cpsv_it_open_24,52},
  {cpsv_it_open_25,53},
  {cpsv_it_open_26,54},
  {cpsv_it_open_27,55},
  {cpsv_it_open_28,56},
  {cpsv_it_open_29,57},
  {cpsv_it_open_30,58},
  {cpsv_it_open_31,59},
  {cpsv_it_open_32,60},
  {cpsv_it_open_33,61},
  {cpsv_it_open_34,62},
  {cpsv_it_open_35,63},
  {cpsv_it_open_36,64},
  {cpsv_it_open_37,65},
  {cpsv_it_open_38,66},
  {cpsv_it_open_39,67},
  {cpsv_it_open_40,68},
  {cpsv_it_open_41,69},
  {cpsv_it_open_42,70},
  {cpsv_it_open_43,71},
  {cpsv_it_open_44,72},
  {cpsv_it_open_45,73},
  {cpsv_it_open_46,74},
  {cpsv_it_open_47,75},
  {cpsv_it_open_48,76},
  {cpsv_it_open_49,77},
  {cpsv_it_open_50,78},
  {cpsv_it_open_51,79},

  {cpsv_it_close_01,80},
  {cpsv_it_close_02,81},
  {cpsv_it_close_03,82},
  {cpsv_it_close_04,83},
  {cpsv_it_close_05,84},
  {cpsv_it_close_06,85},

  {cpsv_it_unlink_01,86},
  {cpsv_it_unlink_02,87},
  {cpsv_it_unlink_03,88},
  {cpsv_it_unlink_04,89},
  {cpsv_it_unlink_05,90},
  {cpsv_it_unlink_06,91},
  {cpsv_it_unlink_07,92},
  {cpsv_it_unlink_08,93},
  {cpsv_it_unlink_09,94},
#if 0
  {cpsv_it_unlink_10,95},
#endif
  {cpsv_it_unlink_11,96},

  {cpsv_it_rdset_01,97},
  {cpsv_it_rdset_02,98},
  {cpsv_it_rdset_03,99},
  {cpsv_it_rdset_04,100},
  {cpsv_it_rdset_05,101},
#if 0
  {cpsv_it_rdset_06,102},
#endif

  {cpsv_it_repset_01,103},
  {cpsv_it_repset_02,104},
  {cpsv_it_repset_03,105},
  {cpsv_it_repset_04,106},
  {cpsv_it_repset_05,107},

  {cpsv_it_status_01,108},
  {cpsv_it_status_02,109},
  {cpsv_it_status_03,110},
  {cpsv_it_status_05,111},
  {cpsv_it_status_06,112},

  {cpsv_it_seccreate_01,113},
  {cpsv_it_seccreate_02,114},
  {cpsv_it_seccreate_03,115},
  {cpsv_it_seccreate_04,116},
  {cpsv_it_seccreate_05,117},
  {cpsv_it_seccreate_07,118},
  {cpsv_it_seccreate_09,119},
  {cpsv_it_seccreate_10,120},
  {cpsv_it_seccreate_11,121},
  {cpsv_it_seccreate_12,122},
  {cpsv_it_seccreate_13,123},
  {cpsv_it_seccreate_14,124},
  {cpsv_it_seccreate_15,125},

  {cpsv_it_secdel_01,126},
  {cpsv_it_secdel_02,127},
  {cpsv_it_secdel_03,128},
  {cpsv_it_secdel_04,129},
  {cpsv_it_secdel_08,130},
  {cpsv_it_secdel_09,131},

  {cpsv_it_expset_01,132},
  {cpsv_it_expset_02,133},
  {cpsv_it_expset_03,134},
#if 0
  {cpsv_it_expset_04,135},
#endif
  {cpsv_it_expset_05,136},
  {cpsv_it_expset_06,137},
  {cpsv_it_expset_07,138},

  {cpsv_it_iterinit_01,139},
  {cpsv_it_iterinit_02,140},
  {cpsv_it_iterinit_03,141},
  {cpsv_it_iterinit_04,142},
  {cpsv_it_iterinit_05,143},
  {cpsv_it_iterinit_06,144},
#if 0 
  {cpsv_it_iterinit_07,145},
#endif
  {cpsv_it_iterinit_08,146},
  {cpsv_it_iterinit_09,147},
  {cpsv_it_iterinit_10,148},
  {cpsv_it_iterinit_12,149},

  {cpsv_it_iternext_01,150},
  {cpsv_it_iternext_02,151},
  {cpsv_it_iternext_03,152},
  {cpsv_it_iternext_04,153},
  {cpsv_it_iternext_05,154},
  {cpsv_it_iternext_06,155},
  {cpsv_it_iternext_08,156},
  {cpsv_it_iternext_09,157},

  {cpsv_it_iterfin_01,158},
  {cpsv_it_iterfin_02,159},
  {cpsv_it_iterfin_04,160},

  {cpsv_it_write_01,161},
  {cpsv_it_write_02,162},
  {cpsv_it_write_03,163},
  {cpsv_it_write_04,164},
  {cpsv_it_write_05,165},
  {cpsv_it_write_06,166},
  {cpsv_it_write_07,167},
  {cpsv_it_write_08,168},
  {cpsv_it_write_09,169},
  {cpsv_it_write_10,170},
  {cpsv_it_write_11,171},
  {cpsv_it_write_12,172},
  {cpsv_it_write_14,173},
  {cpsv_it_write_16,174},

  {cpsv_it_read_01,175},
  {cpsv_it_read_02,176},
  {cpsv_it_read_03,177},
  {cpsv_it_read_04,178},
  {cpsv_it_read_05,179},
  {cpsv_it_read_06,180},
  {cpsv_it_read_07,181},
  {cpsv_it_read_08,182},
  {cpsv_it_read_10,183},
  {cpsv_it_read_11,184},
  {cpsv_it_read_13,185},

  {cpsv_it_sync_01,186},
  {cpsv_it_sync_02,187},
  {cpsv_it_sync_03,188},
  {cpsv_it_sync_04,189},
  {cpsv_it_sync_05,190},
  {cpsv_it_sync_06,191},
  {cpsv_it_sync_07,192},
  {cpsv_it_sync_08,193},
  {cpsv_it_sync_09,194},
  {cpsv_it_sync_10,195},
  {cpsv_it_sync_11,196},
  {cpsv_it_sync_12,197},
  {cpsv_it_sync_13,198},
  {cpsv_it_sync_14,199},

  {cpsv_it_overwrite_01,200},
  {cpsv_it_overwrite_02,201},
  {cpsv_it_overwrite_03,202},
  {cpsv_it_overwrite_04,203},
  {cpsv_it_overwrite_05,204},
  {cpsv_it_overwrite_06,205},
  {cpsv_it_overwrite_07,206},
  {cpsv_it_overwrite_08,207},
  {cpsv_it_overwrite_09,208},
  {cpsv_it_overwrite_10,209},
  {cpsv_it_overwrite_11,210},
  {cpsv_it_overwrite_12,211},

  {cpsv_it_openclbk_01,212},
  {cpsv_it_openclbk_02,213},
  {cpsv_it_syncclbk_01,214},

  {cpsv_it_onenode_01,215},
  {cpsv_it_onenode_02,216},
  {cpsv_it_onenode_03,217},
  {cpsv_it_onenode_04,218},
  {cpsv_it_onenode_05,219},
  {cpsv_it_onenode_07,220},
  {cpsv_it_onenode_08,221},
  {cpsv_it_onenode_13,222},
  {cpsv_it_onenode_17,223},
  {cpsv_it_onenode_18,224},
  {cpsv_it_onenode_19,225},
  
  {cpsv_it_open_52,226},
  {cpsv_it_open_53,227},
  {cpsv_it_open_54,228},
  {cpsv_it_close_07,229},
  
  {cpsv_it_arr_default_sec,230},
  {cpsv_it_read_withoutwrite,231},
  {cpsv_it_read_withnullbuf,232},
  {cpsv_it_close_08,233},
  {cpsv_it_arr_invalid_param,234},
  {cpsv_it_seccreate_16,235},
  {cpsv_it_seccreate_17,236},
  {cpsv_it_seccreate_18,237},
#if 0
  /* Test procedure unknown */
  {cpsv_it_onenode_10,222},
  {cpsv_it_onenode_11,223},
  {cpsv_it_onenode_12,224},
  {cpsv_it_noncolloc_01,235},
  {cpsv_it_unlinktest,236},
#endif

  {NULL,-1}
};
#endif

void tet_run_cpsv_app()
{

  TET_CPSV_INST inst;
  tet_readFile();
  fill_testcase_data();
  tet_cpsv_get_inputs(&inst);

#if (TET_PATCH==1)
  if(inst.test_list == 1)
#endif

  tet_initialize();
  tet_cpsv_fill_inputs(&inst);

#if (TET_PATCH==1)
  tet_run_cpsv_instance(&inst);
  sleep(5);
  tet_test_cleanup();
#endif
}



void tet_cpsv_startup() 
{
/* Using the default mode for startup */
    ncs_agents_startup(); 

#if (TET_D == 1)
   tet_run_cpd();
#endif

#if (TET_ND == 1)
   tet_run_cpnd();
#endif

#if (TET_A == 1)
   tet_run_cpsv_app();
#endif

}

void tet_cpsv_end()
{
  tet_infoline(" Ending the agony.. ");
}
