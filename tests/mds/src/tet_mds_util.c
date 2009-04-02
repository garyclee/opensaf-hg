/********************************************************************************************
#
# modification:
# 2008-11   Jony <jiangnan_hw@huawei.com>  Huawei Technologies CO., LTD.
#  1.  Provide test case tet_startup and tet_cleanup functions. The tet_startup function 
#      is defined as tet_mds_startup(), as well as the tet_cleanup function is defined 
#      as tet_mds_end().
#  2.  If "tetware-patch" is not used, change the name of test case array from svc_testlist[] 
#      to tet_testlist[]. 
#  3.  If "tetware-patch" is not used, delete the third parameter of each item in this array. 
#      And add some functions to implement the function of those parameters.
#  4.  If "tetware-patch" is not used, delete tet_mds_tds_cleanup() and parts of 
#      tet_mds_tds_startup() function, which invoke the functions in the patch.
#  5.  If "tetware-patch" is required, one can add a macro definition "TET_PATCH=1" in 
#      compilation process.
#
**********************************************************************************************/


#include "tet_startup.h"
#include "mds_papi.h"
#include "ncs_mda_papi.h"
#include "tet_mdstipc.h"

void tet_mds_tds_startup(void);

void tet_mds_startup(void);
void tet_mds_cleanup(void);

void (*tet_startup)()=tet_mds_startup;
void (*tet_cleanup)()=tet_mds_cleanup;


#if (TET_PATCH==1)

struct tet_testlist svc_testlist[]={
  {tet_svc_install_tp,1,1},
  {tet_svc_install_tp,1,2},
  {tet_svc_install_tp,1,3},
  {tet_svc_install_tp,1,5},
  {tet_svc_install_tp,1,6},
  {tet_svc_install_tp,1,7},
  {tet_svc_install_tp,1,8},
  {tet_svc_install_tp,1,9},
  {tet_svc_install_tp,1,10},
  {tet_svc_install_tp,1,11},
  {tet_svc_install_tp,1,12},
  {tet_svc_install_tp,1,13},
  {tet_svc_install_tp,1,14},
  {tet_svc_install_tp,1,15},
  {tet_svc_install_tp,1,14},
  {tet_svc_install_tp,1,16},
  {tet_svc_install_tp,1,17},

  {tet_svc_unstall_tp,2,1},
  {tet_svc_unstall_tp,2,2},
  {tet_svc_unstall_tp,2,3},
  {tet_svc_unstall_tp,2,4},
  {tet_svc_unstall_tp,2,5},
  
  {tet_svc_install_upto_MAX,3},

  {tet_svc_subscr_ADEST,4,1},
  {tet_svc_subscr_ADEST,4,2},
  {tet_svc_subscr_ADEST,4,3},
  {tet_svc_subscr_ADEST,4,4},
  {tet_svc_subscr_ADEST,4,5},
  {tet_svc_subscr_ADEST,4,6},
  {tet_svc_subscr_ADEST,4,7},
  {tet_svc_subscr_ADEST,4,8},
  {tet_svc_subscr_ADEST,4,9},
  {tet_svc_subscr_ADEST,4,10},
  {tet_svc_subscr_ADEST,4,11},
  {tet_svc_subscr_ADEST,4,12},

  {tet_svc_subscr_VDEST,5,1},
  {tet_svc_subscr_VDEST,5,2},
  {tet_svc_subscr_VDEST,5,3},
  {tet_svc_subscr_VDEST,5,4},
  {tet_svc_subscr_VDEST,5,5},
  {tet_svc_subscr_VDEST,5,6},
  {tet_svc_subscr_VDEST,5,7},
  {tet_svc_subscr_VDEST,5,8},
  {tet_svc_subscr_VDEST,5,9},
  {tet_svc_subscr_VDEST,5,10},
  {tet_svc_subscr_VDEST,5,11},
  {tet_svc_subscr_VDEST,5,12},

  {tet_just_send_tp,7,1},
  {tet_just_send_tp,7,2},
  {tet_just_send_tp,7,3},
  {tet_just_send_tp,7,4},
  {tet_just_send_tp,7,5},
  {tet_just_send_tp,7,6},
  {tet_just_send_tp,7,7},
  {tet_just_send_tp,7,8},
  {tet_just_send_tp,7,9},
  {tet_just_send_tp,7,10},
  {tet_just_send_tp,7,11},
  {tet_just_send_tp,7,12},
  {tet_just_send_tp,7,13},
  {tet_just_send_tp,7,14},

  {tet_send_ack_tp,8,1},
  {tet_send_ack_tp,8,2},
  {tet_send_ack_tp,8,3},
  {tet_send_ack_tp,8,4},
  {tet_send_ack_tp,8,5},
  {tet_send_ack_tp,8,6},
  {tet_send_ack_tp,8,7},
  {tet_send_ack_tp,8,8},
  {tet_send_ack_tp,8,9},
  {tet_send_ack_tp,8,10},
  {tet_send_ack_tp,8,11},
  {tet_send_ack_tp,8,12},
  {tet_send_ack_tp,8,13},
  {tet_broadcast_to_svc_tp,11,1},
  {tet_broadcast_to_svc_tp,11,2},
  {tet_broadcast_to_svc_tp,11,3},
  {tet_broadcast_to_svc_tp,11,4},
  {tet_broadcast_to_svc_tp,11,5},
  {tet_broadcast_to_svc_tp,11,6},

  {tet_send_response_tp,12,1},
  {tet_send_response_tp,12,2},
  {tet_send_response_tp,12,3},
  {tet_send_response_tp,12,4},    
  {tet_send_response_tp,12,5}, 
  {tet_send_response_tp,12,6},
  {tet_send_response_tp,12,7},
  {tet_send_response_tp,12,8},
  {tet_send_response_tp,12,9},
  {tet_send_response_tp,12,10},
  {tet_send_response_tp,12,11},

  {tet_send_response_ack_tp,13,1},
  {tet_send_response_ack_tp,13,2},
  {tet_send_response_ack_tp,13,3},
  {tet_send_response_ack_tp,13,4},
  {tet_send_response_ack_tp,13,5},
  {tet_send_response_ack_tp,13,6},
  {tet_send_response_ack_tp,13,7},
  {tet_send_response_ack_tp,13,8},
  {tet_direct_just_send_tp,16,1},
  {tet_direct_just_send_tp,16,2},
  {tet_direct_just_send_tp,16,3},
  {tet_direct_just_send_tp,16,4},
  {tet_direct_just_send_tp,16,5},
  {tet_direct_just_send_tp,16,6},
  {tet_direct_just_send_tp,16,7},
  {tet_direct_just_send_tp,16,8},
  {tet_direct_just_send_tp,16,9},
  {tet_direct_just_send_tp,16,10},
  {tet_direct_just_send_tp,16,11},
  {tet_direct_just_send_tp,16,12},
  {tet_direct_just_send_tp,16,13},
  {tet_direct_just_send_tp,16,14},
  {tet_direct_just_send_tp,16,15},

  {tet_direct_send_ack_tp,17,1},
  {tet_direct_send_ack_tp,17,2},
  {tet_direct_send_ack_tp,17,3},
  {tet_direct_send_ack_tp,17,4},
  {tet_direct_send_ack_tp,17,5},
  {tet_direct_send_ack_tp,17,6},
  {tet_direct_send_ack_tp,17,7},
  {tet_direct_send_ack_tp,17,8},
  {tet_direct_send_ack_tp,17,9},
  {tet_direct_send_ack_tp,17,10},
  {tet_direct_send_ack_tp,17,11},
  {tet_direct_send_ack_tp,17,12}, 
  {tet_direct_send_ack_tp,17,13},


  {tet_direct_broadcast_to_svc_tp,20,1},
  {tet_direct_broadcast_to_svc_tp,20,2},
  {tet_direct_broadcast_to_svc_tp,20,3},
  {tet_direct_broadcast_to_svc_tp,20,4},
  {tet_direct_broadcast_to_svc_tp,20,5},
  {tet_direct_broadcast_to_svc_tp,20,6},
  {tet_direct_broadcast_to_svc_tp,20,7},
  {tet_direct_broadcast_to_svc_tp,20,8},

  {tet_direct_send_response_tp,21,1},
  {tet_direct_send_response_tp,21,2},
  {tet_direct_send_response_tp,21,3},
  {tet_direct_send_response_tp,21,4},
  {tet_direct_send_response_tp,21,5},

  {tet_direct_send_response_ack_tp,22,1},
  {tet_direct_send_response_ack_tp,22,2},
  {tet_direct_send_response_ack_tp,22,3},
  {tet_direct_send_response_ack_tp,22,4},
  {tet_direct_send_response_ack_tp,22,5},
  {ncs_agents_shut_start,25,0},

  {tet_send_all_tp,25,1},
  {tet_send_all_tp,25,2},

  {tet_direct_send_all_tp,26,1},
  {tet_direct_send_all_tp,26,2},
  {tet_direct_send_all_tp,26,3},
  {tet_direct_send_all_tp,26,4},
  {tet_direct_send_all_tp,26,5},
  {tet_direct_send_all_tp,26,6},

  {tet_query_pwe_tp,30,1},
  {tet_query_pwe_tp,30,2},

  {NULL,0}
};

#else

/*****************Function Modification*******************/
void tet_svc_install_tp_01()
{
  tet_svc_install_tp(1);
}

void tet_svc_install_tp_02()
{
  tet_svc_install_tp(2);
}

void tet_svc_install_tp_03()
{
  tet_svc_install_tp(3);
}

void tet_svc_install_tp_05()
{
  tet_svc_install_tp(5);
}

void tet_svc_install_tp_06()
{
  tet_svc_install_tp(6);
}

void tet_svc_install_tp_07()
{
  tet_svc_install_tp(7);
}

void tet_svc_install_tp_08()
{
  tet_svc_install_tp(8);
}

void tet_svc_install_tp_09()
{
  tet_svc_install_tp(9);
}

void tet_svc_install_tp_10()
{
  tet_svc_install_tp(10);
}

void tet_svc_install_tp_11()
{
  tet_svc_install_tp(11);
}

void tet_svc_install_tp_12()
{
  tet_svc_install_tp(12);
}

void tet_svc_install_tp_13()
{
  tet_svc_install_tp(13);
}

void tet_svc_install_tp_14()
{
  tet_svc_install_tp(14);
}

void tet_svc_install_tp_15()
{
  tet_svc_install_tp(15);
}

void tet_svc_install_tp_16()
{
  tet_svc_install_tp(16);
}

void tet_svc_install_tp_17()
{
  tet_svc_install_tp(17);
}

void tet_svc_unstall_tp_01()
{
  tet_svc_unstall_tp(1);
}

void tet_svc_unstall_tp_02()
{
  tet_svc_unstall_tp(2);
}

void tet_svc_unstall_tp_03()
{
  tet_svc_unstall_tp(3);
}

void tet_svc_unstall_tp_04()
{
  tet_svc_unstall_tp(4);
}

void tet_svc_unstall_tp_05()
{
  tet_svc_unstall_tp(5);
}

void tet_svc_subscr_ADEST_01()
{
  tet_svc_subscr_ADEST(1);
}

void tet_svc_subscr_ADEST_02()
{
  tet_svc_subscr_ADEST(2);
}

void tet_svc_subscr_ADEST_03()
{
  tet_svc_subscr_ADEST(3);
}

void tet_svc_subscr_ADEST_04()
{
  tet_svc_subscr_ADEST(4);
}

void tet_svc_subscr_ADEST_05()
{
  tet_svc_subscr_ADEST(5);
}

void tet_svc_subscr_ADEST_06()
{
  tet_svc_subscr_ADEST(6);
}

void tet_svc_subscr_ADEST_07()
{
  tet_svc_subscr_ADEST(7);
}

void tet_svc_subscr_ADEST_08()
{
  tet_svc_subscr_ADEST(8);
}

void tet_svc_subscr_ADEST_09()
{
  tet_svc_subscr_ADEST(9);
}

void tet_svc_subscr_ADEST_10()
{
  tet_svc_subscr_ADEST(10);
}

void tet_svc_subscr_ADEST_11()
{
  tet_svc_subscr_ADEST(11);
}

void tet_svc_subscr_ADEST_12()
{
  tet_svc_subscr_ADEST(12);
}

void tet_svc_subscr_VDEST_01()
{
  tet_svc_subscr_VDEST(1);
}

void tet_svc_subscr_VDEST_02()
{
  tet_svc_subscr_VDEST(2);
}

void tet_svc_subscr_VDEST_03()
{
  tet_svc_subscr_VDEST(3);
}

void tet_svc_subscr_VDEST_04()
{
  tet_svc_subscr_VDEST(4);
}

void tet_svc_subscr_VDEST_05()
{
  tet_svc_subscr_VDEST(5);
}

void tet_svc_subscr_VDEST_06()
{
  tet_svc_subscr_VDEST(6);
}

void tet_svc_subscr_VDEST_07()
{
  tet_svc_subscr_VDEST(7);
}

void tet_svc_subscr_VDEST_08()
{
  tet_svc_subscr_VDEST(8);
}

void tet_svc_subscr_VDEST_09()
{
  tet_svc_subscr_VDEST(9);
}

void tet_svc_subscr_VDEST_10()
{
  tet_svc_subscr_VDEST(10);
}

void tet_svc_subscr_VDEST_11()
{
  tet_svc_subscr_VDEST(11);
}

void tet_svc_subscr_VDEST_12()
{
  tet_svc_subscr_VDEST(12);
}

void tet_just_send_tp_01()
{
  tet_just_send_tp(1);
}

void tet_just_send_tp_02()
{
  tet_just_send_tp(2);
}

void tet_just_send_tp_03()
{
  tet_just_send_tp(3);
}

void tet_just_send_tp_04()
{
  tet_just_send_tp(4);
}

void tet_just_send_tp_05()
{
  tet_just_send_tp(5);
}

void tet_just_send_tp_06()
{
  tet_just_send_tp(6);
}

void tet_just_send_tp_07()
{
  tet_just_send_tp(7);
}

void tet_just_send_tp_08()
{
  tet_just_send_tp(8);
}

void tet_just_send_tp_09()
{
  tet_just_send_tp(9);
}

void tet_just_send_tp_10()
{
  tet_just_send_tp(10);
}

void tet_just_send_tp_11()
{
  tet_just_send_tp(11);
}

void tet_just_send_tp_12()
{
  tet_just_send_tp(12);
}

void tet_just_send_tp_13()
{
  tet_just_send_tp(13);
}

void tet_just_send_tp_14()
{
  tet_just_send_tp(14);
}

void tet_send_ack_tp_01()
{
  tet_send_ack_tp(1);
}

void tet_send_ack_tp_02()
{
  tet_send_ack_tp(2);
}

void tet_send_ack_tp_03()
{
  tet_send_ack_tp(3);
}

void tet_send_ack_tp_04()
{
  tet_send_ack_tp(4);
}

void tet_send_ack_tp_05()
{
  tet_send_ack_tp(5);
}

void tet_send_ack_tp_06()
{
  tet_send_ack_tp(6);
}

void tet_send_ack_tp_07()
{
  tet_send_ack_tp(7);
}

void tet_send_ack_tp_08()
{
  tet_send_ack_tp(8);
}

void tet_send_ack_tp_09()
{
  tet_send_ack_tp(9);
}

void tet_send_ack_tp_10()
{
  tet_send_ack_tp(10);
}

void tet_send_ack_tp_11()
{
  tet_send_ack_tp(11);
}

void tet_send_ack_tp_12()
{
  tet_send_ack_tp(12);
}

void tet_send_ack_tp_13()
{
  tet_send_ack_tp(13);
}

void tet_broadcast_to_svc_tp_01()
{
  tet_broadcast_to_svc_tp(1);
}

void tet_broadcast_to_svc_tp_02()
{
  tet_broadcast_to_svc_tp(2);
}

void tet_broadcast_to_svc_tp_03()
{
  tet_broadcast_to_svc_tp(3);
}

void tet_broadcast_to_svc_tp_04()
{
  tet_broadcast_to_svc_tp(4);
}

void tet_broadcast_to_svc_tp_05()
{
  tet_broadcast_to_svc_tp(5);
}

void tet_broadcast_to_svc_tp_06()
{
  tet_broadcast_to_svc_tp(6);
}

void tet_send_response_tp_01()
{
  tet_send_response_tp(1);
}

void tet_send_response_tp_02()
{
  tet_send_response_tp(2);
}

void tet_send_response_tp_03()
{
  tet_send_response_tp(3);
}

void tet_send_response_tp_04()
{
  tet_send_response_tp(4);
}

void tet_send_response_tp_05()
{
  tet_send_response_tp(5);
}

void tet_send_response_tp_06()
{
  tet_send_response_tp(6);
}

void tet_send_response_tp_07()
{
  tet_send_response_tp(7);
}

void tet_send_response_tp_08()
{
  tet_send_response_tp(8);
}

void tet_send_response_tp_09()
{
  tet_send_response_tp(9);
}

void tet_send_response_tp_10()
{
  tet_send_response_tp(10);
}

void tet_send_response_tp_11()
{
  tet_send_response_tp(11);
}

void tet_send_response_ack_tp_01()
{
  tet_send_response_ack_tp(1);
}

void tet_send_response_ack_tp_02()
{
  tet_send_response_ack_tp(2);
}

void tet_send_response_ack_tp_03()
{
  tet_send_response_ack_tp(3);
}

void tet_send_response_ack_tp_04()
{
  tet_send_response_ack_tp(4);
}

void tet_send_response_ack_tp_05()
{
  tet_send_response_ack_tp(5);
}

void tet_send_response_ack_tp_06()
{
  tet_send_response_ack_tp(6);
}

void tet_send_response_ack_tp_07()
{
  tet_send_response_ack_tp(7);
}

void tet_send_response_ack_tp_08()
{
  tet_send_response_ack_tp(8);
}

void tet_direct_just_send_tp_01()
{
  tet_direct_just_send_tp(1);
}

void tet_direct_just_send_tp_02()
{
  tet_direct_just_send_tp(2);
}

void tet_direct_just_send_tp_03()
{
  tet_direct_just_send_tp(3);
}

void tet_direct_just_send_tp_04()
{
  tet_direct_just_send_tp(4);
}

void tet_direct_just_send_tp_05()
{
  tet_direct_just_send_tp(5);
}

void tet_direct_just_send_tp_06()
{
  tet_direct_just_send_tp(6);
}

void tet_direct_just_send_tp_07()
{
  tet_direct_just_send_tp(7);
}

void tet_direct_just_send_tp_08()
{
  tet_direct_just_send_tp(8);
}

void tet_direct_just_send_tp_09()
{
  tet_direct_just_send_tp(9);
}

void tet_direct_just_send_tp_10()
{
  tet_direct_just_send_tp(10);
}

void tet_direct_just_send_tp_11()
{
  tet_direct_just_send_tp(11);
}

void tet_direct_just_send_tp_12()
{
  tet_direct_just_send_tp(12);
}

void tet_direct_just_send_tp_13()
{
  tet_direct_just_send_tp(13);
}

void tet_direct_just_send_tp_14()
{
  tet_direct_just_send_tp(14);
}

void tet_direct_just_send_tp_15()
{
  tet_direct_just_send_tp(15);
}

void tet_direct_send_ack_tp_01()
{
  tet_direct_send_ack_tp(1);
}

void tet_direct_send_ack_tp_02()
{
  tet_direct_send_ack_tp(2);
}

void tet_direct_send_ack_tp_03()
{
  tet_direct_send_ack_tp(3);
}

void tet_direct_send_ack_tp_04()
{
  tet_direct_send_ack_tp(4);
}

void tet_direct_send_ack_tp_05()
{
  tet_direct_send_ack_tp(5);
}

void tet_direct_send_ack_tp_06()
{
  tet_direct_send_ack_tp(6);
}

void tet_direct_send_ack_tp_07()
{
  tet_direct_send_ack_tp(7);
}

void tet_direct_send_ack_tp_08()
{
  tet_direct_send_ack_tp(8);
}

void tet_direct_send_ack_tp_09()
{
  tet_direct_send_ack_tp(9);
}

void tet_direct_send_ack_tp_10()
{
  tet_direct_send_ack_tp(10);
}

void tet_direct_send_ack_tp_11()
{
  tet_direct_send_ack_tp(11);
}

void tet_direct_send_ack_tp_12()
{
  tet_direct_send_ack_tp(12);
}

void tet_direct_send_ack_tp_13()
{
  tet_direct_send_ack_tp(13);
}

void tet_direct_broadcast_to_svc_tp_01()
{
  tet_direct_broadcast_to_svc_tp(1);
}

void tet_direct_broadcast_to_svc_tp_02()
{
  tet_direct_broadcast_to_svc_tp(2);
}

void tet_direct_broadcast_to_svc_tp_03()
{
  tet_direct_broadcast_to_svc_tp(3);
}

void tet_direct_broadcast_to_svc_tp_04()
{
  tet_direct_broadcast_to_svc_tp(4);
}

void tet_direct_broadcast_to_svc_tp_05()
{
  tet_direct_broadcast_to_svc_tp(5);
}

void tet_direct_broadcast_to_svc_tp_06()
{
  tet_direct_broadcast_to_svc_tp(6);
}

void tet_direct_broadcast_to_svc_tp_07()
{
  tet_direct_broadcast_to_svc_tp(7);
}

void tet_direct_broadcast_to_svc_tp_08()
{
  tet_direct_broadcast_to_svc_tp(8);
}

void tet_direct_send_response_tp_01()
{
  tet_direct_send_response_tp(1);
}

void tet_direct_send_response_tp_02()
{
  tet_direct_send_response_tp(2);
}

void tet_direct_send_response_tp_03()
{
  tet_direct_send_response_tp(3);
}

void tet_direct_send_response_tp_04()
{
  tet_direct_send_response_tp(4);
}

void tet_direct_send_response_tp_05()
{
  tet_direct_send_response_tp(5);
}

void tet_direct_send_response_ack_tp_01()
{
  tet_direct_send_response_ack_tp(1);
}

void tet_direct_send_response_ack_tp_02()
{
  tet_direct_send_response_ack_tp(2);
}

void tet_direct_send_response_ack_tp_03()
{
  tet_direct_send_response_ack_tp(3);
}

void tet_direct_send_response_ack_tp_04()
{
  tet_direct_send_response_ack_tp(4);
}

void tet_direct_send_response_ack_tp_05()
{
  tet_direct_send_response_ack_tp(5);
}

void tet_send_all_tp_01()
{
  tet_send_all_tp(1);
}

void tet_send_all_tp_02()
{
  tet_send_all_tp(2);
}

void tet_direct_send_all_tp_01()
{
  tet_direct_send_all_tp(1);
}

void tet_direct_send_all_tp_02()
{
  tet_direct_send_all_tp(2);
}

void tet_direct_send_all_tp_03()
{
  tet_direct_send_all_tp(3);
}

void tet_direct_send_all_tp_04()
{
  tet_direct_send_all_tp(4);
}

void tet_direct_send_all_tp_05()
{
  tet_direct_send_all_tp(5);
}

void tet_direct_send_all_tp_06()
{
  tet_direct_send_all_tp(6);
}

void tet_query_pwe_tp_01()
{
  tet_query_pwe_tp(1);
}

void tet_query_pwe_tp_02()
{
  tet_query_pwe_tp(2);
}

struct tet_testlist tet_testlist[]={

  {tet_svc_install_tp_01, 1},
  {tet_svc_install_tp_02, 1},
  {tet_svc_install_tp_03, 1},
  {tet_svc_install_tp_05, 1},
  {tet_svc_install_tp_06, 1},
  {tet_svc_install_tp_07, 1},
  {tet_svc_install_tp_08, 1},
  {tet_svc_install_tp_09, 1},
  {tet_svc_install_tp_10, 1},
  {tet_svc_install_tp_11, 1},
  {tet_svc_install_tp_12, 1},
  {tet_svc_install_tp_13, 1},
  {tet_svc_install_tp_14, 1},
  {tet_svc_install_tp_15, 1},
  {tet_svc_install_tp_14, 1},
  {tet_svc_install_tp_16, 1},
  {tet_svc_install_tp_17, 1},
  
  {tet_svc_unstall_tp_01, 2},
  {tet_svc_unstall_tp_02, 2},
  {tet_svc_unstall_tp_03, 2},
  {tet_svc_unstall_tp_04, 2},
  {tet_svc_unstall_tp_05, 2},
  
  {tet_svc_install_upto_MAX, 3},
  
  {tet_svc_subscr_ADEST_01, 4},
  {tet_svc_subscr_ADEST_02, 4},
  {tet_svc_subscr_ADEST_03, 4},
  {tet_svc_subscr_ADEST_04, 4},
  {tet_svc_subscr_ADEST_05, 4},
  {tet_svc_subscr_ADEST_06, 4},
  {tet_svc_subscr_ADEST_07, 4},
  {tet_svc_subscr_ADEST_08, 4},
  {tet_svc_subscr_ADEST_09, 4},
  {tet_svc_subscr_ADEST_10, 4},
  {tet_svc_subscr_ADEST_11, 4},
  {tet_svc_subscr_ADEST_12, 4},
  
  {tet_svc_subscr_VDEST_01, 5},
  {tet_svc_subscr_VDEST_02, 5},
  {tet_svc_subscr_VDEST_03, 5},
  {tet_svc_subscr_VDEST_04, 5},
  {tet_svc_subscr_VDEST_05, 5},
  {tet_svc_subscr_VDEST_06, 5},
  {tet_svc_subscr_VDEST_07, 5},
  {tet_svc_subscr_VDEST_08, 5},
  {tet_svc_subscr_VDEST_09, 5},
  {tet_svc_subscr_VDEST_10, 5},
  {tet_svc_subscr_VDEST_11, 5},
  {tet_svc_subscr_VDEST_12, 5},
  
  {tet_just_send_tp_01, 7},
  {tet_just_send_tp_02, 7},
  {tet_just_send_tp_03, 7},
  {tet_just_send_tp_04, 7},
  {tet_just_send_tp_05, 7},
  {tet_just_send_tp_06, 7},
  {tet_just_send_tp_07, 7},
  {tet_just_send_tp_08, 7},
  {tet_just_send_tp_09, 7},
  {tet_just_send_tp_10, 7},
  {tet_just_send_tp_11, 7},
  {tet_just_send_tp_12, 7},
  {tet_just_send_tp_13, 7},
  {tet_just_send_tp_14, 7},
  
  {tet_send_ack_tp_01, 8},
  {tet_send_ack_tp_02, 8},
  {tet_send_ack_tp_03, 8},
  {tet_send_ack_tp_04, 8},
  {tet_send_ack_tp_05, 8},
  {tet_send_ack_tp_06, 8},
  {tet_send_ack_tp_07, 8},
  {tet_send_ack_tp_08, 8},
  {tet_send_ack_tp_09, 8},
  {tet_send_ack_tp_10, 8},
  {tet_send_ack_tp_11, 8},
  {tet_send_ack_tp_12, 8},
  {tet_send_ack_tp_13, 8},
  
  {tet_broadcast_to_svc_tp_01, 11},
  {tet_broadcast_to_svc_tp_02, 11},
  {tet_broadcast_to_svc_tp_03, 11},
  {tet_broadcast_to_svc_tp_04, 11},
  {tet_broadcast_to_svc_tp_05, 11},
  {tet_broadcast_to_svc_tp_06, 11},
  
  {tet_send_response_tp_01, 12},
  {tet_send_response_tp_02, 12},
  {tet_send_response_tp_03, 12},
  {tet_send_response_tp_04, 12},
  {tet_send_response_tp_05, 12},
  {tet_send_response_tp_06, 12},
  {tet_send_response_tp_07, 12},
  {tet_send_response_tp_08, 12},
  {tet_send_response_tp_09, 12},
  {tet_send_response_tp_10, 12},
  {tet_send_response_tp_11, 12},
  
  {tet_send_response_ack_tp_01, 13},
  {tet_send_response_ack_tp_02, 13},
  {tet_send_response_ack_tp_03, 13},
  {tet_send_response_ack_tp_04, 13},
  {tet_send_response_ack_tp_05, 13},
  {tet_send_response_ack_tp_06, 13},
  {tet_send_response_ack_tp_07, 13},
  {tet_send_response_ack_tp_08, 13},
  
  {tet_direct_just_send_tp_01, 16},
  {tet_direct_just_send_tp_02, 16},
  {tet_direct_just_send_tp_03, 16},
  {tet_direct_just_send_tp_04, 16},
  {tet_direct_just_send_tp_05, 16},
  {tet_direct_just_send_tp_06, 16},
  {tet_direct_just_send_tp_07, 16},
  {tet_direct_just_send_tp_08, 16},
  {tet_direct_just_send_tp_09, 16},
  {tet_direct_just_send_tp_10, 16},
  {tet_direct_just_send_tp_11, 16},
  {tet_direct_just_send_tp_12, 16},
  {tet_direct_just_send_tp_13, 16},
  {tet_direct_just_send_tp_14, 16},
  {tet_direct_just_send_tp_15, 16},
  
  {tet_direct_send_ack_tp_01, 17},
  {tet_direct_send_ack_tp_02, 17},
  {tet_direct_send_ack_tp_03, 17},
  {tet_direct_send_ack_tp_04, 17},
  {tet_direct_send_ack_tp_05, 17},
  {tet_direct_send_ack_tp_06, 17},
  {tet_direct_send_ack_tp_07, 17},
  {tet_direct_send_ack_tp_08, 17},
  {tet_direct_send_ack_tp_09, 17},
  {tet_direct_send_ack_tp_10, 17},
  {tet_direct_send_ack_tp_11, 17},
  {tet_direct_send_ack_tp_12, 17},
  {tet_direct_send_ack_tp_13, 17},
  
  {tet_direct_broadcast_to_svc_tp_01, 20},
  {tet_direct_broadcast_to_svc_tp_02, 20},
  {tet_direct_broadcast_to_svc_tp_03, 20},
  {tet_direct_broadcast_to_svc_tp_04, 20},
  {tet_direct_broadcast_to_svc_tp_05, 20},
  {tet_direct_broadcast_to_svc_tp_06, 20},
  {tet_direct_broadcast_to_svc_tp_07, 20},
  {tet_direct_broadcast_to_svc_tp_08, 20},
  
  {tet_direct_send_response_tp_01, 21},
  {tet_direct_send_response_tp_02, 21},
  {tet_direct_send_response_tp_03, 21},
  {tet_direct_send_response_tp_04, 21},
  {tet_direct_send_response_tp_05, 21},
  
  {tet_direct_send_response_ack_tp_01, 22},
  {tet_direct_send_response_ack_tp_02, 22},
  {tet_direct_send_response_ack_tp_03, 22},
  {tet_direct_send_response_ack_tp_04, 22},
  {tet_direct_send_response_ack_tp_05, 22},
  
  {ncs_agents_shut_start, 25},
  {tet_send_all_tp_01, 25},
  {tet_send_all_tp_02, 25},
  
  {tet_direct_send_all_tp_01, 26},
  {tet_direct_send_all_tp_02, 26},
  {tet_direct_send_all_tp_03, 26},
  {tet_direct_send_all_tp_04, 26},
  {tet_direct_send_all_tp_05, 26},
  {tet_direct_send_all_tp_06, 26},
  
  {tet_query_pwe_tp_01, 30},
  {tet_query_pwe_tp_02, 30},

  {NULL, -1}
};

#endif

/*************************************
TETWARE RELATED : ACTUAL INVOCATION
**************************************/
void tet_mds_tds_startup(void)
{
#if (TET_PATCH==1)

  int no_iterations=1,test_case,i;

  
    int SERV_SUIT=0;
    
  char *tmpprt=NULL;
  tmpprt= (char *) getenv("TET_ITERATION");
  if(tmpprt)
    no_iterations= atoi(tmpprt);
  else
    no_iterations=0;
  
  tmpprt= (char *) getenv("TET_SERV_SUITE");
  if(tmpprt)
    SERV_SUIT= atoi(tmpprt);
  else
    SERV_SUIT=0; 
  tmpprt= (char *) getenv("TET_CASE");
  if(tmpprt)
    test_case = atoi(tmpprt);
  else
    test_case =0; 


  tet_infoline("MDS_Startup : Starting");

    {  
      for(i=1;i<=no_iterations;i++)
        {
          if(i!=1)
            {
              if(ncs_agents_shutdown(0,0)!=NCSCC_RC_SUCCESS)
                {
                  perror("\n\n ----- NCS AGENTS SHUTDOWN FAILED ----- \n\n");
                  tet_printf("\n ----- NCS AGENTS SHUTDOWN FAILED ----- \n");
                  break;
                }
              sleep(5);
              if(ncs_agents_startup(0,0)!=NCSCC_RC_SUCCESS)
                {
                  perror("\n\n ----- NCS AGENTS START UP FAILED ------- \n\n");
                  tet_printf("\n ----- NCS AGENTS START UP FAILED ------- \n");
                  break;
                }
            }         
          tet_printf("\n--------------  ITERATION %d -----------\n",i);
          
          if(SERV_SUIT)
            {
              /*Starting Service related test cases*/
              tware_mem_ign();  
              tet_test_start(test_case,svc_testlist);
              tware_mem_dump();
              sleep(1);
              tware_mem_dump();
              ncs_tmr_whatsout();
            }
        }
    }
  tet_mds_tds_cleanup();
  
  return;
}
void tet_mds_tds_cleanup(void)
{
  tet_test_cleanup();
  exit(0);

#endif

  return;
}

void tet_mds_startup(void) 
{
/* Using the default mode for startup */
    ncs_agents_startup(0,NULL); 

    tet_mds_tds_startup();
}

void tet_mds_cleanup()
{
  tet_infoline(" Ending the agony.. ");
}
