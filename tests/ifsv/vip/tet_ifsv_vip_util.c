/********************************************************************************************
#
# modification:
# 2008-11   Jony <jiangnan_hw@huawei.com>  Huawei Technologies CO., LTD.
#  1.  Provide test case tet_startup and tet_cleanup functions. The tet_startup function
#      is defined as tet_ifsv_startup(), as well as the tet_cleanup function is defined 
#      as tet_ifsv_end().
#  2.  If "tetware-patch" is not used, change the name of test case array from api_test[] 
#      to tet_testlist[]. 
#  3.  If "tetware-patch" is not used, delete the third parameter of each item in this array. 
#      And add some functions to implement the function of those parameters.
#  4.  If "tetware-patch" is not used, delete parts of tet_run_ifsv_app() function, which 
#      invoke the functions in the patch.
#  5.  If "tetware-patch" is required, one can add a macro definition "TET_PATCH=1" in 
#      compilation process.
#
*********************************************************************************************/


#include "tet_startup.h"
#include "ncs_main_papi.h"

extern void tet_ncs_ifsv_svc_req_vip_install(int i);
extern void tet_ncs_ifsv_svc_req_vip_free(int iOption);
extern void tet_ncs_ifsv_svc_req_node_1(int iOption);
extern void tet_ncs_ifsv_svc_req_node_2(int iOption);
extern void dummy(void);
extern void gl_ifsv_defs();
extern int shelf_id,slot_id,gl_listNumber,gl_tCase;



void tet_run_ifsv_app();

void tet_ifsv_startup(void);
void tet_ifsv_end(void);

void (*tet_startup)()=tet_ifsv_startup;
void (*tet_cleanup)()=tet_ifsv_end;


#if (TET_PATCH==1)
struct tet_testlist api_test[]=
  {
    {tet_ncs_ifsv_svc_req_vip_install,1,1},
    {tet_ncs_ifsv_svc_req_vip_install,2,2},
    {tet_ncs_ifsv_svc_req_vip_install,3,3},
    {tet_ncs_ifsv_svc_req_vip_install,4,4},
    {tet_ncs_ifsv_svc_req_vip_install,5,5},
    {tet_ncs_ifsv_svc_req_vip_install,6,6},
    {tet_ncs_ifsv_svc_req_vip_install,7,7},
    {tet_ncs_ifsv_svc_req_vip_install,8,8},
    {tet_ncs_ifsv_svc_req_vip_install,9,9},
    {tet_ncs_ifsv_svc_req_vip_install,10,10},
#if 0
    {tet_ncs_ifsv_svc_req_vip_install,11,11},
#endif
    {tet_ncs_ifsv_svc_req_vip_install,12,12},
    {tet_ncs_ifsv_svc_req_vip_install,13,13},
    {tet_ncs_ifsv_svc_req_vip_install,14,14},
    {tet_ncs_ifsv_svc_req_vip_install,15,15},
    {tet_ncs_ifsv_svc_req_vip_install,16,16},
    {tet_ncs_ifsv_svc_req_vip_install,17,17},
    {tet_ncs_ifsv_svc_req_vip_install,18,18},
    {tet_ncs_ifsv_svc_req_vip_install,19,19},
    {tet_ncs_ifsv_svc_req_vip_install,20,20},
    {tet_ncs_ifsv_svc_req_vip_install,21,21},
    {tet_ncs_ifsv_svc_req_vip_install,22,22},
    {tet_ncs_ifsv_svc_req_vip_install,23,23},
#if 0
    {tet_ncs_ifsv_svc_req_vip_install,24,24},
#endif
    {tet_ncs_ifsv_svc_req_vip_install,25,25},
    {tet_ncs_ifsv_svc_req_vip_install,26,26},
    {tet_ncs_ifsv_svc_req_vip_free,27,1},
    {tet_ncs_ifsv_svc_req_vip_free,28,2},
    {tet_ncs_ifsv_svc_req_vip_free,29,3},
    {tet_ncs_ifsv_svc_req_vip_free,30,4},
    {tet_ncs_ifsv_svc_req_vip_free,31,5},
    {tet_ncs_ifsv_svc_req_vip_free,32,6},
    /*{tet_ncs_ifsv_svc_req_failover_test,14,1},
      {tet_ncs_ifsv_svc_req_switchover_test,15,1},
      {tet_ncs_ifsv_free_spec_ip,31,1},*/
    {NULL,-1}
  };

struct tet_testlist vip_twonode_test_node1[]=
  {
    {tet_ncs_ifsv_svc_req_node_1,1,1},
    {tet_ncs_ifsv_svc_req_node_1,2,2},
    {tet_ncs_ifsv_svc_req_node_1,3,3},
    {tet_ncs_ifsv_svc_req_node_1,4,4},
    {tet_ncs_ifsv_svc_req_node_1,5,5},
    {tet_ncs_ifsv_svc_req_node_1,6,6},
    {NULL,-1}
  };

struct tet_testlist vip_twonode_test_node2[]=
  {
    {tet_ncs_ifsv_svc_req_node_2,1,1},
    {tet_ncs_ifsv_svc_req_node_2,2,2},
    {tet_ncs_ifsv_svc_req_node_2,3,3},
    {tet_ncs_ifsv_svc_req_node_2,4,4},
    {tet_ncs_ifsv_svc_req_node_2,5,5},
    {tet_ncs_ifsv_svc_req_node_2,6,6},
    {NULL,-1}
  };

struct tet_testlist dummy_1_tetlist[]=
  {
    {dummy,1},
    {dummy,2},
    {dummy,3},
    {dummy,4},
    {dummy,5},
    {dummy,6},
    {dummy,7},
    {dummy,8},
    {dummy,9},
    {dummy,10},
    {dummy,11},
    {dummy,12},
    {dummy,13},
    {dummy,14},
    {dummy,15},
    {dummy,16},
    {dummy,17},
    {dummy,18},
    {dummy,19},
    {dummy,20},
    {dummy,21},
    {dummy,22},
    {dummy,23},
    {dummy,24},
    {dummy,25},
    {dummy,26},
    {dummy,27},
    {dummy,28},
    {dummy,29},
    {dummy,30},
    {dummy,31},
    {NULL,-1}
  };

struct tet_testlist dummy_2_tetlist[]=
  {
    {dummy,1},
    {dummy,2},
    {dummy,3},
    {dummy,4},
    {dummy,5},
    {dummy,6},
    {NULL,-1}
  };

void tet_ifsv_cleanup()
{
  tet_test_cleanup();
}

#else
/*************Function Modification***************/
void tet_ncs_ifsv_svc_req_vip_install_01()
{
  tet_ncs_ifsv_svc_req_vip_install(1);
}

void tet_ncs_ifsv_svc_req_vip_install_02()
{
  tet_ncs_ifsv_svc_req_vip_install(2);
}

void tet_ncs_ifsv_svc_req_vip_install_03()
{
  tet_ncs_ifsv_svc_req_vip_install(3);
}

void tet_ncs_ifsv_svc_req_vip_install_04()
{
  tet_ncs_ifsv_svc_req_vip_install(4);
}

void tet_ncs_ifsv_svc_req_vip_install_05()
{
  tet_ncs_ifsv_svc_req_vip_install(5);
}

void tet_ncs_ifsv_svc_req_vip_install_06()
{
  tet_ncs_ifsv_svc_req_vip_install(6);
}

void tet_ncs_ifsv_svc_req_vip_install_07()
{
  tet_ncs_ifsv_svc_req_vip_install(7);
}

void tet_ncs_ifsv_svc_req_vip_install_08()
{
  tet_ncs_ifsv_svc_req_vip_install(8);
}

void tet_ncs_ifsv_svc_req_vip_install_09()
{
  tet_ncs_ifsv_svc_req_vip_install(9);
}

void tet_ncs_ifsv_svc_req_vip_install_10()
{
  tet_ncs_ifsv_svc_req_vip_install(10);
}

void tet_ncs_ifsv_svc_req_vip_install_11()
{
  tet_ncs_ifsv_svc_req_vip_install(11);
}

void tet_ncs_ifsv_svc_req_vip_install_12()
{
  tet_ncs_ifsv_svc_req_vip_install(12);
}

void tet_ncs_ifsv_svc_req_vip_install_13()
{
  tet_ncs_ifsv_svc_req_vip_install(13);
}

void tet_ncs_ifsv_svc_req_vip_install_14()
{
  tet_ncs_ifsv_svc_req_vip_install(14);
}

void tet_ncs_ifsv_svc_req_vip_install_15()
{
  tet_ncs_ifsv_svc_req_vip_install(15);
}

void tet_ncs_ifsv_svc_req_vip_install_16()
{
  tet_ncs_ifsv_svc_req_vip_install(16);
}

void tet_ncs_ifsv_svc_req_vip_install_17()
{
  tet_ncs_ifsv_svc_req_vip_install(17);
}

void tet_ncs_ifsv_svc_req_vip_install_18()
{
  tet_ncs_ifsv_svc_req_vip_install(18);
}

void tet_ncs_ifsv_svc_req_vip_install_19()
{
  tet_ncs_ifsv_svc_req_vip_install(19);
}

void tet_ncs_ifsv_svc_req_vip_install_20()
{
  tet_ncs_ifsv_svc_req_vip_install(20);
}

void tet_ncs_ifsv_svc_req_vip_install_21()
{
  tet_ncs_ifsv_svc_req_vip_install(21);
}

void tet_ncs_ifsv_svc_req_vip_install_22()
{
  tet_ncs_ifsv_svc_req_vip_install(22);
}

void tet_ncs_ifsv_svc_req_vip_install_23()
{
  tet_ncs_ifsv_svc_req_vip_install(23);
}

void tet_ncs_ifsv_svc_req_vip_install_24()
{
  tet_ncs_ifsv_svc_req_vip_install(24);
}

void tet_ncs_ifsv_svc_req_vip_install_25()
{
  tet_ncs_ifsv_svc_req_vip_install(25);
}

void tet_ncs_ifsv_svc_req_vip_install_26()
{
  tet_ncs_ifsv_svc_req_vip_install(26);
}

void tet_ncs_ifsv_svc_req_vip_free_01()
{
  tet_ncs_ifsv_svc_req_vip_free(1);
}

void tet_ncs_ifsv_svc_req_vip_free_02()
{
  tet_ncs_ifsv_svc_req_vip_free(2);
}

void tet_ncs_ifsv_svc_req_vip_free_03()
{
  tet_ncs_ifsv_svc_req_vip_free(3);
}

void tet_ncs_ifsv_svc_req_vip_free_04()
{
  tet_ncs_ifsv_svc_req_vip_free(4);
}

void tet_ncs_ifsv_svc_req_vip_free_05()
{
  tet_ncs_ifsv_svc_req_vip_free(5);
}

void tet_ncs_ifsv_svc_req_vip_free_06()
{
  tet_ncs_ifsv_svc_req_vip_free(6);
}


struct tet_testlist tet_testlist[]={
  {tet_ncs_ifsv_svc_req_vip_install_01, 1},
  {tet_ncs_ifsv_svc_req_vip_install_02, 2},
  {tet_ncs_ifsv_svc_req_vip_install_03, 3},
  {tet_ncs_ifsv_svc_req_vip_install_04, 4},
  {tet_ncs_ifsv_svc_req_vip_install_05, 5},
  {tet_ncs_ifsv_svc_req_vip_install_06, 6},
  {tet_ncs_ifsv_svc_req_vip_install_07, 7},
  {tet_ncs_ifsv_svc_req_vip_install_08, 8},
  {tet_ncs_ifsv_svc_req_vip_install_09, 9},
  {tet_ncs_ifsv_svc_req_vip_install_10, 10},
#if 0
  {tet_ncs_ifsv_svc_req_vip_install_11, 11},
#endif
  {tet_ncs_ifsv_svc_req_vip_install_12, 12},
  {tet_ncs_ifsv_svc_req_vip_install_13, 13},
  {tet_ncs_ifsv_svc_req_vip_install_14, 14},
  {tet_ncs_ifsv_svc_req_vip_install_15, 15},
  {tet_ncs_ifsv_svc_req_vip_install_16, 16},
  {tet_ncs_ifsv_svc_req_vip_install_17, 17},
  {tet_ncs_ifsv_svc_req_vip_install_18, 18},
  {tet_ncs_ifsv_svc_req_vip_install_19, 19},
  {tet_ncs_ifsv_svc_req_vip_install_20, 20},
  {tet_ncs_ifsv_svc_req_vip_install_21, 21},
  {tet_ncs_ifsv_svc_req_vip_install_22, 22},
  {tet_ncs_ifsv_svc_req_vip_install_23, 23},
#if 0
    {tet_ncs_ifsv_svc_req_vip_install,24,24},
#endif
  {tet_ncs_ifsv_svc_req_vip_install_25, 25},
  {tet_ncs_ifsv_svc_req_vip_install_26, 26},
  {tet_ncs_ifsv_svc_req_vip_free_01, 27},
  {tet_ncs_ifsv_svc_req_vip_free_02, 28},
  {tet_ncs_ifsv_svc_req_vip_free_03, 29},
  {tet_ncs_ifsv_svc_req_vip_free_04, 30},
  {tet_ncs_ifsv_svc_req_vip_free_05, 31},
  {tet_ncs_ifsv_svc_req_vip_free_06, 32},
  /*{tet_ncs_ifsv_svc_req_failover_test, 14},
  {tet_ncs_ifsv_svc_req_switchover_test, 15},
  {tet_ncs_ifsv_free_spec_ip, 31},*/

  {NULL, -1}
};

#endif

void tet_run_ifsv_app()
{
  char *temp=NULL;

  gl_ifsv_defs();
  
  temp=(char *)getenv("TET_SHELF_ID");
  shelf_id=atoi(temp);

  temp=(char *)getenv("TET_SLOT_ID");
  slot_id=atoi(temp);

#if (TET_PATCH==1)
  tware_mem_ign();

#ifdef TET_A
  if(gl_listNumber == 1)
    {
      /*sysid = tet_remgetsys();
        printf ("sysid: %d\n",sysid);

        if(sysid == 0)
        tet_test_start(gl_tCase,dummy_1_tetlist);
        else   */
      tet_test_start(gl_tCase,api_test);
    }

  printf ("Befor MEM DUMP*********************\n");
#endif
      
  printf("\nPRESS ENTER TO GET MEM DUMP");
  tware_mem_dump();
  sleep(4);
  tware_mem_dump();
  tet_ifsv_cleanup();
#endif /*TET_PATCH==1*/
}


void tet_ifsv_startup()
{
  /* Using the default mode for startup */
    ncs_agents_startup(0,NULL); 

    tet_run_ifsv_app();
}

void tet_ifsv_end()
{
  tet_infoline(" Ending the agony.. ");
}
