#include <stdio.h>
#include "ncs_lib.h"
#include "saAis.h"
#include "mds_papi.h"
#include "srmsv_papi.h"
#include "srma_papi.h"


typedef enum {
    SRMSV_INIT_SUCCESS_T=1,
    SRMSV_INIT_NULL_CLBK_T,
    SRMSV_INIT_BAD_HANDLE_T,
    SRMSV_INIT_SUCCESS2_T,
    SRMSV_INIT_SUCCESS3_T,
    SRMSV_INIT_SUCCESS4_T,
    SRMSV_INIT_SUCCESS5_T,
    SRMSV_INIT_SUCCESS6_T,
    SRMSV_INIT_SUCCESS7_T,
}SRMSV_INIT_TC_TYPE;

struct ncsSrmsvInitialize {
    NCS_SRMSV_CALLBACK *callback;
    NCS_SRMSV_HANDLE *client_hdl;
    NCS_SRMSV_ERR exp_output;
    char *result_string;
};

typedef enum {
    SRMSV_SEL_SUCCESS_T=1,
    SRMSV_SEL_SUCCESS2_T,
    SRMSV_SEL_SUCCESS3_T,
    SRMSV_SEL_NULL_HDL_T,
    SRMSV_SEL_BAD_HANDLE_T,
    SRMSV_SEL_BAD_HANDLE2_T,
    SRMSV_SEL_INVALID_PARAM_T,
    SRMSV_SEL_BAD_HANDLE3_T
}SRMSV_SEL_TC_TYPE;

struct ncsSrmsvSelectionObject {
    NCS_SRMSV_HANDLE *client_hdl;
    SaSelectionObjectT *sel_obj;
    NCS_SRMSV_ERR exp_output;
    char *result_string;
};

typedef enum {
    SRMSV_DISPATCH_ONE_SUCCESS_T=1,
    SRMSV_DISPATCH_ALL_SUCCESS_T,
    SRMSV_DISPATCH_ALL_SUCCESS2_T,
    SRMSV_DISPATCH_ALL_SUCCESS3_T,
    SRMSV_DISPATCH_INVALID_FLAG_T,
    SRMSV_DISPATCH_NULL_HDL_T,
    SRMSV_DISPATCH_NULL_HDL2_T,
    SRMSV_DISPATCH_NULL_HDL3_T,
    SRMSV_DISPATCH_BAD_FLAGS_T,
    SRMSV_DISPATCH_BAD_HANDLE_T,
    SRMSV_DISPATCH_BAD_HANDLE2_T,
    SRMSV_DISPATCH_BAD_HANDLE3_T,
    SRMSV_DISPATCH_ONE_SUCCESS2_T,
}SRMSV_DISPATCH_TC_TYPE;

struct ncsSrmsvDispatch {
    NCS_SRMSV_HANDLE *client_hdl;
    SaDispatchFlagsT flags;
    NCS_SRMSV_ERR exp_output;
    char *result_string;
};

typedef enum {
    SRMSV_FIN_SUCCESS_T=1,
    SRMSV_FIN_SUCCESS2_T,
    SRMSV_FIN_SUCCESS3_T,
    SRMSV_FIN_SUCCESS4_T,
    SRMSV_FIN_BAD_HANDLE_T,
    SRMSV_FIN_BAD_HANDLE2_T,
}SRMSV_FIN_TC_TYPE;

struct ncsSrmsvFinalize {
    NCS_SRMSV_HANDLE *client_hdl;
    NCS_SRMSV_ERR exp_output;
    char *result_string;
};

typedef enum {
    SRMSV_STOP_MON_BAD_HANDLE_T=1,
    SRMSV_STOP_MON_BAD_HANDLE2_T,
    SRMSV_STOP_MON_SUCCESS_T,
    SRMSV_STOP_MON_BAD_OPERATION_T,
    SRMSV_RSRC_STOP_WITHOUT_START_T,
}SRMSV_STOP_MON_TC_TYPE;

struct ncsSrmsvStopMon {
    NCS_SRMSV_HANDLE *client_hdl;
    NCS_SRMSV_ERR exp_output;
    char *result_string;
};

typedef enum {
    SRMSV_RESTART_MON_BAD_HANDLE_T=1,
    SRMSV_RESTART_MON_BAD_HANDLE2_T,
    SRMSV_RESTART_MON_BAD_OPERATION_T,
    SRMSV_RESTART_MON_SUCCESS_T,
    SRMSV_RESTART_MON_FIN_HDL_T,
}SRMSV_RESTART_MON_TC_TYPE;

struct ncsSrmsvRestartMon {
    NCS_SRMSV_HANDLE *client_hdl;
    NCS_SRMSV_ERR exp_output;
    char *result_string;
};

typedef enum {
    SRMSV_RSRC_MON_BAD_HANDLE_T=1,
    SRMSV_RSRC_MON_BAD_HANDLE2_T,
    SRMSV_RSRC_MON_BAD_HANDLE3_T,
    SRMSV_RSRC_MON_INVALID_PARAM_T,
    SRMSV_RSRC_MON_INVALID_PARAM2_T,
    SRMSV_RSRC_MON_PID_ZERO_T,
    SRMSV_RSRC_MON_CPU_UTIL_T,
    SRMSV_RSRC_MON_CPU_KERNEL_T,
    SRMSV_RSRC_MON_CPU_USER_T,
    SRMSV_RSRC_MON_UTIL_ONE_T,
    SRMSV_RSRC_MON_UTIL_FIVE_T,
    SRMSV_RSRC_MON_UTIL_FIFTEEN_T,
    SRMSV_RSRC_MON_MEM_USED_T,
    SRMSV_RSRC_MON_MEM_USED2_T,
    SRMSV_RSRC_MON_MEM_FREE_T,
    SRMSV_RSRC_MON_SWAP_USED_T,
    SRMSV_RSRC_MON_SWAP_FREE_T,
    SRMSV_RSRC_MON_USED_BUF_MEM_T,
    SRMSV_RSRC_MON_PROC_EXIT_T,
    SRMSV_RSRC_MON_PROC_CHILD_EXIT_T,
    SRMSV_RSRC_MON_PROC_GRANDCHILD_EXIT_T,
    SRMSV_RSRC_MON_PROC_DESC_EXIT_T,
    SRMSV_RSRC_REMOTE_MON_CPU_UTIL_T,
    SRMSV_RSRC_REMOTE_MON_CPU_KERNEL_T,
    SRMSV_RSRC_REMOTE_MON_CPU_USER_T,
    SRMSV_RSRC_REMOTE_MON_UTIL_ONE_T,
    SRMSV_RSRC_REMOTE_MON_UTIL_FIVE_T,
    SRMSV_RSRC_REMOTE_MON_UTIL_FIFTEEN_T,
    SRMSV_RSRC_REMOTE_MON_MEM_USED_T,
    SRMSV_RSRC_REMOTE_MON_MEM_FREE_T,
    SRMSV_RSRC_REMOTE_MON_SWAP_USED_T,
    SRMSV_RSRC_REMOTE_MON_SWAP_FREE_T,
    SRMSV_RSRC_REMOTE_MON_USED_BUF_MEM_T,
    SRMSV_RSRC_ALL_MON_CPU_UTIL_T,
    SRMSV_RSRC_ALL_MON_CPU_KERNEL_T,
    SRMSV_RSRC_ALL_MON_CPU_USER_T,
    SRMSV_RSRC_ALL_MON_UTIL_ONE_T,
    SRMSV_RSRC_ALL_MON_UTIL_FIVE_T,
    SRMSV_RSRC_ALL_MON_UTIL_FIFTEEN_T,
    SRMSV_RSRC_ALL_MON_MEM_USED_T,
    SRMSV_RSRC_ALL_MON_MEM_FREE_T,
    SRMSV_RSRC_ALL_MON_SWAP_USED_T,
    SRMSV_RSRC_ALL_MON_SWAP_FREE_T,
    SRMSV_RSRC_ALL_MON_USED_BUF_MEM_T,
    SRMSV_RSRC_WMK_MON_CPU_UTIL_T,
    SRMSV_RSRC_WMK_MON_CPU_KERNEL_T,
    SRMSV_RSRC_WMK_MON_CPU_USER_T,
    SRMSV_RSRC_WMK_MON_UTIL_ONE_T,
    SRMSV_RSRC_WMK_MON_UTIL_FIVE_T,
    SRMSV_RSRC_WMK_MON_UTIL_FIFTEEN_T,
    SRMSV_RSRC_WMK_MON_MEM_USED_T,
    SRMSV_RSRC_WMK_MON_MEM_FREE_T,
    SRMSV_RSRC_WMK_MON_SWAP_USED_T,
    SRMSV_RSRC_WMK_MON_SWAP_FREE_T,
    SRMSV_RSRC_WMK_MON_USED_BUF_MEM_T,
    SRMSV_RSRC_WMK_REMOTE_CPU_UTIL_T,
    SRMSV_RSRC_WMK_REMOTE_CPU_KERNEL_T,
    SRMSV_RSRC_WMK_REMOTE_CPU_USER_T,
    SRMSV_RSRC_WMK_REMOTE_UTIL_ONE_T,
    SRMSV_RSRC_WMK_REMOTE_UTIL_FIVE_T,
    SRMSV_RSRC_WMK_REMOTE_UTIL_FIFTEEN_T,
    SRMSV_RSRC_WMK_REMOTE_MEM_USED_T,
    SRMSV_RSRC_WMK_REMOTE_MEM_FREE_T,
    SRMSV_RSRC_WMK_REMOTE_SWAP_USED_T,
    SRMSV_RSRC_WMK_REMOTE_SWAP_FREE_T,
    SRMSV_RSRC_WMK_REMOTE_USED_BUF_MEM_T,
    SRMSV_RSRC_WMK_ALL_CPU_UTIL_T,
    SRMSV_RSRC_WMK_ALL_CPU_KERNEL_T,
    SRMSV_RSRC_WMK_ALL_CPU_USER_T,
    SRMSV_RSRC_WMK_ALL_UTIL_ONE_T,
    SRMSV_RSRC_WMK_ALL_UTIL_FIVE_T,
    SRMSV_RSRC_WMK_ALL_UTIL_FIFTEEN_T,
    SRMSV_RSRC_WMK_ALL_MEM_USED_T,
    SRMSV_RSRC_WMK_ALL_MEM_FREE_T,
    SRMSV_RSRC_WMK_ALL_SWAP_USED_T,
    SRMSV_RSRC_WMK_ALL_SWAP_FREE_T,
    SRMSV_RSRC_WMK_ALL_USED_BUF_MEM_T,
    SRMSV_RSRC_ERR_NOT_EXIST_T,
    SRMSV_RSRC_MON_PROC_MEM_T,
    SRMSV_RSRC_WMK_PROC_MEM_T,
    SRMSV_RSRC_MON_PROC_CPU_T,
    SRMSV_RSRC_WMK_PROC_CPU_T,
    SRMSV_RSRC_WMK_ERR_NOT_EXIST_T,
    SRMSV_RSRC_INVALID_PARAM_T,
}SRMSV_RSRC_MON_TC_TYPE;

struct ncsSrmsvStartrsrcmon{
    NCS_SRMSV_HANDLE *client_hdl;
    NCS_SRMSV_MON_INFO *mon_info;
    NCS_SRMSV_RSRC_HANDLE *rsrc_hdl;
    NCS_SRMSV_ERR exp_output;
    char *result_string;
};

typedef enum {
    SRMSV_RSRC_SUBSCR_BAD_HANDLE_T=1,
    SRMSV_RSRC_SUBSCR_BAD_HANDLE2_T,
    SRMSV_RSRC_SUBSCR_BAD_HANDLE3_T,
    SRMSV_RSRC_SUBSCR_BAD_HANDLE4_T,
    SRMSV_RSRC_SUBSCR_REQ_T,
    SRMSV_RSRC_SUBSCR_INVALID_PARAM_T,
    SRMSV_RSRC_SUBSCR_CPU_UTIL_T,
    SRMSV_RSRC_SUBSCR_USER_UTIL_T,
    SRMSV_RSRC_SUBSCR_UTIL_ONE_T,
    SRMSV_RSRC_SUBSCR_UTIL_FIVE_T,
    SRMSV_RSRC_SUBSCR_UTIL_FIFTEEN_T,
    SRMSV_RSRC_SUBSCR_MEM_USED_T,
    SRMSV_RSRC_SUBSCR_MEM_USED2_T,
    SRMSV_RSRC_SUBSCR_MEM_USED3_T,
    SRMSV_RSRC_SUBSCR_SWAP_USED_T,
    SRMSV_RSRC_SUBSCR_SWAP_FREE_T,
    SRMSV_RSRC_SUBSCR_USED_BUF_MEM_T,
    SRMSV_RSRC_SUBSCR_PROC_EXIT_T,
    SRMSV_RSRC_SUBSCR_PROC_CHILD_EXIT_T,
    SRMSV_RSRC_SUBSCR_PROC_GRANDCHILD_EXIT_T,
    SRMSV_RSRC_SUBSCR_PROC_DESC_EXIT_T,
    SRMSV_RSRC_SUBSCR_REMOTE_CPU_UTIL_T,
    SRMSV_RSRC_SUBSCR_REMOTE_KERNEL_UTIL_T,
    SRMSV_RSRC_SUBSCR_REMOTE_USER_UTIL_T,
    SRMSV_RSRC_SUBSCR_REMOTE_UTIL_ONE_T,
    SRMSV_RSRC_SUBSCR_REMOTE_UTIL_FIVE_T,
    SRMSV_RSRC_SUBSCR_REMOTE_UTIL_FIFTEEN_T,
    SRMSV_RSRC_SUBSCR_REMOTE_MEM_USED_T,
    SRMSV_RSRC_SUBSCR_REMOTE_MEM_FREE_T,
    SRMSV_RSRC_SUBSCR_REMOTE_SWAP_USED_T,
    SRMSV_RSRC_SUBSCR_REMOTE_SWAP_FREE_T,
    SRMSV_RSRC_SUBSCR_REMOTE_USED_BUF_MEM_T,
    SRMSV_RSRC_SUBSCR_ALL_CPU_UTIL_T,
    SRMSV_RSRC_SUBSCR_ALL_KERNEL_UTIL_T,
    SRMSV_RSRC_SUBSCR_ALL_USER_UTIL_T,
    SRMSV_RSRC_SUBSCR_ALL_UTIL_ONE_T,
    SRMSV_RSRC_SUBSCR_ALL_UTIL_FIVE_T,
    SRMSV_RSRC_SUBSCR_ALL_UTIL_FIFTEEN_T,
    SRMSV_RSRC_SUBSCR_ALL_MEM_FREE_T,
    SRMSV_RSRC_SUBSCR_ALL_SWAP_USED_T,
    SRMSV_RSRC_SUBSCR_ALL_SWAP_FREE_T,
    SRMSV_RSRC_SUBSCR_ALL_USED_BUF_MEM_T,
    SRMSV_RSRC_SUBSCR_AFTER_STOP_T,
    SRMSV_RSRC_SUBSCR_WMK_CPU_UTIL_T,
    SRMSV_RSRC_SUBSCR_WMK_KERNEL_UTIL_T,
    SRMSV_RSRC_SUBSCR_WMK_USER_UTIL_T,
    SRMSV_RSRC_SUBSCR_WMK_UTIL_ONE_T,
    SRMSV_RSRC_SUBSCR_WMK_UTIL_FIVE_T,
    SRMSV_RSRC_SUBSCR_WMK_UTIL_FIFTEEN_T,
    SRMSV_RSRC_SUBSCR_WMK_MEM_USED_T,
    SRMSV_RSRC_SUBSCR_WMK_MEM_FREE_T,
    SRMSV_RSRC_SUBSCR_WMK_SWAP_USED_T,
    SRMSV_RSRC_SUBSCR_WMK_SWAP_FREE_T,
    SRMSV_RSRC_SUBSCR_WMK_USED_BUF_MEM_T,
    SRMSV_RSRC_SUBSCR_WMK_REMOTE_CPU_UTIL_T,
    SRMSV_RSRC_SUBSCR_WMK_REMOTE_KERNEL_UTIL_T,
    SRMSV_RSRC_SUBSCR_WMK_REMOTE_USER_UTIL_T,
    SRMSV_RSRC_SUBSCR_WMK_REMOTE_UTIL_ONE_T,
    SRMSV_RSRC_SUBSCR_WMK_REMOTE_UTIL_FIVE_T,
    SRMSV_RSRC_SUBSCR_WMK_REMOTE_UTIL_FIFTEEN_T,
    SRMSV_RSRC_SUBSCR_WMK_REMOTE_MEM_USED_T,
    SRMSV_RSRC_SUBSCR_WMK_REMOTE_MEM_FREE_T,
    SRMSV_RSRC_SUBSCR_WMK_REMOTE_SWAP_USED_T,
    SRMSV_RSRC_SUBSCR_WMK_REMOTE_SWAP_FREE_T,
    SRMSV_RSRC_SUBSCR_WMK_REMOTE_USED_BUF_MEM_T,
    SRMSV_RSRC_SUBSCR_WMK_ALL_CPU_UTIL_T,
    SRMSV_RSRC_SUBSCR_WMK_ALL_KERNEL_UTIL_T,
    SRMSV_RSRC_SUBSCR_WMK_ALL_USER_UTIL_T,
    SRMSV_RSRC_SUBSCR_WMK_ALL_UTIL_ONE_T,
    SRMSV_RSRC_SUBSCR_WMK_ALL_UTIL_FIVE_T,
    SRMSV_RSRC_SUBSCR_WMK_ALL_UTIL_FIFTEEN_T,
    SRMSV_RSRC_SUBSCR_WMK_ALL_MEM_USED_T,
    SRMSV_RSRC_SUBSCR_WMK_ALL_MEM_FREE_T,
    SRMSV_RSRC_SUBSCR_WMK_ALL_SWAP_USED_T,
    SRMSV_RSRC_SUBSCR_WMK_ALL_SWAP_FREE_T,
    SRMSV_RSRC_SUBSCR_WMK_ALL_USED_BUF_MEM_T,
    SRMSV_RSRC_SUBSCR_REM_NOT_EXIST_T,
}SRMSV_RSRC_SUBSCR_TC_TYPE;

struct ncsSrmsvSubscrrsrcmon{
    NCS_SRMSV_HANDLE *client_hdl;
    NCS_SRMSV_RSRC_INFO *rsrc_info;
    NCS_SRMSV_SUBSCR_HANDLE *subscr_hdl;
    NCS_SRMSV_ERR exp_output;
    char *result_string;
};

typedef enum {
    SRMSV_RSRC_STOP_BAD_HANDLE_T=1,
    SRMSV_RSRC_STOP_CPU_UTIL_T,
    SRMSV_RSRC_STOP_KERNEL_UTIL_T,
    SRMSV_RSRC_STOP_CPU_USER_T,
    SRMSV_RSRC_STOP_UTIL_ONE_T,
    SRMSV_RSRC_STOP_UTIL_FIVE_T,
    SRMSV_RSRC_STOP_UTIL_FIFTEEN_T,
    SRMSV_RSRC_STOP_MEM_USED_T,
    SRMSV_RSRC_STOP_MEM_FREE_T,
    SRMSV_RSRC_STOP_SWAP_USED_T,
    SRMSV_RSRC_STOP_SWAP_FREE_T,
    SRMSV_RSRC_STOP_USED_BUF_MEM_T,
    SRMSV_RSRC_STOP_PROC_EXIT_T,
    SRMSV_RSRC_STOP_CHILD_EXIT_T,
    SRMSV_RSRC_STOP_GRANDCHILD_EXIT_T,
    SRMSV_RSRC_STOP_DESC_EXIT_T,
}SRMSV_RSRC_STOP_TC_TYPE;

struct ncsSrmsvStoprsrcmon{
    NCS_SRMSV_RSRC_HANDLE *rsrc_hdl;
    NCS_SRMSV_ERR exp_output;
    char *result_string;
};

typedef enum {
    SRMSV_RSRC_UNSUBSCR_INVALID_PARAM_T=1,
    SRMSV_RSRC_UNSUBSCR_INVALID_PARAM2_T,
    SRMSV_RSRC_UNSUBSCR_CPU_UTIL_T,
    SRMSV_RSRC_UNSUBSCR_KERNEL_UTIL_T,
    SRMSV_RSRC_UNSUBSCR_USER_UTIL_T,
    SRMSV_RSRC_UNSUBSCR_UTIL_ONE_T,
    SRMSV_RSRC_UNSUBSCR_UTIL_FIVE_T,
    SRMSV_RSRC_UNSUBSCR_UTIL_FIFTEEN_T,
    SRMSV_RSRC_UNSUBSCR_MEM_USED_T,
    SRMSV_RSRC_UNSUBSCR_MEM_USED2_T,
    SRMSV_RSRC_UNSUBSCR_MEM_FREE_T,
    SRMSV_RSRC_UNSUBSCR_SWAP_USED_T,
    SRMSV_RSRC_UNSUBSCR_SWAP_FREE_T,
    SRMSV_RSRC_UNSUBSCR_USED_BUF_MEM_T,
    SRMSV_RSRC_UNSUBSCR_PROC_EXIT_T,
    SRMSV_RSRC_UNSUBSCR_PROC_CHILD_EXIT_T,
    SRMSV_RSRC_UNSUBSCR_PROC_GRANDCHILD_EXIT_T,
    SRMSV_RSRC_UNSUBSCR_PROC_DESC_EXIT_T,
    SRMSV_RSRC_UNSUBSCR_AFTER_UNSUBSCRIBE_T,  
}SRMSV_RSRC_UNSUBSCR_TC_TYPE;

struct ncsSrmsvUnsubscrmon{
    NCS_SRMSV_SUBSCR_HANDLE *subscr_hdl;
    NCS_SRMSV_ERR exp_output;
    char *result_string;
};

typedef enum{
    SRMSV_RSRC_GET_WMK_VAL_CPU_UTIL_T=1,
    SRMSV_RSRC_GET_WMK_VAL_KERNEL_UTIL_T,
    SRMSV_RSRC_GET_WMK_VAL_USER_UTIL_T,
    SRMSV_RSRC_GET_WMK_VAL_UTIL_ONE_T,
    SRMSV_RSRC_GET_WMK_VAL_UTIL_FIVE_T,
    SRMSV_RSRC_GET_WMK_VAL_UTIL_FIFTEEN_T,
    SRMSV_RSRC_GET_WMK_VAL_MEM_USED_T,
    SRMSV_RSRC_GET_WMK_VAL_MEM_FREE_T,
    SRMSV_RSRC_GET_WMK_VAL_SWAP_USED_T,
    SRMSV_RSRC_GET_WMK_VAL_SWAP_FREE_T,
    SRMSV_RSRC_GET_WMK_VAL_USED_BUF_MEM_T,
    SRMSV_RSRC_GET_WMK_REM_CPU_UTIL_T,
    SRMSV_RSRC_GET_WMK_REM_KERNEL_UTIL_T,
    SRMSV_RSRC_GET_WMK_REM_USER_UTIL_T,
    SRMSV_RSRC_GET_WMK_REM_UTIL_ONE_T,
    SRMSV_RSRC_GET_WMK_REM_UTIL_FIVE_T,
    SRMSV_RSRC_GET_WMK_REM_UTIL_FIFTEEN_T,
    SRMSV_RSRC_GET_WMK_REM_MEM_USED_T,
    SRMSV_RSRC_GET_WMK_REM_MEM_FREE_T,
    SRMSV_RSRC_GET_WMK_REM_SWAP_USED_T,
    SRMSV_RSRC_GET_WMK_REM_SWAP_FREE_T,
    SRMSV_RSRC_GET_WMK_REM_USED_BUF_MEM_T,
    SRMSV_RSRC_GET_WMK_ALL_CPU_UTIL_T,
    SRMSV_RSRC_GET_WMK_ALL_KERNEL_UTIL_T,
    SRMSV_RSRC_GET_WMK_ALL_USER_UTIL_T,
    SRMSV_RSRC_GET_WMK_ALL_UTIL_ONE_T,
    SRMSV_RSRC_GET_WMK_ALL_UTIL_FIVE_T,
    SRMSV_RSRC_GET_WMK_ALL_UTIL_FIFTEEN_T,
    SRMSV_RSRC_GET_WMK_ALL_MEM_USED_T,
    SRMSV_RSRC_GET_WMK_ALL_MEM_FREE_T,
    SRMSV_RSRC_GET_WMK_ALL_SWAP_USED_T,
    SRMSV_RSRC_GET_WMK_ALL_SWAP_FREE_T,
    SRMSV_RSRC_GET_WMK_ALL_USED_BUF_MEM_T,
    SRMSV_RSRC_GET_WMK_VAL_BAD_HANDLE_T,
    SRMSV_RSRC_GET_WMK_VAL_BAD_HANDLE2_T,
    SRMSV_RSRC_GET_WMK_VAL_MEM_FREE2_T,
    SRMSV_RSRC_GET_WMK_REM_NOT_EXIST_T,
    SRMSV_RSRC_GET_WMK_VAL_PROC_MEM_T,
    SRMSV_RSRC_GET_WMK_VAL_PROC_CPU_T,
}SRMSV_RSRC_GET_WMK_VAL_YPE;

struct ncsSrmsvGetWmkVal{
    NCS_SRMSV_HANDLE *srmsv_hdl;
    NCS_SRMSV_RSRC_INFO *rsrc_info;
    NCS_SRMSV_WATERMARK_TYPE wmk_type;
    NCS_SRMSV_ERR exp_output;
    char *result_string;
};

typedef struct tet_srmsv_inst {
    int inst_num;
    int num_sys;
    int tetlist_index;
    int test_case_num;
    int test_list;
    int rem_node_num;
    char *arch;
    int num_of_iter;
    int node_id;
    int pid;
}TET_SRMSV_INST;
                                                                                                                                                                      
 void srma_it_init_01(void);
 void srma_it_init_02(void);
 void srma_it_init_03(void);
 void srma_it_sel_obj_01(void);
 void srma_it_sel_obj_02(void);
 void srma_it_sel_obj_03(void);
 void srma_it_sel_obj_04(void);
 void srma_it_sel_obj_05(void);
 void srma_it_sel_obj_06(void);
 void srma_it_sel_obj_07(void);
 void srma_it_stop_mon_01(void);
 void srma_it_stop_mon_02(void);
 void srma_it_stop_mon_03(void);
 void srma_it_stop_mon_04(void);
 void srma_it_stop_mon_05(void);
 void srma_it_stop_mon_06(void);
 void srma_it_restartmon_01(void);
 void srma_it_restartmon_02(void);
 void srma_it_restartmon_03(void);
 void srma_it_restartmon_04(void);
 void srma_it_restartmon_05(void);
 void srma_it_startmon_01(void);
 void srma_it_startmon_02(void);
 void srma_it_startmon_03(void);
 void srma_it_startmon_04(void);
 void srma_it_startmon_05(void);
 void srma_it_startmon_06(void);
 void srma_it_startmon_07(void);
 void srma_it_startmon_08(void);
 void srma_it_startmon_09(void);
 void srma_it_startmon_10(void);
 void srma_it_startmon_11(void);
 void srma_it_startmon_12(void);
 void srma_it_startmon_13(void);
 void srma_it_stop_rsrcmon_01(void);
 void srma_it_stop_rsrcmon_02(void);
 void srma_it_stop_rsrcmon_03(void);
 void srma_it_stop_rsrcmon_04(void);
 void srma_it_stop_rsrcmon_05(void);
 void srma_it_stop_rsrcmon_06(void);
 void srma_it_subscr_rsrcmon_01(void);
 void srma_it_subscr_rsrcmon_02(void);
 void srma_it_subscr_rsrcmon_03(void);
 void srma_it_subscr_rsrcmon_04(void);
 void srma_it_subscr_rsrcmon_05(void);
 void srma_it_subscr_rsrcmon_06(void);
 void srma_it_subscr_rsrcmon_07(void);
 void srma_it_subscr_rsrcmon_08(void);
 void srma_it_subscr_rsrcmon_09(void);
 void srma_it_subscr_rsrcmon_10(void);
 void srma_it_subscr_rsrcmon_11(void);
 void srma_it_unsubscr_rsrcmon_01(void);
 void srma_it_unsubscr_rsrcmon_02(void);
 void srma_it_unsubscr_rsrcmon_03(void);
 void srma_it_unsubscr_rsrcmon_04(void);
 void srma_it_disp_01(void);
 void srma_it_disp_02(void);
 void srma_it_disp_03(void);
 void srma_it_disp_04(void);
 void srma_it_disp_05(void);
 void srma_it_disp_06(void);
 void srma_it_disp_07(void);
 void srma_it_disp_08(void);
 void srma_it_disp_09(void);
 void srma_it_fin_01(void);
 void srma_it_fin_02(void);
 void srma_it_fin_03(void);
 void srma_it_fin_04(void);
 void srma_it_clbk_01(void);
 void srma_it_clbk_02(void);
 void srma_it_onenode_02(void);
 void srma_it_onenode_03(void);
 void srma_it_onenode_04(void);
 void srma_it_onenode_05(void);
 void srma_it_onenode_06(void);
 void srma_it_onenode_07(void);
 void srma_it_onenode_08(void);
 void srma_it_onenode_09(void);
 void srma_it_onenode_10(void);
 void srma_it_onenode_11(void);
 void srma_it_onenode_12(void);
 void srma_it_onenode_13(void);
 void srma_it_onenode_14(void);
 void srma_it_onenode_15(void);
 void srma_it_onenode_17(void);
 void srma_it_onenode_18(void);
 void srma_it_onenode_19(void);
 void srma_it_onenode_20(void);
 void srma_it_onenode_21(void);
 void srma_it_onenode_22(void);
 void srma_it_onenode_23(void);
 void srma_it_onenode_24(void);
 void srma_it_wmk_01(void);
 void srma_it_wmk_02(void);
 void srma_it_wmk_03(void);
 void srma_it_wmk_04(void);
 void srma_it_wmk_05(void);
 void srma_it_wmk_06(void);
 void srma_it_wmk_07(void);
 void srma_it_wmk_08(void);
 void srma_it_wmk_09(void);
 void srma_it_wmk_10(void);
 void srma_it_wmk_11(void);
 void srma_it_wmk_12(void);
 void srma_it_wmk_13(void);
 void srma_it_wmk_14(void);
 void srma_it_wmk_15(void);
 void srma_it_wmk_16(void);
 void srma_it_wmk_17(void);
 void srma_it_wmk_18(void);
 void event_thread_blocking (NCSCONTEXT arg);
 void fill_srma_testcase_data(void);
void srma_it_threshold_proc_mem(void);

void srma_it_wmk_proc_mem(void);
void srma_it_threshold_proc_cpu(void);
void srma_it_wmk_proc_cpu(void);
void tet_run_srmsv(void);



