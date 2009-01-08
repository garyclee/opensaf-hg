#include <stdio.h>
#include <stdlib.h>

#include "tet_api.h"
#include "tet_startup.h"
#include "tet_ifa.h"

void tet_ifsv_startup(void);
void tet_ifsv_cleanup(void);

int ip_on_intf_flag=0; /* 0 - success, 256 - failure */
int api_res, intf_res, cli_res; /* intf_res: 1 - success 0 - failure */
int install_infra_flag = VIP_ADDR_NORMAL;
char *ip, *flag;
char *vip_appl_name1, *vip_appl_name2;
char  *vip_intf1, *vip_intf2;
unsigned int vip_ip1,vip_current_ip1, vip_ip2,vip_current_ip2, vip_switchover;
 
void ifsv_vip_free_fill(int);
void ifsv_vip_install_fill(int);
void fill_vip(char *, uns32, char *, uns32);

void vip_wait()
{
  /*flag = (char *) getenv("VIP_WAIT");*/
  if ((flag == NULL) || (!atoi(flag)))
    getchar();
  else if(atoi(flag) == 1)
    return;
  else 
    sleep(atoi(flag));
}

void networkaddr_to_ascii(uint32_t net_ip)
{
  struct in_addr addr;
  ip = (char*)calloc(16,sizeof(char));
  addr.s_addr = ntohl(net_ip);
  strncpy(ip, (char *)inet_ntoa(addr), strlen((char *)inet_ntoa(addr))+1);
}

int ip_on_intf(char *intf)
{
  char cmd[]="var1=`/sbin/ip addr show %s | grep \"inet.*%s\" | tr -s \" \" | cut -f3 -d\" \"`; test \"$var1\" = \"%s/24\"";
  char fcmd[100];
  int ipresult;

  sprintf(fcmd, cmd, intf, ip, ip);
  /*printf("%u\n", system(fcmd));*/
  ipresult=system(fcmd);

  if (ipresult == ip_on_intf_flag)
    return 1; /* expected result match */
  else
    return 0; /* expected result mismatch */
}

void vip_result(char *saf_msg, int resflag)
     /* resflag=1 means to print the TET_RESULT else not. */
{
  printf("\n\n%s\n",saf_msg);
  tet_printf("%s",saf_msg);

  printf("expectedresult-%d : api_res-%d : NCSCC_RC_FAILURE-%d\n", 
         expectedResult, api_res, NCSCC_RC_FAILURE);

  if (NCSCC_RC_FAILURE==expectedResult||api_res==NCSCC_RC_FAILURE)
    {
      printf("Failed with error type : %s(%d)\n",
             VIP_ERRORS[req_info.info.i_vip_install.o_err],
             req_info.info.i_vip_install.o_err);
      tet_printf("Failed with error type : %s(%d)",
                 VIP_ERRORS[req_info.info.i_vip_install.o_err],
                 req_info.info.i_vip_install.o_err);
    }

  if (intf_res == 0 && ip_on_intf_flag == 0)
    {
      printf("The addr %s is NOT PRESENT on %s.\n", ip, 
             i_vip_install.i_intf_name); 
      tet_printf("The addr %s is NOT PRESENT on %s.", ip, 
                 i_vip_install.i_intf_name); 
    }

  if (intf_res == 0 && ip_on_intf_flag == 256)
    {
      printf("The addr %s is STILL PRESENT on %s.\n", ip, 
             i_vip_install.i_intf_name); 
      tet_printf("The addr %s is STILL PRESENT on %s.", ip, 
                 i_vip_install.i_intf_name); 
    }
   
  if(resflag == 1)
    {
      if(api_res == expectedResult && intf_res == 1)
        {
          printf("TEST CASE PASS");
          tet_result(TET_PASS);
        }
      else
        {
          printf("TEST CASE FAIL");
          tet_result(TET_FAIL);
        }
    }
  vip_wait();
}

void tet_ncs_ifsv_svc_req_vip_free(int iOption)
{
  printf("\n-----------------------------------------------------------\n");
  api_res=0;
  ifsv_vip_free_fill(iOption);

  req_info.i_req_type=NCS_IFSV_REQ_VIP_FREE;
  req_info.info.i_vip_free=i_vip_free;

  api_res=ncs_ifsv_svc_req(&req_info);
  result(FREE_TESTCASE[iOption-1],expectedResult);
  /*result("ncs_ifsv_svc_req() for vip Uninstallation",expectedResult);*/

  if(NCSCC_RC_FAILURE==expectedResult||api_res==NCSCC_RC_FAILURE)
    {
      printf("\n\nFailed with error type : %s",
             VIP_ERRORS[req_info.info.i_vip_free.o_err]);
    }

  printf("\nNumber of API calls: %d",iOption);
  printf("\n-----------------------------------------------------------\n");
}

void ifsv_vip_free_fill(int i)
{
  switch(i)
    {
    case 1:
      strcpy(i_vip_free.i_handle.vipApplName,"");
      i_vip_free.i_handle.poolHdl=1;
      i_vip_free.i_infraFlag=0;
      expectedResult=NCSCC_RC_FAILURE;
      break;

    case 2:
      strcpy(i_vip_free.i_handle.vipApplName,vip_appl_name1);
      i_vip_free.i_handle.poolHdl=0;
      i_vip_free.i_infraFlag=0;
      expectedResult=NCSCC_RC_FAILURE;
      break;

    case 3:
      strcpy(i_vip_free.i_handle.vipApplName,"xyz");
      i_vip_free.i_handle.poolHdl=1;
      i_vip_free.i_infraFlag=0;
      expectedResult=NCSCC_RC_FAILURE;
      break;

    case 4:
      strcpy(i_vip_free.i_handle.vipApplName,vip_appl_name1);
      i_vip_free.i_handle.poolHdl=1000;
      i_vip_free.i_infraFlag=0;
      expectedResult=NCSCC_RC_FAILURE;
      break;

    case 5:
      install_infra_flag = VIP_ADDR_NORMAL;
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      printf("Press a key to continue\n");
      vip_wait();
      /*ifsv_vip_install_fill(6);
        req_info.i_req_type=NCS_IFSV_REQ_VIP_INSTALL;
        req_info.info.i_vip_install=i_vip_install;
        api_res=ncs_ifsv_svc_req(&req_info);
        printf("Installation success.");*/

      strcpy(i_vip_free.i_handle.vipApplName,vip_appl_name1);
      i_vip_free.i_handle.poolHdl=1;
      i_vip_free.i_infraFlag=VIP_ADDR_NORMAL;
      expectedResult=NCSCC_RC_SUCCESS;
      break;

    case 6:
      /*ifsv_vip_install_fill(6);
        req_info.i_req_type=NCS_IFSV_REQ_VIP_INSTALL;
        req_info.info.i_vip_install=i_vip_install;
        api_res=ncs_ifsv_svc_req(&req_info);
        printf("Installation success.");*/

      strcpy(i_vip_free.i_handle.vipApplName,vip_appl_name1);
      i_vip_free.i_handle.poolHdl=1;
      i_vip_free.i_infraFlag=0;
      expectedResult=NCSCC_RC_FAILURE;
      break;

    case IFSV_VIP_FREE_APPL_SUCCESS:
      strcpy(i_vip_free.i_handle.vipApplName,applName);
      i_vip_free.i_handle.poolHdl=hdl;
      i_vip_free.i_infraFlag=VIP_ADDR_NORMAL;

      req_info.i_req_type=NCS_IFSV_REQ_VIP_FREE;
      req_info.info.i_vip_free=i_vip_free;

      api_res=ncs_ifsv_svc_req(&req_info);
      if(api_res==NCSCC_RC_SUCCESS)
        {
          printf("\n\nVIP delete of application (%s,%d)\n", applName, hdl);
        }
      else
        printf("\n\nFailed with error type : %s",
               VIP_ERRORS[req_info.info.i_vip_free.o_err]);
      break;
    }
}

/*void tet_ncs_ifsv_svc_req_vip_install(int iOption)*/
void ifsv_vip_install_fill(int iOption)
{
  printf("\n-----------------------------------------------------------\n");
  expectedResult=NCSCC_RC_FAILURE; 
  ifsv_vip_install_fill(iOption);

  /*printf ("api_res is : %d\n", api_res);*/
  api_res = ncs_ifsv_svc_req(&req_info);
  intf_res = ip_on_intf(i_vip_install.i_intf_name);
  /*printf ("api_res is : %d\n", api_res);
    printf ("oerror is : %d\n", req_info.info.i_vip_install.o_err);*/
#if 0
  if(iOption==IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_SAME_IP_SAME_INTF)
    {
      int status;
      int rc;
      wait(&status);
      if(WIFEXITED(status)==0)
        api_res=NCSCC_RC_FAILURE;
      else
        {
          rc = WEXITSTATUS(status);
          printf("child status: %d, parent status: %d\n", rc, api_res);
          if(api_res == NCSCC_RC_SUCCESS && rc == NCSCC_RC_SUCCESS)
            api_res = NCSCC_RC_SUCCESS;
          else
            api_res = NCSCC_RC_FAILURE;
        }
    }
#endif
  vip_result(INSTALL_TESTCASE[iOption-1],1);

  if(NCSCC_RC_FAILURE==expectedResult||api_res==NCSCC_RC_FAILURE)
    {
      printf("\n\nFailed with error type : %s",
             VIP_ERRORS[req_info.info.i_vip_install.o_err]);
    }
  
  if(iOption==IFSV_VIP_INSTALL_ON_INTF_DOWN)
    {
      printf("\n\nPress any key to continue....\n");
      vip_wait();
      system("ifconfig eth1 up");
    }

  if(iOption==IFSV_VIP_INSTALL_SAME_AS_EXISTING_NORMAL_IP)
    system("ifconfig eth0 0.0.0.0");

  printf("\n\nPress any key to continue....\n");
  vip_wait();

  if(uninstallip==1)
    {
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      uninstallip=0;
    }

  printf("\nNumber of API calls: %d",iOption);

  printf("\n\nPress any key to continue....\n");
  vip_wait();

  printf("\n-----------------------------------------------------------\n");
}

void fill_vip(char *app, uns32 handl, char *intf, uns32 ip)
{
  int sleeptime = 0;
  api_res=0;
  strcpy(i_vip_install.i_handle.vipApplName,app);
  strcpy(i_vip_install.i_intf_name,intf);
  i_vip_install.i_handle.poolHdl=handl;
  i_vip_install.i_ip_addr.ipaddr.type=NCS_IP_ADDR_TYPE_IPV4;
  i_vip_install.i_ip_addr.ipaddr.info.v4=ip;
  i_vip_install.i_ip_addr.mask_len=24;
  i_vip_install.i_infraFlag=install_infra_flag;

  req_info.i_req_type=NCS_IFSV_REQ_VIP_INSTALL;
  req_info.info.i_vip_install=i_vip_install;

  api_res = ncs_ifsv_svc_req(&req_info);
  while ((api_res == NCSCC_RC_FAILURE) && 
         (req_info.info.i_vip_install.o_err == NCSCC_RC_VIP_RETRY 
          || req_info.info.i_vip_install.o_err == NCSCC_RC_VIP_INTERNAL_ERROR))
    {
      if (sleeptime > 35)
        {
          printf("Vip Install tried for 35 iterations in 35 seconds.\
 Giving up....");
          break;
        }  
      sleep(1);
      sleeptime++;
      printf("Retrying due to the error : %s\n", 
             VIP_ERRORS[req_info.info.i_vip_install.o_err]);
      api_res = ncs_ifsv_svc_req(&req_info);
    }
 
  networkaddr_to_ascii(i_vip_install.i_ip_addr.ipaddr.info.v4);
  if (i_vip_install.i_intf_name)
    intf_res = ip_on_intf(i_vip_install.i_intf_name);
  else
    intf_res = 1;

  /*cli_res = */
}

/*void ifsv_vip_install_fill(int i)*/
void tet_ncs_ifsv_svc_req_vip_install(int i)
{
  printf("\nStart of test case %d....\n", i);
  install_infra_flag = VIP_ADDR_NORMAL;
  switch(i)
    {
    case IFSV_VIP_INSTALL_NULL_APPL:
      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 256;
      fill_vip("",1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],1);
      break;

    case IFSV_VIP_INSTALL_NULL_HDL:
      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 256;
      fill_vip("",0,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],1);
      break;

    case IFSV_VIP_INSTALL_INVALID_INTF:
      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 256;
      fill_vip(vip_appl_name1,1,"eeth0",vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],1);
      break;

    case IFSV_VIP_INSTALL_NULL_INTF:
      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 256;
      fill_vip(vip_appl_name1,1,"",vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],1);
      break;

    case IFSV_VIP_INSTALL_NULL_IP:
      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 256;
      fill_vip(vip_appl_name1,1,vip_intf1,0);
      vip_result(INSTALL_TESTCASE[i-1],1);
      break;

    case IFSV_VIP_INSTALL_SUCC:
      /* still incomplete. Need to check the verification points like GARP is 
         sent or not */
      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],1);

      char command[100],*temp;         
      temp=(char *)getenv("PCS_ROOT");
      networkaddr_to_ascii(i_vip_install.i_ip_addr.ipaddr.info.v4);
      sprintf(command, "sh %s/xterm_run.sh %s %d %s %d %d %s %s", temp,
              vip_appl_name1,1,ip,24,7,vip_intf1,"ACTIVE-VIP");
      int res=system (command);
      if (res == 0)
        printf ("Hit found");

      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_DIFF_IP_DIFF_INTF_SAME_NODE:
      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],0);

      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,2,vip_intf2,vip_ip2);
      vip_result(INSTALL_TESTCASE[i-1],1);

      /* Free both VIPs */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      hdl=2;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_DIFF_IP_SAME_INTF_SAME_NODE:
      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],0);

      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,2,vip_intf1,vip_ip2);
      vip_result(INSTALL_TESTCASE[i-1],1);

      /* Free both VIPs */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      hdl=2;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_SAME_IP_DIFF_INTF_SAME_NODE:
      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],0);

      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 256;
      fill_vip(vip_appl_name1,2,vip_intf2,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],1);

      /* Free both VIPs */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_SAME_IP_SAME_INTF:
      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],0);

      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 0; /* ip exists due to first call on the same intf. */
      fill_vip(vip_appl_name1,2,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],1);

      /* Free both VIPs */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_SAME_IP_SAME_INTF:
      /*This case should be run in different ifA BUT SAME NODEi.e.
        comment out this case in one ifA and run only this case in other ifA*/
      /*fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
        expectedResult=NCSCC_RC_SUCCESS;*/
      gl_pid=fork();
      if(gl_pid==0)
        {
          printf("*****child: This is child....... \n");
          fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
          expectedResult=NCSCC_RC_FAILURE;
          vip_result(INSTALL_TESTCASE[i-1],1);
          /*req_info.i_req_type=NCS_IFSV_REQ_VIP_INSTALL;
            req_info.info.i_vip_install=i_vip_install;
            api_res=ncs_ifsv_svc_req(&req_info);
            printf("*****child: api_res is %d\n",api_res);
            if(api_res == 2)
            printf("\n\n*****CHILD:Failed with error type : %s",
            VIP_ERRORS[req_info.info.i_vip_install.o_err]);*/
          exit(api_res);
        }
      if(gl_pid!=0)
        {
          fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
          expectedResult=NCSCC_RC_SUCCESS;
          vip_result(INSTALL_TESTCASE[i-1],1);
          getchar();

          strcpy(applName,vip_appl_name1);
          hdl=1;
          ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
        }
      break;

    case IFSV_VIP_INSTALL_SAME_EXISTING_PARAMS:
      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],0);

      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 0; /* ip exists due to first call on the same intf. */
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],1);

      /* Free both VIPs */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_SAME_IP_DIFF_INTF:
      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],0);

      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 256;
      fill_vip(vip_appl_name1,1,vip_intf2,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],1);

      /* Free both VIPs */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_DIFF_IP_SAME_INTF:
      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],0);

      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 256; 
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip2);
      vip_result(INSTALL_TESTCASE[i-1],1);

      /* Free both VIPs */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_DIFF_IP_DIFF_INTF:
      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],0);

      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 256; 
      fill_vip(vip_appl_name1,1,vip_intf2,vip_ip2);
      vip_result(INSTALL_TESTCASE[i-1],1);

      /* Free both VIPs */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_INSTALL_DEL_INSTALL:
      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],1);

      /* Free the existing VIP */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_SAME_IP_SAME_INTF:
      /*Test with deletion commented out. Two records for a intf(above is 
        second record on eth1, both with infraFlag 0).
        Even after application quits the ip addr exists. */ 
      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],0);

      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name2,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],1);

      /*In case the above installation succeeds*/
      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      strcpy(applName,vip_appl_name2);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_SAME_IP_DIFF_INTF_SAME_NODE:
      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],0);

      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 256;
      fill_vip(vip_appl_name2,1,vip_intf2,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],1);

      strcpy(applName,vip_appl_name2);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_DIFF_IP_SAME_INTF:
      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],0);

      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name2,1,vip_intf1,vip_ip2);
      vip_result(INSTALL_TESTCASE[i-1],1);

      /*Free the above VIP*/
      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      strcpy(applName,vip_appl_name2);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_DIFF_IP_DIFF_INTF_SAME_NODE:
      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name1,1,vip_intf1,vip_ip1);
      vip_result(INSTALL_TESTCASE[i-1],0);

      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name2,1,vip_intf2,vip_ip2);
      vip_result(INSTALL_TESTCASE[i-1],1);

      /*Free the above VIP*/
      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      strcpy(applName,vip_appl_name2);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

      /* BELOW 2 TEST CASES NEEDS TO BE CHECKED. Replaced eth1 with eth2 */
    case IFSV_VIP_INSTALL_ON_INTF_DOWN:
      /*Bring down eth2 and try to install VIP on it*/
      system("vconfig add eth2 999");
      system("ifconfig eth2.999 down");

      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 256;
      fill_vip(vip_appl_name2,2,"eth2.999",0xc0a84d05);

      /*fill_vip(vip_appl_name2,1,vip_intf1,vip_ip1);*/
      vip_result(INSTALL_TESTCASE[i-1],1);
      strcpy(applName,vip_appl_name2);
      hdl=2;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);

      break;

    case IFSV_VIP_INSTALL_INTF_DOWN_AND_UP:
      sleep(2);
      system("ifconfig eth2.999 up");
      sleep(2);
      expectedResult=NCSCC_RC_SUCCESS;
      ip_on_intf_flag = 0;
      fill_vip(vip_appl_name2,3,"eth2.999",0xc0a84d09);
      system("ifconfig eth2.999 down");
      system("ifconfig eth2.999 up");
      vip_result(INSTALL_TESTCASE[i-1],1);

      /*Free the above VIP*/
      strcpy(applName,vip_appl_name2);
      hdl=3;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;
      /*------------------------------------------------------------------*/

#if 0 /*redundant*/
    case IFSV_VIP_INSTALL_INVALID_IP:
      fill_vip(vip_appl_name1,1,vip_intf1,0);
      expectedResult=NCSCC_RC_FAILURE;
      break;
#endif

    case IFSV_VIP_INSTALL_SAME_AS_EXISTING_NORMAL_IP:
      system("ip addr add 192.168.77.07/24 dev bond0");
      sleep(2);
      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 256;
      fill_vip(vip_appl_name2,4,vip_intf1,0xc0a84d07);
      vip_result(INSTALL_TESTCASE[i-1],1);

      system("ip addr del 192.168.77.07/24 dev bond0");
      if (api_res==NCSCC_RC_SUCCESS)
        {
          /*Free the above VIP just to make sure the ip is cleaned in case 
            the installation succeeds.*/
          strcpy(applName,vip_appl_name2);
          hdl=4;
          ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
        }
      break;
      
    case IFSV_VIP_INSTALL_TOO_LONG_APPL:
#if 0
      fill_vip("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefgh",1,vip_intf1,vip_ip1);
      expectedResult=NCSCC_RC_FAILURE;
#endif
      break;

    case IFSV_VIP_INSTALL_INVALID_INFRAFLAG:   
      expectedResult=NCSCC_RC_FAILURE;
      ip_on_intf_flag = 256;
      install_infra_flag = 5;
      fill_vip(vip_appl_name2,5,vip_intf1,0xc0a84d11);
      vip_result(INSTALL_TESTCASE[i-1],1);
      break;

    case IFSV_VIP_INSTALL_INFRAFLAG_ADDR_SPECIFIC:   
      install_infra_flag = VIP_ADDR_SPECIFIC;
      fill_vip(vip_appl_name2,5,vip_intf1,0xc0a84d11);
         
      printf("press any key ...\n");
      vip_wait();

      /* Freeing the ip with infra flag VIP_ADDR_SWITCH */
      strcpy(i_vip_free.i_handle.vipApplName,vip_appl_name2);
      i_vip_free.i_handle.poolHdl=5;
      i_vip_free.i_infraFlag=VIP_ADDR_SPECIFIC;

      req_info.i_req_type=NCS_IFSV_REQ_VIP_FREE;
      req_info.info.i_vip_free=i_vip_free;

      api_res=ncs_ifsv_svc_req(&req_info);
      if(api_res==NCSCC_RC_SUCCESS)
        printf("\n\nVIP delete of application (%s,5)\n",vip_appl_name2);
      else
        printf("\n\nFailed with error type : %s",
               VIP_ERRORS[req_info.info.i_vip_free.o_err]);

      ip_on_intf_flag = 256;
      networkaddr_to_ascii(0xc0a84d11);
      tet_printf("IFSV_VIP_INSTALL_INFRAFLAG_ADDR_SPECIFIC:");
      printf("IFSV_VIP_INSTALL_INFRAFLAG_ADDR_SPECIFIC:");
      if (ip_on_intf(vip_intf1))
        {
          tet_result(TET_PASS);
          printf("TEST PASS\n");
          /*Check in VIP-DB cli*/
        }
      else
        {
          tet_result(TET_FAIL);
          printf("TEST FAIL\n");
        }

      break;
    }

  printf("\npress enter to continue...\n");
  vip_wait();
  printf("\n-----------------------------------------------------------\n");
}

void tet_ncs_ifsv_free_spec_ip(int iOption)
{
  char opt, *app, *intf;
  int handle;

  app = (char *) malloc(10);
  intf = (char *) malloc(5);
  i_vip_free.i_infraFlag=VIP_ADDR_NORMAL;

 getOption:
  printf ("Specify whether the ip is Stale or Normal (S/N)?");
  scanf ("%c", &opt);
  printf ("Application Name? ");
  scanf ("%s", app);
  printf ("Handle? ");
  scanf ("%d", &handle);
  printf ("Interface? ");
  scanf ("%s", intf);
  if (opt == 'N' || opt == 'n')
    {
      strcpy(applName,app);
      hdl=handle;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
    }
  else if (opt == 'S' || opt == 's')
    {
      fill_vip(app,handle,intf,0);
      printf("Check the ip installation on eth1 and vip-dump\n");
      strcpy(applName,app);
      hdl=handle;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
    }
  else
    {
      printf("Exiting ...\n");
      exit(0);
    }
  scanf ("%c", &opt);
  printf ("One more?(Y/N)");
  scanf ("%c", &opt);
  if (opt == 'Y' || opt == 'y')
    goto getOption;
  else 
    printf ("Exiting ...\n");
  printf("--------------------------------------------\n");
}

void tet_ncs_ifsv_svc_req_failover_test(int iOption)
{
  char opt;
  i_vip_free.i_infraFlag=VIP_ADDR_NORMAL;

  printf ("After failover or Before failover (A/B)?");
  scanf ("%c", &opt);
  if (opt == 'B' || opt == 'b')
    {
      fill_vip("FAILOVER",1,vip_intf1,vip_ip1);
      printf("\nCheck the ip-addr and press enter to continue...");
      getchar();
    }
  else if (opt == 'A' || opt == 'a')
    {
      fill_vip("FAILOVER",1,vip_intf1,0);
      printf("Check the ip installation on eth1 and vip-dump\n");
      getchar();
      strcpy(applName,"FAILOVER");
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
    }
  else
    {
      printf ("Wrong option. Exiting ...\n");
    }
  printf("--------------------------------------------\n");
}


extern int gl_sync_pointnum;
extern int fill_syncparameters(int vote);

void tet_ncs_ifsv_svc_req_node_1(int iOption)
{
  gl_sync_pointnum=0;
  i_vip_free.i_infraFlag=VIP_ADDR_NORMAL;
  switch(iOption)
    {
    case IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_SAME_IP_DIFF_INTF_DIFF_NODE:
      fill_vip(vip_appl_name1,1,vip_intf1,0xc0a84d11);
      expectedResult=NCSCC_RC_SUCCESS;

      /*req_info.i_req_type=NCS_IFSV_REQ_VIP_INSTALL;
        req_info.info.i_vip_install=i_vip_install;

        api_res=ncs_ifsv_svc_req(&req_info);*/
      result("IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_SAME_IP_DIFF_INTF_DIFF_NODE:"
             ,expectedResult);

      if(api_res==NCSCC_RC_FAILURE)
        printf("\n\nFailure : %d",req_info.info.i_vip_install.o_err);
      fill_syncparameters(TET_SV_YES); 

      /* Free the existing VIP */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      fill_syncparameters(TET_SV_YES); 
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;
    
    case IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_SAME_IP_DIFF_INTF_DIFF_NODE:
      fill_vip(vip_appl_name1,1,vip_intf1,0xc0a84d11);
      expectedResult=NCSCC_RC_SUCCESS;

      /*req_info.i_req_type=NCS_IFSV_REQ_VIP_INSTALL;
        req_info.info.i_vip_install=i_vip_install;

        api_res=ncs_ifsv_svc_req(&req_info);*/
      result("IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_SAME_IP_DIFF_INTF_DIFF_NODE:"
             ,expectedResult);

      if(api_res==NCSCC_RC_FAILURE)
        printf("\n\nFailure : %d",req_info.info.i_vip_install.o_err);
      fill_syncparameters(TET_SV_YES); 

      /* Free the existing VIP */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      fill_syncparameters(TET_SV_YES); 
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;
    
    case IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_SAME_IP_DIFF_INTF_DIFF_NODE:
      fill_vip(vip_appl_name1,1,vip_intf1,0xc0a84d11);
      expectedResult=NCSCC_RC_SUCCESS;

      /*req_info.i_req_type=NCS_IFSV_REQ_VIP_INSTALL;
        req_info.info.i_vip_install=i_vip_install;

        api_res=ncs_ifsv_svc_req(&req_info);*/
      result("IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_SAME_IP_DIFF_INTF_DIFF_NODE:"
             ,expectedResult);

      if(api_res==NCSCC_RC_FAILURE)
        printf("\n\nFailure : %d",req_info.info.i_vip_install.o_err);

      fill_syncparameters(TET_SV_YES); 

      /* Free the existing VIP */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      fill_syncparameters(TET_SV_YES); 
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_DIFF_IP_DIFF_INTF_DIFF_NODE:
      fill_vip(vip_appl_name1,1,vip_intf1,0xc0a84d11);
      expectedResult=NCSCC_RC_SUCCESS;

      /*req_info.i_req_type=NCS_IFSV_REQ_VIP_INSTALL;
        req_info.info.i_vip_install=i_vip_install;

        api_res=ncs_ifsv_svc_req(&req_info);*/
      result("IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_DIFF_IP_DIFF_INTF_DIFF_NODE:"
             ,expectedResult);

      if(api_res==NCSCC_RC_FAILURE)
        printf("\n\nFailure : %d",req_info.info.i_vip_install.o_err);

      fill_syncparameters(TET_SV_YES); 

      /* Free the existing VIP */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      fill_syncparameters(TET_SV_YES); 
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_UNINSTALL_FROM_NON_OWNER:
      fill_vip(vip_appl_name1,1,vip_intf1,0xc0a84d11);
      expectedResult=NCSCC_RC_SUCCESS;

      /*req_info.i_req_type=NCS_IFSV_REQ_VIP_INSTALL;
        req_info.info.i_vip_install=i_vip_install;

        api_res=ncs_ifsv_svc_req(&req_info);*/
      result("IFSV_VIP_UNINSTALL_FROM_NON_OWNER: ",expectedResult);
      fill_syncparameters(TET_SV_YES); 

      if(api_res==NCSCC_RC_FAILURE)
        printf("\n\nFailure : %d",req_info.info.i_vip_install.o_err);

      /* Free the existing VIP */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      fill_syncparameters(TET_SV_YES); 
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      break;

    case IFSV_VIP_TWO_INSTALL_ONE_UNINSTALL:
      fill_vip(vip_appl_name1,1,vip_intf1,0xc0a84d11);
      expectedResult=NCSCC_RC_SUCCESS;

      /*req_info.i_req_type=NCS_IFSV_REQ_VIP_INSTALL;
        req_info.info.i_vip_install=i_vip_install;

        api_res=ncs_ifsv_svc_req(&req_info);*/
      result("IFSV_VIP_TWO_INSTALL_ONE_UNINSTALL: ",expectedResult);
      fill_syncparameters(TET_SV_YES); 

      if(api_res==NCSCC_RC_FAILURE)
        printf("\n\nFailure : %d",req_info.info.i_vip_install.o_err);

      /* Free the existing VIP */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      fill_syncparameters(TET_SV_YES); 
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      fill_syncparameters(TET_SV_YES); 
      break;
    }
  printf("\n\nPress any key to continue....\n");
  vip_wait();
  printf("********************************************************\n");
}

void tet_ncs_ifsv_svc_req_node_2(int iOption)
{
  gl_sync_pointnum=0;
  switch(iOption)
    {
    case IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_SAME_IP_DIFF_INTF_DIFF_NODE:
      fill_syncparameters(TET_SV_YES); 
      fill_vip(vip_appl_name1,2,vip_intf1,0xc0a84d11);
      expectedResult=NCSCC_RC_FAILURE;

      /*req_info.i_req_type=NCS_IFSV_REQ_VIP_INSTALL;
        req_info.info.i_vip_install=i_vip_install;

        api_res=ncs_ifsv_svc_req(&req_info);*/
      result("IFSV_VIP_INSTALL_SAME_APPL_DIFF_HDL_SAME_IP_DIFF_INTF_DIFF_NODE:"
             ,expectedResult);

      if(api_res==NCSCC_RC_FAILURE)
        {
          printf("\n\nFailure : %d",req_info.info.i_vip_install.o_err);
          fill_syncparameters(TET_SV_YES); 
          break;
        }

      /* Free the existing VIP */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      /*result("ncs_ifsv_free_req() for vip installation.",expectedResult);*/
      fill_syncparameters(TET_SV_YES); 
      break;
    
    case IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_SAME_IP_DIFF_INTF_DIFF_NODE:
      fill_syncparameters(TET_SV_YES); 
      fill_vip(vip_appl_name1,1,vip_intf1,0xc0a84d11);
      expectedResult=NCSCC_RC_FAILURE;

      /*req_info.i_req_type=NCS_IFSV_REQ_VIP_INSTALL;
        req_info.info.i_vip_install=i_vip_install;

        api_res=ncs_ifsv_svc_req(&req_info);*/
      result("IFSV_VIP_INSTALL_SAME_APPL_SAME_HDL_SAME_IP_DIFF_INTF_DIFF_NODE:"
             ,expectedResult);

      if(api_res==NCSCC_RC_FAILURE)
        {
          printf("\n\nFailure : %d",req_info.info.i_vip_install.o_err);
          fill_syncparameters(TET_SV_YES); 
          break;
        }

      /* Free the existing VIP */
      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      /*result("ncs_ifsv_free_req() for vip installation.",expectedResult);*/
      fill_syncparameters(TET_SV_YES); 
      break;
    
    case IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_SAME_IP_DIFF_INTF_DIFF_NODE:
      fill_syncparameters(TET_SV_YES);

      fill_vip(vip_appl_name2,1,vip_intf1,0xc0a84d11);
      expectedResult=NCSCC_RC_FAILURE;

      /*req_info.i_req_type=NCS_IFSV_REQ_VIP_INSTALL;
        req_info.info.i_vip_install=i_vip_install;

        api_res=ncs_ifsv_svc_req(&req_info);*/
      result("IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_SAME_IP_DIFF_INTF_DIFF_NODE:"
             ,expectedResult);

      if(api_res==NCSCC_RC_FAILURE)
        {
          printf("\n\nFailure : %d",req_info.info.i_vip_install.o_err);
          fill_syncparameters(TET_SV_YES); 
          break;
        }

      /* Free the existing VIP */
      strcpy(applName,vip_appl_name2);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      fill_syncparameters(TET_SV_YES); 
      break;

    case IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_DIFF_IP_DIFF_INTF_DIFF_NODE:
      fill_syncparameters(TET_SV_YES);

      fill_vip(vip_appl_name2,1,vip_intf1,0xc0a84d12);
      expectedResult=NCSCC_RC_SUCCESS;

      /*req_info.i_req_type=NCS_IFSV_REQ_VIP_INSTALL;
        req_info.info.i_vip_install=i_vip_install;

        api_res=ncs_ifsv_svc_req(&req_info);*/
      result("IFSV_VIP_INSTALL_DIFF_APPL_SAME_HDL_DIFF_IP_DIFF_INTF_DIFF_NODE:"
             ,expectedResult);

      if(api_res==NCSCC_RC_FAILURE)
        {
          printf("\n\nFailure : %d",req_info.info.i_vip_install.o_err);
          fill_syncparameters(TET_SV_YES); 
          break;
        }

      /* Free the existing VIP */
      strcpy(applName,vip_appl_name2);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      fill_syncparameters(TET_SV_YES);
      break;

    case IFSV_VIP_UNINSTALL_FROM_NON_OWNER:
      fill_syncparameters(TET_SV_YES); 
      strcpy(i_vip_free.i_handle.vipApplName,vip_appl_name1);
      i_vip_free.i_handle.poolHdl=1;
      i_vip_free.i_infraFlag=VIP_ADDR_NORMAL;
      expectedResult=NCSCC_RC_FAILURE;

      req_info.i_req_type=NCS_IFSV_REQ_VIP_FREE;
      req_info.info.i_vip_free=i_vip_free;

      api_res=ncs_ifsv_svc_req(&req_info);
      result("IFSV_VIP_UNINSTALL_FROM_NON_OWNER: ",expectedResult);

      if(api_res==NCSCC_RC_FAILURE)
        {
          printf("\n\nFailure : %d",req_info.info.i_vip_install.o_err);
        }
      fill_syncparameters(TET_SV_YES); 
      break;

    case IFSV_VIP_TWO_INSTALL_ONE_UNINSTALL:
      fill_syncparameters(TET_SV_YES); 
      fill_vip(vip_appl_name1,1,vip_intf1,0xc0a84d11);

      /*req_info.i_req_type=NCS_IFSV_REQ_VIP_INSTALL;
        req_info.info.i_vip_install=i_vip_install;

        api_res=ncs_ifsv_svc_req(&req_info);*/

      expectedResult=NCSCC_RC_FAILURE;
      result("IFSV_VIP_TWO_INSTALL_ONE_UNINSTALL: ",expectedResult);
      /* added above two lines from the place below if-else(commented out) */

      fill_syncparameters(TET_SV_YES); 
      if(api_res==NCSCC_RC_FAILURE)
        {
          printf("\n\nFailure : %d",req_info.info.i_vip_install.o_err);
          break;
        }

      /* Free the existing VIP */
      /*printf("\n\nNow check the ip address on eth1 also VIP-DUMP.
        Press Y/N....\n");
        scanf("%c", &c);

        if(c=='Y' || c=='y')
        expectedResult=NCSCC_RC_SUCCESS;
        else
        expectedResult=NCSCC_RC_FAILURE;*/

      strcpy(applName,vip_appl_name1);
      hdl=1;
      ifsv_vip_free_fill(IFSV_VIP_FREE_APPL_SUCCESS);
      fill_syncparameters(TET_SV_YES); 
      break;
    }
  printf("\n\nPress any key to continue....\n");
  vip_wait();
  printf("********************************************************\n");
}

void result(char *saf_msg,int ref)
{
  printf("\n\n%s",&saf_msg[0]);
  tet_printf("%s",&saf_msg[0]);

  if(api_res==ref)
    {
      printf("\nSuccess : %s",gl_ifsv_error_code[api_res]);

      tet_printf("Success : %s",gl_ifsv_error_code[api_res]);
      tet_result(TET_PASS);
      if(api_res==NCSCC_RC_SUCCESS)
        {
        }
    }
  else
    {
      printf("\nFailure : %s",gl_ifsv_error_code[api_res]);

      tet_printf("Failure : %s",gl_ifsv_error_code[api_res]);
      tet_result(TET_FAIL);
    }
}

void dummy(void)
{
  printf("dummy\n");
  tet_result(TET_PASS);
}

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

void tet_ifsv_startup()
{
  tet_run_ifsv_app();
}
void tet_ifsv_cleanup()
{
  tet_test_cleanup();
}

void gl_ifsv_defs()
{
  char *temp=NULL;
  struct in_addr ip;

  printf("\n\n********************\n");
  printf("\nGlobal structure variables\n");

  temp=(char *)getenv("TET_CASE");
  if(temp)
    {
      gl_tCase=atoi(temp);
    }
  else
    {
      gl_tCase=-1;
    }
  printf("\nTest Case Number: %d",gl_tCase);

  temp=(char *)getenv("TET_ITERATION");
  if(temp)
    {
      gl_iteration=atoi(temp);
    }
  else
    {
      gl_iteration=1;
    }
  printf("\nNumber of iterations: %d",gl_iteration);

  temp=(char *)getenv("TET_LIST_NUMBER");
  if(temp&&(atoi(temp)>0&&(atoi(temp)<3)))
    {
      gl_listNumber=atoi(temp);
    }
  else
    {
      gl_listNumber=-1;
    }

  temp=(char *)getenv("VIP_APP_NAME1");
  if(temp)
    vip_appl_name1=temp;
  else
    {
      printf("Application1 name is not specified\n");
      exit(0);
    }

  temp=(char *)getenv("VIP_APP_NAME2");
  if(temp)
    vip_appl_name2=temp;
  else
    {
      printf("Application2 name is not specified\n");
      exit(0);
    }

   temp=(char *)getenv("VIP_INTF1");
  if(temp)
    vip_intf1=temp;
  else
    {
      printf("Interface 1  not  specified\n");
      exit(0);
    } 

    temp=(char *)getenv("VIP_INTF2");
  if(temp)
    vip_intf2=temp;
  else
    {
      printf("Interface 2  not  specified\n");
      exit(0);
    }
  temp=(char *)getenv("VIP_IP1");
  if (temp)
    {
      inet_aton(temp,&ip);
      vip_ip1=ntohl(ip.s_addr);
    }
  else
    vip_ip1=3232255233U;

  temp=(char *)getenv("VIP_CURRENT_IP1");
  if (temp)
    { 
      inet_aton(temp,&ip);
      vip_current_ip1=ntohl(ip.s_addr);
    }
  else
    vip_current_ip1=3232255235U;

  temp=(char *)getenv("VIP_IP2");
  if (temp)
    {
      inet_aton(temp,&ip);
      vip_ip2=ntohl(ip.s_addr);
    }
  else
    vip_ip2=3232255234U;
    vip_switchover=0;
   temp=(char *)getenv("VIP_CURRENT_IP2");
  if (temp)
    { 
      inet_aton(temp,&ip);
      vip_current_ip2=ntohl(ip.s_addr);
    } 
  else
    vip_current_ip2=3232255236U;

  flag = (char *) getenv("VIP_WAIT");

  printf("\nTest List to be executed: %d",gl_listNumber);
  printf("\n\n********************\n");
}

void tet_run_ifsv_app()
{
  char *temp=NULL;

  gl_ifsv_defs();
  
  temp=(char *)getenv("TET_SHELF_ID");
  shelf_id=atoi(temp);

  temp=(char *)getenv("TET_SLOT_ID");
  slot_id=atoi(temp);

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
}
