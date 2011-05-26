#include <stdio.h>
#include <stdlib.h>
#include <tet_api.h>
#include "mds_papi.h"
#include "ncs_mda_papi.h"
#include "ncs_main_papi.h"
#include "tet_mdstipc.h"
#include "ncssysf_tmr.h"
void tet_mds_tds_startup(void);
void tet_mds_tds_cleanup(void);
extern void tet_sender_startup(int);
extern void tet_receiver_startup(int);
void tet_sender_cleanup(void);
void tet_receiver_cleanup(void);
static int  tet_verify_version(MDS_HDL, NCSMDS_SVC_ID,NCSMDS_SVC_ID ,
                               MDS_SVC_PVT_SUB_PART_VER,NCSMDS_CHG );
static MDS_CLIENT_MSG_FORMAT_VER gl_set_msg_fmt_ver;
#if 0 
void (*tet_startup)() = tet_mds_tds_startup;
void (*tet_cleanup)() = tet_mds_tds_cleanup;
#endif

void ncs_agents_shut_start(int choice)
{
  int FAIL=0;

    tet_printf("\n\n ncs_agents_shut_start\n");
    if(ncs_agents_shutdown()!=NCSCC_RC_SUCCESS)
    {
      perror("\n\n ----- NCS AGENTS SHUTDOWN FAILED ----- \n\n");
      tet_printf("ncs_agents_shutdown(): FAIL");
      FAIL=1;
    }
    else
    tet_printf("ncs_agents_shutdown(): SUCCESS");
   
  sleep(5);
    if(ncs_agents_startup()!=NCSCC_RC_SUCCESS)
    {
      perror("\n\n ----- NCS AGENTS START UP FAILED ------- \n\n");
      tet_printf("ncs_agents_startup(): FAIL");
      FAIL=1;
    }
    else
    tet_printf("ncs_agents_startup(): SUCCESS");

  sleep(5);

  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n**********************************************************\n");    
}


MDS_SVC_ID svc_ids[3]={2006,2007,2008};


/*****************************************************************************/
/************        SERVICE API TEST PURPOSES   *****************************/
/*****************************************************************************/
void tet_vdest_install_thread()
{
  printf(" Inside Thread");
  if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                          NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                          NCSMDS_SCOPE_NONE,true,false)!=NCSCC_RC_SUCCESS)
    {
      perror("Install Failed");
    }
}
void  tet_svc_install_tp(int choice)
{
  int FAIL=0;
  gl_vdest_indx=0;
  printf("\n\n************ tet_svc_install_tp %d ***********\n",choice);
  tet_printf("\t\t\t tet_svc_install_tp");
  /*start up*/
  tet_printf("\tCreating a MxN VDEST with id =2000");
  if(create_vdest(NCS_VDEST_TYPE_MxN,2000)!=NCSCC_RC_SUCCESS)
    FAIL=1;
  switch(choice)
    {
    case 1:
      tet_printf("Case 1 : \tInstalling the service 500");
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,500,1,
                              NCSMDS_SCOPE_NONE,true,false)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }  
      tet_printf("\tNot able to Install an ALREADY installed service 500");
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,500,1,
                              NCSMDS_SCOPE_NONE,true,false)==NCSCC_RC_SUCCESS)
        { 
          tet_printf("Fail");
          FAIL=1;
        }     
      tet_printf("\tUninstalling the above service");
      if(mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,500)!=
         NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");  
      break;
    case 2:
      tet_printf("Case 2 : \tInstalling the Internal MIN service INTMIN with \
NODE scope without MDS Q ownership and failing to Retrieve");
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                              NCSMDS_SVC_ID_INTERNAL_MIN,1,
                              NCSMDS_SCOPE_INTRANODE,false,false)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("INstall Fail");
          FAIL=1; 
        }  
      if( mds_service_retrieve(gl_tet_vdest[0].mds_pwe1_hdl,
                               NCSMDS_SVC_ID_INTERNAL_MIN
                               ,SA_DISPATCH_ALL)==NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tUninstalling the above service");
      if(mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,
                               NCSMDS_SVC_ID_INTERNAL_MIN)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");  
      break;
    case 3:
      tet_printf("Case 3 : \tInstalling the External MIN service EXTMIN with \
CHASSIS scope");
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                              NCSMDS_SCOPE_INTRACHASSIS,true,false)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }  
      tet_printf("\tUninstalling the above service");
      if(mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");  
      break;
    case 5:
      tet_printf("Case 5 : \tInstalling the service with MAX id:32767 with \
CHASSIS scope");
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,NCSMDS_MAX_SVCS,1,
                              NCSMDS_SCOPE_INTRACHASSIS,
                              true,false)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }  
      tet_printf("\tUninstalling the above service");
      if(mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,NCSMDS_MAX_SVCS)!=
         NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");  
      break;
    case 6:
      tet_printf("Case 6 : \tNot able to Install the service with id > MAX:\
 32767 with CHASSIS scope");
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,NCSMDS_MAX_SVCS+1,1,
                              NCSMDS_SCOPE_INTRACHASSIS,
                              true,false)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }  
      else
        tet_printf("Success"); 
      break;
    case 7:
      tet_printf("Case 7 : \t Not able to Install the service with id =0 ");
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,0,1,NCSMDS_SCOPE_NONE,
                              false,false)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }  
      else
        tet_printf("Success");  
      break;
    case 8:
      tet_printf("Case 8 : \tNot able to Install the service with Invalid PWE \
 Handle");
      if( mds_service_install((MDS_HDL)(long)NULL,100,1,NCSMDS_SCOPE_NONE,false,false)==
          NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }  
      else
        tet_printf("Success");  
      if( mds_service_install(0,100,1,NCSMDS_SCOPE_NONE,false,false)==
          NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }  
      else
        tet_printf("Success");  
      if( mds_service_install(gl_tet_vdest[0].mds_vdest_hdl,100,1,
                              NCSMDS_SCOPE_NONE,false,false)== NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }  
      else
        tet_printf("Success");  
      break;
    case 9:
      tet_printf("Case 9 : \tNot able to Install a Service with Svc id > \
External  MIN 2000 which doesn't chose MDS Q Ownership");
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN+1,1,
                              NCSMDS_SCOPE_INTRANODE,false,false)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }  
      else
        tet_printf("Success");  
      break;
    case 10:
      tet_printf("Case 10 : \tInstalling the External MIN service EXTMIN in \
a seperate thread and Uninstalling it here");
      /*Install thread*/
      if(tet_create_task((NCS_OS_CB)tet_vdest_install_thread,
                         gl_tet_vdest[0].svc[0].task.t_handle)
         ==NCSCC_RC_SUCCESS )
        {
          printf("\tTask has been Created\t");fflush(stdout);
        }
      sleep(2);
      fflush(stdout);
      /*Now Release the Install Thread*/
      if(m_NCS_TASK_RELEASE(gl_tet_vdest[0].svc[0].task.t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
      if(gl_tet_vdest[0].svc_count==0)
        {
          tet_printf("Install Fail");
          FAIL=1;
        }
      else
        tet_printf("Success");
      fflush(stdout);

      tet_printf("\tUninstalling the above service");
      if(mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");  
      break;
    case 11:
      tet_printf("Case 11: \tInstallation of the same Service id with different \
sub-part versions on the same pwe handle, must fail");
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                              NCSMDS_SVC_ID_INTERNAL_MIN ,1,
                              NCSMDS_SCOPE_INTRANODE,true,false)!= NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                              NCSMDS_SVC_ID_INTERNAL_MIN ,2,
                              NCSMDS_SCOPE_INTRANODE,true,false)!= NCSCC_RC_FAILURE)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tUninstalling the above service");
      if(mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,NCSMDS_SVC_ID_INTERNAL_MIN)!=
         NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      else
        tet_printf("Success");
      break;
 
    case 12:
      tet_printf("Case 12: \tInstallation of the same Service id with same \
sub-part versions on the same pwe handle, must fail");
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN ,1,
                              NCSMDS_SCOPE_INTRANODE,true,false)!= NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN ,1,
                              NCSMDS_SCOPE_INTRANODE,true,false)!= NCSCC_RC_FAILURE)
        {
          tet_printf("Fail1");
          FAIL=1;
        }
      tet_printf("\tUninstalling the above service");
      if(mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,NCSMDS_SVC_ID_EXTERNAL_MIN)!=
         NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      else
        tet_printf("Success");
      break;
    case 13:
      tet_printf("Case 13: \tInstall a service with _mds_svc_pvt_ver = 0\
                      i_mds_svc_pvt_ver =  255 and i_mds_svc_pvt_ver = A random value, which is >0 and <255");
      if(adest_get_handle()!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if( mds_service_install(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN ,0,
                              NCSMDS_SCOPE_NONE,true,true)!= NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_uninstall(gl_tet_adest.mds_pwe1_hdl,NCSMDS_SVC_ID_EXTERNAL_MIN)!=
         NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      if( mds_service_install(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_INTERNAL_MIN ,255,
                              NCSMDS_SCOPE_INTRACHASSIS,true,true)!= NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_uninstall(gl_tet_adest.mds_pwe1_hdl,NCSMDS_SVC_ID_INTERNAL_MIN)!=
         NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if( mds_service_install(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_INTERNAL_MIN ,125,
                              NCSMDS_SCOPE_INTRANODE,true,true)!= NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_uninstall(gl_tet_adest.mds_pwe1_hdl,NCSMDS_SVC_ID_INTERNAL_MIN)!=
         NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      else 
        tet_printf("Success");
      break;

    case 14:
      tet_printf("Case 14: Install a service with \
                      i_fail_no_active_sends = 0 and i_fail_no_active_sends = 1");
      if(adest_get_handle()!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if( mds_service_install(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN ,1,
                              NCSMDS_SCOPE_NONE,true,FAIL)!= NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_uninstall(gl_tet_adest.mds_pwe1_hdl,NCSMDS_SVC_ID_EXTERNAL_MIN)!=
         NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if( mds_service_install(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_INTERNAL_MIN ,1,
                              NCSMDS_SCOPE_INTRACHASSIS,true,true)!= NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_uninstall(gl_tet_adest.mds_pwe1_hdl,NCSMDS_SVC_ID_INTERNAL_MIN)!=
         NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      else
        tet_printf("Success");
      break;

    case 15:
      tet_printf("Case 15: \tInstallation of a service with same service id and \
                      i_fail_no_active_sends again on the same pwe handle, must fail");
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN ,1,
                              NCSMDS_SCOPE_INTRANODE,true,true)!= NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN ,1,
                              NCSMDS_SCOPE_INTRANODE,true,true)!= NCSCC_RC_FAILURE)
        {
          tet_printf("Fail1");
          FAIL=1;
        }
      tet_printf("\tUninstalling the above service");
      if(mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,NCSMDS_SVC_ID_EXTERNAL_MIN)!=
         NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      else
        tet_printf("Success");
      break;
    case 16:
      tet_printf("Case 16: \tInstallation of a service with same service id and \
 different i_fail_no_active_sends again on the same pwe handle, must fail");
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN ,1,
                              NCSMDS_SCOPE_INTRANODE,true,true)!= NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN ,1,
                              NCSMDS_SCOPE_INTRANODE,true,FAIL)!= NCSCC_RC_FAILURE)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tUninstalling the above service");
      if(mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,NCSMDS_SVC_ID_EXTERNAL_MIN)!=
         NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      else
        tet_printf("Success");
      break;
    case 17:
      tet_printf("Case 17: \tFor MDS_INSTALL API, supply \
i_yr_svc_hdl = (2^32) and i_yr_svc_hdl = (2^64 -1)"); 
      gl_tet_svc.yr_svc_hdl = 4294967296ULL; 
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN ,1,
                              NCSMDS_SCOPE_INTRANODE,true,true)!= NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      gl_tet_svc.yr_svc_hdl = 18446744073709551615ULL;
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                              NCSMDS_SVC_ID_INTERNAL_MIN ,1,
                              NCSMDS_SCOPE_INTRANODE,true,FAIL)!= NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tUninstalling the above services");
      if( (mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,NCSMDS_SVC_ID_EXTERNAL_MIN)!=
           NCSCC_RC_SUCCESS) && (mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,NCSMDS_SVC_ID_INTERNAL_MIN)!=
                                 NCSCC_RC_SUCCESS))
        {
          tet_printf("Fail");
          FAIL=1;
        }

      else
        tet_printf("Success");
      break;

    }
  /*clean up*/
  tet_printf("\tDestroying the VDEST 2000");
  if(destroy_vdest(2000)!=NCSCC_RC_SUCCESS)
    FAIL=1;
  
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n**********************************************************\n");    
}


void tet_svc_install_upto_MAX()
{
  int FAIL=0;
  int id=0;
  gl_vdest_indx=0;
  printf("\n\n***************** tet_svc_install_upto_MAX ****************\n");
  tet_printf("\t\t\ttet_svc_install_upto_MAX");
  /*start up*/
  tet_printf("\tCreating a N-way VDEST with id =1001");
  if(create_vdest(NCS_VDEST_TYPE_MxN,1001)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    } 
  
  printf("\n\tNo. of Services = %d\n",gl_tet_vdest[0].svc_count);
  fflush(stdout);sleep(1);
  /*With MDS Q ownership*/
  tet_printf("\tInstalling the services from SVC_ID_INTERNAL_MIN to +500");
  for(id=NCSMDS_SVC_ID_INTERNAL_MIN+1;(id<=NCSMDS_SVC_ID_INTERNAL_MIN+500)&&
        (!FAIL);id++)
    {
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,id,1,
                              NCSMDS_SCOPE_NONE,true,false)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
    }
  tet_printf("\tMAX number of service that can be created on a VDEST = %d",
             gl_tet_vdest[0].svc_count);
  printf("\n\tNo. of Services= %d\n",gl_tet_vdest[0].svc_count);fflush(stdout);
  sleep(1);
  sleep(3);
  tet_printf("\tUninstalling the above service");
  for(id=gl_tet_vdest[0].svc_count-1;id>=0;id--)
    {
      if(mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,
                               gl_tet_vdest[0].svc[id].svc_id)
         !=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
    }
  printf("\n\tNo. of Services = %d\n",gl_tet_vdest[0].svc_count);
  fflush(stdout);sleep(1);
  /*clean up*/
  tet_printf("\tDestroying the VDEST 1001");
  if(destroy_vdest(1001)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    } 
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);          
  
  printf("\n*************************************************************\n"); 
}
void tet_vdest_uninstall_thread()
{
  printf(" Inside Thread");
  if(mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,500)!=
     NCSCC_RC_SUCCESS)
    {
      perror("Uninstall Failed");
    }
}
void tet_svc_unstall_tp(int choice)
{
  int FAIL=0;
  gl_vdest_indx=0;
  printf("\n\n************** tet_svc_uninstall_tp %d **************\n",choice);
  tet_printf("\t\t\t tet_svc_unstall_tp");
  /*start up*/
  tet_printf("\tCreating a N-way VDEST with id =1001");
  if(create_vdest(NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,1001)!=
     NCSCC_RC_SUCCESS)
    {      tet_infoline("Fail");FAIL=1;    }  
  switch(choice)
    {
    case 1:
      tet_printf("Case 1 :\n\tInstalling the service 500");
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,500,1,
                              NCSMDS_SCOPE_INTRANODE,true,false)!=NCSCC_RC_SUCCESS)
        { 
          tet_printf("Fail");
          FAIL=1;
        }
      
      tet_printf("\tUninstalling the above service");
      if(mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,500)!=
         NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }  
      else
        tet_printf("Success");  
      break;
    case 2:
      tet_printf("Case 2 : \tNot able to Uninstall an ALREADY uninstalled \
 service");
      if(mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,500)==
         NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }     
      else  
        tet_printf("Success");
      break;
    case 3:
      tet_printf("Case 3 : \tNot able to Uninstall a Service which doesn't \
 exist ");
      if(mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,600)==
         NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }     
      else
        tet_printf("Success");  
      break;  

    case 4:
      tet_printf("Case 4 : \tInstalling a service EXTMIN");
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                              NCSMDS_SCOPE_INTRACHASSIS,
                              true,false)!=NCSCC_RC_SUCCESS)
        {    tet_printf("Fail");FAIL=1;  }
      
      tet_printf("\tNot able to Uninstall the above Service with NULL PWE \
 handle");
      if(mds_service_uninstall((MDS_HDL)(long)NULL,NCSMDS_SVC_ID_EXTERNAL_MIN)
         == NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }     
      
      tet_printf("\tNot able to Uninstall the above Service with ZERO PWE \
 handle");
      if(mds_service_uninstall(0,NCSMDS_SVC_ID_EXTERNAL_MIN)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }     
      tet_printf("\tNot able to Uninstall the above Service with VDEST \
 handle");
      if(mds_service_uninstall(gl_tet_vdest[0].mds_vdest_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }     
      tet_printf("\tUninstalling the above service");
      if(mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN)!= NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        } 
      else
        tet_printf("Success");   
      break; 

    case 5:
      tet_printf("Case 5 :\n\tInstalling the service 500");
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,500,1,
                              NCSMDS_SCOPE_INTRANODE,true,false)!=NCSCC_RC_SUCCESS)
        { 
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tUninstalling the above service in a seperate thread");
      /*Uninstall thread*/
      if(tet_create_task((NCS_OS_CB)tet_vdest_uninstall_thread,
                         gl_tet_vdest[0].svc[0].task.t_handle)
         ==NCSCC_RC_SUCCESS )
        {
          printf("\tTask has been Created\t");fflush(stdout);
        }
      sleep(2);
      fflush(stdout);
      /*Now Release the Uninstall Thread*/
      if(m_NCS_TASK_RELEASE(gl_tet_vdest[0].svc[0].task.t_handle)
         ==NCSCC_RC_SUCCESS)
        {
          printf("\tTASK is released\n");
        }
      if(gl_tet_vdest[0].svc_count)
        {    
          tet_printf("Uninstall Fail");
          FAIL=1; 
        } 
      else
        tet_printf("Success");   
      fflush(stdout);                
      break;
    } 
  /*clean up*/
  tet_printf("\tDestroying the VDEST 1001");
  if(destroy_vdest(1001)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);          
  printf("\n***********************************************************\n");   
}
static int  tet_verify_version(MDS_HDL mds_hdl,NCSMDS_SVC_ID your_scv_id,NCSMDS_SVC_ID req_svc_id,
                               MDS_SVC_PVT_SUB_PART_VER svc_pvt_ver,NCSMDS_CHG change)  
{
  int i,j,k,ret_val=0;
  if(is_service_on_adest(mds_hdl,your_scv_id)==NCSCC_RC_SUCCESS)
    { 
      for(i=0;i<gl_tet_adest.svc_count;i++)
        {
          if(gl_tet_adest.svc[i].svc_id==your_scv_id)
            {
              for(j=0;j<gl_tet_adest.svc[i].subscr_count;j++)
                {
                  if(gl_tet_adest.svc[i].svcevt[j].svc_id==req_svc_id)
                    {
                      if((gl_tet_adest.svc[i].svcevt[j].event==change) &&
                         (gl_tet_adest.svc[i].svcevt[j].rem_svc_pvt_ver == svc_pvt_ver))
                        ret_val= 1; 
                      else
                        ret_val= 0; 
                    

                    }

                }
            }
        }
    }
  else
    {
      for(i=0;i<gl_vdest_indx;i++)
        {
          for(j=0;j<gl_tet_vdest[i].svc_count;j++)
            {
              if(gl_tet_vdest[i].svc[j].svc_id==your_scv_id)
                {
                  for(k=0;k<gl_tet_vdest[i].svc[j].subscr_count;k++)
                    {
                      if(gl_tet_vdest[i].svc[j].svcevt[k].svc_id==req_svc_id)
                        {
                          if((gl_tet_vdest[i].svc[j].svcevt[k].event==change) &&
                             (gl_tet_vdest[i].svc[j].svcevt[k].rem_svc_pvt_ver == svc_pvt_ver))
                            ret_val= 1;
                          else
                            ret_val= 0;

                        }
                    }

                }
            }
        }
    }
  return ret_val;
}
void tet_svc_subscr_VDEST(int choice)
{
  int FAIL=0;
  int id;
  int i;
  MDS_SVC_ID svcids[]={600,700};
  MDS_SVC_ID itself[]={500};
  MDS_SVC_ID svc_id_sixhd[]={600};
  gl_vdest_indx=0;
  
  printf("\n\n*************** tet_svc_subscr_VDEST %d ************\n",choice);
  tet_printf("\t\t\t tet_svc_subscr_VDEST");
  /*start up*/
  tet_printf("\tCreating a N-way VDEST with id =1001");
  if(create_vdest(NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,1001)!=
     NCSCC_RC_SUCCESS)
    {    tet_printf("Fail");FAIL=1;  }  
  
  tet_printf("\tInstalling the service 500,600,700 with CHASSIS scope");      
  for(id=500;id<=700;id=id+100)
    {  
      if( mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,id,1,
                              NCSMDS_SCOPE_INTRACHASSIS,true,false)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
    }
  switch(choice)
    {
    case 1: 
      /*install scope = subscribe scope*/
      tet_printf("Case 1 : \t500 Subscription to:600,700 where Install scope =\
Subscription scope");
      if( mds_service_subscribe(gl_tet_vdest[0].mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,2,
                                svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        {
          if(mds_service_cancel_subscription(gl_tet_vdest[0].mds_pwe1_hdl,
                                             500,2,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          tet_printf("Success");
        }  
      break;

    case 2:
      /*install scope > subscribe scope*/
      tet_printf("Case 2 : \t500 Subscription to:600,700 where Install scope >\
 Subscription scope (NODE)");
      if( mds_service_subscribe(gl_tet_vdest[0].mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRANODE,2,
                                svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        {
          if(mds_service_cancel_subscription(gl_tet_vdest[0].mds_pwe1_hdl,
                                             500,2,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          tet_printf("Success");
        }
      break;
    case 3:
      /*install scope < subscribe scope*/
      tet_printf("Case 3 : \tNot able to: 500 Subscription to:600,700 where\
 Install scope < Subscription scope(PWE)");
      if( mds_service_subscribe(gl_tet_vdest[0].mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_NONE,2,svcids)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");  
      break;
    case 4:
      tet_printf("Case 4 : \tNot able to subscribe again:500 Subscribing to\
 service 600");
      if( mds_service_subscribe(gl_tet_vdest[0].mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,2,
                                svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      if( mds_service_subscribe(gl_tet_vdest[0].mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,1,
                                svcids)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        {
          if(mds_service_cancel_subscription(gl_tet_vdest[0].mds_pwe1_hdl,500,
                                             2,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          tet_printf("Success");
        }  
      break;
    case 5:
      tet_printf("Case 5 : \tNot able to Cancel the subscription which doesn't\
 exist");
      if(mds_service_cancel_subscription(gl_tet_vdest[0].mds_pwe1_hdl,500,2,
                                         svcids)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");  
      break;
    case 6:
      tet_printf("Case 6 : \tA service subscribing for Itself");
      if( mds_service_subscribe(gl_tet_vdest[0].mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,1,
                                itself)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        {
          if(mds_service_cancel_subscription(gl_tet_vdest[0].mds_pwe1_hdl,500,
                                             1,itself)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }  
          tet_printf("Success");
        }  
      break;  
    case 8: 
      tet_printf("Case 8 : \tAble to subscribe when numer of services to be \
 subscribed is = 0");
      if( mds_service_subscribe(gl_tet_vdest[0].mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,0,
                                svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        {
          if(mds_service_cancel_subscription(gl_tet_vdest[0].mds_pwe1_hdl,
                                             500,0,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          tet_printf("Success");
        }  
      break;
    case 9: 
      tet_printf("Case 9 : \tNot able to subscribe when supplied services \
 array is NULL");
      if( mds_service_subscribe(gl_tet_vdest[0].mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,1,
                                NULL)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      tet_printf("Success");
          
      break;
    }  
  /*clean up*/
  tet_printf("\tUninstalling all the services on this VDEST");
  for(id=gl_tet_vdest[0].svc_count-1;id>=0;id--)
    mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,
                          gl_tet_vdest[0].svc[id].svc_id);
  switch(choice)
    {
    case 7:
      tet_printf("Case 7 : \tA Service which is NOT installed; trying to\
 subscribe for 600, 700");
      if( mds_service_subscribe(gl_tet_vdest[0].mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,2,
                                svcids)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");  
      break;

    case 10:
      tet_printf("Case 10: \t Cross check whether i_rem_svc_pvt_ver returned in service \
event callback, matches the service private sub-part version of the remote service (subscribee)");
      tet_printf("\tGetting an ADEST handle");
      if(adest_get_handle()!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      gl_tet_adest.svc_count=0; /*fix for blocking regression*/
      tet_printf("\tInstalling the services");
      if(mds_service_install(gl_tet_adest.mds_pwe1_hdl,500,2,
                             NCSMDS_SCOPE_INTRACHASSIS,true,false)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,600,1,
                             NCSMDS_SCOPE_INTRACHASSIS,true,false)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,700,2,
                             NCSMDS_SCOPE_INTRACHASSIS,true,false)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tSubscribing for the services");
      if( mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,2,
                                svcids)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      /* verifying the rem svc ver from 600 and 700*/
      tet_printf("\tChanging the role of vdest to active");
      if(vdest_change_role(1001,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tRetrieving the events");
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Retrieve Fail");
          FAIL=1;
        }


      tet_printf("\tVerifying for the versions for UP event");
      if( (tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,600,1,NCSMDS_UP))!=1 ||
          (tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,700,2,NCSMDS_UP)!=1))
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tChanging the role of vdest role to quiesced");
      if(vdest_change_role(1001,V_DEST_RL_QUIESCED)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tRetrieving the events");
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Retrieve Fail");
          FAIL=1;
        }
      if( (tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,600,1,NCSMDS_NO_ACTIVE))!=1 ||
          (tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,700,2,NCSMDS_NO_ACTIVE)!=1))
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tChanging the role of vdest role to standby");
      if(vdest_change_role(1001,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      printf(" \n Sleeping for 3 minutes \n");
      sleep(200);
      tet_printf("\tRetrieving the events");
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Retrieve Fail");
          FAIL=1;
        }
      if( (tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,600,0,NCSMDS_DOWN))!=1 ||
          (tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,700,0,NCSMDS_DOWN)!=1))
        {
          tet_printf("Fail");
          FAIL=1;
        }

      tet_printf("\tUninstalling 600 and 700");
      for(id=600;id<=700;id=id+100)
        {
          if(  mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,id)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
        }

      tet_printf("\tCancelling the subscription");
      if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,500,2,
                                         svcids)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(  mds_service_uninstall(gl_tet_adest.mds_pwe1_hdl,500)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }



      break;

    case 11:
      tet_printf("Case 11: \tWhen the Subscribee comes again with different sub-part version,\
 cross check these changes are properly returned in the service event callback");
      tet_printf("\tGetting an ADEST handle");
      if(adest_get_handle()!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      gl_tet_adest.svc_count=0; /*fix for blocking regression*/
      tet_printf("\tInstalling the services");
      if(mds_service_install(gl_tet_adest.mds_pwe1_hdl,500,2,
                             NCSMDS_SCOPE_INTRACHASSIS,true,false)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,600,1,
                             NCSMDS_SCOPE_INTRACHASSIS,true,false)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,700,2,
                             NCSMDS_SCOPE_INTRACHASSIS,true,false)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tSubscribing for the services");
      if( mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,2,
                                svcids)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      /* verifying the rem svc ver from 600 and 700*/
      tet_printf("\tChanging the role of vdest to active");
      if(vdest_change_role(1001,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tRetrieving the events");
      sleep(1);
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Retrieve Fail");
          FAIL=1;
        }


      tet_printf("\tVerifying for the versions for UP event");
      if( (tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,600,1,NCSMDS_UP))!=1 ||
          (tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,700,2,NCSMDS_UP)!=1))
        {
          tet_printf("Fail");
          FAIL=1;
        }

      tet_printf("\tUninstalling 600 and 700");
      for(id=600;id<=700;id=id+100)
        {
          if(  mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,id)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
        }
      if(mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,600,3,
                             NCSMDS_SCOPE_INTRACHASSIS,true,false)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,700,4,
                             NCSMDS_SCOPE_INTRACHASSIS,true,false)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      sleep(1);
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Retrieve Fail");
          FAIL=1;
        }


      tet_printf("\tVerifying for the versions for second  UP event");
      if( (tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,600,3,NCSMDS_NEW_ACTIVE))!=1 ||
          (tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,700,4,NCSMDS_NEW_ACTIVE)!=1))
        {
          tet_printf("Fail");
          FAIL=1;
        }

      tet_printf("\tCancelling the subscription");
      if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,500,2,
                                         svcids)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tUninstalling 600 and 700");
      for(id=600;id<=700;id=id+100)
        {
          if(  mds_service_uninstall(gl_tet_vdest[0].mds_pwe1_hdl,id)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
        }

      if(  mds_service_uninstall(gl_tet_adest.mds_pwe1_hdl,500)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }




      break;

    case 12:
      tet_printf("Case 13: \tIn the NO_ACTIVE event notification,\
 the remote service subpart version is set to the last active instance.s remote-service sub-part version");
      tet_printf("\tGetting an ADEST handle");
      if(adest_get_handle()!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      gl_tet_adest.svc_count=0; /*fix for blocking regression*/
      tet_printf("\tCreating a N-way VDEST with id =1001");
      if(create_vdest(NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,1002)!=
         NCSCC_RC_SUCCESS)
        {    tet_printf("Fail");FAIL=1;  }

      tet_printf("\tInstalling the services");
      if(mds_service_install(gl_tet_adest.mds_pwe1_hdl,500,2,
                             NCSMDS_SCOPE_INTRACHASSIS,true,false)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,600,1,
                             NCSMDS_SCOPE_INTRACHASSIS,true,false)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_install(gl_tet_vdest[1].mds_pwe1_hdl,600,2,
                             NCSMDS_SCOPE_INTRACHASSIS,true,false)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tSubscribing for the services");
      if( mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,1,
                                svc_id_sixhd)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      /* verifying the rem svc ver from 600 and 700*/
      tet_printf("\tChanging the role of vdest 1001 to active");
      if(vdest_change_role(1001,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tRetrieving the events");
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Retrieve Fail");
          FAIL=1;
        }


      tet_printf("\tVerifying for the versions for UP event");
      if( tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,600,1,NCSMDS_UP)!=1) 
   
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tChanging the role of vdest 1002 to active");
      if(vdest_change_role(1002,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      tet_printf("\tChanging the role of vdest 1001 to standby");
      if(vdest_change_role(1001,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tChanging the role of vdest 1002 to standby");
      if(vdest_change_role(1002,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      sleep(1);
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Retrieve Fail");
          FAIL=1;
        }
      tet_printf("\tVerifying for the versions for NO ACTIVE event");
      if( tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,600,2,NCSMDS_NO_ACTIVE)!=1)

        {
          tet_printf("Fail");
          FAIL=1;
        }


      tet_printf("\tUninstalling 600 ");
      for(i=0; i<=1;i++)
        {
          if(  mds_service_uninstall(gl_tet_vdest[i].mds_pwe1_hdl,600)!=NCSCC_RC_SUCCESS)

            {
              tet_printf("Fail");
              FAIL=1;
            }
        } 


      tet_printf("\tCancelling the subscription");
      if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,500,1,
                                         svc_id_sixhd)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(  mds_service_uninstall(gl_tet_adest.mds_pwe1_hdl,500)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tDestroying the VDEST 1002");
      if(destroy_vdest(1002)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      break;
    }

 
  tet_printf("\tDestroying the VDEST");
  if(destroy_vdest(1001)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n***************************************************\n");
}

void tet_adest_cancel_thread()
{
  MDS_SVC_ID svcids[]={600,700};
  printf(" Inside Thread");
  if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,500,2,
                                     svcids)!=NCSCC_RC_SUCCESS)
    {
      perror("Cancel Failed");
    }
}
void tet_adest_retrieve_thread()
{
  printf(" Inside Thread");
  if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,
                          SA_DISPATCH_ALL)==NCSCC_RC_FAILURE)
    {
      perror("Retrieve Failed");
    }
}
void tet_svc_subscr_ADEST(int choice)
{
  int FAIL=0;
  int id;
  MDS_SVC_ID svcids[]={600,700};
  MDS_SVC_ID itself[]={500};
  MDS_SVC_ID one[]={600};
  MDS_SVC_ID two[]={700};
  
  printf("\n\n*********** tet_svc_subscr_ADEST %d ****************\n",choice);
  tet_printf("\t\t\t tet_svc_subscr_ADEST");
  /*start up*/
  tet_printf("\tGetting an ADEST handle");
  if(adest_get_handle()!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  
  gl_tet_adest.svc_count=0; /*fix for blocking regression*/
  
  tet_printf("\tInstalling the services 500,600,700 with CHASSIS scope");
  for(id=500;id<=700;id=id+100)
    {  
      if( mds_service_install(gl_tet_adest.mds_pwe1_hdl,id,1,
                              NCSMDS_SCOPE_INTRACHASSIS,true,false)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
    }
  switch(choice)
    {  
      /*install scope = subscribe scope*/
    case 1:
      tet_printf("Case 1 : \t500 Subscription to:600,700 where Install scope =\
 Subscription scope");
      if( mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,2,
                                svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        {
          tet_printf(" Retrieve only ONE event");
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,
                                  SA_DISPATCH_ONE)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Retrieve ONE FAIL");
              FAIL=1;
            }
          else
            tet_printf("Success");

          tet_printf(" Retrieve ALL event");
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Retrieve ALL FAIL");
              FAIL=1;
            }
          else
            tet_printf("Success");

          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,500,2,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          tet_printf("Success");
        } 
      break;
    case 2:  
      /*install scope > subscribe scope*/
      tet_printf("Case 2 : \t500 Subscription to:600,700 where Install scope >\
 Subscription scope (NODE)");
      if( mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRANODE,2,
                                svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        {
          tet_printf("Retrieve with  Invalid NULL/0 PWE Handle");
          if(mds_service_retrieve((MDS_HDL)(long)NULL,500,SA_DISPATCH_ALL)==NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail"); 
              FAIL=1;
            }
          if(mds_service_retrieve(0,500,SA_DISPATCH_ALL)==NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          tet_printf("Not able to Retrieve Events for a Service which is not \
 installed");
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,100,
                                  SA_DISPATCH_ALL)==NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,SA_DISPATCH_ALL);

          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,500,2,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          tet_printf("Success");
        }
      break;
    case 3:
      /*install scope < subscribe scope*/
      tet_printf("Case 3 : \tNot able to: 500 Subscription to:600,700 where\
 Install scope < Subscription scope(PWE)");
      if( mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_NONE,2,svcids)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else  
        tet_printf("Success");
      break;
    case 4: 
      tet_printf("Case 4 : \tNot able to subscribe again:500 Subscribing to\
 service 600");
      if( mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,2,
                                svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      if( mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,1,
                                svcids)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        {
          mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,SA_DISPATCH_ALL);
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,500,2,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          tet_printf("Success");
        } 
      break;
    case 5:  
      tet_printf("Case 5 : \tNot able to Cancel the subscription which doesn't\
 exist");
      if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,500,2,
                                         svcids)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");  
      break;
    case 6:  
      tet_printf("Case 6 : \tA service subscribing for Itself");
      if( mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,1,
                                itself)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        {
          tet_printf("Not able to Retrieve with Invalid Dispatch Flag");
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,
                                  4)==NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,SA_DISPATCH_ALL);
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,500,1,
                                             itself)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }  
          tet_printf("Success");
        }
      break;  
      /*install scope = subscribe scope*/
    case 8:
      tet_printf("Case 8 : \t500 Subscription to:600,700 Cancel this \
 Subscription in a seperate thread");
      if( mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,2,
                                svcids)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      else
        {
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,
                                  SA_DISPATCH_ALL)==NCSCC_RC_FAILURE)
            {
              tet_printf("Retrieve Fail");
              FAIL=1;
            }
          tet_printf("Cancel in a seperate thread");
          /*Cancel thread*/
          if(tet_create_task((NCS_OS_CB)tet_adest_cancel_thread,
                             gl_tet_adest.svc[0].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          sleep(2);
          fflush(stdout);
          /*Now Release the Cancel Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[0].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            {
              printf("\tTASK is released\n");
            }
          if(gl_tet_adest.svc[0].subscr_count)
            {
              tet_printf("Cancel Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");

        }
      break;
    case 10:
      tet_printf("Case 10 : \t500 Subscription to:600,700 Cancel this \
 Subscription in a seperate thread");
      if( mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,2,
                                svcids)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      else
        {
          tet_printf("Retrieve in a seperate thread");
          /*Retrieve thread*/
          if(tet_create_task((NCS_OS_CB)tet_adest_retrieve_thread,
                             gl_tet_adest.svc[0].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          sleep(2);
          fflush(stdout);
          /*Now Release the Retrieve Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[0].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            {
              printf("\tTASK is released\n");
            }
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,500,2,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          tet_printf("Success");

        }
      break;

    case 9:
      tet_printf("Case 9 : \t500 Subscription to:600,700 in two seperate\
 Subscription calls but Cancels both in a single cancellation call");
      if( mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,1,
                                one)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if( mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,1,
                                two)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      else
        {
          mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,SA_DISPATCH_ONE);
          mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,SA_DISPATCH_ONE);
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,
                                  SA_DISPATCH_ONE)==NCSCC_RC_SUCCESS)
            {
              tet_printf("Retrieve Fail");
              FAIL=1;
            }
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,500,2,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          tet_printf("Success");
        }
      break;

    case 11:
      tet_printf("\tAdest: Cross check whether i_rem_svc_pvt_ver\
                      returned in service event callback, matches the service private sub-part\
                      version of the remote service (subscribee)");
      mds_service_uninstall(gl_tet_adest.mds_pwe1_hdl,700);
      tet_printf("\tInstalling serive 700");
      if(mds_service_install(gl_tet_adest.mds_pwe1_hdl,700,2,
                             NCSMDS_SCOPE_INTRACHASSIS,true,false)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail ");
          FAIL=1;
        }
      tet_printf("\tSubscribing for the services");
      if( mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRACHASSIS,2,
                                svcids)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail ");
          FAIL=1;
        }
      sleep(1);
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Retrieve Fail");
          FAIL=1;
        }
     
      tet_printf("\tVerifying the svc pvt version for UP events");
      if( (tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,600,1,NCSMDS_UP))!=1 ||
          (tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,700,2,NCSMDS_UP)!=1))
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\tUninstalling 600 and 700");
      for(id=600;id<=700;id=id+100)
        {
          if(  mds_service_uninstall(gl_tet_adest.mds_pwe1_hdl,id)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
        }
      sleep(1);
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,500,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Retrieve Fail");
          FAIL=1;
        }
      tet_printf("\tVerifying the svc pvt version for DOWN events");

      if(tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,600,1,NCSMDS_DOWN)!=1 ||
         tet_verify_version(gl_tet_adest.mds_pwe1_hdl,500,700,2,NCSMDS_DOWN)!=1)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      tet_printf("\tCancelling the subscription");
      if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,500,2,
                                         svcids)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      break;
    }

  /*clean up*/
  tet_printf("\tUninstalling all the services on this ADESt");
  for(id=gl_tet_adest.svc_count-1;id>=0;id--)
    mds_service_uninstall(gl_tet_adest.mds_pwe1_hdl,
                          gl_tet_adest.svc[id].svc_id);
  switch(choice)
    {
    case 7:
      tet_printf("Case 7 : \tA Service which is NOT installed; trying to \
subscribe for 600, 700");
      if( mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,500,
                                NCSMDS_SCOPE_INTRANODE,2,
                                svcids)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");  
      break; 
    } 
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n*************************************************************\n");
}
uint32_t tet_initialise_setup(bool fail_no_active_sends)
{
  int i,FAIL=0;
  gl_vdest_indx=0;
  tet_printf("\tGet an ADEST handle,Create PWE=2 on ADEST,Install EXTMIN and INTMIN svc\
 on ADEST,Install INTMIN,EXTMIN services on ADEST's PWE=2,\n\tCreate VDEST 100\
 and VDEST 200,Change the role of VDEST 200 to ACTIVE, \n\tInstall EXTMIN \
 service on VDEST 100,Install INTMIN, EXTMIN services on VDEST 200 ");
  adest_get_handle();
  
  if(create_vdest(NCS_VDEST_TYPE_MxN,100)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  if(create_vdest(NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,
                  200)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  
  if(vdest_change_role(gl_tet_vdest[1].vdest,
                       V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  
  if(create_pwe_on_adest(gl_tet_adest.mds_adest_hdl,2)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  for(i=NCSMDS_SVC_ID_INTERNAL_MIN;i<=NCSMDS_SVC_ID_EXTERNAL_MIN;
      i=i+(NCSMDS_SVC_ID_EXTERNAL_MIN-NCSMDS_SVC_ID_INTERNAL_MIN))
    if(mds_service_install(gl_tet_adest.mds_pwe1_hdl,i,3,NCSMDS_SCOPE_NONE,
                           true,fail_no_active_sends)!=NCSCC_RC_SUCCESS)
      {
        tet_printf("Fail");
        FAIL=1;
      }
  

  
  for(i=NCSMDS_SVC_ID_INTERNAL_MIN;i<=NCSMDS_SVC_ID_EXTERNAL_MIN;
      i=i+(NCSMDS_SVC_ID_EXTERNAL_MIN-NCSMDS_SVC_ID_INTERNAL_MIN))
    if(mds_service_install(gl_tet_adest.pwe[0].mds_pwe_hdl,i,3,NCSMDS_SCOPE_NONE,
                           true,fail_no_active_sends)!=NCSCC_RC_SUCCESS)
      {    
        tet_printf("Fail");
        FAIL=1; 
      }
  
  if(mds_service_install(gl_tet_vdest[0].mds_pwe1_hdl,
                         NCSMDS_SVC_ID_EXTERNAL_MIN,2,NCSMDS_SCOPE_NONE,
                         true,fail_no_active_sends)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  
  if(mds_service_install(gl_tet_vdest[1].mds_pwe1_hdl,
                         NCSMDS_SVC_ID_INTERNAL_MIN,1,NCSMDS_SCOPE_NONE,
                         true,fail_no_active_sends)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  if(mds_service_install(gl_tet_vdest[1].mds_pwe1_hdl,
                         NCSMDS_SVC_ID_EXTERNAL_MIN,1,NCSMDS_SCOPE_NONE,
                         true,fail_no_active_sends)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  
  return FAIL;
  
}
uint32_t tet_cleanup_setup()
{
  int i,id,FAIL=0;
  tet_printf("\tUninstalling the services on both VDESTs and ADEST");
  printf("\n\t UnInstalling the Services on both the VDESTs\n");
  for(i=gl_vdest_indx-1;i>=0;i--)
    {  
      for(id=gl_tet_vdest[i].svc_count-1;id>=0;id--)
        {
          mds_service_retrieve(gl_tet_vdest[i].mds_pwe1_hdl,
                               gl_tet_vdest[i].svc[id].svc_id,
                               SA_DISPATCH_ALL);
          if(mds_service_uninstall(gl_tet_vdest[i].mds_pwe1_hdl,
                                   gl_tet_vdest[i].svc[id].svc_id)
             !=NCSCC_RC_SUCCESS)
            {  
              tet_printf("Vdest Svc Uninstall Fail");  
            }  
        }
    }
  fflush(stdout);
  printf("\nDestroying the VDESTS"); 
  tet_printf("\tDestroying both the VDESTs and PWE=2 on ADEST");
  for(i=gl_vdest_indx-1;i>=0;i--)
    {
      /*changing its role to Standby*/
      if(vdest_change_role(gl_tet_vdest[i].vdest,V_DEST_RL_STANDBY)
         !=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
        }
      /*destroy all VDESTs */
      if(destroy_vdest(gl_tet_vdest[i].vdest)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("VDEST DESTROY Fail");
          FAIL=1; 
        }
    }
#if 1
  /*Uninstalling the 2000/INTMIN service on PWE1 ADEST*/
  for(i=NCSMDS_SVC_ID_EXTERNAL_MIN;i>=NCSMDS_SVC_ID_INTERNAL_MIN;i=i-NCSMDS_SVC_ID_INTERNAL_MIN)
    {
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,i,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Adest Svc  Retrieve Fail");
          FAIL=1;
        }
      if(mds_service_uninstall(gl_tet_adest.mds_pwe1_hdl,i)!=
         NCSCC_RC_SUCCESS)
        {
          tet_printf("Adest Svc  Uninstall Fail");
          FAIL=1;
        }
    }
  /*Uninstalling the 2000/INTMIN on PWE2 of ADEST*/
  printf("\n\t ADEST : PWE 2 : Uninstalling Services 2000/INTMIN\n"); 
  for(i=NCSMDS_SVC_ID_EXTERNAL_MIN;i>=NCSMDS_SVC_ID_INTERNAL_MIN;i=i-NCSMDS_SVC_ID_INTERNAL_MIN)
    {
      if(mds_service_retrieve(gl_tet_adest.pwe[0].mds_pwe_hdl,i,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Adest Svc PWE 2 Retrieve Fail");
          FAIL=1; 
        }  
      if(mds_service_uninstall(gl_tet_adest.pwe[0].mds_pwe_hdl,i)!=
         NCSCC_RC_SUCCESS)
        {    
          tet_printf("Adest Svc PWE 2 Uninstall Fail");
          FAIL=1; 
        }
    }
#endif
  printf("\nADEST PWE2 Destroyed"); 
  if(destroy_pwe_on_adest(gl_tet_adest.pwe[0].mds_pwe_hdl)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("ADEST PWE2 Destroy Fail");
      FAIL=1; 
    }
  
  return FAIL;
}
/*----------------SIMPLE SEND TEST CASES------------------------------------*/
void tet_just_send_tp(int choice)
{
  int FAIL=0;
  int id;
  MDS_SVC_ID svcids[]={NCSMDS_SVC_ID_EXTERNAL_MIN};
  MDS_SVC_ID svc_ids[]={NCSMDS_SVC_ID_INTERNAL_MIN};
  gl_vdest_indx=0;
  
  char tmp[]=" Hi Receiver ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);

  printf("\n\n************* tet_just_send_tp_ADEST %d ************\n",choice);
  tet_printf("\t\t\t tet_just_send_tp");
  /*start up*/
  if(tet_initialise_setup(false))
    {
      printf("\n\t\t Setup Initialisation has Failed \n");
      FAIL=1;
    }
  else
    {
      /*--------------------------------------------------------------------*/
      if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN,
                               NCSMDS_SCOPE_NONE,1,svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL)
         !=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      switch(choice)
        {

        case 1:
          tet_printf("Case 1 : \tNow send Low, Medium, High and Very High\
 Priority messages to Svc EXTMIN on Active Vdest in sequence");
          if(mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_LOW,
                           mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("LOW Success");
          if(mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_MEDIUM,
                           mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("MEDIUM Success");
          if(mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_HIGH,
                           mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("HIGH Success");
          if(mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_VERY_HIGH,
                           mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("VERY HIGH Success");
          break;
        case 2:
          tet_printf("Case 2 : \tNot able to send a message to Svc EXTMIN with\
 Invalid pwe handle");
          if(mds_just_send(gl_tet_adest.pwe[0].mds_pwe_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_LOW,
                           mesg)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 3:
          tet_printf("Case 3 : \tNot able to send a message to Svc EXTMIN with\
 NULL pwe handle");
          if(mds_just_send((MDS_HDL)(long)NULL,NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,gl_tet_vdest[1].vdest,
                           MDS_SEND_PRIORITY_LOW,mesg)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 4:
          tet_printf("Case 4 : \tNot able to send a message to Svc EXTMIN with\
 Wrong DEST");
          if(mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,0,
                           MDS_SEND_PRIORITY_LOW,mesg)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 5:
          tet_printf("Case 5 : \tNot able to send a message to service which \
 is Not installed");
          if(mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,1800,
                           gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_LOW,
                           mesg)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 6:
          tet_printf("Case 6 : \tAble to send a messages 1000 times  to Svc \
 2000 on Active Vdest");
          for(id=1;id<=1000;id++)
            if(mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                             NCSMDS_SVC_ID_EXTERNAL_MIN,
                             NCSMDS_SVC_ID_EXTERNAL_MIN,
                             gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_LOW,
                             mesg)!=NCSCC_RC_SUCCESS)
              {      
                tet_printf("Fail");
                FAIL=1;
                break; 
              }
          if(id==1001)
            {
              printf("\n\n\t\t ALL THE MESSAGES GOT DELIVERED\n\n");
              sleep(1);
              tet_printf("Success");
            }
          break;
        case 7:
          tet_printf("Case 7 : \tSend a message to unsubscribed Svc INTMIN on\
 Active Vdest:Implicit/Explicit Combination ");
          if(mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_INTERNAL_MIN,
                           gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_LOW,
                           mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svc_ids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          if(mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_INTERNAL_MIN,
                           gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_LOW,
                           mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svc_ids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          break;
        case 8:
          tet_printf("Case 8 : \tNot able to send a message to Svc EXTMIN with\
 Improper Priority");
          if(mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           gl_tet_vdest[1].vdest,5,mesg)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 9:
          tet_printf("Case 9 : \tNot able to send aNULL message to Svc EXTMIN\
 on Active Vdest");
          if(mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_LOW,
                           NULL)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 10:
          tet_printf("Case 10 : \tNow send a message( > \
 MDTM_NORMAL_MSG_FRAG_SIZE) to Svc EXTMIN");
          memset(mesg->send_data,'S',2*1400+2 );
          mesg->send_len=2*1400+2;
          if(mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_LOW,
                           mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 11:
          tet_printf("Case 11 : \tNot able to Send a message with Invalid Send\
 type");
          svc_to_mds_info.i_mds_hdl=gl_tet_adest.mds_pwe1_hdl;
          svc_to_mds_info.i_svc_id=NCSMDS_SVC_ID_EXTERNAL_MIN;
          svc_to_mds_info.i_op=MDS_SEND;
          svc_to_mds_info.info.svc_send.i_msg=mesg;
          svc_to_mds_info.info.svc_send.i_to_svc=NCSMDS_SVC_ID_EXTERNAL_MIN;
          svc_to_mds_info.info.svc_send.i_priority=MDS_SEND_PRIORITY_LOW;
          svc_to_mds_info.info.svc_send.i_sendtype=12; /*wrong send type*/
          svc_to_mds_info.info.svc_send.info.snd.i_to_dest=
            gl_tet_vdest[1].vdest;
          if(ncsmds_api(&svc_to_mds_info)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 12:
          tet_printf("Case 12: \tWhile Await Active timer is On: send a \
 message to Svc EXTMIN Vdest=200");
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          sleep(2);
          if(mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_LOW,
                           mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          break;
        case 13:
          tet_printf("Case 13: \tSend a message to Svc EXTMIN on QUIESCED \
 Vdest=200");
          if(vdest_change_role(200,V_DEST_RL_QUIESCED)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          sleep(2);
          if(mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_HIGH,
                           mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          break;
        case 14:
          tet_printf("Case 14 :\tCopy Callback returning Failure: Send Fails");
          /*Returning Failure at Copy CallBack*/
          gl_COPY_CB_FAIL=1;
          if(mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_LOW,
                           mesg)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          /*Resetting it*/
          gl_COPY_CB_FAIL=0;
          break;
        }
      /*clean up*/
      tet_printf("\tCancelling the subscription");
      if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                         NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                         svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      /*--------------------------------------------------------------------*/
      /*clean up*/
      if(tet_cleanup_setup())
        {
          perror("\n\t\t Setup Clean Up has Failed \n");
          getchar();
          FAIL=1;
        }
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n***********************************************************\n");
  fflush(stdout);
}
void tet_send_ack_tp(int choice)
{
  int FAIL=0;
  int id;
  MDS_SVC_ID svcids[]={NCSMDS_SVC_ID_EXTERNAL_MIN};
  MDS_SVC_ID svc_ids[]={NCSMDS_SVC_ID_INTERNAL_MIN};
  
  char tmp[]=" Hi Receiver! Are you there? ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);
  
  printf("\n\n************** tet_send_ack_tp_ADEST %d *************\n",choice);
  tet_printf("\t\t\t tet_send_ack_tp");
  /*start up*/
  if(tet_initialise_setup(false))
    {
      printf("\n\t\t Setup Initialisation has Failed \n");
      FAIL=1;
    }
  else
    {
      /*--------SEND ACKNOWLEDGE-------------------------------------------*/
      if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN,
                               NCSMDS_SCOPE_NONE,1,svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL)
         !=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      
      switch(choice)
        {
        case 1:
          tet_printf("Case 1 : \tNow send LOW,MEDIUM,HIGH and VERY HIGH \
 Priority messages with ACK to Svc on ACtive VDEST in sequence");
          if(mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              gl_tet_vdest[1].vdest,100,MDS_SEND_PRIORITY_LOW,
                              mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("LOW Success");
          if(mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              gl_tet_vdest[1].vdest,100,
                              MDS_SEND_PRIORITY_MEDIUM,mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("MEDIUM Success");
          if(mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              gl_tet_vdest[1].vdest,100,MDS_SEND_PRIORITY_HIGH,
                              mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("HIGH Success");
          if(mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              gl_tet_vdest[1].vdest,100,
                              MDS_SEND_PRIORITY_VERY_HIGH,
                              mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("VERY HIGH Success");
          break;
        case 2:
          tet_printf("Case 2 : \tNot able to send a message with ACK to 1700\
 with Invalid pwe handle");
          if(mds_send_get_ack(gl_tet_adest.pwe[0].mds_pwe_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              gl_tet_vdest[1].vdest,100,MDS_SEND_PRIORITY_LOW,
                              mesg)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 3:
          tet_printf("Case 3 : \tNot able to send a message with ACK to 1700\
 with NULL pwe handle");
          if(mds_send_get_ack((MDS_HDL)(long)NULL,NCSMDS_SVC_ID_EXTERNAL_MIN,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              gl_tet_vdest[1].vdest,100,
                              MDS_SEND_PRIORITY_LOW,mesg)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 4:
          tet_printf("Case 4 : \tNot able to send a message with ACK to 1700\
 with Wrong DEST");
          if(mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,0,100,
                              MDS_SEND_PRIORITY_LOW,mesg)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 5:
          tet_printf("Case 5 : \tNot able to send a message with ACK to\
 service which is Not installed");
          if(mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,1800,
                              gl_tet_vdest[1].vdest,100,MDS_SEND_PRIORITY_LOW,
                              mesg)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 6:
          tet_printf("Case 6 : \tNot able to send a message with ACK to OAC\
 service on this DEST");
          if(mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                              gl_tet_vdest[1].vdest,100,MDS_SEND_PRIORITY_LOW,
                              mesg)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 7:
          tet_printf("Case 7 : \tAble to send a message with ACK 1000 times \
 to service 1700");
          for(id=1;id<=1000;id++)
            if(mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,
                                gl_tet_vdest[1].vdest,100,
                                MDS_SEND_PRIORITY_LOW,mesg)!=NCSCC_RC_SUCCESS)
              {      tet_printf("Fail");FAIL=1;break;    }
          if(id==1001)
            {
              printf("\n\n\t\t ALL THE MESSAGES GOT DELIVERED\n\n");
              sleep(1);
              tet_printf("Success");
            }
          break;
        case 8:
          tet_printf("Case 8 : \tSend a message with ACK to unsubscribed \
 service 1600");
          if(mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              NCSMDS_SVC_ID_INTERNAL_MIN,
                              gl_tet_vdest[1].vdest,100,MDS_SEND_PRIORITY_LOW,
                              mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svc_ids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              NCSMDS_SVC_ID_INTERNAL_MIN,
                              gl_tet_vdest[1].vdest,100,MDS_SEND_PRIORITY_LOW,
                              mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svc_ids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          break;
        case 9:
          tet_printf("Case 9 : \tNot able to send_ack a message with Improper\
 Priority to 1700");
          if(mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              gl_tet_vdest[1].vdest,100,0,
                              mesg)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 10:
          tet_printf("Case 10 : \tNot able to send a NULL message with ACK to\
 1700");
          if(mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              gl_tet_vdest[1].vdest,100,MDS_SEND_PRIORITY_LOW,
                              NULL)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 11:
          tet_printf("Case 11 : \tNow send a message( > \
                  MDTM_NORMAL_MSG_FRAG_SIZE) to 1700");
          memset(mesg->send_data,'S',2*1400+2 );
          mesg->send_len=2*1400+2;
          if(mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              gl_tet_vdest[1].vdest,100,MDS_SEND_PRIORITY_LOW,
                              mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 12:
          tet_printf("Case 12: \tWhile Await Active timer is On: send_ack a \
 message to Svc EXTMIN Vdest=200");
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          sleep(2);
          if(mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              gl_tet_vdest[1].vdest,1000,
                              MDS_SEND_PRIORITY_LOW,
                              mesg)!=NCSCC_RC_REQ_TIMOUT)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 13:
          tet_printf("Case 13: \tSend_ack a message to Svc EXTMIN on QUIESCED \
 Vdest=200");
          if(vdest_change_role(200,V_DEST_RL_QUIESCED)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          sleep(2);
          if(mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              gl_tet_vdest[1].vdest,100,MDS_SEND_PRIORITY_LOW,
                              mesg)!=NCSCC_RC_REQ_TIMOUT)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          break;
        }
      /*------------------------------------------------------------------*/
      /*clean up*/
      tet_printf("\tCancelling the subscription");
      if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                         NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                         svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      /*--------------------------------------------------------------------*/
      /*clean up*/
      if(tet_cleanup_setup())
        {
          perror("\n\t\t Setup Clean Up has Failed \n");
          getchar();
          FAIL=1;
        }
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n***********************************************************\n");
  fflush(stdout);
}
void tet_query_pwe_tp(int choice)
{
  int FAIL=0;
  MDS_SVC_ID svcids[]={NCSMDS_SVC_ID_EXTERNAL_MIN};
  printf("\n\n************************* tet_query_pwe_tp ****************\n");
  tet_printf("\t\t\ttet_query_pwe_tp : %d",choice);
  /*Start up*/
  if(tet_initialise_setup(false))
    {
      tet_printf("\n\t\t Setup Initialisation has Failed \n");
      FAIL=1;
    }
  else
    {
      /*---------------------------------------------------------------------*/
      switch(choice)
        {
        case 1:
          tet_printf("Get the details of all the Services on this core");
          if(mds_service_query_for_pwe(gl_tet_adest.mds_pwe1_hdl,
                                       NCSMDS_SVC_ID_EXTERNAL_MIN)
             !=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_service_query_for_pwe(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                       NCSMDS_SVC_ID_EXTERNAL_MIN)
             !=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_service_query_for_pwe(gl_tet_vdest[0].mds_pwe1_hdl,
                                       NCSMDS_SVC_ID_EXTERNAL_MIN)
             !=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_service_query_for_pwe(gl_tet_vdest[1].mds_pwe1_hdl,
                                       NCSMDS_SVC_ID_EXTERNAL_MIN)
             !=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_service_query_for_pwe(gl_tet_vdest[1].mds_pwe1_hdl,
                                       NCSMDS_SVC_ID_INTERNAL_MIN)
             !=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
      
          mds_service_redundant_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                          NCSMDS_SVC_ID_EXTERNAL_MIN,
                                          NCSMDS_SCOPE_NONE,1,
                                          svcids);
          mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL);
      
          /*mds_query_vdest_for_anchor(gl_tet_adest.mds_pwe1_hdl,2000,100,
            2000,0);*/
      
          mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                          NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                          svcids);
          fflush(stdout);
          break;
        case 2:
          tet_printf("Not able to Query PWE with Invalid PWE Handle");
          if(mds_service_query_for_pwe((MDS_HDL)(long)NULL,NCSMDS_SVC_ID_EXTERNAL_MIN)
             ==NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_service_query_for_pwe(0,NCSMDS_SVC_ID_EXTERNAL_MIN)
             ==NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_service_query_for_pwe(gl_tet_vdest[0].mds_vdest_hdl,
                                       NCSMDS_SVC_ID_EXTERNAL_MIN)
             ==NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          break;
        case 3:
          mds_query_vdest_for_anchor(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,100,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,0);

          break;
        }      
      /*---------------------------------------------------------------------*/
      /*clean up*/
      if(tet_cleanup_setup())
        {
          perror("\n\t\t Setup Clean Up has Failed \n");
          FAIL=1;
        }
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n******************************************************\n");
  fflush(stdout);
  
}
void tet_adest_rcvr_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  
  char tmp[]=" Hi Sender! My Name is RECEIVER ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));   
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);
  
  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_adest_sel_obj_found(3)) )
    {
      printf("\n\t\t Got the message: trying to retreive it\n");fflush(stdout);
      mds_service_retrieve(gl_tet_adest.pwe[0].mds_pwe_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/ 
      if(gl_rcvdmsginfo.rsp_reqd)
        {
          if(mds_send_response(gl_tet_adest.pwe[0].mds_pwe_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN,mesg)
             !=NCSCC_RC_SUCCESS)
            {      printf("Response Fail");FAIL=1;    }
          else
            printf("Response Success");
          gl_rcvdmsginfo.rsp_reqd=0;
        }
    }
  fflush(stdout);  
}
void tet_adest_rcvr_svc_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  
  char tmp[]=" Hi Sender! My Name is RECEIVER ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));   
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);
  
  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_adest_sel_obj_found(0)) )
    {
      printf("\n\t\t Got the message: trying to retreive it\n");fflush(stdout);
      sleep(1);
      mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/ 
      if(gl_rcvdmsginfo.rsp_reqd)
        {
          if(mds_send_response(gl_tet_adest.mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN,
                               mesg)!=NCSCC_RC_SUCCESS)
            {      printf("Response Fail");FAIL=1;    }
          else
            printf("Response Success");
          gl_rcvdmsginfo.rsp_reqd=0;
        }
    }
  fflush(stdout);  
}
void tet_vdest_rcvr_resp_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  char tmp[]=" Hi Sender! My Name is RECEIVER ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);    
  
  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_vdest_sel_obj_found(1,1)) )
    {
      mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/ 
      if(gl_rcvdmsginfo.rsp_reqd)
        {
          if(mds_send_response(gl_tet_vdest[1].mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN,
                               mesg)!=NCSCC_RC_SUCCESS)
            {      printf("Response Fail");FAIL=1;    }
          else
            printf("Response Success");
        }
    }
  fflush(stdout);  
}
void tet_send_response_tp(int choice)
{
  int FAIL=0;
  MDS_SVC_ID svcids[]={NCSMDS_SVC_ID_EXTERNAL_MIN};
  void tet_adest_rcvr_thread();
  
  char tmp[]=" Hi Receiver! What is your Name? ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));   
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);
  
  printf("\n\n*********** tet_send_response_tp %d ****************\n",choice);
  tet_printf("\t\t\ttet_send_response_tp");
  /*Start up*/
  if(tet_initialise_setup(false))
    {
      tet_printf("\n\t\t Setup Initialisation has Failed \n");
      FAIL=1;
    }

  else
    {
      /*--------------------------------------------------------------------*/
      switch(choice)
        {
        case 1:
          if(mds_service_subscribe(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          tet_printf("\tCase 1 : Svc INTMIN on PWE=2 of ADEST sends a LOW \
 Priority message to Svc NCSMDS_SVC_ID_EXTERNAL_MIN and expects a Response");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_adest_rcvr_thread,
                             gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/  
          if(mds_send_get_response(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   gl_tet_adest.adest,500,
                                   MDS_SEND_PRIORITY_LOW,
                                   mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            {
              printf("\tTASK is released\n");
            }
          fflush(stdout);                
          if(mds_service_cancel_subscription(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 2:
          if(mds_service_subscribe(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          tet_printf("\tCase 2 : Svc INTMIN on PWE=2 of ADEST sends a MEDIUM \
 Priority message to Svc EXTMIN and expects a Response");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_adest_rcvr_thread,
                             gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/  
          if(mds_send_get_response(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   gl_tet_adest.adest,500,
                                   MDS_SEND_PRIORITY_MEDIUM,
                                   mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            {
              printf("\tTASK is released\n");
            }
          fflush(stdout);                
          if(mds_service_cancel_subscription(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 3:
          if(mds_service_subscribe(gl_tet_vdest[1].mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)==NCSCC_RC_FAILURE)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          tet_printf("\tCase 3 : Svc INTMIN on Active VEST=200 sends a HIGH \
 Priority message to Svc EXTERNAL_MIN on ADEST and expects a Response");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_adest_rcvr_svc_thread,
                             gl_tet_adest.svc[0].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/  
          if(mds_send_get_response(gl_tet_vdest[1].mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   gl_tet_adest.adest,500,
                                   MDS_SEND_PRIORITY_HIGH,
                                   mesg)==NCSCC_RC_FAILURE)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[0].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            {
              printf("\tTASK is released\n");
            }
          fflush(stdout);
          if(mds_service_cancel_subscription(gl_tet_vdest[1].mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,
                                             1,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 4:
          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          tet_printf("\tCase 4 :Svc EXTMIN of ADEST sends a VERYHIGH Priority \
 message to Svc EXTMIN on Active Vdest=200 and expects a Response");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_vdest_rcvr_resp_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/  
          if(mds_send_get_response(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   gl_tet_vdest[1].vdest,500,
                                   MDS_SEND_PRIORITY_VERY_HIGH,
                                   mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            {
              printf("\tTASK is released\n");
            }
          fflush(stdout); 
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 5:
          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          tet_printf("\tCase 5 :While Await ActiveTimer ON:SvcEXTMIN of ADEST \
sends a message to Svc EXTMIN on Active Vdest=200 and Times out");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_vdest_rcvr_resp_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          sleep(2);
          /*Sender*/  
          if(mds_send_get_response(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   gl_tet_vdest[1].vdest,500,
                                   MDS_SEND_PRIORITY_VERY_HIGH,
                                   mesg)!=NCSCC_RC_REQ_TIMOUT)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            {
              printf("\tTASK is released\n");
            }
          fflush(stdout); 
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 6:
          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          tet_printf("\tCase 6 :Svc EXTMIN of ADEST sends a VERYHIGH Priority \
 message to Svc EXTMIN on QUIESCED Vdest=200 and Times out");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_vdest_rcvr_resp_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          if(vdest_change_role(200,V_DEST_RL_QUIESCED)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          sleep(2);         
          /*Sender*/  
          if(mds_send_get_response(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   gl_tet_vdest[1].vdest,500,
                                   MDS_SEND_PRIORITY_VERY_HIGH,
                                   mesg)!=NCSCC_RC_REQ_TIMOUT)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            {
              printf("\tTASK is released\n");
            }
          fflush(stdout);
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;                  
        case 7: 
          tet_printf("\tCase 7 :Implicit Subscription:Svc INTMIN on PWE=2 of \
 ADEST sends a LOW Priority message to Svc EXTMIN and expects a Response");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_adest_rcvr_thread,
                             gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/  
          if(mds_send_get_response(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   gl_tet_adest.adest,500,
                                   MDS_SEND_PRIORITY_LOW,
                                   mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            {
              printf("\tTASK is released\n");
            }
          fflush(stdout);
          /*Implicit explicit combination*/
          if(mds_service_subscribe(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_adest_rcvr_thread,
                             gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/  
          if(mds_send_get_response(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   gl_tet_adest.adest,500,
                                   MDS_SEND_PRIORITY_LOW,
                                   mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            {
              printf("\tTASK is released\n");
            }
          fflush(stdout);

          if(mds_service_cancel_subscription(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
          
        case 8: 
          tet_printf("\tCase 8 :Not able to send a message to a Service which \
 doesn't exist");
          /*Sender*/  
          if(mds_send_get_response(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,3000,
                                   gl_tet_adest.adest,500,
                                   MDS_SEND_PRIORITY_LOW,mesg)
             ==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          break;
          
        case 9:
          tet_printf("Case 9 : \tNot able to send_response a message to Svc \
 2000 with Improper Priority");
          /*Sender*/  
          if(mds_send_get_response(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   gl_tet_adest.adest,100,6,
                                   mesg)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          break;    
        case 10:
          tet_printf("Case 10 : \tNot able to send_response a NULL message to \
 Svc EXTMIN on Active Vdest");
          /*Sender*/  
          if(mds_send_get_response(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   gl_tet_adest.adest,500,
                                   MDS_SEND_PRIORITY_LOW,
                                   NULL)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          break;         
        case 11:
          tet_printf("Case 11 : \tNow send_response a message(> \
 MDTM_NORMAL_MSG_FRAG_SIZE) to Svc EXTMIN on Active Vdest");
          memset(mesg->send_data,'S',2*1400+2 );
          mesg->send_len=2*1400+2;
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_adest_rcvr_thread,
                             gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/  
          if(mds_send_get_response(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   gl_tet_adest.adest,500,
                                   MDS_SEND_PRIORITY_LOW,
                                   mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            {
              printf("\tTASK is released\n");
            }
          fflush(stdout);
          break;         
#if 0
        case 12:
          tet_printf("Case 12 : \tAble to send a messages 200 times  to Svc \
 2000 on Active Vdest");
          for(id=1;id<=200;id++)
            {
              /*Receiver thread*/
              if(tet_create_task((NCS_OS_CB)tet_adest_rcvr_thread,
                                 gl_tet_adest.svc[2].task.t_handle)
                 ==NCSCC_RC_SUCCESS )
                {
                  printf("\tTask has been Created\t");fflush(stdout);
                }
              /*Sender*/           
              if(mds_send_get_response(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                       NCSMDS_SVC_ID_INTERNAL_MIN,
                                       2000,gl_tet_adest.adest,500,
                                       MDS_SEND_PRIORITY_LOW,
                                       mesg)!=NCSCC_RC_SUCCESS)
                {      tet_printf("Fail");FAIL=1;break;    }  
            }
          if(id==201)
            {
              printf("\n\n\t\t ALL THE MESSAGES GOT DELIVERED\n\n");
              sleep(1);
              tet_printf("Success");
            } 
          break;         
#endif
        }
      /*------------------------------------------------------------------*/
      /*clean up*/
      if(tet_cleanup_setup())
        {
          perror("\n\t\t Setup Clean Up has Failed \n");
          getchar();
          FAIL=1;
        }
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n********************************************************\n");
  fflush(stdout);

}
void tet_vdest_rcvr_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  char tmp[]=" Yes Sender! I am in. Message Delivered?";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);    

  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_vdest_sel_obj_found(1,1)) )
    {
      mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/ 
      if(gl_rcvdmsginfo.rsp_reqd)
        {
          if(mds_sendrsp_getack(gl_tet_vdest[1].mds_pwe1_hdl,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,0,
                                mesg)!=NCSCC_RC_SUCCESS)
            {      printf("Response Fail");FAIL=1;    }
          else
            printf("Response Ack Success");
          gl_rcvdmsginfo.rsp_reqd=0;
          /*if(mds_send_redrsp_getack(gl_tet_vdest[1].mds_pwe1_hdl,
            2000,300)!=NCSCC_RC_SUCCESS)
            {      tet_printf("Response Fail");FAIL=1;    }
            else
            tet_printf("Red Response Success");      */
        }
    }
  fflush(stdout);  
}

void tet_Dadest_all_rcvr_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  char tmp[]=" Hi Sender! My Name is RECEIVER ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);

  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_adest_sel_obj_found(1)) )
    {
      printf("\n\t\t Got the message: trying to retreive it\n");fflush(stdout);
      mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/
      if(gl_direct_rcvmsginfo.rsp_reqd)
        {
          printf("i am here\n");
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_direct_response(gl_tet_adest.mds_pwe1_hdl,
                                 NCSMDS_SVC_ID_EXTERNAL_MIN,0,
                                 MDS_SENDTYPE_RSP,gl_set_msg_fmt_ver)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          gl_rcvdmsginfo.rsp_reqd=0;
        }
    }
  fflush(stdout);
}
/*Adest sending direct response to vdest*/
void tet_Dadest_all_chgrole_rcvr_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;

  printf("\n\tInside CHG ROLE ADEST direct Receiver Thread");fflush(stdout);
  if( (svc_id=is_adest_sel_obj_found(1)) )
    {
      printf("\n\t\t Got the message: trying to retreive it\n");fflush(stdout);
      sleep(2);
      mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/
      if(gl_direct_rcvmsginfo.rsp_reqd)
        {
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_direct_response(gl_tet_adest.mds_pwe1_hdl,
                                 NCSMDS_SVC_ID_EXTERNAL_MIN,1500,
                                 MDS_SENDTYPE_RSP,gl_set_msg_fmt_ver)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }

          sleep(10);
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }

          gl_rcvdmsginfo.rsp_reqd=0;
        }
    }
  fflush(stdout);
}

void tet_adest_all_chgrole_rcvr_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  char tmp[]=" Hi Sender! My Name is RECEIVER ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);

  printf("\n\tInside CHG ROLE ADEST Receiver Thread");fflush(stdout);
  if( (svc_id=is_adest_sel_obj_found(1)) )
    {
      printf("\n\t\t Got the message: trying to retreive it\n");fflush(stdout);
      sleep(1);
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("\tFail");
          FAIL=1;
        }
      else
        {
          /*after that send a response to the sender, if it expects*/
          if(gl_rcvdmsginfo.rsp_reqd)
            {
              if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
                {
                  tet_printf("Fail");
                  FAIL=1;
                }
              if(mds_send_response(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,mesg)!=NCSCC_RC_SUCCESS)
                {
                  tet_printf("Fail");
                  FAIL=1;
                }

               
              sleep(10);
              if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
                {
                  tet_printf("Fail");
                  FAIL=1;
                }

              gl_rcvdmsginfo.rsp_reqd=0;
            }
        }
    }
  fflush(stdout);
}
void tet_vdest_all_rcvr_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  char tmp[]=" Hi Sender! My Name is RECEIVER ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);

  printf("\n\tInside VDEST  Receiver Thread");fflush(stdout);
  sleep(10);
  if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
    {
      tet_printf("Fail");
      FAIL=1;
    }

  if( (svc_id=is_vdest_sel_obj_found(1,1)) )
    {
      printf("\n\t\t Got the message: trying to retreive it\n");fflush(stdout);
      sleep(1);
      mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/
      if(gl_rcvdmsginfo.rsp_reqd)
        {
          if(mds_send_response(gl_tet_vdest[1].mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN,mesg)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          gl_rcvdmsginfo.rsp_reqd=0;
        }
    }
  fflush(stdout);
}
void tet_adest_all_rcvrack_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  uint32_t rs;
  char tmp[]=" Hi Sender! My Name is RECEIVER ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);

  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_adest_sel_obj_found(1)) )
    {
      printf("\n\t\t Got the message: trying to retreive it\n");fflush(stdout);
      mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/
      if(gl_rcvdmsginfo.rsp_reqd)
        {
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }

          rs=mds_sendrsp_getack(gl_tet_adest.mds_pwe1_hdl,0,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,mesg);
               
          printf("\nResponse code is %d",rs);
          gl_rcvdmsginfo.rsp_reqd=0;
        }
    }
  fflush(stdout);
}

void tet_adest_all_rcvrack_chgrole_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  uint32_t rs;
  char tmp[]=" Hi Sender! My Name is RECEIVER ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);

  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_adest_sel_obj_found(1)) )
    {
      printf("\n\t\t Got the message: trying to retreive it\n");fflush(stdout);
      mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/
      if(gl_rcvdmsginfo.rsp_reqd)
        {
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(tet_create_task((NCS_OS_CB)tet_adest_all_chgrole_rcvr_thread,
                             gl_tet_adest.svc[1].task.t_handle)
             == NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout); 
            }
          rs=mds_sendrsp_getack(gl_tet_adest.mds_pwe1_hdl,0,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,mesg);
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");
          fflush(stdout);

          printf("\nResponse code is %d",rs);
          gl_rcvdmsginfo.rsp_reqd=0;
        }
    }
  fflush(stdout);
}

void tet_Dadest_all_rcvrack_chgrole_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  uint32_t rs;

  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_adest_sel_obj_found(1)) )
    {
      printf("\n\t\t Got the message: trying to retreive it\n");fflush(stdout);
      mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/
      if(gl_direct_rcvmsginfo.rsp_reqd)
        {
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(tet_create_task((NCS_OS_CB)tet_adest_all_chgrole_rcvr_thread,
                             gl_tet_adest.svc[1].task.t_handle)
             == NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          if(mds_direct_response(gl_tet_vdest[1].mds_pwe1_hdl,
                                 NCSMDS_SVC_ID_EXTERNAL_MIN,0,
                                 MDS_SENDTYPE_SNDRACK,gl_set_msg_fmt_ver)!=NCSCC_RC_SUCCESS)
            {      printf("Response Fail");FAIL=1;    }
          else
            printf("Response Success");
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");
          fflush(stdout);

          printf("\nResponse code is %d",rs);
          gl_rcvdmsginfo.rsp_reqd=0;
        }
    }
  fflush(stdout);
}

void tet_change_role_thread()
{
  int FAIL=0;
  sleep(10);
  printf("\n\tInside Chnage role Thread");fflush(stdout);
  if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
    {
      tet_printf("Fail");
      FAIL=1;
    }

  fflush(stdout);
}

void tet_adest_all_rcvr_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  char tmp[]=" Hi Sender! My Name is RECEIVER ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);

  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_adest_sel_obj_found(1)) )
    {
      printf("\n\t\t Got the message: trying to retreive it\n");fflush(stdout);
      mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/
      if(gl_rcvdmsginfo.rsp_reqd)
        {
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_send_response(gl_tet_adest.mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN,mesg)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          gl_rcvdmsginfo.rsp_reqd=0;
        }
    }
  fflush(stdout);
}
void tet_send_all_tp(int choice)
{
  int FAIL=0;
  MDS_SVC_ID svcids[]={NCSMDS_SVC_ID_EXTERNAL_MIN};
  void tet_vdest_rcvr_thread();
  void tet_adest_all_rcvr_thread();
  void tet_vdest_all_rcvr_thread();
  char tmp[]=" Hi Receiver! Are you there? ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));   
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);
  
  printf("\n\n************** tet_send_all_tp %d ********\n",choice);
  tet_printf("\t\t\ttet_send_all_tp");
  /*--------------------------------------------------------------------*/
  switch(choice)
    {
    case 1:
      tet_printf("\tCase 1 :Sender service installed with i_fail_no_active_sends = true \
                                            and there is no-active instance of the receiver service");

      tet_printf("\tSetting up the setup");
      if(tet_initialise_setup(true))
        {
          tet_printf("\n\t\t Setup Initialisation has Failed \n");
      
          FAIL=1;
        }
      if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN,
                               NCSMDS_SCOPE_NONE,1,
                               svcids)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      /*SND*/
      tet_printf("\t Sending the message to no active instance");
      if( (mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                         NCSMDS_SVC_ID_EXTERNAL_MIN,
                         NCSMDS_SVC_ID_EXTERNAL_MIN,
                         gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_LOW,
                         mesg)!=MDS_RC_MSG_NO_BUFFERING) )
        {
          tet_printf("Fail");
          FAIL=1;
        }
      else
        tet_printf("Success");
      /*SNDACK*/

      tet_printf("\t Sendack to the no active instance");
      if((mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           gl_tet_vdest[1].vdest,
                           0,MDS_SEND_PRIORITY_LOW,
                           mesg)!=MDS_RC_MSG_NO_BUFFERING) )
        {
          tet_printf("Fail");
          FAIL=1;
        }
      else
        tet_printf("Success");
      /*SNDRSP*/
      tet_printf("\t Send response to the no active instance");
      if((mds_send_get_response(gl_tet_adest.mds_pwe1_hdl,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,
                                gl_tet_vdest[1].vdest,
                                0,MDS_SEND_PRIORITY_LOW,
                                mesg)!=MDS_RC_MSG_NO_BUFFERING) )
        {
          tet_printf("Fail");
          FAIL=1;
        }
      /*RSP*/
      tet_printf("\tChange role to active"); 
      if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      if(tet_create_task((NCS_OS_CB)tet_adest_all_rcvr_thread,
                         gl_tet_adest.svc[1].task.t_handle)
         == NCSCC_RC_SUCCESS )
        {
          printf("\tTask has been Created\t");fflush(stdout);
        }


      if((mds_send_get_response(gl_tet_vdest[1].mds_pwe1_hdl,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,
                                gl_tet_adest.adest,
                                0,MDS_SEND_PRIORITY_LOW,
                                mesg)!= NCSCC_RC_SUCCESS) )
        {
          tet_printf("Fail");
          FAIL=1;
        }

      else
        tet_printf("Success"); 

      /*Now Stop and Release the Receiver Thread*/
      if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[1].task.t_handle)
         ==NCSCC_RC_SUCCESS)
        printf("\tTASK is released\n");                
      fflush(stdout);                
      /*SNDRACK*/
#if 0
      tet_printf("\tChange role to active"); 
      if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      if(tet_create_task((NCS_OS_CB)tet_adest_all_rcvrack_thread,
                         gl_tet_adest.svc[1].task.t_handle)
         == NCSCC_RC_SUCCESS )
        {
          printf("\tTask has been Created\t");fflush(stdout);
        }

      tet_printf("\tSending msg with rsp ack");
      if((mds_send_get_response(gl_tet_vdest[1].mds_pwe1_hdl,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,
                                gl_tet_adest.adest,
                                0,MDS_SEND_PRIORITY_LOW,
                                mesg)!= NCSCC_RC_SUCCESS) )
        {
          tet_printf("Fail from response");
          FAIL=1;
        }

      else
        tet_printf("Success"); 

      /*Now Stop and Release the Receiver Thread*/
      if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[1].task.t_handle)
         ==NCSCC_RC_SUCCESS)
        printf("\tTASK is released\n");                
      fflush(stdout);                
#endif
      if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                         NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                         svcids)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      fflush(stdout);
      /*clean up*/
      if(tet_cleanup_setup())
        {
          perror("\n\t\t Setup Clean Up has Failed \n");
          getchar();
          FAIL=1;
        }

      break;

    case 2:
      tet_printf("\tCase 2 :Sender service installed with i_fail_no_active_sends = false \
                                            and there is no-active instance of the receiver service");

      if(tet_initialise_setup(false))
        {
          tet_printf("\n\t\t Setup Initialisation has Failed \n");
          FAIL=1;
        }
      if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN,
                               NCSMDS_SCOPE_NONE,1,
                               svcids)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

 
      if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      tet_printf("\t Sending the message to no active instance");
      if( (mds_just_send(gl_tet_adest.mds_pwe1_hdl,
                         NCSMDS_SVC_ID_EXTERNAL_MIN,
                         NCSMDS_SVC_ID_EXTERNAL_MIN,
                         gl_tet_vdest[1].vdest,MDS_SEND_PRIORITY_LOW,
                         mesg)!=NCSCC_RC_SUCCESS) && gl_rcvdmsginfo.msg_fmt_ver != 3 )
        {
          tet_printf("Fail");
          FAIL=1;
        }
      sleep(10);
      tet_printf("\tChange role to active");
      if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      /*SNDACK*/
      tet_printf("\tChange role to standby");
      if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      else
        tet_printf("Success");
      /*Create thread to change role*/

      if(tet_create_task((NCS_OS_CB)tet_change_role_thread,
                         gl_tet_adest.svc[1].task.t_handle)
         == NCSCC_RC_SUCCESS )
        {
          printf("\tTask has been Created\t");fflush(stdout);
        }

      tet_printf("\t Sendack to the no active instance");
      if((mds_send_get_ack(gl_tet_adest.mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,
                           gl_tet_vdest[1].vdest,
                           1500,MDS_SEND_PRIORITY_LOW,
                           mesg)!=NCSCC_RC_SUCCESS) && (gl_rcvdmsginfo.msg_fmt_ver != 3))
        {
          tet_printf("Fail");
          FAIL=1;

        }

      else
        tet_printf("Success");

      if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[1].task.t_handle)
         ==NCSCC_RC_SUCCESS)
        printf("\tTASK is released\n");
      fflush(stdout);
      /*SNDRSP*/

      tet_printf("\tChange role to standby");
      if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }

      else
        tet_printf("Success");
      if(tet_create_task((NCS_OS_CB)tet_vdest_all_rcvr_thread,
                         gl_tet_vdest[1].svc[1].task.t_handle)
         == NCSCC_RC_SUCCESS )
        {
          printf("\tTask has been Created\t");fflush(stdout);
        }
      tet_printf("\t Sendrsp to the no active instance");
      if((mds_send_get_response(gl_tet_adest.mds_pwe1_hdl,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,
                                gl_tet_vdest[1].vdest,
                                1500,MDS_SEND_PRIORITY_LOW,
                                mesg)!= NCSCC_RC_SUCCESS)&& (gl_rcvdmsginfo.msg_fmt_ver != 3) )
        {
          tet_printf("msg fmt ver is %d",gl_rcvdmsginfo.msg_fmt_ver);
          tet_printf("Fail frm rsp");
          FAIL=1;
        }

      else
        tet_printf("Success");
      if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
         ==NCSCC_RC_SUCCESS)
        printf("\tTASK is released\n");
      /*RSP*/
      if(tet_create_task((NCS_OS_CB)tet_adest_all_chgrole_rcvr_thread,
                         gl_tet_adest.svc[1].task.t_handle)
         == NCSCC_RC_SUCCESS )
        {
          printf("\tTask has been Created\t");fflush(stdout);
        }

      tet_printf("\t rsp to the no active instance");
      if((mds_send_get_response(gl_tet_vdest[1].mds_pwe1_hdl,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,
                                gl_tet_adest.adest,
                                2500,MDS_SEND_PRIORITY_LOW,
                                mesg)!= NCSCC_RC_SUCCESS )&&gl_rcvdmsginfo.msg_fmt_ver != 3) 
        {
          tet_printf("Fail");
          FAIL=1;
        }

      else
        tet_printf("Success"); 

      /*Now Stop and Release the Receiver Thread*/
      if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[1].task.t_handle)
         ==NCSCC_RC_SUCCESS)
        printf("\tTASK is released\n");                
      fflush(stdout);                
#if 0
      /*SNDRACK*/

      if(tet_create_task((NCS_OS_CB)tet_adest_all_rcvrack_chgrole_thread,
                         gl_tet_adest.svc[1].task.t_handle)
         == NCSCC_RC_SUCCESS )
        {
          printf("\tTask has been Created\t");fflush(stdout);
        }
      tet_printf("\t Send ack being sent");

      if((mds_send_get_response(gl_tet_vdest[1].mds_pwe1_hdl,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,
                                NCSMDS_SVC_ID_EXTERNAL_MIN,
                                gl_tet_adest.adest,
                                1500,MDS_SEND_PRIORITY_LOW,
                                mesg)!= NCSCC_RC_SUCCESS) )
        {
          tet_printf("Fail");
          FAIL=1;
        }

      else
        tet_printf("Success"); 

      /*Now Stop and Release the Receiver Thread*/
      if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[1].task.t_handle)
         ==NCSCC_RC_SUCCESS)
        printf("\tTASK is released\n");                
      fflush(stdout);                

#endif
      if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                         NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                         svcids)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      fflush(stdout);
      /*clean up*/
      if(tet_cleanup_setup())
        {
          perror("\n\t\t Setup Clean Up has Failed \n");
          getchar();
          FAIL=1;
        }

      break;
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n*******************************************************\n");
  fflush(stdout);

}
void tet_send_response_ack_tp(int choice)
{
  int FAIL=0;
  MDS_SVC_ID svcids[]={NCSMDS_SVC_ID_EXTERNAL_MIN};
  void tet_vdest_rcvr_thread();

  char tmp[]=" Hi Receiver! Are you there? ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));   
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);

  printf("\n\n************** tet_send_response_ack_tp %d ********\n",choice);
  tet_printf("\t\t\ttet_send_response_ack_tp");
  /*Start up*/
  if(tet_initialise_setup(false))
    {
      tet_printf("\n\t\t Setup Initialisation has Failed \n");
      FAIL=1;
    }

  else
    {
      /*--------------------------------------------------------------------*/
      switch(choice)
        {
        case 1:
          tet_printf("\tCase 1 : Svc EXTMIN on ADEST sends a LOW Priority \
                                                message to Svc EXTMIN on VDEST=200 and expects a Response");
          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_vdest_rcvr_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */ 
          if(mds_send_get_response(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,200,
                                   900,MDS_SEND_PRIORITY_LOW,
                                   mesg)!=NCSCC_RC_SUCCESS)
            {      tet_printf("Send get Response Fail");FAIL=1;    }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");                
          fflush(stdout);                
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 2:
          tet_printf("\tCase 2 : Svc EXTMIN on ADEST sends a MEDIUM Priority \
 message to Svc EXTMIN on VDEST=200 and expects a Response");
          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_vdest_rcvr_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */ 
          if(mds_send_get_response(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,200,
                                   900,MDS_SEND_PRIORITY_MEDIUM,
                                   mesg)!=NCSCC_RC_SUCCESS)
            {      tet_printf("Send get Response Fail");FAIL=1;    }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");                
          fflush(stdout);                
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 3:
          tet_printf("\tCase 3 : Svc EXTMIN on ADEST sends a HIGH Priority \
 message to Svc EXTMIN on VDEST=200 and expects a Response");
          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_vdest_rcvr_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */ 
          if(mds_send_get_response(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,200,
                                   900,MDS_SEND_PRIORITY_HIGH,
                                   mesg)!=NCSCC_RC_SUCCESS)
            {      tet_printf("Send get Response Fail");FAIL=1;    }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");                
          fflush(stdout);                
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 4:
          tet_printf("\tCase 4 :Svc EXTMIN on ADEST sends a VERYHIGH Priority \
 message to Svc EXTMIN on VDEST=200 and expects a Response");
          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_vdest_rcvr_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */ 
          if(mds_send_get_response(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,200,
                                   900,MDS_SEND_PRIORITY_VERY_HIGH,
                                   mesg)!=NCSCC_RC_SUCCESS)
            {      tet_printf("Send get Response Fail");FAIL=1;    }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");                
          fflush(stdout);                
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 5:
          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          tet_printf("\tCase 5 :While Await Active Timer ON:SvcEXTMIN of ADEST\
 sends a message to Svc EXTMIN on Active Vdest=200 and Times out");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_vdest_rcvr_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          sleep(2);         
          /*Sender */ 
          if(mds_send_get_response(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,200,
                                   300,MDS_SEND_PRIORITY_VERY_HIGH,
                                   mesg)!=NCSCC_RC_REQ_TIMOUT)
            { 
              tet_printf("Send get Response Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");                
          fflush(stdout);                
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 6:
          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          tet_printf("\tCase 6 :SvcEXTMIN of ADEST sends message to SvcEXTMIN \
 on QUIESCED Vdest=200 and Times out");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_vdest_rcvr_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          if(vdest_change_role(200,V_DEST_RL_QUIESCED)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          sleep(2);         
          /*Sender */ 
          if(mds_send_get_response(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,200,
                                   300,MDS_SEND_PRIORITY_VERY_HIGH,
                                   mesg)!=NCSCC_RC_REQ_TIMOUT)
            {
              /*tet_printf("Send get Response Fail");*/
              tet_printf("Send get Response is not Timing OUT: Why");
              FAIL=1;
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");                
          fflush(stdout);                
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 7:
          tet_printf("\tCase 7 :Implicit Subscription: Svc EXTL_MIN on ADEST \
sends a LOWPriority message to Svc EXTMIN on VDEST=200 and expects Response");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_vdest_rcvr_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */ 
          if(mds_send_get_response(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,200,
                                   900,MDS_SEND_PRIORITY_LOW,
                                   mesg)!=NCSCC_RC_SUCCESS)
            {      tet_printf("Send get Response Fail");FAIL=1;    }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");                
          fflush(stdout);                
          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_vdest_rcvr_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */ 
          if(mds_send_get_response(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,200,
                                   900,MDS_SEND_PRIORITY_LOW,
                                   mesg)!=NCSCC_RC_SUCCESS)
            {      tet_printf("Send get Response Fail");FAIL=1;    }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");                
          fflush(stdout);                
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          break;
        case 8:
          tet_printf("Case 8 : \tNow send_response a message(> \
 MDTM_NORMAL_MSG_FRAG_SIZE) to Svc EXTMIN on Active Vdest");
          memset(mesg->send_data,'S',2*1400+2 );
          mesg->send_len=2*1400+2;
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_vdest_rcvr_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */ 
          if(mds_send_get_response(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,200,
                                   900,MDS_SEND_PRIORITY_LOW,
                                   mesg)!=NCSCC_RC_SUCCESS)
            {      tet_printf("Send get Response Fail");FAIL=1;    }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");                
          fflush(stdout);                
          break;         
        }
      /*--------------------------------------------------------------------*/
      /*clean up*/
      if(tet_cleanup_setup())
        {
          perror("\n\t\t Setup Clean Up has Failed \n");
          getchar();
          FAIL=1;
        }
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n*******************************************************\n");
  fflush(stdout);
  
}
void tet_vdest_Srcvr_thread()
{
  MDS_SVC_ID svc_id;
  
  char tmp[]=" Yes Sender! I am STANDBY? ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));  
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);
  
  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_vdest_sel_obj_found(0,0)) )
    {
      mds_service_retrieve(gl_tet_vdest[0].mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/ 
      if(gl_rcvdmsginfo.rsp_reqd)
        {
          gl_rcvdmsginfo.rsp_reqd=0;
        }
      /*      if(mds_send_redrsp_getack(gl_tet_vdest[0].mds_pwe1_hdl,2000,
              300)!=NCSCC_RC_SUCCESS)
              {      tet_printf("Response Ack Fail");FAIL=1;    }
              else
              tet_printf("Red Response Ack Success");      */
    }
  fflush(stdout);  
}
void tet_vdest_Arcvr_thread()
{
  MDS_SVC_ID svc_id;
    
  char tmp[]=" Yes Man! I am in: Message Delivered? ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));  
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);
        
  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_vdest_sel_obj_found(0,0)) )
    {
      mds_service_retrieve(gl_tet_vdest[0].mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL);
#if 0 /*AVM RE-CHECk*/
      /*after that send a response to the sender, if it expects*/ 
      if(gl_rcvdmsginfo.rsp_reqd)
        {
          if(mds_send_redrsp_getack(gl_tet_vdest[0].mds_pwe1_hdl,
                                    NCSMDS_SVC_ID_EXTERNAL_MIN,0,
                                    mesg)!=NCSCC_RC_SUCCESS)
            {      printf("Response Ack Fail");FAIL=1;    }
          else
            printf("Red Response Ack Success");      
          gl_rcvdmsginfo.rsp_reqd=0;
        }          
    #endif
    }
  fflush(stdout);  
}
void tet_broadcast_to_svc_tp(int choice)
{
  int FAIL=0;
  MDS_SVC_ID svcids[]={NCSMDS_SVC_ID_EXTERNAL_MIN};
    
  char tmp[]=" Hi All ";
  TET_MDS_MSG *mesg;
  mesg = (TET_MDS_MSG*)malloc(sizeof(TET_MDS_MSG));
  memset(mesg, 0, sizeof(TET_MDS_MSG));   
  memcpy(mesg->send_data,tmp,sizeof(tmp));
  mesg->send_len=sizeof(tmp);

  printf("\n\n************ tet_broadcast_to_svc_tp %d ************\n",choice);
  tet_printf("\t\t\ttet_broadcast_to_svc_tp");
  /*Start up*/
  if(tet_initialise_setup(false))
    {
      printf("\n\t\t Setup Initialisation has Failed \n");
      FAIL=1;
    }
  else
    {
      /*--------------------------------------------------------------------*/
      switch(choice)
        {
        case 1:
          tet_printf("\tCase 1 : Svc INTMIN on VDEST=200 Broadcasting a LOW \
 Priority message to Svc EXTMIN");
          if(mds_service_subscribe(gl_tet_vdest[1].mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_broadcast_to_svc(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  NCSMDS_SCOPE_NONE,MDS_SEND_PRIORITY_LOW,
                                  mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          if(mds_service_cancel_subscription(gl_tet_vdest[1].mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,
                                             1,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 2:
          tet_printf("\tCase 2 : Svc INTMIN on ADEST Broadcasting a MEDIUM \
 Priority message to Svc EXTMIN");
          if(mds_service_subscribe(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_broadcast_to_svc(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  NCSMDS_SCOPE_NONE,MDS_SEND_PRIORITY_MEDIUM,
                                  mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");     
          if(mds_service_cancel_subscription(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 3:
          tet_printf("\tCase 3 :Svc INTMIN on VDEST=200 Redundant Broadcasting\
 a HIGH priority message to Svc EXTMIN");
          if(mds_service_subscribe(gl_tet_vdest[1].mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_cancel_subscription(gl_tet_vdest[1].mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,
                                             1,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 4:
          tet_printf("\tCase 4 :Svc INTMIN on VDEST=200 Not able to Broadcast\
 a message with Invalid Priority");
          if(mds_service_subscribe(gl_tet_vdest[1].mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_broadcast_to_svc(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  NCSMDS_SCOPE_NONE,5,mesg)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          if(mds_service_cancel_subscription(gl_tet_vdest[1].mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,
                                             1,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;        
        case 5:
          tet_printf("\tCase 5 :Svc INTMIN on VDEST=200 Not able to Broadcast\
 a NULL message");
          if(mds_service_subscribe(gl_tet_vdest[1].mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_broadcast_to_svc(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  NCSMDS_SCOPE_NONE,MDS_SEND_PRIORITY_LOW,
                                  NULL)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          if(mds_service_cancel_subscription(gl_tet_vdest[1].mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,
                                             1,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 6:
          tet_printf("Case 6 : \tSvc INTMIN on VDEST=200 Broadcasting a VERY \
 HIGH Priority message (>MDTM_NORMAL_MSG_FRAG_SIZE) to Svc EXTMIN");
          memset(mesg->send_data,'S',2*1400+2 );
          mesg->send_len=2*1400+2;
          if(mds_service_subscribe(gl_tet_vdest[1].mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_broadcast_to_svc(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  NCSMDS_SCOPE_NONE,
                                  MDS_SEND_PRIORITY_VERY_HIGH,
                                  mesg)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          if(mds_service_cancel_subscription(gl_tet_vdest[1].mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,
                                             1,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;        
        }
      /*---------------------------------------------------------------------*/
      /*clean up*/
      if(tet_cleanup_setup())
        {
          perror("\n\t\t Setup Clean Up has Failed \n");
          getchar();
          FAIL=1;
        }
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n********************************************************\n");
  fflush(stdout);
}
/*---------------- DIRECT SEND TEST CASES--------------------------------*/

void tet_direct_just_send_tp(int choice)
{
  int FAIL=0;
  int id=0;
  MDS_SVC_ID svcids[]={NCSMDS_SVC_ID_EXTERNAL_MIN};
  MDS_SVC_ID svc_ids[]={NCSMDS_SVC_ID_INTERNAL_MIN};
  char message[]="Direct Message";
  char big_message[8000];

  printf("\n\n********** tet_direct_just_send_tp %d ***********\n",choice);
  tet_printf("\t\t\t tet_direct_just_send_tp");
  /*start up*/
  if(tet_initialise_setup(false))
    { 
      tet_printf("\n\t\t Setup Initialisation has Failed \n");
      FAIL=1;
    }

  else
    {
      /*-------------------------------DIRECT SEND --------------------------*/
      if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN,
                               NCSMDS_SCOPE_NONE,1,svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      switch(choice)
        {
        case 1:
          tet_printf("Case 1 : \tNow Direct send Low,Medium,High and Very High\
 Priority messages to Svc EXTMIN on Active Vdest=200");
          /*Low*/
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");

          /*Medium*/

          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_MEDIUM,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");

          /*High*/

          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_HIGH,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");

          /*Very High*/

          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_VERY_HIGH,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 2:
          tet_printf("Case 2 : \tNot able to send a message to 2000 with \
 Invalid pwe handle");
          if(mds_direct_send_message(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 3:
          tet_printf("Case 3 : \tNot able to send a message to 2000 with NULL \
 pwe handle");
          if(mds_direct_send_message((MDS_HDL)(long)NULL,NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SND,
                                     gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 4:
          tet_printf("Case 4 : \tNot able to send a message to Svc EXTMIN on \
 Vdest=200 with Wrong DEST");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SND,0,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 5:
          tet_printf("Case 5 : \tNot able to send a message to service which \
is Not installed");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,3000,1,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 6:
          tet_printf("Case 6 : \tAble to send a message 1000 times  to \
 Svc EXTMIN on Active Vdest=200");
          for(id=1;id<=1000;id++)
            {
              if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                         NCSMDS_SVC_ID_EXTERNAL_MIN,
                                         NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                         MDS_SENDTYPE_SND,
                                         gl_tet_vdest[1].vdest,
                                         0,MDS_SEND_PRIORITY_LOW,
                                         message)!=NCSCC_RC_SUCCESS)
                {    
                  tet_printf("Fail");
                  FAIL=1; 
                }
            }
          if(id==1001)
            {
              printf("\n\n\t\t ALL THE MESSAGES GOT DELIVERED\n\n");
              sleep(1);
              tet_printf("Success");
            }
          break;
        case 7:
          tet_printf("Case 7 : \tImplicit Subscription: Direct send a message\
 to unsubscribed Svc INTMIN");
          /*Implicit Subscription*/
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_INTERNAL_MIN,1,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,
                                     0,MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          /*Explicit subscription*/
          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,svc_ids)
             !=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_INTERNAL_MIN,1,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,
                                     0,MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svc_ids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          break;
        case 8:
          tet_printf("Case 8 : \tNot able to send a message to Svc EXTMIN with\
 Improper Priority");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,0,
                                     5,
                                     message)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 9:
          tet_printf("Case 9 :Not able to send a message to Svc EXTMIN with\
 Direct Buff as NULL");
          svc_to_mds_info.i_mds_hdl=gl_tet_adest.mds_pwe1_hdl;
          svc_to_mds_info.i_svc_id=NCSMDS_SVC_ID_EXTERNAL_MIN;
          svc_to_mds_info.i_op=MDS_DIRECT_SEND;
          svc_to_mds_info.info.svc_direct_send.i_direct_buff=NULL;
          svc_to_mds_info.info.svc_direct_send.i_direct_buff_len = 
            strlen(message)+1;
          svc_to_mds_info.info.svc_direct_send.i_to_svc=
            NCSMDS_SVC_ID_EXTERNAL_MIN;
          svc_to_mds_info.info.svc_direct_send.i_msg_fmt_ver=1;
          svc_to_mds_info.info.svc_direct_send.i_priority=
            MDS_SEND_PRIORITY_LOW;
          svc_to_mds_info.info.svc_direct_send.i_sendtype=MDS_SENDTYPE_SND;
          svc_to_mds_info.info.svc_direct_send.info.snd.i_to_dest=
            gl_tet_vdest[1].vdest;
          if(ncsmds_api(&svc_to_mds_info)==NCSCC_RC_SUCCESS)
            { tet_printf("Fail");FAIL=1;    }
          else
            tet_printf("Success");
          break;
        case 10:
          tet_printf("Case 10 : \tNot able to send a message with Invalid \
 Sendtype(<0 or >11)");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     -1,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");

          break;
        case 11:
          tet_printf("Case 11 : \tNot able to send a message with Invalid \
 Message length");
          direct_buff=m_MDS_ALLOC_DIRECT_BUFF(strlen(message)+1);
          memset(direct_buff, 0, sizeof(direct_buff));
          if(direct_buff==NULL)
            perror("Direct Buffer not allocated properly");
          memcpy(direct_buff,(uint8_t *)message,strlen(message)+1);
          svc_to_mds_info.i_mds_hdl=gl_tet_adest.mds_pwe1_hdl;
          svc_to_mds_info.i_svc_id=NCSMDS_SVC_ID_EXTERNAL_MIN;
          
          svc_to_mds_info.i_op=MDS_DIRECT_SEND;
          svc_to_mds_info.info.svc_direct_send.i_direct_buff=direct_buff;
          svc_to_mds_info.info.svc_direct_send.i_direct_buff_len = -1;
          /*wrong length*/
          svc_to_mds_info.info.svc_direct_send.i_to_svc=
            NCSMDS_SVC_ID_EXTERNAL_MIN;
          svc_to_mds_info.info.svc_direct_send.i_msg_fmt_ver=1;
          svc_to_mds_info.info.svc_direct_send.i_priority=
            MDS_SEND_PRIORITY_LOW;
          svc_to_mds_info.info.svc_direct_send.i_sendtype=MDS_SENDTYPE_SND;
          svc_to_mds_info.info.svc_direct_send.info.snd.i_to_dest=
            gl_tet_vdest[1].vdest;
          if(ncsmds_api(&svc_to_mds_info)==NCSCC_RC_SUCCESS)
            { tet_printf("Fail");FAIL=1;    }
          else
            tet_printf("Success");
          break;
        case 12:
          tet_printf("Case 12 : \tWhile Await Active Timer ON: Direct send a \
 Low Priority message to Svc EXTMIN on Vdest=200");
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          sleep(2);
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,
                                     0,MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          break;
        case 13:
          tet_printf("Case 13 : \tDirect send a Medium Priority message to \
 Svc EXTMIN on QUIESCED Vdest=200");
          if(vdest_change_role(200,V_DEST_RL_QUIESCED)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          sleep(2);
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_MEDIUM,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          break;
        case 14:
          tet_printf("Case 14 : \tNot able to send a message of size >(\
 MDS_DIRECT_BUF_MAXSIZE) to 2000");

          memset(big_message, 's', 8000);
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SND,gl_tet_adest.adest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     big_message)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 15:
          tet_printf("Case 15 : \tAble to send a message of size =(\
 MDS_DIRECT_BUF_MAXSIZE) to 2000");
          
          memset(big_message, '\0', 8000);
          memset(big_message, 's', 7999);
          
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SND,gl_tet_adest.adest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     big_message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        }
      if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                         NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                         svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      fflush(stdout);
      /*--------------------------------------------------------------------*/
      /*clean up*/
      if(tet_cleanup_setup())
        {
          printf("\n\t\t Setup Clean Up has Failed \n");
          FAIL=1;
        }
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n*********************************************************\n");
  fflush(stdout);

}

void tet_direct_send_all_tp(int choice)
{
  int FAIL=0;
  MDS_SVC_ID svcids[]={NCSMDS_SVC_ID_EXTERNAL_MIN};
  char message[]="Direct Message";
  void tet_Dvdest_rcvr_all_thread();
  void tet_Dvdest_rcvr_all_rack_thread();
  void tet_Dvdest_rcvr_all_chg_role_thread();
  void tet_Dadest_all_rcvrack_chgrole_thread();
  void tet_Dadest_all_chgrole_rcvr_thread();
  void tet_Dadest_all_rcvr_thread();
  printf("\n\n********** tet_direct_send_all_tp %d ***********\n",choice);
  tet_printf("\t\t\t tet_direct_send_all_tp");
  /*start up*/
  if(tet_initialise_setup(false))
    { 
      tet_printf("\n\t\t Setup Initialisation has Failed \n");
      FAIL=1;
    }

  else
    {
      /*-------------------------------DIRECT SEND --------------------------*/
      if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN,
                               NCSMDS_SCOPE_NONE,1,svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      switch(choice)
        {
        case 1:
          gl_set_msg_fmt_ver=0;
          tet_printf("Case 1 : \tDirect send a message with i_msg_fmt_ver = 0 for all send types");
          /*SND*/
          tet_printf("\tDirect sending the message");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          /*SNDACK*/
          tet_printf("\tDirect send with ack"); 
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,
                                     0,MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");
          /*SNDRSP*/
          tet_printf("\tDirect send with rsp");
          if(tet_create_task((NCS_OS_CB)tet_Dvdest_rcvr_all_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             == NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_vdest[1].vdest,
                                     0,MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");
          fflush(stdout);
          /*SNDRACK*/
          tet_printf("\t Direct send with response ack");
          if(tet_create_task((NCS_OS_CB)tet_Dvdest_rcvr_all_rack_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_FAILURE)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");
          sleep(2); /*in 16.0.2*/
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");
          fflush(stdout);         
          break;

        case 2:
          gl_set_msg_fmt_ver=3;
          tet_printf("Case 2 : \tDirect send a message with i_msg_fmt_ver = i_rem_svc_pvt_ver for all send types");
          /*SND*/
          tet_printf("\tDirect sending the message");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          /*SNDACK*/
          tet_printf("\tDirect send with ack"); 
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,
                                     0,MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");
          /*SNDRSP*/
          tet_printf("\tDirect send with rsp");
          if(tet_create_task((NCS_OS_CB)tet_Dvdest_rcvr_all_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             == NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_vdest[1].vdest,
                                     0,MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");
          fflush(stdout);
          /*SNDRACK*/
          tet_printf("\t Direct send with response ack");
          if(tet_create_task((NCS_OS_CB)tet_Dvdest_rcvr_all_rack_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_FAILURE)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");
          sleep(2); /*in 16.0.2*/
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");
          fflush(stdout);         
          break;

        case 3:
          gl_set_msg_fmt_ver=4;
          tet_printf("Case 3 : \tDirect send a message with i_msg_fmt_ver > i_rem_svc_pvt_ver for all send types");
          /*SND*/
          tet_printf("\tDirect sending the message");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          /*SNDACK*/
          tet_printf("\tDirect send with ack"); 
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,
                                     0,MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");
          /*SNDRSP*/
          tet_printf("\tDirect send with rsp");
          if(tet_create_task((NCS_OS_CB)tet_Dvdest_rcvr_all_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             == NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_vdest[1].vdest,
                                     0,MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");
          fflush(stdout);
          /*SNDRACK*/
          tet_printf("\t Direct send with response ack");
          if(tet_create_task((NCS_OS_CB)tet_Dvdest_rcvr_all_rack_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_FAILURE)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");
          sleep(2); /*in 16.0.2*/
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");
          fflush(stdout);         
          break;

        case 4:
          gl_set_msg_fmt_ver=2;
          tet_printf("Case 4 : \tDirect send a message with i_msg_fmt_ver < i_rem_svc_pvt_ver for all send types");
          /*SND*/
          tet_printf("\tDirect sending the message");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          /*SNDACK*/
          tet_printf("\tDirect send with ack"); 
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,
                                     0,MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");
          /*SNDRSP*/
          tet_printf("\tDirect send with rsp");
          if(tet_create_task((NCS_OS_CB)tet_Dvdest_rcvr_all_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             == NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_vdest[1].vdest,
                                     0,MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");
          fflush(stdout);
          /*SNDRACK*/
          tet_printf("\t Direct send with response ack");
          if(tet_create_task((NCS_OS_CB)tet_Dvdest_rcvr_all_rack_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_FAILURE)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");
          sleep(2); /*in 16.0.2*/
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");
          fflush(stdout);         
          break;

        case 5:
          gl_set_msg_fmt_ver=1;
          tet_printf("\tCase 5 : Direct send when Sender service installed with i_fail_no_active_sends = false \
                          and there is no-active instance of the receiver service");
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }

          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          tet_printf("\t Sending the message to no active instance");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_FAILURE)

            {
              tet_printf("Fail");
              FAIL=1;
            }
          sleep(10);
          tet_printf("\tChange role to active");
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          /*SNDACK*/
          tet_printf("\tChange role to standby");
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }

          else
            tet_printf("Success");
          /*Create thread to change role*/

          if(tet_create_task((NCS_OS_CB)tet_change_role_thread,
                             gl_tet_adest.svc[1].task.t_handle)
             == NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }

          tet_printf("\t Sendack to the no active instance");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_FAILURE)

            {
              tet_printf("Fail");
              FAIL=1;

            }

          else
            tet_printf("Success");

          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");
          fflush(stdout);
          /*SNDRSP*/
          tet_printf("\tChange role to standby");
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }

          else
            tet_printf("Success");

          if(tet_create_task((NCS_OS_CB)tet_Dvdest_rcvr_all_chg_role_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             == NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }

          tet_printf("\t Sendrsp to the no active instance");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_FAILURE)


            {
              tet_printf("Fail");
              FAIL=1;
            }

          else
            tet_printf("Success");
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");


          /*RSP*/
          if(tet_create_task((NCS_OS_CB)tet_Dadest_all_chgrole_rcvr_thread,
                             gl_tet_adest.svc[1].task.t_handle)
             == NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }


          tet_printf("\t Sendrsp to the no active instance");
          if(mds_direct_send_message(gl_tet_vdest[1].mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_adest.adest,1500,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)


            {
              tet_printf("Fail");
              FAIL=1;
            }

          else
            tet_printf("Success");

          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");
          fflush(stdout);

#if 0
          tet_printf("\tChange role to standby");
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }

          else
            tet_printf("Success");

          /*SNDRACK*/

          if(tet_create_task((NCS_OS_CB)tet_Dadest_all_rcvrack_chgrole_thread,
                             gl_tet_adest.svc[1].task.t_handle)
             == NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          tet_printf("\t Send rsp ack being sent");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_vdest[1].vdest,3000,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_FAILURE)


            {
              tet_printf("Fail");
              FAIL=1;
            }

          else
            tet_printf("Success"); 

          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");                
          fflush(stdout);                

#endif
          break;

        case 6:
          gl_set_msg_fmt_ver=1;
          tet_printf("\tCase 6 : Direct send when Sender service installed with \
                                            i_fail_no_active_sends = true and there is no-active instance of the receiver service");
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          /*Unistalling setup to install again with req params*/
          if(tet_cleanup_setup())
            {
              printf("\n\t\t Setup Clean Up has Failed \n");
              FAIL=1;
            }
          if(tet_initialise_setup(true))
            { 
              tet_printf("\n\t\t Setup Initialisation has Failed \n");
              FAIL=1;
            }

          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,svcids)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
                            
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }

          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          /*SND*/
          tet_printf("\tDirect Sending the message to no active instance");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SND,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)!=MDS_RC_MSG_NO_BUFFERING)

            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");

          /*SNDACK*/
          tet_printf("\tDirect  Sendack to the no active instance");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)!=MDS_RC_MSG_NO_BUFFERING)

            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");
          /*SNDRSP*/
          tet_printf("\t Send response to the no active instance");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_vdest[1].vdest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)!=MDS_RC_MSG_NO_BUFFERING)

            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");


          /*RSP*/
          tet_printf("\tChange role to active"); 
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }

          if(tet_create_task((NCS_OS_CB)tet_Dadest_all_rcvr_thread,
                             gl_tet_adest.svc[1].task.t_handle)
             == NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }


          if(mds_direct_send_message(gl_tet_vdest[1].mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_adest.adest,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)

            {
              tet_printf("Fail");
              FAIL=1;
            }

          else
            tet_printf("Success"); 

          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");                
          fflush(stdout);                
          /*SNDRACK*/
#if 0
          tet_printf("\tChange role to active"); 
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }

          if(tet_create_task((NCS_OS_CB)tet_adest_all_rcvrack_thread,
                             gl_tet_adest.svc[1].task.t_handle)
             == NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }


          if((mds_send_get_response(gl_tet_vdest[1].mds_pwe1_hdl,
                                    NCSMDS_SVC_ID_EXTERNAL_MIN,
                                    NCSMDS_SVC_ID_EXTERNAL_MIN,
                                    gl_tet_adest.adest,
                                    3000,MDS_SEND_PRIORITY_LOW,
                                    mesg)!= NCSCC_RC_SUCCESS) )
            {
              tet_printf("Fail");
              FAIL=1;
            }

          else
            tet_printf("Success"); 

          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");                
          fflush(stdout);                
#endif
          break;
        }
      gl_set_msg_fmt_ver=0;
      if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                         NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                         svcids)!=NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");
          FAIL=1;
        }
      fflush(stdout);
      /*--------------------------------------------------------------------*/
      /*clean up*/
      if(tet_cleanup_setup())
        {
          printf("\n\t\t Setup Clean Up has Failed \n");
          FAIL=1;
        }
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n*********************************************************\n");
  fflush(stdout);

}

void tet_direct_send_ack_tp(int choice)
{
  int FAIL=0;
  int id=0;
  MDS_SVC_ID svcids[]={NCSMDS_SVC_ID_EXTERNAL_MIN};
  MDS_SVC_ID svc_ids[]={NCSMDS_SVC_ID_INTERNAL_MIN};
  char message[]="Direct Message";
  printf("\n\n*********** tet_direct_send_ack_tp %d *************\n",choice);
  tet_printf("\t\t\t tet_direct_send_ack_tp");
  /*start up*/
  if(tet_initialise_setup(false))
    { 
      tet_printf("\n\t\t Setup Initialisation has Failed \n");
      FAIL=1;
    }
  else
    {
      /*----------DIRECT SEND ----------------------------------------------*/
      if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN,
                               NCSMDS_SCOPE_NONE,1,svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      switch(choice)
        {
        case 1:
          tet_printf("Case 1 : \tNow Direct send_ack Low,Medium,High and Very \
 High Priority message to Svc EXTMIN on Active Vdest=200 in sequence");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,
                                     20,MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          /*Medium*/
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,
                                     20,MDS_SEND_PRIORITY_MEDIUM,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          /*High*/
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,
                                     20,MDS_SEND_PRIORITY_HIGH,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          /*Very High*/
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,
                                     20,MDS_SEND_PRIORITY_VERY_HIGH,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 2:
          tet_printf("Case 2 : \tNot able to send_ack a message to 2000 with \
 Invalid pwe handle");
          if(mds_direct_send_message(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDACK,
                                     gl_tet_vdest[1].vdest,20,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 3:
          tet_printf("Case 3 : \tNot able to send_ack a message to EXTMIN with\
 NULL pwe handle");
          if(mds_direct_send_message((MDS_HDL)(long)NULL,NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDACK,
                                     gl_tet_vdest[1].vdest,20,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 4:
          tet_printf("Case 4 : \tNot able to send_ack a message to Svc EXTMIN \
on Vdest=200 with Wrong DEST");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDACK,0,20,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 5:
          tet_printf("Case 5 : \tNot able to send_ack a message to service \
 which is Not installed");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,3000,1,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,
                                     20,MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 6:
          tet_printf("Case 6 : \tAble to send_ack a message 1000 times  to \
                          Svc EXTMIN on Active Vdest=200");
          for(id=1;id<=1000;id++)
            {
              if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                         NCSMDS_SVC_ID_EXTERNAL_MIN,
                                         NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                         MDS_SENDTYPE_SNDACK,
                                         gl_tet_vdest[1].vdest,20,
                                         MDS_SEND_PRIORITY_LOW,
                                         message)!=NCSCC_RC_SUCCESS)
                {    
                  tet_printf("Fail");
                  FAIL=1; 
                }
            }
          if(id==1001)
            {
              printf("\n\n\t\t ALL THE MESSAGES GOT DELIVERED\n\n");
              sleep(1);
              tet_printf("Success");
            }
          break;
        case 7:
          tet_printf("Case 7 : \tImplicit Subscription: Direct send_ack a \
 message to unsubscribed Svc INTMIN");
          /*Implicit Subscription*/
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_INTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,
                                     20,MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");

          printf("\n\n\tNow doing Explicit Subscription\n");
          /*Explicit Subscription*/
          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,svc_ids)
             !=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_INTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,
                                     20,MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");

          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svc_ids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }

          break;
        case 8:
          tet_printf("Case 8 : \tNot able to send_ack a message to Svc EXTMIN \
 with Improper Priority");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,
                                     0,5,message)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          break;
        case 9:
          tet_printf("Case 9 : \tNot able to send_ack a message with Invalid \
 Sendtype(<0 or >11)");
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     12,gl_tet_vdest[1].vdest,
                                     0,MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_SUCCESS)
            {
              tet_printf("Fail");
              FAIL=1;
            }
          else
            tet_printf("Success");
#if 0
          /*Allocating memory for the direct buffer*/
          direct_buff=m_MDS_ALLOC_DIRECT_BUFF(strlen(message)+1);
          memset(direct_buff, 0, sizeof(direct_buff));
          if(direct_buff==NULL)
            perror("Direct Buffer not allocated properly");
          memcpy(direct_buff,(uint8_t *)message,strlen(message)+1);
          svc_to_mds_info.i_mds_hdl=gl_tet_adest.mds_pwe1_hdl;
          svc_to_mds_info.i_svc_id=2000;
          svc_to_mds_info.i_op=MDS_DIRECT_SEND;
          svc_to_mds_info.info.svc_direct_send.i_direct_buff=direct_buff;
          svc_to_mds_info.info.svc_direct_send.i_direct_buff_len=
            strlen(message)+1;
          svc_to_mds_info.info.svc_direct_send.i_to_svc=2000;
          svc_to_mds_info.info.svc_direct_send.i_priority=
            MDS_SEND_PRIORITY_LOW;
          svc_to_mds_info.info.svc_direct_send.i_sendtype=12;/*wrong sendtype*/
          svc_to_mds_info.info.svc_direct_send.info.snd.i_to_dest=
            gl_tet_vdest[1].vdest;
          if(ncsmds_api(&svc_to_mds_info)==NCSCC_RC_SUCCESS)
            { tet_printf("Fail");FAIL=1;    }
          else
            {
              tet_printf("Success");
              /*Freeing Direct Buffer*/
              m_MDS_FREE_DIRECT_BUFF(direct_buff);
            }
#endif
          break;
        case 10:
          tet_printf("Case 10 :Not able to send_ack a message to Svc EXTMIN\
 with Direct Buff as NULL");
          /*Allocating memory for the direct buffer*/
          direct_buff=m_MDS_ALLOC_DIRECT_BUFF(strlen(message)+1);
          memset(direct_buff, 0, sizeof(direct_buff));
          if(direct_buff==NULL)
            perror("Direct Buffer not allocated properly");
          memcpy(direct_buff,(uint8_t *)message,strlen(message)+1);
          svc_to_mds_info.i_mds_hdl=gl_tet_adest.mds_pwe1_hdl;
          svc_to_mds_info.i_svc_id=NCSMDS_SVC_ID_EXTERNAL_MIN;
          svc_to_mds_info.i_op=MDS_DIRECT_SEND;
          svc_to_mds_info.info.svc_direct_send.i_direct_buff=NULL;/*wrong*/
          svc_to_mds_info.info.svc_direct_send.i_direct_buff_len=
            strlen(message)+1;
          svc_to_mds_info.info.svc_direct_send.i_to_svc=
            NCSMDS_SVC_ID_EXTERNAL_MIN;
          svc_to_mds_info.info.svc_direct_send.i_msg_fmt_ver=1;
          svc_to_mds_info.info.svc_direct_send.i_priority=
            MDS_SEND_PRIORITY_LOW;
          svc_to_mds_info.info.svc_direct_send.i_sendtype=MDS_SENDTYPE_SNDACK;
          svc_to_mds_info.info.svc_direct_send.info.snd.i_to_dest=
            gl_tet_vdest[1].vdest;
          if(ncsmds_api(&svc_to_mds_info)==NCSCC_RC_SUCCESS)
            { tet_printf("Fail");FAIL=1;    }
          else
            {
              tet_printf("Success");
              /*Freeing Direct Buffer*/
              m_MDS_FREE_DIRECT_BUFF(direct_buff);
            }
          break;
        case 11:
          tet_printf("Case 11 : \tNot able to send_ack a message with Invalid \
 Message length");
          /*Allocating memory for the direct buffer*/
          direct_buff=m_MDS_ALLOC_DIRECT_BUFF(strlen(message)+1);
          memset(direct_buff, 0, sizeof(direct_buff));
          if(direct_buff==NULL)
            perror("Direct Buffer not allocated properly");
          memcpy(direct_buff,(uint8_t *)message,strlen(message)+1);
          svc_to_mds_info.i_mds_hdl=gl_tet_adest.mds_pwe1_hdl;
          svc_to_mds_info.i_svc_id=NCSMDS_SVC_ID_EXTERNAL_MIN;
          svc_to_mds_info.i_op=MDS_DIRECT_SEND;
          svc_to_mds_info.info.svc_direct_send.i_direct_buff=direct_buff;
          svc_to_mds_info.info.svc_direct_send.i_direct_buff_len=
            -1;/*wrong length*/
          svc_to_mds_info.info.svc_direct_send.i_to_svc=
            NCSMDS_SVC_ID_EXTERNAL_MIN;
          svc_to_mds_info.info.svc_direct_send.i_msg_fmt_ver=1;
          svc_to_mds_info.info.svc_direct_send.i_priority=
            MDS_SEND_PRIORITY_LOW;
          svc_to_mds_info.info.svc_direct_send.i_sendtype=MDS_SENDTYPE_SNDACK;
          svc_to_mds_info.info.svc_direct_send.info.snd.i_to_dest=
            gl_tet_vdest[1].vdest;
          if(ncsmds_api(&svc_to_mds_info)==NCSCC_RC_SUCCESS)
            { tet_printf("Fail");FAIL=1;    }
          else
            tet_printf("Success");

          break;
        case 12:
          tet_printf("Case 12 : \tWhile Await Active Timer ON: Direct send_ack\
 a message to Svc EXTMIN on Vdest=200");
          if(vdest_change_role(200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          sleep(2);
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,
                                     NCSMDS_SVC_ID_INTERNAL_MIN,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_REQ_TIMOUT)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          break;
        case 13:
          tet_printf("Case 13 : \tDirect send a Medium Priority message to \
 Svc EXTMIN on QUIESCED Vdest=200");
          if(vdest_change_role(200,V_DEST_RL_QUIESCED)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          sleep(2);
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDACK,gl_tet_vdest[1].vdest,
                                     NCSMDS_SVC_ID_INTERNAL_MIN,
                                     MDS_SEND_PRIORITY_MEDIUM,
                                     message)!=NCSCC_RC_REQ_TIMOUT)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");
          if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          break;
        }
      if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                         NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                         svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      fflush(stdout);
      /*--------------------------------------------------------------------*/
      /*clean up*/
      if(tet_cleanup_setup())
        {
          perror("\n\t\t Setup Clean Up has Failed \n");
          getchar();
          FAIL=1;
        }
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n***************************************************\n");
  fflush(stdout);

}

void tet_Dadest_rcvr_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_adest_sel_obj_found(3)) )
    {
      /*Will this if leads to any problems*/
      if(mds_service_retrieve(gl_tet_adest.pwe[0].mds_pwe_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              SA_DISPATCH_ALL)==NCSCC_RC_SUCCESS)
        {
          /*after that send a response to the sender, if it expects*/ 
          if(gl_direct_rcvmsginfo.rsp_reqd)
            {
              if(mds_direct_response(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_RSP,0)==NCSCC_RC_FAILURE)
                {      
                  printf("Direct Response Fail");
                  FAIL=1;
                }
              else
                printf("\nDirect Response Success/TimeOUT");
            }
        }
    }
  fflush(stdout);  
}

void tet_Dvdest_rcvr_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_vdest_sel_obj_found(1,1)) )
    {
      mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/ 
      if(gl_direct_rcvmsginfo.rsp_reqd)
        {
          if(mds_direct_response(gl_tet_vdest[1].mds_pwe1_hdl,
                                 NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                 MDS_SENDTYPE_SNDRACK,0)!=NCSCC_RC_SUCCESS)
            {      printf("Response Fail");FAIL=1;    }
          else
            printf("Response Ack Success");
        }
    }
  fflush(stdout);  
}
void tet_Dvdest_rcvr_all_rack_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_vdest_sel_obj_found(1,1)) )
    {
      mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/
      if(gl_direct_rcvmsginfo.rsp_reqd)
        {
          if(mds_direct_response(gl_tet_vdest[1].mds_pwe1_hdl,
                                 NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                 MDS_SENDTYPE_SNDRACK,0)!=NCSCC_RC_SUCCESS)
            {      printf("Response Fail");FAIL=1;    }
          else
            printf("Response Ack Success");
        }
    }
  fflush(stdout);
}

void tet_Dvdest_rcvr_all_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_vdest_sel_obj_found(1,1)) )
    {
      mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/
      if(gl_direct_rcvmsginfo.rsp_reqd)
        {
          if(mds_direct_response(gl_tet_vdest[1].mds_pwe1_hdl,
                                 NCSMDS_SVC_ID_EXTERNAL_MIN,0,
                                 MDS_SENDTYPE_RSP,gl_set_msg_fmt_ver)!=NCSCC_RC_SUCCESS)
            {      printf("Response Fail");FAIL=1;    }
          else
            printf("Response Success");
        }
    }
  fflush(stdout);
}
void tet_Dvdest_rcvr_all_chg_role_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  printf("\n\tInside Receiver Thread");fflush(stdout);
  sleep(10);
  if(vdest_change_role(200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
    {
      tet_printf("Fail");
      FAIL=1;
    }
  if( (svc_id=is_vdest_sel_obj_found(1,1)) )
    {
      sleep(2);
      mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/
      if(gl_direct_rcvmsginfo.rsp_reqd)
        {
          if(mds_direct_response(gl_tet_vdest[1].mds_pwe1_hdl,
                                 NCSMDS_SVC_ID_EXTERNAL_MIN,0,
                                 MDS_SENDTYPE_RSP,gl_set_msg_fmt_ver)!=NCSCC_RC_SUCCESS)
            {      printf("Response Fail");FAIL=1;    }
          else
            printf("Response Success");
        }
    }
  fflush(stdout);
}

void tet_Dvdest_Srcvr_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_vdest_sel_obj_found(0,0)) )
    {
      mds_service_retrieve(gl_tet_vdest[0].mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/ 
      if(gl_direct_rcvmsginfo.rsp_reqd)
        {
          if(mds_direct_response(gl_tet_vdest[0].mds_pwe1_hdl,
                                 NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                 MDS_SENDTYPE_RRSP,200)!=NCSCC_RC_SUCCESS)
            {      printf("Red Response Fail");FAIL=1;    }
          else
            printf("Red Response Success");
        }
    
    }
  fflush(stdout);  
}
void tet_Dvdest_Srcvr_all_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_vdest_sel_obj_found(0,0)) )
    {
      mds_service_retrieve(gl_tet_vdest[0].mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/
      if(gl_direct_rcvmsginfo.rsp_reqd)
        {
          if(mds_direct_response(gl_tet_vdest[0].mds_pwe1_hdl,
                                 NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                 MDS_SENDTYPE_RRSP,200)!=NCSCC_RC_SUCCESS)
            {      printf("Red Response Fail");FAIL=1;    }
          else
            printf("Red Response Success");
        }

    }
  fflush(stdout);
}

void tet_Dvdest_Arcvr_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_vdest_sel_obj_found(0,0)) )
    {
      mds_service_retrieve(gl_tet_vdest[0].mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/ 
      if(gl_direct_rcvmsginfo.rsp_reqd)
        {
          if(mds_direct_response(gl_tet_vdest[0].mds_pwe1_hdl,
                                 NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                 MDS_SENDTYPE_REDRACK,0)!=NCSCC_RC_SUCCESS)
            {      printf("Red Response Ack Fail");FAIL=1;    }
          else
            printf("Red Response Ack Success");      
        }          
    }
  fflush(stdout);  
}

void tet_Dvdest_Arcvr_all_thread()
{
  int FAIL=0;
  MDS_SVC_ID svc_id;
  printf("\n\tInside Receiver Thread");fflush(stdout);
  if( (svc_id=is_vdest_sel_obj_found(0,0)) )
    {
      mds_service_retrieve(gl_tet_vdest[0].mds_pwe1_hdl,
                           NCSMDS_SVC_ID_EXTERNAL_MIN,SA_DISPATCH_ALL);
      /*after that send a response to the sender, if it expects*/
      if(gl_direct_rcvmsginfo.rsp_reqd)
        {
          if(mds_direct_response(gl_tet_vdest[0].mds_pwe1_hdl,
                                 NCSMDS_SVC_ID_EXTERNAL_MIN,gl_set_msg_fmt_ver,
                                 MDS_SENDTYPE_REDRACK,0)!=NCSCC_RC_SUCCESS)
            {      printf("Red Response Ack Fail");FAIL=1;    }
          else
            printf("Red Response Ack Success");
        }
    }
  fflush(stdout);
}

void tet_direct_send_response_tp(int choice)
{
  int FAIL=0;
  MDS_SVC_ID svcids[]={NCSMDS_SVC_ID_EXTERNAL_MIN};
  char message[]="Direct Message";
  void tet_Dadest_rcvr_thread();
  void tet_Dvdest_rcvr_thread();
  void tet_Dvdest_Srcvr_thread();
  void tet_Dvdest_Arcvr_thread();
  printf("\n\n*********** tet_direct_send_response_tp %d *********\n",choice);
  tet_printf("\t\t\ttet_direct_send_response_tp");
  /*Start up*/
  if(tet_initialise_setup(false))
    { 
      tet_printf("\n\t\t Setup Initialisation has Failed \n");
      FAIL=1;
    }

  else
    {
      /*-------------------------------------------------------------------*/
      if(mds_service_subscribe(gl_tet_adest.pwe[0].mds_pwe_hdl,
                               NCSMDS_SVC_ID_INTERNAL_MIN,
                               NCSMDS_SCOPE_NONE,1,svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      if(mds_service_retrieve(gl_tet_adest.pwe[0].mds_pwe_hdl,
                              NCSMDS_SVC_ID_INTERNAL_MIN,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      switch(choice)
        {
        case 1:
          tet_printf("\tCase 1 : Svc INTMIN on PWE=2 of ADEST sends a LOW \
 Priority message to Svc EXTMIN and expects a Response");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_Dadest_rcvr_thread,
                             gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/  
          if(mds_direct_send_message(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                     NCSMDS_SVC_ID_INTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_adest.adest,
                                     0,MDS_SEND_PRIORITY_LOW,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");               
          fflush(stdout); 
          break;
        case 2:
          tet_printf("\tCase 2 : Svc INTMIN on PWE=2 of ADEST sends a MEDIUM \
 Priority message to Svc EXTMIN and expects a Response");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_Dadest_rcvr_thread,
                             gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/  
          if(mds_direct_send_message(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                     NCSMDS_SVC_ID_INTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_adest.adest,
                                     0,MDS_SEND_PRIORITY_MEDIUM,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");               
          fflush(stdout); 
          break;
        case 3:
          tet_printf("\tCase 3 : Svc INTMIN on PWE=2 of ADEST sends a HIGH \
 Priority message to Svc EXTMIN and expects a Response");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_Dadest_rcvr_thread,
                             gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/  
          if(mds_direct_send_message(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                     NCSMDS_SVC_ID_INTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_adest.adest,
                                     0,MDS_SEND_PRIORITY_HIGH,
                                     message)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");               
          fflush(stdout); 
          break;
        case 4:
          tet_printf("\tCase 4 :Svc INTMIN on PWE=2 of ADEST sends a VERYHIGH\
 Priority message to Svc EXTMIN and expects a Response");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_Dadest_rcvr_thread,
                             gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/  
          if(mds_direct_send_message(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                     NCSMDS_SVC_ID_INTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_adest.adest,
                                     0,MDS_SEND_PRIORITY_VERY_HIGH,
                                     message)==NCSCC_RC_FAILURE)
            {    
              tet_printf("High Priority Direct Sent Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");               
          fflush(stdout); 
          break;                           
        case 5:
          tet_printf("\tCase 5 :Implicit/Explicit Svc INTMIN on PWE=2 of ADEST\
 sends a message to Svc EXTMIN and expects a Response");
          if(mds_service_cancel_subscription(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,
                                             1,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Subscription Cancellation Fail");
              FAIL=1; 
            }
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_Dadest_rcvr_thread,
                             gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/  
          if(mds_direct_send_message(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                     NCSMDS_SVC_ID_INTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_adest.adest,
                                     0,MDS_SEND_PRIORITY_VERY_HIGH,
                                     message)==NCSCC_RC_FAILURE)
            {    
              tet_printf("High Priority Direct Sent Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");               
          fflush(stdout); 
          sleep(3);
          if(mds_service_subscribe(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_Dadest_rcvr_thread,
                             gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender*/  
          if(mds_direct_send_message(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                     NCSMDS_SVC_ID_INTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDRSP,gl_tet_adest.adest,
                                     0,MDS_SEND_PRIORITY_HIGH,
                                     message)==NCSCC_RC_FAILURE)
            {    
              tet_printf("High Priority Direct Sent Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_adest.svc[2].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");               
          fflush(stdout); 
          break;                           
        }
      if(mds_service_cancel_subscription(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                         NCSMDS_SVC_ID_INTERNAL_MIN,
                                         1,svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Subscription Cancellation Fail");
          FAIL=1; 
        }
      fflush(stdout);    
      /*-------------------------------------------------------------*/
      /*clean up*/
      if(tet_cleanup_setup())
        {
          perror("\n\t\t Setup Clean Up has Failed \n");
          getchar();
          FAIL=1;
        }
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n***********************************************\n");
  fflush(stdout);
}
void tet_direct_send_response_ack_tp(int choice)
{
  int FAIL=0;
  MDS_SVC_ID svcids[]={NCSMDS_SVC_ID_EXTERNAL_MIN};
  char message[]="Direct Message";
  void tet_Dvdest_rcvr_thread();
  printf("\n\n*********** tet_direct_send_response_ack_tp %d ******\n",choice);
  tet_printf("\t\t\ttet_direct_send_response_ack_tp");
  /*Start up*/
  if(tet_initialise_setup(false))
    {
      tet_printf("\n\t\t Setup Initialisation has Failed \n");
      FAIL=1;
    }

  else
    {
      /*-----------------------------------------------------------------*/
      if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                               NCSMDS_SVC_ID_EXTERNAL_MIN,
                               NCSMDS_SCOPE_NONE,1,svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                              NCSMDS_SVC_ID_EXTERNAL_MIN,
                              SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
         
      switch(choice)
        {
        case 1:
          tet_printf("\tCase 1 : Svc EXTMIN on ADEST sends a LOW priority \
 message to Svc EXTMIN on VDEST=200 and expects a Response");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_Dvdest_rcvr_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */ 
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDRSP,200,0,
                                     MDS_SEND_PRIORITY_LOW,
                                     message)==NCSCC_RC_FAILURE)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          sleep(2); /*in 16.0.2*/
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");               
          fflush(stdout); 
          break;
        case 2:
          tet_printf("\tCase 2 : Svc EXTMIN on ADEST sends a MEDIUM priority \
 message to Svc EXTMIN on VDEST=200 and expects a Response");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_Dvdest_rcvr_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */ 
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDRSP,200,0,
                                     MDS_SEND_PRIORITY_MEDIUM,
                                     message)==NCSCC_RC_FAILURE)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          sleep(2); /*in 16.0.2*/
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");               
          fflush(stdout); 
          break;
        case 3:
          tet_printf("\tCase 3 : Svc EXTMIN on ADEST sends a HIGH priority \
 message to Svc EXTMIN on VDEST=200 and expects a Response");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_Dvdest_rcvr_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */ 
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDRSP,200,0,
                                     MDS_SEND_PRIORITY_HIGH,
                                     message)==NCSCC_RC_FAILURE)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");               
          fflush(stdout); 
          break;
        case 4:
          tet_printf("\tCase 4 : Svc EXTMIN on ADEST sends a VERYHIGH priority\
 message to Svc EXTMIN on VDEST=200 and expects a Response");
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_Dvdest_rcvr_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */ 
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDRSP,200,0,
                                     MDS_SEND_PRIORITY_VERY_HIGH,
                                     message)==NCSCC_RC_FAILURE)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          sleep(2); /*in 16.0.2*/
          /*Now Stop and Release the Receiver Thread*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");               
          fflush(stdout); 
          break;                           
        case 5:
          tet_printf("\tCase 5 :Implicit/Explicit Svc EXTMIN on ADEST sends a \
 message to Svc EXTMIN on VDEST=200 and expects a Response");
          if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                             svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_Dvdest_rcvr_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */ 
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDRSP,200,0,
                                     MDS_SEND_PRIORITY_VERY_HIGH,
                                     message)==NCSCC_RC_FAILURE)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          sleep(2); /*in 16.0.2*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");               
          fflush(stdout); 
          if(mds_service_subscribe(gl_tet_adest.mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_EXTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_EXTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          /*Receiver thread*/
          if(tet_create_task((NCS_OS_CB)tet_Dvdest_rcvr_thread,
                             gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS )
            {
              printf("\tTask has been Created\t");fflush(stdout);
            }
          /*Sender */ 
          if(mds_direct_send_message(gl_tet_adest.mds_pwe1_hdl,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,
                                     NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                     MDS_SENDTYPE_SNDRSP,200,0,
                                     MDS_SEND_PRIORITY_HIGH,
                                     message)==NCSCC_RC_FAILURE)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success"); 
          /*Now Stop and Release the Receiver Thread*/
          sleep(2); /*in 16.0.2*/
          if(m_NCS_TASK_RELEASE(gl_tet_vdest[1].svc[1].task.t_handle)
             ==NCSCC_RC_SUCCESS)
            printf("\tTASK is released\n");               
          fflush(stdout); 
          break;                           
        }
      /*-----------------------------------------------------------*/
        
      if(mds_service_cancel_subscription(gl_tet_adest.mds_pwe1_hdl,
                                         NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                         svcids)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      fflush(stdout);
    
      /*clean up*/
      if(tet_cleanup_setup())
        {
          perror("\n\t\t Setup Clean Up has Failed \n");
          getchar();
          FAIL=1;
        }
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n***********************************************\n");
  fflush(stdout);
}

void tet_direct_broadcast_to_svc_tp(int choice)
{
  int FAIL=0;
  MDS_SVC_ID svcids[]={NCSMDS_SVC_ID_EXTERNAL_MIN};
  printf("\n\n******* tet_direct_broadcast_to_svc_tp %d **********\n",choice);
  tet_printf("\t\t\ttet_direct_broadcast_to_svc_tp");
  /*Start up*/
  if(tet_initialise_setup(false))
    { printf("\n\t\t Setup Initialisation has Failed \n");}
  else
    {
      /*-------------------------------------------------------------*/
      switch(choice)
        {
        case 1:
          tet_printf("\tCase 1 : Svc INTMIN on VDEST=200 Broadcasting a Low \
 Priority message to Svc EXTMIN");
          if(mds_service_subscribe(gl_tet_vdest[1].mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_direct_broadcast_message(gl_tet_vdest[1].mds_pwe1_hdl,
                                          NCSMDS_SVC_ID_INTERNAL_MIN,
                                          NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                          MDS_SENDTYPE_BCAST,
                                          NCSMDS_SCOPE_NONE,
                                          MDS_SEND_PRIORITY_LOW)
             !=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");   
          if(mds_service_cancel_subscription(gl_tet_vdest[1].mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,
                                             1,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 2:
          tet_printf("\tCase 2 :Svc INTMIN on VDEST=200 Broadcasting a Medium\
 Priority message to Svc EXTMIN");
          if(mds_service_subscribe(gl_tet_vdest[1].mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_direct_broadcast_message(gl_tet_vdest[1].mds_pwe1_hdl,
                                          NCSMDS_SVC_ID_INTERNAL_MIN,
                                          NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                          MDS_SENDTYPE_BCAST,
                                          NCSMDS_SCOPE_NONE,
                                          MDS_SEND_PRIORITY_MEDIUM)
             !=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");   
          if(mds_service_cancel_subscription(gl_tet_vdest[1].mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,
                                             1,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 3:
          tet_printf("\tCase 3 : Svc INTMIN on VDEST=200 Broadcasting a High \
 Priority message to Svc EXTMIN");
          if(mds_service_subscribe(gl_tet_vdest[1].mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,
                                   svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_direct_broadcast_message(gl_tet_vdest[1].mds_pwe1_hdl,
                                          NCSMDS_SVC_ID_INTERNAL_MIN,
                                          NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                          MDS_SENDTYPE_BCAST,
                                          NCSMDS_SCOPE_NONE,
                                          MDS_SEND_PRIORITY_HIGH)
             !=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");   
          if(mds_service_cancel_subscription(gl_tet_vdest[1].mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,
                                             1,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 4:
          tet_printf("\tCase 4 : Svc INTMIN on VDEST=200 Broadcasting a Very \
 High Priority message to Svc EXTMIN");
          if(mds_service_subscribe(gl_tet_vdest[1].mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,svcids)
             !=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_direct_broadcast_message(gl_tet_vdest[1].mds_pwe1_hdl,
                                          NCSMDS_SVC_ID_INTERNAL_MIN,
                                          NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                          MDS_SENDTYPE_BCAST,
                                          NCSMDS_SCOPE_NONE,
                                          MDS_SEND_PRIORITY_VERY_HIGH)
             !=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");   
          if(mds_service_cancel_subscription(gl_tet_vdest[1].mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,
                                             1,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;                           
        case 5:
          tet_printf("\tCase 5 : Svc INTMIN on ADEST Broadcasting a message to\
 Svc EXTMIN");
          if(mds_service_subscribe(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,svcids)
             !=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_direct_broadcast_message(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                          NCSMDS_SVC_ID_INTERNAL_MIN,
                                          NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                          MDS_SENDTYPE_BCAST,
                                          NCSMDS_SCOPE_NONE,
                                          MDS_SEND_PRIORITY_LOW)
             !=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");     
          if(mds_service_cancel_subscription(gl_tet_adest.pwe[0].mds_pwe_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,
                                             1,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 6:
          tet_printf("\tCase 6 :Svc INTMIN on VDEST=200 Redundant Broadcasting\
 a message to Svc EXTMIN");
          if(mds_service_subscribe(gl_tet_vdest[1].mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,svcids)
             !=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_direct_broadcast_message(gl_tet_vdest[1].mds_pwe1_hdl,
                                          NCSMDS_SVC_ID_INTERNAL_MIN,
                                          NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                          MDS_SENDTYPE_RBCAST,
                                          NCSMDS_SCOPE_NONE,
                                          MDS_SEND_PRIORITY_LOW)
             !=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");     
          if(mds_service_cancel_subscription(gl_tet_vdest[1].mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,
                                             1,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;
        case 7:
          tet_printf("\tCase 7 :Svc INTMIN on VDEST=200 Not able to Broadcast\
a  message with Invalid Priority");
          if(mds_service_subscribe(gl_tet_vdest[1].mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,svcids)
             !=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_direct_broadcast_message(gl_tet_vdest[1].mds_pwe1_hdl,
                                          NCSMDS_SVC_ID_INTERNAL_MIN,
                                          NCSMDS_SVC_ID_EXTERNAL_MIN,1,
                                          MDS_SENDTYPE_BCAST,
                                          NCSMDS_SCOPE_NONE,0)
             ==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");   
          if(mds_service_cancel_subscription(gl_tet_vdest[1].mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,
                                             1,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;

        case 8:
          tet_printf("\tCase 8 :Svc INTMIN on VDEST=200 Not able to Broadcast\
a message with NULL Direct Buff");
          if(mds_service_subscribe(gl_tet_vdest[1].mds_pwe1_hdl,
                                   NCSMDS_SVC_ID_INTERNAL_MIN,
                                   NCSMDS_SCOPE_NONE,1,svcids)
             !=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          if(mds_service_retrieve(gl_tet_vdest[1].mds_pwe1_hdl,
                                  NCSMDS_SVC_ID_INTERNAL_MIN,
                                  SA_DISPATCH_ALL)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          svc_to_mds_info.i_mds_hdl=gl_tet_vdest[1].mds_pwe1_hdl;
          svc_to_mds_info.i_svc_id=NCSMDS_SVC_ID_INTERNAL_MIN;
          svc_to_mds_info.i_op=MDS_DIRECT_SEND;    
  
          svc_to_mds_info.info.svc_direct_send.i_direct_buff=NULL;
          svc_to_mds_info.info.svc_direct_send.i_direct_buff_len=20;
          svc_to_mds_info.info.svc_direct_send.i_to_svc=
            NCSMDS_SVC_ID_EXTERNAL_MIN;
          svc_to_mds_info.info.svc_direct_send.i_msg_fmt_ver=1;
          svc_to_mds_info.info.svc_direct_send.i_priority
            = MDS_SEND_PRIORITY_LOW;
          svc_to_mds_info.info.svc_direct_send.i_sendtype=MDS_SENDTYPE_BCAST;
          svc_to_mds_info.info.svc_direct_send.info.bcast.i_bcast_scope
            = NCSMDS_SCOPE_NONE;
  
          if(ncsmds_api(&svc_to_mds_info)==NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          else
            tet_printf("Success");   

          if(mds_service_cancel_subscription(gl_tet_vdest[1].mds_pwe1_hdl,
                                             NCSMDS_SVC_ID_INTERNAL_MIN,
                                             1,svcids)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
          fflush(stdout);
          break;                
        }
      /*----------------------------------------------------------------*/
      /*clean up*/
      if(tet_cleanup_setup())
        {
          perror("\n\t\t Setup Clean Up has Failed \n");
          getchar();
          FAIL=1;
        }
    }

  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n*************************************************\n");
  fflush(stdout);
}

/****************************************************************/
/**************             ADEST TEST PUPOSES      *************/
/****************************************************************/
void tet_destroy_PWE_ADEST_twice_tp()
{
  int FAIL=0;
  tet_printf("            tet_destroy_PWE_ADEST_twice_tp");
  printf("\n*********tet_destroy_PWE_ADEST_twice_tp*****************\n");    
  /*start up*/
  memset(&gl_tet_adest,'\0', sizeof(gl_tet_adest)); /*zeroizing*/
    
    
  tet_printf("Getting an ADEST handle");
  if(adest_get_handle()!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
    
  tet_printf("Creating a PWE=2 on this ADEST");
  if(create_pwe_on_adest(gl_tet_adest.mds_adest_hdl,2)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
    
  tet_printf("Destroying the PWE=2 on this ADEST");
  if(destroy_pwe_on_adest(gl_tet_adest.pwe[0].mds_pwe_hdl)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
    
  tet_printf("Not able to Destroy ALREADY destroyed PWE=2 on this ADEST");
  if(destroy_pwe_on_adest(gl_tet_adest.pwe[0].mds_pwe_hdl)==NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
    
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n************************************************************\n");  
}
void tet_create_PWE_ADEST_twice_tp()
{
  int FAIL=0;
  tet_printf("            tet_create_PWE_ADEST_twice_tp");
  printf("\n**************tet_create_PWE_ADEST_twice_tp*************\n");     
  /*start up*/
  memset(&gl_tet_adest,'\0', sizeof(gl_tet_adest)); /*zeroizing*/
    
    
  tet_printf("Getting an ADEST handle");
  if(adest_get_handle()!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
    
  tet_printf("Creating a PWE=2 on this ADEST");
  if(create_pwe_on_adest(gl_tet_adest.mds_adest_hdl,2)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
    
  tet_printf("Able Create ALREADY created PWE=2 on this ADEST");
  if(create_pwe_on_adest(gl_tet_adest.mds_adest_hdl,2)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  else
    destroy_pwe_on_adest(gl_tet_adest.pwe[0].mds_pwe_hdl);
    
    
  /*clean up*/
  tet_printf("Destroying the PWE=2 on this ADEST");
  if(destroy_pwe_on_adest(gl_tet_adest.pwe[0].mds_pwe_hdl)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
    
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n*************************************************\n");         
}
void tet_create_default_PWE_ADEST_tp()
{
  int FAIL=0;
  tet_printf("            tet_create_default_PWE_ADEST_tp");
  printf("\n*********tet_create_default_PWE_ADEST_tp***************\n");     
  /*start up*/
  memset(&gl_tet_adest,'\0', sizeof(gl_tet_adest)); /*zeroizing*/
    
  tet_printf("Getting an ADEST handle");
  if(adest_get_handle()!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
    
  tet_printf("Able to Create a DEFUALT PWE=1 on this ADEST");
  if(create_pwe_on_adest(gl_tet_adest.mds_adest_hdl,1)!=NCSCC_RC_SUCCESS)
    {      tet_printf("Fail");
    FAIL=1;
    }
  else
    {
      if(destroy_pwe_on_adest(gl_tet_adest.pwe[0].mds_pwe_hdl)
         !=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      tet_printf("Success");       
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);
  printf("\n***********************************************************\n");   
}
void tet_create_PWE_upto_MAX_tp()
{
  int id;
  int FAIL=0;
  tet_printf("            tet_create_PWE_upto_MAX_tp");
  printf("\n**********tet_create_PWE_upto_MAX_tp****************\n");     
  /*start up*/
  memset(&gl_tet_adest,'\0', sizeof(gl_tet_adest)); /*zeroizing*/
    
  tet_printf("Getting an ADEST handle");
  if(adest_get_handle()!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
    
  printf("\n\t No. of PWEs = %d \t",gl_tet_adest.pwe_count);
  fflush(stdout);sleep(1);
  tet_printf("Creating PWEs with pwe_id from 2 to 1999");
  for(id=2;id<=NCSMDS_MAX_PWES;id++)
    {
      create_pwe_on_adest(gl_tet_adest.mds_adest_hdl,id);
    }
  printf("\n\t No. of PWEs = %d \t",gl_tet_adest.pwe_count);
  fflush(stdout);sleep(1);
  if(gl_tet_adest.pwe_count==NCSMDS_MAX_PWES-1)
    tet_printf("Success: %d PWEs got created",gl_tet_adest.pwe_count);
  else
    {
      tet_printf("Failed to Create the PWE=2000");tet_result(TET_FAIL);
    }
  tet_printf("Destroying all the PWEs from 2 to 1999");
  for(id=gl_tet_adest.pwe_count-1;id>=0;id--)
    {
      destroy_pwe_on_adest(gl_tet_adest.pwe[id].mds_pwe_hdl);
    }
  printf("\n\t No. of PWEs = %d \t",gl_tet_adest.pwe_count);
  if(gl_tet_adest.pwe_count==0)   
    {
      tet_printf("Success: All the PWEs got Destroyed");
      tet_result(TET_PASS);
    }
  else
    {
      tet_printf("Fail");
      tet_result(TET_FAIL);
    }
  printf("\n*********************************************************\n");
}

/********************************************************************/
/**************       VDEST TEST PUPOSES                  ***********/
/********************************************************************/
void tet_create_MxN_VDEST(int choice)
{
  int FAIL=0;
  tet_printf("          tet_create_MxN_VDEST");
  gl_vdest_indx=0;
  printf("\n**********tet_create_MxN_VDEST %d *************\n",choice);
  /*Start up*/
  memset(&gl_tet_vdest,'\0', sizeof(gl_tet_vdest)); /*zeroizing*/
  switch(choice)
    {
    case 1:
      tet_printf("Creating a VDEST in MXN model with MIN vdest_id\n");
      if(create_vdest(NCS_VDEST_TYPE_MxN,
                      NCSVDA_EXTERNAL_UNNAMED_MIN)==NCSCC_RC_SUCCESS)
        {
          if(destroy_vdest(NCSVDA_EXTERNAL_UNNAMED_MIN)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
        }  
      else
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      break;
    case 2:   
      tet_printf("Not able to create a VDEST with vdest_id above the \
 MAX RANGE");
      if(create_vdest(NCS_VDEST_TYPE_MxN,
                      NCSVDA_EXTERNAL_UNNAMED_MAX+1)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      break;   
    case 3:
      tet_printf("Not able to create a VDEST with vdest_id = 0");
      if(create_vdest(NCS_VDEST_TYPE_MxN,0)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      break;   
    case 4:
      tet_printf("Create a VDEST with vdest_id below the MIN RANGE");   
      if(create_vdest(NCS_VDEST_TYPE_MxN,
                      NCSVDA_EXTERNAL_UNNAMED_MIN-1)==NCSCC_RC_SUCCESS)
        {
          if(destroy_vdest(NCSVDA_EXTERNAL_UNNAMED_MIN-1)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
        }
      else
        {    
          tet_printf("Fail");
          FAIL=1; 
        }    
      break;
    }    
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);  
  printf("\n************************************************************\n");  
}
void tet_create_Nway_VDEST(int choice)
{
  int FAIL=0;
  tet_printf("          tet_create_Nway_VDEST");
  gl_vdest_indx=0;
    
  printf("\n**************tet_create_Nway_VDEST %d ***********\n",choice);
    
  memset(&gl_tet_vdest,'\0', sizeof(gl_tet_vdest)); /*zeroizing*/
  switch(choice)
    {
    case 1:
      tet_printf("Creating a VDEST in N-way model with MAX vdest_id\n");
      if(create_vdest(NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,
                      NCSVDA_EXTERNAL_UNNAMED_MAX)==NCSCC_RC_SUCCESS)
        {
          if(destroy_vdest(NCSVDA_EXTERNAL_UNNAMED_MAX)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
        }  
      else
        {    
          tet_printf("Fail");
          FAIL=1; 
        }  
      break;
    case 2:   
      tet_printf("Not able to create a VDEST with vdest_id above the \
 MAX RANGE");
      if(create_vdest(NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,
                      NCSVDA_EXTERNAL_UNNAMED_MAX+1)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }  
      break;
    case 3:
      tet_printf("Not able to create a VDEST with vdest_id = 0");
      if(create_vdest(NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,
                      0)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      break;
    case 4:
      tet_printf("Create a VDEST with vdest_id below the MIN RANGE");   
      if(create_vdest(NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,
                      NCSVDA_EXTERNAL_UNNAMED_MIN-1)==NCSCC_RC_SUCCESS)
        {
          if(destroy_vdest(NCSVDA_EXTERNAL_UNNAMED_MIN-1)!=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
        }
      else
        {    
          tet_printf("Fail");
          FAIL=1; 
        }    
      break;
    }   
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);    
  printf("\n*******************************************************\n"); 
}
void tet_create_OAC_VDEST_tp(int choice)
{
  int FAIL=0;
  tet_printf("          tet_create_OAC_VDEST_tp");
  gl_vdest_indx=0;
  printf("\n********** tet_create_OAC_VDEST_tp %d ********\n",choice);
  /*Start up*/
  memset(&gl_tet_vdest,'\0', sizeof(gl_tet_vdest)); /*zeroizing*/
  switch(choice)
    {
    case 1:
      tet_printf("Creating a OAC service on VDEST in MXN model\n");
      if(create_vdest(NCS_VDEST_TYPE_MxN,
                      NCSVDA_EXTERNAL_UNNAMED_MIN)==NCSCC_RC_SUCCESS)
        {
          sleep(2);/*Introduced in Build 12*/
          if(destroy_vdest(NCSVDA_EXTERNAL_UNNAMED_MIN)!=NCSCC_RC_SUCCESS)
            {    
              perror("\n  VDEST_DESTROY is FAILED");
              fflush(stdout);
              FAIL=1; 
            }
        }  
      else
        FAIL=1;
      break;
    case 2:
      tet_printf("Creating a a OAC service on VDEST in N-way model\n");
      if(create_vdest(NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,
                      NCSVDA_EXTERNAL_UNNAMED_MAX)==NCSCC_RC_SUCCESS)
        {
          sleep(2);/*Introduced in Build 12*/
          if(destroy_vdest(NCSVDA_EXTERNAL_UNNAMED_MAX)!=NCSCC_RC_SUCCESS)
            {    
              perror("\n  VDEST_DESTROY is FAILED");
              fflush(stdout);
              FAIL=1; 
            }
        }  
      else
        FAIL=1;    
      break;
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);    
  printf("\n***************************************************\n");     
}
void tet_destroy_VDEST_twice_tp(int choice)
{
  int FAIL=0;
  tet_printf("                tet_destroy_VDEST_twice_tp");
  gl_vdest_indx=0;
  printf("\n************tet_destroy_VDEST_twice_tp %d ************\n",choice);
  /*Start UP*/
  memset(&gl_tet_vdest,'\0', sizeof(gl_tet_vdest)); /*zeroizing*/
  switch(choice)
    {
    case 1:
      tet_printf("Creating a VDEST in N-WAY model");
      if(create_vdest(NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,
                      NCSVDA_EXTERNAL_UNNAMED_MIN)!=NCSCC_RC_SUCCESS)
        {      tet_printf("Fail");FAIL=1;    }
  
      /*clean up*/   
      tet_printf("Destroying the above VDEST");
      if(destroy_vdest(NCSVDA_EXTERNAL_UNNAMED_MIN)!=NCSCC_RC_SUCCESS)
        {      tet_printf("Fail");FAIL=1;    }
      break;
    case 2:
      tet_printf("Not able to Destroy ALREADY destroyed VDEST");
      if(destroy_vdest(NCSVDA_EXTERNAL_UNNAMED_MIN)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      break;
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);    
  printf("\n******************************************************\n");     
}
void tet_destroy_ACTIVE_MxN_VDEST_tp(int choice)
{
  int FAIL=0;
    
  gl_vdest_indx=0;
  printf("\n*********tet_destroy_ACTIVE_MxN_VDEST_tp %d **********\n",choice);
  /*Start UP*/
  memset(&gl_tet_vdest,'\0', sizeof(gl_tet_vdest)); /*zeroizing*/

  switch(choice)
    {
    case 1:
      tet_printf("                  tet_destroy_ACTIVE_MxN_VDEST_tp");
      tet_printf("Creating a VDEST in MxN model");
      if(create_vdest(NCS_VDEST_TYPE_MxN,1400)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }   
      tet_printf("Changing VDEST role to ACTIVE");
      if(vdest_change_role(1400,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      break;
    case 2:
      tet_printf("                  tet_destroy_QUIESCED_MxN_VDEST_tp");
      tet_printf("Creating a VDEST in MxN model");
      if(create_vdest(NCS_VDEST_TYPE_MxN,1400)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }   
      if(vdest_change_role(1400,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      tet_printf("Changing VDEST role to QUIESCED");    
      if(vdest_change_role(1400,V_DEST_RL_QUIESCED)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }   
      break;
    }
  /*clean up*/ 
  tet_printf("Destroying the above VDEST");
  if(destroy_vdest(1400)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
    
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);    
  printf("\n********************************************************\n");     
}
void tet_chg_standby_to_queisced_tp(int choice)
{
  int FAIL=0;
  tet_printf("                  tet_chg_standby_to_queisced_tp");
  gl_vdest_indx=0;
  printf("\n*********tet_chg_standby_to_queisced_tp %d ***********\n",choice);
  /*Start UP*/
  memset(&gl_tet_vdest,'\0', sizeof(gl_tet_vdest)); /*zeroizing*/
  tet_printf("Creating a VDEST in N-way model");
  if(create_vdest(NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,
                  1200)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  switch(choice)
    {
    case 1:   
      tet_printf("Changing VDEST role to ACTIVE");
      if(vdest_change_role(1200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
        
      tet_printf("Again Changing VDEST role to ACTIVE");
      if(vdest_change_role(1200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      break;
    case 2: 
      tet_printf("Changing VDEST role to ACTIVE");
      if(vdest_change_role(1200,V_DEST_RL_ACTIVE)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
        
      tet_printf("Changing VDEST role from ACTIVE to QUIESCED");
      if(vdest_change_role(1200,V_DEST_RL_QUIESCED)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      break;
    case 3: 
      tet_printf("Changing VDEST role from QUIESCED to STANDBY");
      if(vdest_change_role(1200,V_DEST_RL_STANDBY)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      break;
    case 4:
      tet_printf("Not able to Change VDEST role from STANDBY to QUIESCED");
      if(vdest_change_role(1200,V_DEST_RL_QUIESCED)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      break;
    }    
  /*clean up*/ 
  tet_printf("Destroying the above VDEST");
  if(destroy_vdest(1200)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);    
  printf("\n********************************************************\n");     
}
void tet_create_named_VDEST(int choice)
{
  int FAIL=0;
  char vname[10]="MY_VDEST";
  char null[1]="\0";
  MDS_DEST vdest_id,V_id;
  gl_vdest_indx=0;
  tet_printf("                 tet_create_named_VDEST");
  printf("\n************tet_create_named_VDEST %d ************\n",choice);
  /*start up*/
  memset(&gl_tet_vdest,'\0', sizeof(gl_tet_vdest)); /*zeroizing*/
    
  switch(choice)
    {
    case 1:
      tet_printf("Not able to Create a Named VDEST in MxN model with \
 Persistence Flag= true");
      if(create_named_vdest(TRUE,NCS_VDEST_TYPE_MxN,
                            vname)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");
      break;    
    case 2:
      tet_printf("Not able to Create a Named VDEST in N-Way model with\
 Persistence Flag= true");
      if(create_named_vdest(TRUE,NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,
                            vname)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");
      break;    
    case 3:
      tet_printf("Able to Create a Named VDEST in MxN model with Persistence\
 Flag=false");
      if(create_named_vdest(false,NCS_VDEST_TYPE_MxN,
                            vname)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");
      tet_printf("Looking for VDEST id associated with this Named VDEST");
      vdest_id=vdest_lookup(vname);
      if(vdest_id)
        tet_printf("Named VDEST= %s : %d Vdest id",vname,vdest_id);    
      else
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      tet_printf("Destroying the above Named VDEST in MxN model");
      if(destroy_named_vdest(false,vdest_id,vname)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      break;
    case 4:
      tet_printf("Able to Create a Named VDEST in MxN model with OAC \
 Flag= true");
      if(create_named_vdest(false,NCS_VDEST_TYPE_MxN,
                            vname)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");
      tet_printf("Looking for VDEST id associated with this Named VDEST");
      vdest_id=vdest_lookup(vname);
      if(vdest_id)
        tet_printf("Success: Named VDEST= %s : %d Vdest id",vname,vdest_id);   
      else
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      tet_printf("Destroying the above Named VDEST in MxN model");
      if(destroy_named_vdest(false,vdest_id,vname)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      break;    
    case 5:
      tet_printf("Able to Create a Named VDEST in N-Way model with \
  Persistence Flag=false");
      if(create_named_vdest(false,NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,
                            vname)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");
      tet_printf("Looking for VDEST id associated with this Named VDEST");
      vdest_id=vdest_lookup(vname);
      if(vdest_id)
        tet_printf("Success: Named VDEST= %s : %d Vdest id",vname,vdest_id);   
      else
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      tet_printf("Destroying the above Named VDEST in MxN model");
      if(destroy_named_vdest(false,vdest_id,vname)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      break;
    case 6:
      tet_printf("Able to Create a Named VDEST in N-Way model with OAC \
 Flag= true");
      if(create_named_vdest(false,NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,
                            vname)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");
      tet_printf("Looking for VDEST id associated with this Named VDEST");
      vdest_id=vdest_lookup(vname);
      if(vdest_id)
        tet_printf("Success: Named VDEST= %s : %d Vdest id",vname,vdest_id);   
      else
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      tet_printf("Destroying the above Named VDEST in MxN model");
      if(destroy_named_vdest(false,vdest_id,vname)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      break;    
    case 7:
      tet_printf("Check the vdest_id returned: if we Create a Named VDEST in\
 N-Way model for the second time");
      if(create_named_vdest(false,NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,
                            vname)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      vdest_id=vdest_lookup(vname);
      tet_printf("First time : Named VDEST= %s : %d Vdest id",vname,vdest_id); 
      if(create_named_vdest(false,NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,
                            vname)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      V_id=vdest_lookup(vname);
      tet_printf("Second time: Named VDEST= %s : %d Vdest id",vname,V_id);     
      if(vdest_id==V_id)
        tet_printf("Success");
      else
        {
          tet_printf("Not The Same Vdest_id returned");  
          FAIL=1;
        }  
      if(destroy_named_vdest(false,vdest_id,vname)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      if(destroy_named_vdest(false,vdest_id,vname)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }   
      break;            
    case 8:
      tet_printf("Looking for VDEST id associated with a NULL name");
      V_id=vdest_lookup(NULL);
      if(V_id==NCSCC_RC_FAILURE)
        tet_printf("Success");
      else
        FAIL=1; 
      tet_printf("Named VDEST= %s : %d Vdest id",null,V_id);
      break;
    case 9:
      tet_printf("Not able to Create a Named VDEST in MxN model with NULL\
 name");
      if(create_named_vdest(false,NCS_VDEST_TYPE_MxN,
                            NULL)==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      else
        tet_printf("Success");
      break;    
    }
    
  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);  
  printf("\n*********************************************************\n");     
}
void tet_test_PWE_VDEST_tp(int choice)
{
  int FAIL=0;
  tet_printf("                  tet_test_PWE_VDEST_tp");
  gl_vdest_indx=0;
  printf("\n**************tet_test_PWE_VDEST_tp %d ************\n",choice);
  /*Start UP*/
  memset(&gl_tet_vdest,'\0', sizeof(gl_tet_vdest)); /*zeroizing*/
    
  tet_printf("Creating a VDEST in MxN model");
  if(create_vdest(NCS_VDEST_TYPE_MxN,1250)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  switch(choice)
    {
    case 1: 
      tet_printf("Creating a PWE with MAX PWE_id= 2000 on this VDEST");
      if(create_pwe_on_vdest(gl_tet_vdest[0].mds_vdest_hdl,NCSMDS_MAX_PWES)==NCSCC_RC_SUCCESS)
        {
          destroy_pwe_on_vdest(gl_tet_vdest[0].pwe[0].mds_pwe_hdl);
        } 
      else
        {    
          tet_printf("Fail");
          FAIL=1; 
        } 
      break;
    case 2:
      tet_printf("Not able to Create a PWE with PWE_id > MAX i.e > 2000 on \
 this VDEST");
      if(create_pwe_on_vdest(gl_tet_vdest[0].mds_vdest_hdl,NCSMDS_MAX_PWES+1)==NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");FAIL=1;
          destroy_pwe_on_vdest(gl_tet_vdest[0].pwe[0].mds_pwe_hdl);
        } 
      break;
    case 3: 
      tet_printf("Not able to Create a PWE with Invalid PWE_id = 0 on this \
 VDEST");
      if(create_pwe_on_vdest(gl_tet_vdest[0].mds_vdest_hdl,0)==NCSCC_RC_SUCCESS)
        {
          tet_printf("Fail");FAIL=1;
          if(destroy_pwe_on_vdest(gl_tet_vdest[0].pwe[0].mds_pwe_hdl)
             !=NCSCC_RC_SUCCESS)
            {    
              tet_printf("Fail");
              FAIL=1; 
            }
        } 
      break;   
    case 4:
      tet_printf("Creating a PWE with PWE_id= 10 on this VDEST");
      if(create_pwe_on_vdest(gl_tet_vdest[0].mds_vdest_hdl,10)!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
        
      tet_printf("Destroying the PWE=10 on this VDEST");    
      if(destroy_pwe_on_vdest(gl_tet_vdest[0].pwe[0].mds_pwe_hdl)
         !=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
      break;
    case 5:   
      tet_printf("Not able to Destroy an Already Destroyed PWE on this VDEST");
      if(destroy_pwe_on_vdest(gl_tet_vdest[0].pwe[0].mds_pwe_hdl)
         ==NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        } 
      break;
    case 6:   
      tet_printf("Creating a PWE with PWE_id= 20 and OAC service on this \
 VDEST");
      if(create_pwe_on_vdest(gl_tet_vdest[0].mds_vdest_hdl,20)==NCSCC_RC_SUCCESS)
        {
          sleep(2);
          destroy_pwe_on_vdest(gl_tet_vdest[0].pwe[0].mds_pwe_hdl);
        }    
      else
        {    
          tet_printf("Fail");
          FAIL=1; 
        }   
      break;
    }
  /*clean up*/
  tet_printf("Destroying the above VDEST");
  if(destroy_vdest(1250)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
    
  if(FAIL)
    tet_result(TET_FAIL);   
  else
    tet_result(TET_PASS);
  printf("\n**************************************************************\n");
}
void tet_create_PWE_upto_MAX_VDEST()
{
  int FAIL=0;
  int id=0;
  tet_printf("                  tet_create_PWE_upto_MAX_VDEST");
  gl_vdest_indx=0;
  printf("\n*************************tet_create_PWE_upto_MAX_VDEST********\n");
  /*Start UP*/
  memset(&gl_tet_vdest,'\0', sizeof(gl_tet_vdest)); /*zeroizing*/
    
  tet_printf("Creating a VDEST in MxN model");
  if(create_vdest(NCS_VDEST_TYPE_MxN,1200)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
    
  tet_printf("Creating PWEs from 2 to 2000");
  gl_tet_vdest[0].pwe_count=0;/*FIX THIS*/
  printf("\npwe_count = %d",gl_tet_vdest[0].pwe_count);
  fflush(stdout);
  sleep(1);
  for(id=2;id<=NCSMDS_MAX_PWES;id++)
    {
      create_pwe_on_vdest(gl_tet_vdest[0].mds_vdest_hdl,id);
      printf("\tpwe_count = %d\t",gl_tet_vdest[0].pwe_count);
    }   
  fflush(stdout);sleep(1);
  if(gl_tet_vdest[0].pwe_count==NCSMDS_MAX_PWES-1)
    tet_printf("Success: %d",gl_tet_vdest[0].pwe_count);
  else
    tet_printf("Fail");
  tet_printf("Destroying PWEs from NCSMDS_MAX_PWES to 2");
  for(id=gl_tet_vdest[0].pwe_count-1;id>=0;id--)
    {
      destroy_pwe_on_vdest(gl_tet_vdest[0].pwe[id].mds_pwe_hdl);
    }
  if(gl_tet_vdest[0].pwe_count!=0)
    {      tet_printf("Fail");FAIL=1;  }    
  /*clean up*/
  tet_printf("Destroying the above VDEST");
  if(destroy_vdest(1200)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
    
  if(FAIL)
    tet_result(TET_FAIL);   
  else
    tet_result(TET_PASS);
  printf("\n******************************************************\n");
}
void tet_create_default_PWE_VDEST_tp()
{
  int FAIL=0;
  tet_printf("                  tet_create_default_PWE_VDEST_tp");
  gl_vdest_indx=0;
  printf("\n**************tet_create_default_PWE_VDEST_tp****************\n");
  /*Start UP*/
  memset(&gl_tet_vdest,'\0', sizeof(gl_tet_vdest)); /*zeroizing*/
    
  tet_printf("Creating a VDEST in N-WAY model");
  if(create_vdest(NCS_VDEST_TYPE_N_WAY_ROUND_ROBIN,
                  2000)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  tet_printf("Creating DEFAULT  PWE=1 on this VDEST");
  if(create_pwe_on_vdest(gl_tet_vdest[0].mds_vdest_hdl,1)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  destroy_pwe_on_vdest(gl_tet_vdest[0].pwe[0].mds_pwe_hdl);

  /*clean up*/
  tet_printf("Destroying the above VDEST");
  if(destroy_vdest(2000)!=NCSCC_RC_SUCCESS)
    {    
      tet_printf("Fail");
      FAIL=1; 
    }
  if(FAIL)
    tet_result(TET_FAIL);   
  else
    tet_result(TET_PASS);      
  printf("\n**************************************************************\n");
}

void Print_return_status(uint32_t rs)
{
  switch(rs)
    {
    case SA_AIS_OK:
      printf("API is Successfull\n");break;
    case SA_AIS_ERR_LIBRARY:
      printf("Problem with the Library\n");break;
    case SA_AIS_ERR_TIMEOUT:
      printf("Time OUT\n");break;
    case SA_AIS_ERR_TRY_AGAIN:
      printf("The service can't be provided\n");break;
    case SA_AIS_ERR_INVALID_PARAM:
      printf("A parameter is not set properly\n");break;
    case SA_AIS_ERR_NO_MEMORY:
      printf("Out of Memory\n");break;
    case SA_AIS_ERR_NO_RESOURCES:
      printf("The system out of resources\n");break;
    case SA_AIS_ERR_VERSION:
      printf("Version parameter not compatible\n");break;
    }
}
void tet_VDS(int choice)
{
  int FAIL=0,i=0,j;
  char active_ip[15];
  char snpset[100];
  char *vname[]={"MY_VDEST1","MY_VDEST2","MY_VDEST3","MY_VDEST4","MY_VDEST5"};
  MDS_DEST vdest_id[5],v_id;
  gl_vdest_indx=0;
  char *tmpprt=NULL;
  tmpprt= (char *) getenv("ACTIVE_CB_IP");
  strcpy(active_ip,tmpprt);
  tet_printf("                 tet_VDS");
  printf("\n*************************tet_VDS**************************\n");
  /*start up*/
  memset(&gl_tet_vdest,'\0', sizeof(gl_tet_vdest)); /*zeroizing*/
  
  tet_printf("Creating a Named VDEST in MxN model");
  for(j=0;j<5;j++)
    {
      if(create_named_vdest(false,NCS_VDEST_TYPE_MxN,
                            vname[j])!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }
    }
  printf("\n vdest count = %d \n",gl_vdest_indx);
  
  tet_printf("Looking for VDEST id associated with this Named VDEST");
  for(j=0,i=0;j<5;j++)
    {
      vdest_id[i]=vdest_lookup(vname[j]);
      tet_printf("Named VDEST= %s : %d Vdest id",vname[j],vdest_id[i++]);
    }
  /*-----------------------------------------------------------------------*/
  switch(choice)
    {
    case 1:  printf("pkill vds.out");
      system("pkill vds");
      break;
    case 2:  printf("snmpswithover");
      printf("\nActive SCXB IP = %s\n",active_ip);
      sprintf(snpset,"snmpset -v2c -m /usr/share/snmp/mibs/NCS-AVM-MIB -c\
 public %s ncsAvmAdmSwitch.0 i 1",active_ip);
      printf("\n %s\n",snpset);
      system(snpset);
      break;
    case 3: printf("error report");
      SaAmfHandleT amfHandle;
      SaVersionT   version={'B',1,0};
      SaNameT      compName;
      char name[]="safComp=CompT_VDS,safSu=SuT_NCS_SCXB,safNode=Node1_SCXB";
      uint32_t returnvalue;

      returnvalue=saAmfInitialize(&amfHandle,NULL,&version);
      if(SA_AIS_OK==returnvalue)
        {
          printf("saAmfInitialize Successful\n");
          /*error report*/
          compName.length=(uint16_t)strlen(name);
          memcpy(compName.value,name,(uint16_t)strlen(name));
          if(SA_AIS_OK==saAmfComponentErrorReport(amfHandle,&compName,0,
                                                  SA_AMF_COMPONENT_FAILOVER,0))
            printf("saAmfComponentErrorReport Successful\n");
          else
            printf("saAmfComponentErrorReport FAILED\n");
            
          /*finalize*/
          if(SA_AIS_OK==saAmfFinalize(amfHandle))
            printf("saAmfFinalize Successful\n");
        }
      else
        Print_return_status(returnvalue);
      break;
    default: printf(" 1: Pkill VDS\n 2:Snmp switchover \n 3: Error Report \n");
      break;  
    }
  /*------------------------------------------------------------------------*/
  printf("\nPlease Enter any Key\n");
  sleep(5);
  fflush(stdout);
  getchar();
  /*checking*/
  for(j=0;j<5;j++)
    {
      v_id=vdest_lookup(vname[j]);
      if(vdest_id[j]!=v_id)
        {
          printf("\n Not Matched %d",j);
          FAIL=1;
          break;
        }
    }
  if(!FAIL)
    tet_printf("Success");
    
  tet_printf("Destroying the above Named VDEST in MxN model");
  for(j=4;j>=0;j--)
    {
      if(destroy_named_vdest(false,vdest_id[gl_vdest_indx-1],
                             vname[j])!=NCSCC_RC_SUCCESS)
        {    
          tet_printf("Fail");
          FAIL=1; 
        }        
    }

  if(FAIL)
    tet_result(TET_FAIL);
  else
    tet_result(TET_PASS);  
  printf("\n**************************************************************\n");
}
