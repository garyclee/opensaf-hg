/******************************************************************************************
#
# modification:
# 2008-11   Jony <jiangnan_hw@huawei.com>  Huawei Technologies CO., LTD.
#   1.  Provide test case tet_startup and tet_cleanup functions. The tet_startup function 
#       is defined as tet_ifsv_startup(), as well as the tet_cleanup function is defined 
#       as tet_ifsv_end().
#   2.  When "tetware-patch" is not used, if ifsv is compiled with the macro definition 
#       "TET_A", combine the array ifsv_api_test[] and the array ifsv_func_test[] into one 
#       array; if ifsv is compiled with the macro definition "TET_DRIVER", combine the 
#       driver_func_test[] and driver_func_test[] arrays into one array and name it as 
#       tet_testlist[]. 
#   3.  If "tetware-patch" is not used , delete the third parameter of each item in this
#       array. And add some functions to implement the function of those parameters.
#   4.  If "tetware-patch" is not used, delete parts of tet_run_ifsv_app() function, 
#       which invoke the functions in the patch.
#   5.  If "tetware-patch" is required, one can add a macro definition "TET_PATCH=1" in 
#       compilation process.
#
********************************************************************************************/



#include "tet_startup.h"
#include "ncs_main_papi.h"
#include "tet_ifa.h"


void tet_run_ifsv_app();

void tet_ifsv_startup(void);
void tet_ifsv_end(void);

void (*tet_startup)()=tet_ifsv_startup;
void (*tet_cleanup)()=tet_ifsv_end;


#if (TET_PATCH==1)

/**************************************************************************/
/***************************** TET LIST ARRAYS ****************************/
/**************************************************************************/
struct tet_testlist ifsv_api_test[]={

    {tet_interface_add_subscription_loop,1},

    {tet_ncs_ifsv_svc_req_subscribe,2,1},
    {tet_ncs_ifsv_svc_req_subscribe,2,2},
#if 0
    {tet_ncs_ifsv_svc_req_subscribe,2,3},
#endif
    {tet_ncs_ifsv_svc_req_subscribe,2,4},
    {tet_ncs_ifsv_svc_req_subscribe,2,5},
    {tet_ncs_ifsv_svc_req_subscribe,2,6},
    {tet_ncs_ifsv_svc_req_subscribe,2,7},
    {tet_ncs_ifsv_svc_req_subscribe,2,8},
    {tet_ncs_ifsv_svc_req_subscribe,2,9},
    {tet_ncs_ifsv_svc_req_subscribe,2,10},
    {tet_ncs_ifsv_svc_req_subscribe,2,11},
    {tet_ncs_ifsv_svc_req_subscribe,2,12},
    {tet_ncs_ifsv_svc_req_subscribe,2,13},
    {tet_ncs_ifsv_svc_req_subscribe,2,14},
    {tet_ncs_ifsv_svc_req_subscribe,2,15}, 
    {tet_ncs_ifsv_svc_req_subscribe,2,16},
    {tet_ncs_ifsv_svc_req_subscribe,2,17},
    {tet_ncs_ifsv_svc_req_subscribe,2,18},
    {tet_ncs_ifsv_svc_req_subscribe,2,19},
    {tet_ncs_ifsv_svc_req_subscribe,2,20},
    {tet_ncs_ifsv_svc_req_subscribe,2,21},
    {tet_ncs_ifsv_svc_req_subscribe,2,22},
    {tet_ncs_ifsv_svc_req_subscribe,2,23},
    {tet_ncs_ifsv_svc_req_subscribe,2,24},

    {tet_ncs_ifsv_svc_req_unsubscribe,3,1},
    {tet_ncs_ifsv_svc_req_unsubscribe,3,2},
    {tet_ncs_ifsv_svc_req_unsubscribe,3,3},
    {tet_ncs_ifsv_svc_req_unsubscribe,3,4},

    {tet_ncs_ifsv_svc_req_ifrec_add,4,1},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,2},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,3},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,4},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,5},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,6},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,7},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,8},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,9},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,10},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,11},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,12},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,13},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,14},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,15},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,16},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,17},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,18},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,19},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,20},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,21},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,22},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,23},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,24},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,25},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,26},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,27},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,28},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,29},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,30},
#if 0
    {tet_ncs_ifsv_svc_req_ifrec_add,4,31},
#endif
    {tet_ncs_ifsv_svc_req_ifrec_add,4,32},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,33},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,34},
#if 0
    /*no cases*/
    {tet_ncs_ifsv_svc_req_ifrec_add,4,35},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,36},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,37},
#endif
    {tet_ncs_ifsv_svc_req_ifrec_add,4,38},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,39},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,40},
    {tet_ncs_ifsv_svc_req_ifrec_add,4,41},

    {tet_ncs_ifsv_svc_req_ifrec_del,5,1},
    {tet_ncs_ifsv_svc_req_ifrec_del,5,2},
    {tet_ncs_ifsv_svc_req_ifrec_del,5,3},
    {tet_ncs_ifsv_svc_req_ifrec_del,5,4},
    {tet_ncs_ifsv_svc_req_ifrec_del,5,5},

    {tet_ncs_ifsv_svc_req_ifrec_get,6,1},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,2},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,3},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,4},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,5},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,6},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,7},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,8},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,9},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,10},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,11},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,12},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,13},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,14},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,15},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,16},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,17},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,18},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,19},
    {tet_ncs_ifsv_svc_req_ifrec_get,6,20},

    /*ipxs test cases*/
    {tet_ncs_ipxs_svc_req_subscribe,7,1},
    {tet_ncs_ipxs_svc_req_subscribe,7,2},
    {tet_ncs_ipxs_svc_req_subscribe,7,3},
    {tet_ncs_ipxs_svc_req_subscribe,7,4},
    {tet_ncs_ipxs_svc_req_subscribe,7,5},
    {tet_ncs_ipxs_svc_req_subscribe,7,6},
    {tet_ncs_ipxs_svc_req_subscribe,7,7},
#if 0
    /*why 8 n 9 r commented:not supported scope*/
    {tet_ncs_ipxs_svc_req_subscribe,7,8},
    {tet_ncs_ipxs_svc_req_subscribe,7,9},
#endif
    {tet_ncs_ipxs_svc_req_subscribe,7,10},
    {tet_ncs_ipxs_svc_req_subscribe,7,11},
    {tet_ncs_ipxs_svc_req_subscribe,7,12},
    {tet_ncs_ipxs_svc_req_subscribe,7,13},
    {tet_ncs_ipxs_svc_req_subscribe,7,14},
    {tet_ncs_ipxs_svc_req_subscribe,7,15},
    {tet_ncs_ipxs_svc_req_subscribe,7,16},
    {tet_ncs_ipxs_svc_req_subscribe,7,17},
    {tet_ncs_ipxs_svc_req_subscribe,7,18},
    {tet_ncs_ipxs_svc_req_subscribe,7,19},
    {tet_ncs_ipxs_svc_req_subscribe,7,20},
    {tet_ncs_ipxs_svc_req_subscribe,7,21},
    {tet_ncs_ipxs_svc_req_subscribe,7,22},
    {tet_ncs_ipxs_svc_req_subscribe,7,23},
    {tet_ncs_ipxs_svc_req_subscribe,7,24},
    {tet_ncs_ipxs_svc_req_subscribe,7,25},
    {tet_ncs_ipxs_svc_req_subscribe,7,26},
    {tet_ncs_ipxs_svc_req_subscribe,7,27},
    {tet_ncs_ipxs_svc_req_subscribe,7,28},
    {tet_ncs_ipxs_svc_req_subscribe,7,29},
    {tet_ncs_ipxs_svc_req_subscribe,7,30},

    {tet_ncs_ipxs_svc_req_unsubscribe,8,1},
    {tet_ncs_ipxs_svc_req_unsubscribe,8,2},
    {tet_ncs_ipxs_svc_req_unsubscribe,8,3},
    {tet_ncs_ipxs_svc_req_unsubscribe,8,4},

    {tet_ncs_ipxs_svc_req_ipinfo_get,9,1},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,2},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,3},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,4},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,5},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,6},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,7},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,8},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,10},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,11},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,12},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,13},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,14},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,15},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,16},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,17},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,18},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,19},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,20},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,21},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,22},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,23},
    {tet_ncs_ipxs_svc_req_ipinfo_get,9,24},

    {tet_ncs_ipxs_svc_req_ipinfo_set,10,1},
    {tet_ncs_ipxs_svc_req_ipinfo_set,10,2},
    {tet_ncs_ipxs_svc_req_ipinfo_set,10,3},
    {tet_ncs_ipxs_svc_req_ipinfo_set,10,4},
    {tet_ncs_ipxs_svc_req_ipinfo_set,10,5},
    {tet_ncs_ipxs_svc_req_ipinfo_set,10,6},
    {tet_ncs_ipxs_svc_req_ipinfo_set,10,7},
    {tet_ncs_ipxs_svc_req_ipinfo_set,10,8},
    {tet_ncs_ipxs_svc_req_ipinfo_set,10,9},
    {tet_ncs_ipxs_svc_req_ipinfo_set,10,10},
    {tet_ncs_ipxs_svc_req_ipinfo_set,10,11},
    {tet_ncs_ipxs_svc_req_ipinfo_set,10,12},
    {tet_ncs_ipxs_svc_req_ipinfo_set,10,13},
    {tet_ncs_ipxs_svc_req_ipinfo_set,10,14},


    {NULL,0},
  };

struct tet_testlist ifsv_func_test[]=
  {
#if 0
    {tet_interface_get_ifIndex,1},
    {tet_interface_get_SPT,2},
    {tet_interface_update,4},
    {tet_interface_add_subscription,5},
#endif
    /*External Subscription*/
    {tet_interface_subscribe,1},
    {tet_interface_add_subscribe,2},
    {tet_interface_upd_subscribe,3},
    {tet_interface_rmv_subscribe,4},
    {tet_interface_subscribe_all,5}, 
    /*Internal Subscription*/
    {tet_interface_int_subscribe,6},
    {tet_interface_int_add_subscribe,7},
    {tet_interface_int_upd_subscribe,8},
    {tet_interface_int_rmv_subscribe,9},
    {tet_interface_int_subscribe_all,10}, 
    {tet_interface_add_after_unsubscribe,11},
    /*Async*/
    {tet_interface_get_ifindex,12},
    {tet_interface_get_ifindex_internal,13},
    {tet_interface_get_spt,14},
    {tet_interface_get_spt_internal,15},
    /*Sync*/
    {tet_interface_get_ifindex_sync,16},
    {tet_interface_get_ifindex_internal_sync,17},
    {tet_interface_get_spt_sync,18},
    {tet_interface_get_spt_internal_sync,19},

    {tet_interface_add,24},
    {tet_interface_add_internal,25},
    {tet_interface_update,25},

    /*ipxs*/
    {tet_interface_subscribe_ipxs,20},
    {tet_interface_add_subscribe_ipxs,21},
    {tet_interface_upd_subscribe_ipxs,22},
    {tet_interface_rmv_subscribe_ipxs,23},

    {tet_interface_add_ipxs,26},
    {tet_interface_addip_ipxs,27},
    /*Async*/
    {tet_interface_get_ifindex_ipxs,28},
    {tet_interface_get_spt_ipxs,29},
    {tet_interface_get_ipaddr_ipxs,30},
    /*Sync*/
    {tet_interface_get_sync_ifindex_ipxs,31}, /*fail*/
    {tet_interface_get_sync_spt_ipxs,32},
    {tet_interface_get_sync_ipaddr_ipxs,33},

    {tet_interface_disable,34},
    {tet_interface_enable,35},
    {tet_interface_modify,36},
    {tet_interface_seq_alloc,37},
    {tet_interface_timer_aging,38},
#if subscription_loop == 1
    {tet_interface_addip_ipxs_loop,39},
#endif
    {NULL,0},
  };

struct tet_testlist driver_api_test[]=
  {
    {tet_ncs_ifsv_drv_svc_init_req,1,1},
    {tet_ncs_ifsv_drv_svc_init_req,1,2},
    {tet_ncs_ifsv_drv_svc_init_req,1,3},
    {tet_ncs_ifsv_drv_svc_destroy_req,2,1},
    {tet_ncs_ifsv_drv_svc_destroy_req,2,2},
    {tet_ncs_ifsv_drv_svc_destroy_req,2,3},
    {tet_ncs_ifsv_drv_svc_port_reg,3,1},
    {tet_ncs_ifsv_drv_svc_port_reg,3,2},
    {tet_ncs_ifsv_drv_svc_port_reg,3,3},
    {tet_ncs_ifsv_drv_svc_port_reg,3,4},
    {tet_ncs_ifsv_drv_svc_port_reg,3,5},
    {tet_ncs_ifsv_drv_svc_port_reg,3,6},
    {tet_ncs_ifsv_drv_svc_port_reg,3,7},
    {tet_ncs_ifsv_drv_svc_port_reg,3,8},
    {tet_ncs_ifsv_drv_svc_port_reg,3,9},
    {tet_ncs_ifsv_drv_svc_port_reg,3,10},
    {tet_ncs_ifsv_drv_svc_port_reg,3,11},
    {tet_ncs_ifsv_drv_svc_port_reg,3,12},
    {tet_ncs_ifsv_drv_svc_port_reg,3,13},
    {tet_ncs_ifsv_drv_svc_port_reg,3,14},
    {tet_ncs_ifsv_drv_svc_port_reg,3,15},
    {tet_ncs_ifsv_drv_svc_port_reg,3,16},
    {tet_ncs_ifsv_drv_svc_port_reg,3,17},
    {tet_ncs_ifsv_drv_svc_port_reg,3,18},
    {tet_ncs_ifsv_drv_svc_port_reg,3,19},
    {tet_ncs_ifsv_drv_svc_send_req,4,1},
    {tet_ncs_ifsv_drv_svc_send_req,4,2},
    {tet_ncs_ifsv_drv_svc_send_req,4,3},/* run this seperately */
    {NULL,0},
  };

struct tet_testlist driver_func_test[]=
  {
    {tet_ifsv_driver_add_interface,1},
    {tet_ifsv_driver_add_multiple_interface,2},
    {tet_ifsv_driver_disable_interface,3},
    {tet_ifsv_driver_enable_interface,4},
    {tet_ifsv_driver_modify_interface,5},
    {tet_ifsv_driver_seq_alloc,6},
    {tet_ifsv_driver_timer_aging,7},
    {NULL,0},
  };


struct tet_testlist *ifsv_testlist[]={
  [IFSV_API_TEST]=ifsv_api_test,
  [IFSV_FUNC_TEST]=ifsv_func_test,
};

struct tet_testlist *ifsv_driver_testlist[]={
  [IFSV_API_TEST]=driver_api_test,
  [IFSV_FUNC_TEST]=driver_func_test,
};


#else

/********** Test Case************/
void tet_ncs_ifsv_svc_req_subscribe_01()
{
   tet_ncs_ifsv_svc_req_subscribe(1);
}

void tet_ncs_ifsv_svc_req_subscribe_02()
{
   tet_ncs_ifsv_svc_req_subscribe(2);
}

void tet_ncs_ifsv_svc_req_subscribe_03()
{
   tet_ncs_ifsv_svc_req_subscribe(3);
}

void tet_ncs_ifsv_svc_req_subscribe_04()
{
   tet_ncs_ifsv_svc_req_subscribe(4);
}

void tet_ncs_ifsv_svc_req_subscribe_05()
{
   tet_ncs_ifsv_svc_req_subscribe(5);
}

void tet_ncs_ifsv_svc_req_subscribe_06()
{
   tet_ncs_ifsv_svc_req_subscribe(6);
}

void tet_ncs_ifsv_svc_req_subscribe_07()
{
   tet_ncs_ifsv_svc_req_subscribe(7);
}

void tet_ncs_ifsv_svc_req_subscribe_08()
{
   tet_ncs_ifsv_svc_req_subscribe(8);
}

void tet_ncs_ifsv_svc_req_subscribe_09()
{
   tet_ncs_ifsv_svc_req_subscribe(9);
}

void tet_ncs_ifsv_svc_req_subscribe_10()
{
   tet_ncs_ifsv_svc_req_subscribe(10);
}

void tet_ncs_ifsv_svc_req_subscribe_11()
{
   tet_ncs_ifsv_svc_req_subscribe(11);
}

void tet_ncs_ifsv_svc_req_subscribe_12()
{
   tet_ncs_ifsv_svc_req_subscribe(12);
}

void tet_ncs_ifsv_svc_req_subscribe_13()
{
   tet_ncs_ifsv_svc_req_subscribe(13);
}

void tet_ncs_ifsv_svc_req_subscribe_14()
{
   tet_ncs_ifsv_svc_req_subscribe(14);
}

void tet_ncs_ifsv_svc_req_subscribe_15()
{
   tet_ncs_ifsv_svc_req_subscribe(15);
}

void tet_ncs_ifsv_svc_req_subscribe_16()
{
   tet_ncs_ifsv_svc_req_subscribe(16);
}

void tet_ncs_ifsv_svc_req_subscribe_17()
{
   tet_ncs_ifsv_svc_req_subscribe(17);
}

void tet_ncs_ifsv_svc_req_subscribe_18()
{
   tet_ncs_ifsv_svc_req_subscribe(18);
}

void tet_ncs_ifsv_svc_req_subscribe_19()
{
   tet_ncs_ifsv_svc_req_subscribe(19);
}

void tet_ncs_ifsv_svc_req_subscribe_20()
{
   tet_ncs_ifsv_svc_req_subscribe(20);
}

void tet_ncs_ifsv_svc_req_subscribe_21()
{
   tet_ncs_ifsv_svc_req_subscribe(21);
}

void tet_ncs_ifsv_svc_req_subscribe_22()
{
   tet_ncs_ifsv_svc_req_subscribe(22);
}

void tet_ncs_ifsv_svc_req_subscribe_23()
{
   tet_ncs_ifsv_svc_req_subscribe(23);
}

void tet_ncs_ifsv_svc_req_subscribe_24()
{
   tet_ncs_ifsv_svc_req_subscribe(24);
}

void tet_ncs_ifsv_svc_req_unsubscribe_01()
{
   tet_ncs_ifsv_svc_req_unsubscribe(1);
}

void tet_ncs_ifsv_svc_req_unsubscribe_02()
{
   tet_ncs_ifsv_svc_req_unsubscribe(2);
}

void tet_ncs_ifsv_svc_req_unsubscribe_03()
{
   tet_ncs_ifsv_svc_req_unsubscribe(3);
}

void tet_ncs_ifsv_svc_req_unsubscribe_04()
{
   tet_ncs_ifsv_svc_req_unsubscribe(4);
}

void tet_ncs_ifsv_svc_req_ifrec_add_01()
{
   tet_ncs_ifsv_svc_req_ifrec_add(1);
}

void tet_ncs_ifsv_svc_req_ifrec_add_02()
{
   tet_ncs_ifsv_svc_req_ifrec_add(2);
}

void tet_ncs_ifsv_svc_req_ifrec_add_03()
{
   tet_ncs_ifsv_svc_req_ifrec_add(3);
}

void tet_ncs_ifsv_svc_req_ifrec_add_04()
{
   tet_ncs_ifsv_svc_req_ifrec_add(4);
}

void tet_ncs_ifsv_svc_req_ifrec_add_05()
{
   tet_ncs_ifsv_svc_req_ifrec_add(5);
}

void tet_ncs_ifsv_svc_req_ifrec_add_06()
{
   tet_ncs_ifsv_svc_req_ifrec_add(6);
}

void tet_ncs_ifsv_svc_req_ifrec_add_07()
{
   tet_ncs_ifsv_svc_req_ifrec_add(7);
}

void tet_ncs_ifsv_svc_req_ifrec_add_08()
{
   tet_ncs_ifsv_svc_req_ifrec_add(8);
}

void tet_ncs_ifsv_svc_req_ifrec_add_09()
{
   tet_ncs_ifsv_svc_req_ifrec_add(9);
}

void tet_ncs_ifsv_svc_req_ifrec_add_10()
{
   tet_ncs_ifsv_svc_req_ifrec_add(10);
}

void tet_ncs_ifsv_svc_req_ifrec_add_11()
{
   tet_ncs_ifsv_svc_req_ifrec_add(11);
}

void tet_ncs_ifsv_svc_req_ifrec_add_12()
{
   tet_ncs_ifsv_svc_req_ifrec_add(12);
}

void tet_ncs_ifsv_svc_req_ifrec_add_13()
{
   tet_ncs_ifsv_svc_req_ifrec_add(13);
}

void tet_ncs_ifsv_svc_req_ifrec_add_14()
{
   tet_ncs_ifsv_svc_req_ifrec_add(14);
}

void tet_ncs_ifsv_svc_req_ifrec_add_15()
{
   tet_ncs_ifsv_svc_req_ifrec_add(15);
}

void tet_ncs_ifsv_svc_req_ifrec_add_16()
{
   tet_ncs_ifsv_svc_req_ifrec_add(16);
}

void tet_ncs_ifsv_svc_req_ifrec_add_17()
{
   tet_ncs_ifsv_svc_req_ifrec_add(17);
}

void tet_ncs_ifsv_svc_req_ifrec_add_18()
{
   tet_ncs_ifsv_svc_req_ifrec_add(18);
}

void tet_ncs_ifsv_svc_req_ifrec_add_19()
{
   tet_ncs_ifsv_svc_req_ifrec_add(19);
}

void tet_ncs_ifsv_svc_req_ifrec_add_20()
{
   tet_ncs_ifsv_svc_req_ifrec_add(20);
}

void tet_ncs_ifsv_svc_req_ifrec_add_21()
{
   tet_ncs_ifsv_svc_req_ifrec_add(21);
}

void tet_ncs_ifsv_svc_req_ifrec_add_22()
{
   tet_ncs_ifsv_svc_req_ifrec_add(22);
}

void tet_ncs_ifsv_svc_req_ifrec_add_23()
{
   tet_ncs_ifsv_svc_req_ifrec_add(23);
}

void tet_ncs_ifsv_svc_req_ifrec_add_24()
{
   tet_ncs_ifsv_svc_req_ifrec_add(24);
}

void tet_ncs_ifsv_svc_req_ifrec_add_25()
{
   tet_ncs_ifsv_svc_req_ifrec_add(25);
}

void tet_ncs_ifsv_svc_req_ifrec_add_26()
{
   tet_ncs_ifsv_svc_req_ifrec_add(26);
}

void tet_ncs_ifsv_svc_req_ifrec_add_27()
{
   tet_ncs_ifsv_svc_req_ifrec_add(27);
}

void tet_ncs_ifsv_svc_req_ifrec_add_28()
{
   tet_ncs_ifsv_svc_req_ifrec_add(28);
}

void tet_ncs_ifsv_svc_req_ifrec_add_29()
{
   tet_ncs_ifsv_svc_req_ifrec_add(29);
}

void tet_ncs_ifsv_svc_req_ifrec_add_30()
{
   tet_ncs_ifsv_svc_req_ifrec_add(30);
}

void tet_ncs_ifsv_svc_req_ifrec_add_31()
{
   tet_ncs_ifsv_svc_req_ifrec_add(31);
}

void tet_ncs_ifsv_svc_req_ifrec_add_32()
{
   tet_ncs_ifsv_svc_req_ifrec_add(32);;
}

void tet_ncs_ifsv_svc_req_ifrec_add_33()
{
   tet_ncs_ifsv_svc_req_ifrec_add(33);
}

void tet_ncs_ifsv_svc_req_ifrec_add_34()
{
   tet_ncs_ifsv_svc_req_ifrec_add(34);
}

void tet_ncs_ifsv_svc_req_ifrec_add_35()
{
   tet_ncs_ifsv_svc_req_ifrec_add(35);
}

void tet_ncs_ifsv_svc_req_ifrec_add_36()
{
   tet_ncs_ifsv_svc_req_ifrec_add(36);
}

void tet_ncs_ifsv_svc_req_ifrec_add_37()
{
   tet_ncs_ifsv_svc_req_ifrec_add(37);
}

void tet_ncs_ifsv_svc_req_ifrec_add_38()
{
   tet_ncs_ifsv_svc_req_ifrec_add(38);
}

void tet_ncs_ifsv_svc_req_ifrec_add_39()
{
   tet_ncs_ifsv_svc_req_ifrec_add(39);
}

void tet_ncs_ifsv_svc_req_ifrec_add_40()
{
   tet_ncs_ifsv_svc_req_ifrec_add(40);
}

void tet_ncs_ifsv_svc_req_ifrec_add_41()
{
   tet_ncs_ifsv_svc_req_ifrec_add(41);
}

void tet_ncs_ifsv_svc_req_ifrec_del_01()
{
   tet_ncs_ifsv_svc_req_ifrec_del(1);
}

void tet_ncs_ifsv_svc_req_ifrec_del_02()
{
   tet_ncs_ifsv_svc_req_ifrec_del(2);
}

void tet_ncs_ifsv_svc_req_ifrec_del_03()
{
   tet_ncs_ifsv_svc_req_ifrec_del(3);
}

void tet_ncs_ifsv_svc_req_ifrec_del_04()
{
   tet_ncs_ifsv_svc_req_ifrec_del(4);
}

void tet_ncs_ifsv_svc_req_ifrec_del_05()
{
   tet_ncs_ifsv_svc_req_ifrec_del(5);
}

void tet_ncs_ifsv_svc_req_ifrec_get_01()
{
   tet_ncs_ifsv_svc_req_ifrec_get(1);
}

void tet_ncs_ifsv_svc_req_ifrec_get_02()
{
   tet_ncs_ifsv_svc_req_ifrec_get(2);
}

void tet_ncs_ifsv_svc_req_ifrec_get_03()
{
   tet_ncs_ifsv_svc_req_ifrec_get(3);
}

void tet_ncs_ifsv_svc_req_ifrec_get_04()
{
   tet_ncs_ifsv_svc_req_ifrec_get(4);
}

void tet_ncs_ifsv_svc_req_ifrec_get_05()
{
   tet_ncs_ifsv_svc_req_ifrec_get(5);
}

void tet_ncs_ifsv_svc_req_ifrec_get_06()
{
   tet_ncs_ifsv_svc_req_ifrec_get(6);
}

void tet_ncs_ifsv_svc_req_ifrec_get_07()
{
   tet_ncs_ifsv_svc_req_ifrec_get(7);
}

void tet_ncs_ifsv_svc_req_ifrec_get_08()
{
   tet_ncs_ifsv_svc_req_ifrec_get(8);
}

void tet_ncs_ifsv_svc_req_ifrec_get_09()
{
   tet_ncs_ifsv_svc_req_ifrec_get(9);
}

void tet_ncs_ifsv_svc_req_ifrec_get_10()
{
   tet_ncs_ifsv_svc_req_ifrec_get(10);
}

void tet_ncs_ifsv_svc_req_ifrec_get_11()
{
   tet_ncs_ifsv_svc_req_ifrec_get(11);
}

void tet_ncs_ifsv_svc_req_ifrec_get_12()
{
   tet_ncs_ifsv_svc_req_ifrec_get(12);
}

void tet_ncs_ifsv_svc_req_ifrec_get_13()
{
   tet_ncs_ifsv_svc_req_ifrec_get(13);
}

void tet_ncs_ifsv_svc_req_ifrec_get_14()
{
   tet_ncs_ifsv_svc_req_ifrec_get(14);
}

void tet_ncs_ifsv_svc_req_ifrec_get_15()
{
   tet_ncs_ifsv_svc_req_ifrec_get(15);
}

void tet_ncs_ifsv_svc_req_ifrec_get_16()
{
   tet_ncs_ifsv_svc_req_ifrec_get(16);
}

void tet_ncs_ifsv_svc_req_ifrec_get_17()
{
   tet_ncs_ifsv_svc_req_ifrec_get(17);
}

void tet_ncs_ifsv_svc_req_ifrec_get_18()
{
   tet_ncs_ifsv_svc_req_ifrec_get(18);
}

void tet_ncs_ifsv_svc_req_ifrec_get_19()
{
   tet_ncs_ifsv_svc_req_ifrec_get(19);
}

void tet_ncs_ifsv_svc_req_ifrec_get_20()
{
   tet_ncs_ifsv_svc_req_ifrec_get(20);
}

void tet_ncs_ipxs_svc_req_subscribe_01()
{
   tet_ncs_ipxs_svc_req_subscribe(1);
}

void tet_ncs_ipxs_svc_req_subscribe_02()
{
   tet_ncs_ipxs_svc_req_subscribe(2);
}

void tet_ncs_ipxs_svc_req_subscribe_03()
{
   tet_ncs_ipxs_svc_req_subscribe(3);
}

void tet_ncs_ipxs_svc_req_subscribe_04()
{
   tet_ncs_ipxs_svc_req_subscribe(4);
}

void tet_ncs_ipxs_svc_req_subscribe_05()
{
   tet_ncs_ipxs_svc_req_subscribe(5);
}

void tet_ncs_ipxs_svc_req_subscribe_06()
{
   tet_ncs_ipxs_svc_req_subscribe(6);
}

void tet_ncs_ipxs_svc_req_subscribe_07()
{
   tet_ncs_ipxs_svc_req_subscribe(7);
}

void tet_ncs_ipxs_svc_req_subscribe_08()
{
   tet_ncs_ipxs_svc_req_subscribe(8);
}

void tet_ncs_ipxs_svc_req_subscribe_09()
{
   tet_ncs_ipxs_svc_req_subscribe(9);
}

void tet_ncs_ipxs_svc_req_subscribe_10()
{
   tet_ncs_ipxs_svc_req_subscribe(10);
}

void tet_ncs_ipxs_svc_req_subscribe_11()
{
   tet_ncs_ipxs_svc_req_subscribe(11);
}

void tet_ncs_ipxs_svc_req_subscribe_12()
{
   tet_ncs_ipxs_svc_req_subscribe(12);
}

void tet_ncs_ipxs_svc_req_subscribe_13()
{
   tet_ncs_ipxs_svc_req_subscribe(13);
}

void tet_ncs_ipxs_svc_req_subscribe_14()
{
   tet_ncs_ipxs_svc_req_subscribe(14);
}

void tet_ncs_ipxs_svc_req_subscribe_15()
{
   tet_ncs_ipxs_svc_req_subscribe(15);
}

void tet_ncs_ipxs_svc_req_subscribe_16()
{
   tet_ncs_ipxs_svc_req_subscribe(16);
}

void tet_ncs_ipxs_svc_req_subscribe_17()
{
   tet_ncs_ipxs_svc_req_subscribe(17);
}

void tet_ncs_ipxs_svc_req_subscribe_18()
{
   tet_ncs_ipxs_svc_req_subscribe(18);
}

void tet_ncs_ipxs_svc_req_subscribe_19()
{
   tet_ncs_ipxs_svc_req_subscribe(19);
}

void tet_ncs_ipxs_svc_req_subscribe_20()
{
   tet_ncs_ipxs_svc_req_subscribe(20);
}

void tet_ncs_ipxs_svc_req_subscribe_21()
{
   tet_ncs_ipxs_svc_req_subscribe(21);
}

void tet_ncs_ipxs_svc_req_subscribe_22()
{
   tet_ncs_ipxs_svc_req_subscribe(22);
}

void tet_ncs_ipxs_svc_req_subscribe_23()
{
   tet_ncs_ipxs_svc_req_subscribe(23);
}

void tet_ncs_ipxs_svc_req_subscribe_24()
{
   tet_ncs_ipxs_svc_req_subscribe(24);
}

void tet_ncs_ipxs_svc_req_subscribe_25()
{
   tet_ncs_ipxs_svc_req_subscribe(25);
}

void tet_ncs_ipxs_svc_req_subscribe_26()
{
   tet_ncs_ipxs_svc_req_subscribe(26);
}

void tet_ncs_ipxs_svc_req_subscribe_27()
{
   tet_ncs_ipxs_svc_req_subscribe(27);
}

void tet_ncs_ipxs_svc_req_subscribe_28()
{
   tet_ncs_ipxs_svc_req_subscribe(28);
}

void tet_ncs_ipxs_svc_req_subscribe_29()
{
   tet_ncs_ipxs_svc_req_subscribe(29);
}

void tet_ncs_ipxs_svc_req_subscribe_30()
{
   tet_ncs_ipxs_svc_req_subscribe(30);
}

void tet_ncs_ipxs_svc_req_unsubscribe_01()
{
   tet_ncs_ipxs_svc_req_unsubscribe(1);
}

void tet_ncs_ipxs_svc_req_unsubscribe_02()
{
   tet_ncs_ipxs_svc_req_unsubscribe(2);
}

void tet_ncs_ipxs_svc_req_unsubscribe_03()
{
   tet_ncs_ipxs_svc_req_unsubscribe(3);
}

void tet_ncs_ipxs_svc_req_unsubscribe_04()
{
   tet_ncs_ipxs_svc_req_unsubscribe(4);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_01()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(1);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_02()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(2);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_03()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(3);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_04()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(4);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_05()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(5);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_06()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(6);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_07()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(7);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_08()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(8);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_09()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(9);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_10()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(10);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_11()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(11);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_12()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(12);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_13()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(13);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_14()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(14);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_15()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(15);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_16()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(16);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_17()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(17);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_18()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(18);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_19()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(19);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_20()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(20);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_21()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(21);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_22()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(22);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_23()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(23);
}

void tet_ncs_ipxs_svc_req_ipinfo_get_24()
{
   tet_ncs_ipxs_svc_req_ipinfo_get(24);
}

void tet_ncs_ipxs_svc_req_ipinfo_set_01()
{
   tet_ncs_ipxs_svc_req_ipinfo_set(1);
}

void tet_ncs_ipxs_svc_req_ipinfo_set_02()
{
   tet_ncs_ipxs_svc_req_ipinfo_set(2);
}

void tet_ncs_ipxs_svc_req_ipinfo_set_03()
{
   tet_ncs_ipxs_svc_req_ipinfo_set(3);
}

void tet_ncs_ipxs_svc_req_ipinfo_set_04()
{
   tet_ncs_ipxs_svc_req_ipinfo_set(4);
}

void tet_ncs_ipxs_svc_req_ipinfo_set_05()
{
   tet_ncs_ipxs_svc_req_ipinfo_set(5);
}

void tet_ncs_ipxs_svc_req_ipinfo_set_06()
{
   tet_ncs_ipxs_svc_req_ipinfo_set(6);
}

void tet_ncs_ipxs_svc_req_ipinfo_set_07()
{
   tet_ncs_ipxs_svc_req_ipinfo_set(7);
}

void tet_ncs_ipxs_svc_req_ipinfo_set_08()
{
   tet_ncs_ipxs_svc_req_ipinfo_set(8);
}

void tet_ncs_ipxs_svc_req_ipinfo_set_09()
{
   tet_ncs_ipxs_svc_req_ipinfo_set(9);
}

void tet_ncs_ipxs_svc_req_ipinfo_set_10()
{
   tet_ncs_ipxs_svc_req_ipinfo_set(10);
}

void tet_ncs_ipxs_svc_req_ipinfo_set_11()
{
   tet_ncs_ipxs_svc_req_ipinfo_set(11);
}

void tet_ncs_ipxs_svc_req_ipinfo_set_12()
{
   tet_ncs_ipxs_svc_req_ipinfo_set(12);
}

void tet_ncs_ipxs_svc_req_ipinfo_set_13()
{
   tet_ncs_ipxs_svc_req_ipinfo_set(13);
}

void tet_ncs_ipxs_svc_req_ipinfo_set_14()
{
   tet_ncs_ipxs_svc_req_ipinfo_set(14);
}

void tet_ncs_ifsv_drv_svc_init_req_01()
{
   tet_ncs_ifsv_drv_svc_init_req(1);
}

void tet_ncs_ifsv_drv_svc_init_req_02()
{
   tet_ncs_ifsv_drv_svc_init_req(2);
}

void tet_ncs_ifsv_drv_svc_init_req_03()
{
   tet_ncs_ifsv_drv_svc_init_req(3);
}

void tet_ncs_ifsv_drv_svc_destroy_req_01()
{
   tet_ncs_ifsv_drv_svc_destroy_req(1);
}

void tet_ncs_ifsv_drv_svc_destroy_req_02()
{
   tet_ncs_ifsv_drv_svc_destroy_req(2);
}

void tet_ncs_ifsv_drv_svc_destroy_req_03()
{
   tet_ncs_ifsv_drv_svc_destroy_req(3);
}

void tet_ncs_ifsv_drv_svc_port_reg_01()
{
   tet_ncs_ifsv_drv_svc_port_reg(1);
}

void tet_ncs_ifsv_drv_svc_port_reg_02()
{
   tet_ncs_ifsv_drv_svc_port_reg(2);
}

void tet_ncs_ifsv_drv_svc_port_reg_03()
{
   tet_ncs_ifsv_drv_svc_port_reg(3);
}

void tet_ncs_ifsv_drv_svc_port_reg_04()
{
   tet_ncs_ifsv_drv_svc_port_reg(4);
}

void tet_ncs_ifsv_drv_svc_port_reg_05()
{
   tet_ncs_ifsv_drv_svc_port_reg(5);
}

void tet_ncs_ifsv_drv_svc_port_reg_06()
{
   tet_ncs_ifsv_drv_svc_port_reg(6);
}

void tet_ncs_ifsv_drv_svc_port_reg_07()
{
   tet_ncs_ifsv_drv_svc_port_reg(7);
}

void tet_ncs_ifsv_drv_svc_port_reg_08()
{
   tet_ncs_ifsv_drv_svc_port_reg(8);
}

void tet_ncs_ifsv_drv_svc_port_reg_09()
{
   tet_ncs_ifsv_drv_svc_port_reg(9);
}

void tet_ncs_ifsv_drv_svc_port_reg_10()
{
   tet_ncs_ifsv_drv_svc_port_reg(10);
}

void tet_ncs_ifsv_drv_svc_port_reg_11()
{
   tet_ncs_ifsv_drv_svc_port_reg(11);
}

void tet_ncs_ifsv_drv_svc_port_reg_12()
{
   tet_ncs_ifsv_drv_svc_port_reg(12);
}

void tet_ncs_ifsv_drv_svc_port_reg_13()
{
   tet_ncs_ifsv_drv_svc_port_reg(13);
}

void tet_ncs_ifsv_drv_svc_port_reg_14()
{
   tet_ncs_ifsv_drv_svc_port_reg(14);
}

void tet_ncs_ifsv_drv_svc_port_reg_15()
{
   tet_ncs_ifsv_drv_svc_port_reg(15);
}

void tet_ncs_ifsv_drv_svc_port_reg_16()
{
   tet_ncs_ifsv_drv_svc_port_reg(16);
}

void tet_ncs_ifsv_drv_svc_port_reg_17()
{
   tet_ncs_ifsv_drv_svc_port_reg(17);
}

void tet_ncs_ifsv_drv_svc_port_reg_18()
{
   tet_ncs_ifsv_drv_svc_port_reg(18);
}

void tet_ncs_ifsv_drv_svc_port_reg_19()
{
   tet_ncs_ifsv_drv_svc_port_reg(19);
}

void tet_ncs_ifsv_drv_svc_send_req_01()
{
   tet_ncs_ifsv_drv_svc_send_req(1);
}

void tet_ncs_ifsv_drv_svc_send_req_02()
{
   tet_ncs_ifsv_drv_svc_send_req(2);
}

void tet_ncs_ifsv_drv_svc_send_req_03()
{
   tet_ncs_ifsv_drv_svc_send_req(3);
}


/**************************************************************************/
/***************************** TET LIST ARRAYS ****************************/
/**************************************************************************/

struct tet_testlist tet_testlist[] = {

#ifdef TET_A
/******************ifsv_api_test****************/
    {tet_interface_add_subscription_loop,1},

    {tet_ncs_ifsv_svc_req_subscribe_01,2},
    {tet_ncs_ifsv_svc_req_subscribe_02,2},
#if 0
    {tet_ncs_ifsv_svc_req_subscribe_03,2},
#endif
    {tet_ncs_ifsv_svc_req_subscribe_04,2},
    {tet_ncs_ifsv_svc_req_subscribe_05,2},
    {tet_ncs_ifsv_svc_req_subscribe_06,2},
    {tet_ncs_ifsv_svc_req_subscribe_07,2},
    {tet_ncs_ifsv_svc_req_subscribe_08,2},
    {tet_ncs_ifsv_svc_req_subscribe_09,2},
    {tet_ncs_ifsv_svc_req_subscribe_10,2},
    {tet_ncs_ifsv_svc_req_subscribe_11,2},
    {tet_ncs_ifsv_svc_req_subscribe_12,2},
    {tet_ncs_ifsv_svc_req_subscribe_13,2},
    {tet_ncs_ifsv_svc_req_subscribe_14,2},
    {tet_ncs_ifsv_svc_req_subscribe_15,2}, 
    {tet_ncs_ifsv_svc_req_subscribe_16,2},
    {tet_ncs_ifsv_svc_req_subscribe_17,2},
    {tet_ncs_ifsv_svc_req_subscribe_18,2},
    {tet_ncs_ifsv_svc_req_subscribe_19,2},
    {tet_ncs_ifsv_svc_req_subscribe_20,2},
    {tet_ncs_ifsv_svc_req_subscribe_21,2},
    {tet_ncs_ifsv_svc_req_subscribe_22,2},
    {tet_ncs_ifsv_svc_req_subscribe_23,2},
    {tet_ncs_ifsv_svc_req_subscribe_24,2},

    {tet_ncs_ifsv_svc_req_unsubscribe_01,3},
    {tet_ncs_ifsv_svc_req_unsubscribe_02,3},
    {tet_ncs_ifsv_svc_req_unsubscribe_03,3},
    {tet_ncs_ifsv_svc_req_unsubscribe_04,3},

    {tet_ncs_ifsv_svc_req_ifrec_add_01,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_02,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_03,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_04,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_05,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_06,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_07,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_08,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_09,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_10,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_11,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_12,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_13,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_14,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_15,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_16,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_17,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_18,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_19,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_20,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_21,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_22,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_23,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_24,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_25,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_26,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_27,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_28,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_29,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_30,4},
#if 0
    {tet_ncs_ifsv_svc_req_ifrec_add_31,4},
#endif
    {tet_ncs_ifsv_svc_req_ifrec_add_32,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_33,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_34,4},
#if 0
    /*no cases*/
    {tet_ncs_ifsv_svc_req_ifrec_add_35,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_36,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_37,4},
#endif
    {tet_ncs_ifsv_svc_req_ifrec_add_38,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_39,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_40,4},
    {tet_ncs_ifsv_svc_req_ifrec_add_41,4},

    {tet_ncs_ifsv_svc_req_ifrec_del_01,5},
    {tet_ncs_ifsv_svc_req_ifrec_del_02,5},
    {tet_ncs_ifsv_svc_req_ifrec_del_03,5},
    {tet_ncs_ifsv_svc_req_ifrec_del_04,5},
    {tet_ncs_ifsv_svc_req_ifrec_del_05,5},

    {tet_ncs_ifsv_svc_req_ifrec_get_01,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_02,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_03,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_04,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_05,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_06,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_07,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_08,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_09,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_10,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_11,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_12,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_13,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_14,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_15,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_16,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_17,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_18,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_19,6},
    {tet_ncs_ifsv_svc_req_ifrec_get_20,6},

    /*ipxs test cases*/
    {tet_ncs_ipxs_svc_req_subscribe_01,7},
    {tet_ncs_ipxs_svc_req_subscribe_02,7},
    {tet_ncs_ipxs_svc_req_subscribe_03,7},
    {tet_ncs_ipxs_svc_req_subscribe_04,7},
    {tet_ncs_ipxs_svc_req_subscribe_05,7},
    {tet_ncs_ipxs_svc_req_subscribe_06,7},
    {tet_ncs_ipxs_svc_req_subscribe_07,7},
#if 0
    /*why 8 n 9 r commented:not supported scope*/
    {tet_ncs_ipxs_svc_req_subscribe_08,7},
    {tet_ncs_ipxs_svc_req_subscribe_09,7},
#endif
    {tet_ncs_ipxs_svc_req_subscribe_10,7},
    {tet_ncs_ipxs_svc_req_subscribe_11,7},
    {tet_ncs_ipxs_svc_req_subscribe_12,7},
    {tet_ncs_ipxs_svc_req_subscribe_13,7},
    {tet_ncs_ipxs_svc_req_subscribe_14,7},
    {tet_ncs_ipxs_svc_req_subscribe_15,7},
    {tet_ncs_ipxs_svc_req_subscribe_16,7},
    {tet_ncs_ipxs_svc_req_subscribe_17,7},
    {tet_ncs_ipxs_svc_req_subscribe_18,7},
    {tet_ncs_ipxs_svc_req_subscribe_19,7},
    {tet_ncs_ipxs_svc_req_subscribe_20,7},
    {tet_ncs_ipxs_svc_req_subscribe_21,7},
    {tet_ncs_ipxs_svc_req_subscribe_22,7},
    {tet_ncs_ipxs_svc_req_subscribe_23,7},
    {tet_ncs_ipxs_svc_req_subscribe_24,7},
    {tet_ncs_ipxs_svc_req_subscribe_25,7},
    {tet_ncs_ipxs_svc_req_subscribe_26,7},
    {tet_ncs_ipxs_svc_req_subscribe_27,7},
    {tet_ncs_ipxs_svc_req_subscribe_28,7},
    {tet_ncs_ipxs_svc_req_subscribe_29,7},
    {tet_ncs_ipxs_svc_req_subscribe_30,7},

    {tet_ncs_ipxs_svc_req_unsubscribe_01,8},
    {tet_ncs_ipxs_svc_req_unsubscribe_02,8},
    {tet_ncs_ipxs_svc_req_unsubscribe_03,8},
    {tet_ncs_ipxs_svc_req_unsubscribe_04,8},

    {tet_ncs_ipxs_svc_req_ipinfo_get_01,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_02,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_03,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_04,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_05,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_06,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_07,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_08,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_09,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_10,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_11,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_12,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_13,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_14,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_15,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_16,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_17,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_18,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_19,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_20,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_21,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_22,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_23,9},
    {tet_ncs_ipxs_svc_req_ipinfo_get_24,9},

    {tet_ncs_ipxs_svc_req_ipinfo_set_01,10},
    {tet_ncs_ipxs_svc_req_ipinfo_set_02,10},
    {tet_ncs_ipxs_svc_req_ipinfo_set_03,10},
    {tet_ncs_ipxs_svc_req_ipinfo_set_04,10},
    {tet_ncs_ipxs_svc_req_ipinfo_set_05,10},
    {tet_ncs_ipxs_svc_req_ipinfo_set_06,10},
    {tet_ncs_ipxs_svc_req_ipinfo_set_07,10},
    {tet_ncs_ipxs_svc_req_ipinfo_set_08,10},
    {tet_ncs_ipxs_svc_req_ipinfo_set_09,10},
    {tet_ncs_ipxs_svc_req_ipinfo_set_10,10},
    {tet_ncs_ipxs_svc_req_ipinfo_set_11,10},
    {tet_ncs_ipxs_svc_req_ipinfo_set_12,10},
    {tet_ncs_ipxs_svc_req_ipinfo_set_13,10},
    {tet_ncs_ipxs_svc_req_ipinfo_set_14,10},

/*****************ifsv_func_test******************/
  
#if 0
    {tet_interface_get_ifIndex,1},
    {tet_interface_get_SPT,2},
    {tet_interface_update,4},
    {tet_interface_add_subscription,5},
#endif
    /*External Subscription*/
    {tet_interface_subscribe,11},
    {tet_interface_add_subscribe,12},
    {tet_interface_upd_subscribe,13},
    {tet_interface_rmv_subscribe,14},
    {tet_interface_subscribe_all,15}, 
    /*Internal Subscription*/
    {tet_interface_int_subscribe,16},
    {tet_interface_int_add_subscribe,17},
    {tet_interface_int_upd_subscribe,18},
    {tet_interface_int_rmv_subscribe,19},
    {tet_interface_int_subscribe_all,20}, 
    {tet_interface_add_after_unsubscribe,21},
    /*Async*/
    {tet_interface_get_ifindex,22},
    {tet_interface_get_ifindex_internal,23},
    {tet_interface_get_spt,24},
    {tet_interface_get_spt_internal,25},
    /*Sync*/
    {tet_interface_get_ifindex_sync,26},
    {tet_interface_get_ifindex_internal_sync,27},
    {tet_interface_get_spt_sync,28},
    {tet_interface_get_spt_internal_sync,29},

    {tet_interface_add,24},
    {tet_interface_add_internal,25},
    {tet_interface_update,25},

    /*ipxs*/
    {tet_interface_subscribe_ipxs,26},
    {tet_interface_add_subscribe_ipxs,27},
    {tet_interface_upd_subscribe_ipxs,28},
    {tet_interface_rmv_subscribe_ipxs,29},

    {tet_interface_add_ipxs,30},
    {tet_interface_addip_ipxs,31},
    /*Async*/
    {tet_interface_get_ifindex_ipxs,32},
    {tet_interface_get_spt_ipxs,33},
    {tet_interface_get_ipaddr_ipxs,34},
    /*Sync*/
    {tet_interface_get_sync_ifindex_ipxs,35}, /*fail*/
    {tet_interface_get_sync_spt_ipxs,36},
    {tet_interface_get_sync_ipaddr_ipxs,37},

    {tet_interface_disable,38},
    {tet_interface_enable,39},
    {tet_interface_modify,40},
    {tet_interface_seq_alloc,41},
    {tet_interface_timer_aging,42},
#if subscription_loop == 1
    {tet_interface_addip_ipxs_loop,43},
#endif

#endif /*TET_A*/
    
#ifdef TET_DRIVER

    /*****************driver_func_test[]******************/

    {tet_ncs_ifsv_drv_svc_init_req_01,1},
    {tet_ncs_ifsv_drv_svc_init_req_02,1},
    {tet_ncs_ifsv_drv_svc_init_req_03,1},
    
    {tet_ncs_ifsv_drv_svc_destroy_req_01,2},
    {tet_ncs_ifsv_drv_svc_destroy_req_02,2},
    {tet_ncs_ifsv_drv_svc_destroy_req_03,2},
    
    {tet_ncs_ifsv_drv_svc_port_reg_01,3},
    {tet_ncs_ifsv_drv_svc_port_reg_02,3},
    {tet_ncs_ifsv_drv_svc_port_reg_03,3},
    {tet_ncs_ifsv_drv_svc_port_reg_04,3},
    {tet_ncs_ifsv_drv_svc_port_reg_05,3},
    {tet_ncs_ifsv_drv_svc_port_reg_06,3},
    {tet_ncs_ifsv_drv_svc_port_reg_07,3},
    {tet_ncs_ifsv_drv_svc_port_reg_08,3},
    {tet_ncs_ifsv_drv_svc_port_reg_09,3},
    {tet_ncs_ifsv_drv_svc_port_reg_10,3},
    {tet_ncs_ifsv_drv_svc_port_reg_11,3},
    {tet_ncs_ifsv_drv_svc_port_reg_12,3},
    {tet_ncs_ifsv_drv_svc_port_reg_13,3},
    {tet_ncs_ifsv_drv_svc_port_reg_14,3},
    {tet_ncs_ifsv_drv_svc_port_reg_15,3},
    {tet_ncs_ifsv_drv_svc_port_reg_16,3},
    {tet_ncs_ifsv_drv_svc_port_reg_17,3},
    {tet_ncs_ifsv_drv_svc_port_reg_18,3},
    {tet_ncs_ifsv_drv_svc_port_reg_19,3},
    
    {tet_ncs_ifsv_drv_svc_send_req_01,4},
    {tet_ncs_ifsv_drv_svc_send_req_02,4},
    {tet_ncs_ifsv_drv_svc_send_req_03,4},/* run this seperately */
    
/************driver_func_test[]*********************/
  
    {tet_ifsv_driver_add_interface,1},
    {tet_ifsv_driver_add_multiple_interface,2},
    {tet_ifsv_driver_disable_interface,3},
    {tet_ifsv_driver_enable_interface,4},
    {tet_ifsv_driver_modify_interface,5},
    {tet_ifsv_driver_seq_alloc,6},
    {tet_ifsv_driver_timer_aging,7},
    
#endif /*TET_DRIVER*/

    {NULL,-1},
  };


#endif /*TET_PATCH==1*/


void tet_run_ifsv_app()
{
#if (TET_PATCH==1)
  int iterCount,listCount;
#endif

  char *temp=NULL;

  gl_ifsv_defs();

  temp=(char *)getenv("TET_SHELF_ID");
  shelf_id=atoi(temp);

  temp=(char *)getenv("TET_SLOT_ID");
  slot_id=atoi(temp);

#if (TET_PATCH==1)

  tware_mem_ign();

#ifdef TET_A

#if 0

  req_info=(NCS_IFSV_SVC_REQ *)malloc(sizeof(NCS_IFSV_SVC_REQ));
  
  info=(NCS_IPXS_SVC_REQ *)malloc(sizeof(NCS_IPXS_REQ_INFO));

  m_NCS_OS_MEMSET(req_info,'\0',sizeof(NCS_IFSV_SVC_REQ));

  m_NCS_OS_MEMSET(info,'\0',sizeof(NCS_IPXS_REQ_INFO));

#endif

   

  for(iterCount=1;iterCount<=gl_iteration;iterCount++)
    {
      if(gl_listNumber==-1)
        {
            
              for(listCount=1;listCount<3;listCount++)
                {
                  tet_test_start(gl_tCase,ifsv_testlist[listCount]);
                }
            
        }
      else
        {
          if(gl_listNumber<3) 
            {
              tet_test_start(gl_tCase,ifsv_testlist[gl_listNumber]);
            }
        }
        
      printf("\nNumber of iterations: %d",iterCount);
      tet_printf("\nNumber of iterations: %d",iterCount);
#if 0
      sleep(5);
#endif
    }

#endif
      
#ifdef TET_DRIVER

#if 0

  drv_info=(NCS_IFSV_DRV_SVC_REQ *)malloc(sizeof(NCS_IFSV_DRV_SVC_REQ));

  m_NCS_OS_MEMSET(info,'\0',sizeof(NCS_IFSV_DRV_SVC_REQ));

#endif

  for(iterCount=1;iterCount<=gl_iteration;iterCount++)
    {
      if(gl_listNumber==-1)
        {
          for(listCount=1;listCount<3;listCount++)
            {
              tet_test_start(gl_tCase,ifsv_driver_testlist[listCount]);
            }
        }
      else
        {
          if(gl_listNumber<3)
            {
              tet_test_start(gl_tCase,ifsv_driver_testlist[gl_listNumber]);
            }
        }
      printf("\nNumber of iterations: %d",iterCount);
      tet_printf("\nNumber of iterations: %d",iterCount);
#if 0
      sleep(5);
#endif
    }

#endif

  printf("\nPRESS ENTER TO GET MEM DUMP");
  getchar();
  if(cmpip.info.add.i_ipinfo!=NULL)
    {
      free(cmpip.info.add.i_ipinfo); 
      cmpip.info.add.i_ipinfo=NULL;
    }
  tware_mem_dump();
  sleep(2);
  tware_mem_dump();
  tet_test_cleanup(); 

#endif
}


void tet_ifsv_startup()
{
  /* Using the default mode for startup */
    ncs_agents_startup(0,NULL); 
#if 0
  /*First Truncate the file bond.text*/
  FILE *fp;
  char inputfile[]="/opt/motorola/tetware/ncs/ifsv/suites/bond.text";
  if( (fp= fopen(inputfile,"w")) == NULL)
    perror("File not opened");
  else
    {
      if(fclose(fp)==EOF)
        perror("File not closed");
    }
  /*Now run*/
#endif
  tet_run_ifsv_app();
}

void tet_ifsv_end()
{
  tet_infoline(" Ending the agony.. ");
}
