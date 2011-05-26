#include <stdio.h>

#include "tet_api.h"

#include "ncsgl_defs.h"
#include "ncs_osprm.h"
#include "ncssysf_def.h"
#include "ncssysf_tsk.h"
#include "ncssysf_ipc.h"
#include "ncssysf_tmr.h"
#include "ncs_mds_def.h"

#include "saAmf.h"
#include "saAis.h"

#include "tet_saf.h"
#include "tet_avsv.h"

extern SaAmfCSIDescriptorT gl_csidesc;

/* This file contains the UNIT Tst Code  SAF API's  
 just a correction stmt to correct the merge screw up */
#ifdef TET_A
struct tet_testlist avsv_list[]={
      {tet_saf_initialize,1,1},
      {tet_saf_initialize,1,2},
      {tet_saf_initialize,1,3},
      {tet_saf_initialize,1,4},
      {tet_saf_initialize,1,5},

      {tet_saf_comp_register,1,1}, 
      {tet_saf_comp_register,1,2}, 
      {tet_saf_comp_register,1,3}, 
      {tet_saf_comp_register,1,4}, 
      {tet_saf_comp_register,1,5}, 
      {tet_saf_comp_register,1,6}, 
   
     {tet_saf_comp_unreg,1,1}, 
      {tet_saf_comp_unreg,1,2}, 
      {tet_saf_comp_unreg,1,3}, 
      
      {tet_saf_comp_name_get,1,1},
      {tet_saf_comp_name_get,1,2},

          {tet_saf_finalize,1,1},
          {tet_saf_finalize,1,2},
          {tet_saf_finalize,1,3},
          {tet_saf_finalize,1,4},
          {tet_saf_finalize,1,5},
          {tet_saf_finalize,1,6},


       
         {tet_saf_pm_start,1,1},
         {tet_saf_pm_start,1,2},
         {tet_saf_pm_start,1,3},
         {tet_saf_pm_start,1,4},
         {tet_saf_pm_start,1,5},
         {tet_saf_pm_start,1,6},
         {tet_saf_pm_start,1,7},
         {tet_saf_pm_start,1,8},
         {tet_saf_pm_start,1,9},

         {tet_saf_pm_stop,1,1},
         {tet_saf_pm_stop,1,2},
         {tet_saf_pm_stop,1,3},
         {tet_saf_pm_stop,1,4},
         {tet_saf_pm_stop,1,5},
         {tet_saf_pm_stop,1,6},
         {tet_saf_pm_stop,1,7},
         {tet_saf_pm_stop,1,8},
         {tet_saf_pm_stop,1,9},

       
#if 0
#endif
          {NULL,-1}
      };


struct tet_testlist avsv_list_other[]={
                {tet_saf_health_chk_start,1,1},
                {tet_saf_health_chk_start,1,2},
                {tet_saf_health_chk_start,1,3},
                {tet_saf_health_chk_start,1,4},
                {tet_saf_health_chk_start,1,5},
                {tet_saf_health_chk_start,1,6},
                {tet_saf_health_chk_start,1,7},
                {tet_saf_health_chk_start,1,8},
                {tet_saf_health_chk_start,1,9},

                {tet_saf_health_chk_stop,1,1},
                {tet_saf_health_chk_stop,1,2},
                {tet_saf_health_chk_stop,1,3},
                {tet_saf_health_chk_stop,1,4},
                {tet_saf_health_chk_stop,1,5},
                {tet_saf_health_chk_stop,1,6},

                {tet_saf_health_chk_confirm,1,1},
                {tet_saf_health_chk_confirm,1,2},
                {tet_saf_health_chk_confirm,1,3},
                {tet_saf_health_chk_confirm,1,4},
                {tet_saf_health_chk_confirm,1,5},
 
                {tet_saf_ha_state_get,1,1},
                {tet_saf_ha_state_get,1,2},
                  {NULL,-1}
};

void tet_saf_initialize (int sub_test_arg)
{
    SaAmfCallbacksT amfCallbacks;
    SaVersionT      amf_version;   
    SaAisErrorT        error;

    memset(&amfCallbacks, 0, sizeof(SaAmfCallbacksT));

   switch(sub_test_arg) {

       case 1:
    m_TET_GET_AMF_VER(amf_version);

    tet_infoline(" Test Case ID: AVSV_API_1\n");
    tet_infoline("Initializing with AMF with NULL Callbacks \n");
    if  ((error = saAmfInitialize(&gl_TetSafHandle, &amfCallbacks, &amf_version))
                      == SA_AIS_OK) 
        {     
              tet_printf("The ret val from saAmfInitialize is %d\n",error);   
              tet_result(TET_PASS); 
        }
        else 
        {
           tet_printf("The ret val from saAmfInitialize is %d\n",error);   
           tet_result(TET_FAIL); 
        }  

       /* This would just finalize */  
        tet_saf_cleanup(4);  
       break; 

        case 2:
    m_TET_GET_AMF_VER(amf_version);
    tet_infoline(" Test Case ID: AVSV_API_2\n");
    tet_infoline("Initializing with AMF with NULL Value for Callback parameter \n");
    
    if  ((error = saAmfInitialize(&gl_TetSafHandle, 0, &amf_version))
                      == SA_AIS_OK) 
        {
                   
           tet_printf("The ret val from saAmfInitialize is %d\n",error);   
           tet_result(TET_PASS); 
        }
        else 
        {
           tet_printf("The ret val from saAmfInitialize is %d\n",error);
           tet_result(TET_FAIL); 
        }  

       /* This would just finalize */  
        tet_saf_cleanup(4);  
       break; 

      case 3:

    m_TET_GET_AMF_VER(amf_version);
       
        amf_version.releaseCode = 'C';

    tet_infoline(" Test Case ID: AVSV_API_3\n");
    tet_infoline("Initializing with AMF with incorrect version inputs\n");

        if  ((error = saAmfInitialize(&gl_TetSafHandle, &amfCallbacks, &amf_version))
                      == SA_AIS_ERR_VERSION) 
        { 
           tet_printf("The ret val from saAmfInitialize is %d\n",error);
           tet_result(TET_PASS); 
        }
        else 
        {
           tet_printf("The ret val from saAmfInitialize is %d\n",error);
           tet_result(TET_FAIL); 
        } 
 
       break;

       case 4:

    amfCallbacks.saAmfHealthcheckCallback = tet_saf_health_chk_callback;
    amfCallbacks.saAmfCSISetCallback = tet_saf_CSI_set_callback;
                   
    m_TET_GET_AMF_VER(amf_version);

           
    tet_infoline(" Test Case ID: AVSV_API_4\n");
    tet_infoline("Initializing with AMF with correct set of parameters \n");

    if  ((error = saAmfInitialize(&gl_TetSafHandle, &amfCallbacks, &amf_version))
            == SA_AIS_OK)
    {
           tet_printf("The ret val from saAmfInitialize is %d\n",error);
           tet_result(TET_PASS); 
    }
        else 
        {
           tet_printf("The ret val from saAmfInitialize is %d\n",error);
           tet_result(TET_FAIL); 
        } 

       /* this would just finalize */
           tet_saf_cleanup(4);  
 
        break;
     
     case 5:
       
         amfCallbacks.saAmfHealthcheckCallback = tet_saf_health_chk_callback;
         amfCallbacks.saAmfCSISetCallback = tet_saf_CSI_set_callback;

    m_TET_GET_AMF_VER(amf_version);


    tet_infoline(" Test Case ID: AVSV_API_5\n");
    tet_infoline("Initializing with AMF with invalid parameters \n");
    
    if  ((error = saAmfInitialize(&gl_TetSafHandle, &amfCallbacks, NULL))
            == SA_AIS_ERR_INVALID_PARAM)
    {
           tet_printf("The ret val from saAmfInitialize is %d\n",error);
           tet_result(TET_PASS);
    }
        else
        {
           tet_printf("The ret val from saAmfInitialize is %d\n",error);
           tet_result(TET_FAIL);
        }

       /* this would just finalize */
           tet_saf_cleanup(4);

        break;

       } 

}



void tet_saf_comp_register(int sub_test_arg)
{
    SaAisErrorT error;
    /* uint32_t    Comp_type;  */
    SaNameT  SaCompName;
    SaAmfCallbacksT amfCallbacks;
    SaVersionT      amf_version;
    /*SaNameT proxied_comp = {60,"safComp=CompT_Proxy_Prx,safSu=SuT_Proxy2_Prx,safNode=Node1_TypeSCXB"};*/
       
      switch (sub_test_arg) {
      case 1:

        /* dont fill callbacks */
        tet_saf_succ_init(1,0);
    tet_infoline(" Test Case ID: AVSV_API_6\n");   
    tet_infoline(" Component registration with NULL callbacks during saAmfInitialize \n");
    
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

        if ((  error = saAmfComponentRegister(gl_TetSafHandle,&SaCompName,NULL))
                == SA_AIS_ERR_INIT)
        {
           tet_printf("The ret val from saAmfComponentRegister is %d\n",error);
            tet_result(TET_PASS);
        }
        else 
        {
           tet_printf("The ret val from saAmfComponentRegister is %d\n",error);
            tet_result(TET_FAIL); 
        } 
    }

       /* this would just finalize */
           tet_saf_cleanup(4);  

        break;

       case 2:

        tet_saf_succ_init(1,1);
       
    tet_infoline(" Test Case ID: AVSV_API_7\n");
    tet_infoline(" Component registration with valid inputs\n");
    
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

        if ((  error = saAmfComponentRegister(gl_TetSafHandle,&SaCompName,NULL))
                == SA_AIS_OK)
        {
           tet_printf("The ret val from saAmfComponentRegister is %d\n",error);
            tet_result(TET_PASS);

        }
        else 
        {
           tet_printf("The ret val from saAmfComponentRegister is %d\n",error);
            tet_result(TET_FAIL); 
        } 
    }

       /* this would just finalize and un-reg */
           tet_saf_cleanup(6);  

        break;


       case 3:

        tet_saf_succ_init(1,1);
        tet_infoline(" Test Case ID: AVSV_API_8\n");
        tet_infoline(" Component Registration with a component that does not exist\n");
    
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

        strcpy((char *)&SaCompName.value,"COMPOUND");
        SaCompName.length = strlen("COMPOUND"); 

        if ((  error = saAmfComponentRegister(gl_TetSafHandle,&SaCompName,NULL))
                == SA_AIS_ERR_INVALID_PARAM)
        {
           tet_printf("The ret val from saAmfComponentRegister is %d\n",error);
            tet_result(TET_PASS);
        }

        else 
        {
           tet_printf("The ret val from saAmfComponentRegister is %d\n",error);
            tet_result(TET_FAIL); 
        } 

    }
        
       /* this would just finalize */
           tet_saf_cleanup(4);  
        break ;

       case 4:

        tet_saf_succ_init(1,1);

    tet_infoline(" Test Case ID: AVSV_API_9\n");
    tet_infoline(" Component Registration with a handle that does not exist\n");
    
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

        if ((  error = saAmfComponentRegister((SaAmfHandleT)0xabcdefab,&SaCompName,NULL))
                == SA_AIS_ERR_BAD_HANDLE)
        {
           tet_printf("The ret val from saAmfComponentRegister is %d\n",error);
           tet_result(TET_PASS);
        }
        else 
        {
           tet_printf("The ret val from saAmfComponentRegister is %d\n",error);
           tet_result(TET_FAIL); 
        } 
               
    }

       /* this would just finalize */
           tet_saf_cleanup(4);  
       break; 

        case 5:

    tet_saf_succ_init(1,1);

    
    tet_infoline(" Test Case ID: AVSV_API_10\n");
    tet_infoline(" RE-registration of the same component \n");
    

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
                       tet_printf("The ret val from saAmfComponentNameGet is %d\n",error);
       if ((  error = saAmfComponentRegister(gl_TetSafHandle,&SaCompName,NULL)) == SA_AIS_OK)
          {
                       tet_printf("The ret val from saAmfComponentRegister is %d\n",error);
        if ((  error = saAmfComponentRegister(gl_TetSafHandle,&SaCompName,NULL))
                == SA_AIS_ERR_EXIST)
        {
            tet_printf("The ret val from saAmfComponentRegister is %d\n",error);
            tet_infoline(" Component RE-registration Failed \n");
            tet_result(TET_PASS);

        }
        else 
        {
            tet_printf("The ret val from saAmfComponentRegister is %d\n",error);
            tet_result(TET_FAIL); 
        } 
       } 

       tet_printf("The ret val from saAmfComponentRegister is %d\n",error);
    }

         /* this would just finalize */
          tet_saf_cleanup(6);  
        break;

     case 6:

     memset(&amfCallbacks, 0, sizeof(SaAmfCallbacksT));
     m_TET_GET_AMF_VER(amf_version);
     amfCallbacks.saAmfCSISetCallback        = tet_saf_CSI_set_callback;
     amfCallbacks.saAmfCSIRemoveCallback     = tet_saf_CSI_remove_callback;

    tet_infoline(" Test Case ID: AVSV_API_11\n");
    tet_infoline(" Comp Register with not all required callbacks registered in amfInitialize \n");
    

    if  ((error = saAmfInitialize(&gl_TetSafHandle, &amfCallbacks, &amf_version))
                      == SA_AIS_OK)
    {

       if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
       {
             tet_printf("The ret val from saAmfComponentNameGet is %d\n",error);
   
         if ((error = saAmfComponentRegister(gl_TetSafHandle,&SaCompName,NULL)) == SA_AIS_ERR_INIT)
          {
             tet_printf("The ret val from saAmfComponentRegister is %d\n",error);
             tet_result(TET_PASS);

           }
          else
           {
             tet_printf("The ret val from saAmfComponentRegister is %d\n",error);
             tet_result(TET_FAIL);
            }
         }

    }

         /* this would just finalize */
          tet_saf_cleanup(4);
        break;
       } 

}




void tet_saf_comp_name_get(int sub_test_arg)
{

  SaNameT SaCompName;
  SaNameT dummy_comp = {60,"safComp=CompT_NCSP,safSu=SuT_NCSP1,safNode=Node1_TypeSCXB"};
  SaAisErrorT error;
  
  
  /* This would do saAmfInitialize */ 
  tet_saf_succ_init(1,0);
  
  switch(sub_test_arg) 
  { 
  case 1 :  
        tet_infoline(" Test Case ID: AVSV_API_15\n");
        tet_infoline(" Name Get when the component name is set in the env variable \n");
    
        if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
        {
         tet_print("comp name=%s",SaCompName.value);  
         tet_printf("The ret val from saAmfComponentNameGet is %d\n",error);
         tet_result(TET_PASS);  
        }  
        else 
        {
         tet_printf("The ret val from saAmfComponentNameGet is %d\n",error);
         tet_result(TET_FAIL);
        }         
  break;

  case 2:
        tet_infoline(" Test Case ID: AVSV_API_16\n");
        tet_infoline(" Name Get when the called with invalid handle\n");
    
        if((error = saAmfComponentNameGet(1212,&dummy_comp)) == SA_AIS_ERR_BAD_HANDLE)
        {
         tet_printf("The ret val from saAmfComponentNameGet is %d\n",error);
         tet_result(TET_PASS);  
        }  
        else 
        {
         tet_printf("The ret val from saAmfComponentNameGet is %d\n",error);
         tet_result(TET_FAIL);
        }        
       
      break;
  } 
     
  /* This would unreg component and finalize */ 
  tet_saf_cleanup(4);

}








/* UT Functions for saAmfComponentUnregister */

void tet_saf_comp_unreg(int sub_test_arg)
{

    SaAisErrorT error;
    SaNameT  SaCompName;
    /*SaNameT proxied_comp = {60,"safComp=CompT_NCSPrx,safSu=SuT_NCSPrx,safNode=Node1_TypeSCXB"};*/


        switch (sub_test_arg) {
 
        case 1:

        /* This would do saAmfInitialize and saAmfComponentRegister */ 
    tet_infoline(" Test Case ID: AVSV_API_12\n");
    tet_infoline(" Un-Registering a Component  with valid inputs\n");
    tet_saf_succ_init(3,1);
        
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
    
        if ((  error = saAmfComponentUnregister(gl_TetSafHandle,&SaCompName,NULL))
                == SA_AIS_OK)
        {
            tet_result(TET_PASS);
        }
        else 
        {
            tet_result(TET_FAIL); 
        } 
    }

        /* This would just finalize since the un-reg would have happened just before */   
        tet_saf_cleanup(4);
        break ;

        case 2:
       
        tet_infoline(" Test Case ID: AVSV_API_13\n");
        tet_infoline(" Un-Registration a component that is not been registered\n");
        tet_saf_succ_init(1,1);
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    { 
              if ((  error = saAmfComponentRegister(gl_TetSafHandle, &SaCompName,NULL))
                  != SA_AIS_OK)
                {
                  tet_infoline("Component registration failed \n");
                  tet_print("COMP_REGISTER: failure\n");
                } else {
                  tet_print("COMP_REGISTER: success\n");
                }

              if( (tware_comp_init_count(&SaCompName,1)) == 0)
                {
                  tware_comp_init_count(&SaCompName,0);
                }
              else {

                /* increment the instantiate count */
                tware_comp_init_count(&SaCompName,4);
              }
      }

        strcpy((char *)&SaCompName.value,"COMPOUND");
        SaCompName.length = strlen("COMPOUND"); 

        if ((  error = saAmfComponentUnregister(gl_TetSafHandle,&SaCompName,NULL))
                == SA_AIS_ERR_INVALID_PARAM)
        {
            tet_printf("The ret val from saAmfComponentUnRegister is %d\n",error);
            tet_result(TET_PASS);

        }
        else 
        {
            tet_printf("The ret val from saAmfComponentUnRegister is %d\n",error);
            tet_result(TET_FAIL); 
        } 
    
        /* This would unregister and finalize */
        tet_saf_cleanup(6);
        break;


       case 3:
           
            tet_infoline("Test Case ID: AVSV_API_14\n");
            tet_infoline(" Un-Registration a component with an invalid  handle \n");
    
        /* This would do saAmfInitialize and saAmfComponentRegister */ 
        tet_saf_succ_init(3,1);
      if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) != SA_AIS_OK)
      {
       tet_infoline(" Error getting comp name");
       return;
      }
 
        
        if ((  error = saAmfComponentUnregister((SaAmfHandleT)0x122132,&SaCompName,NULL))
                == SA_AIS_ERR_BAD_HANDLE)
        {
            tet_result(TET_PASS);
        }
        else 
        {
            tet_result(TET_FAIL); 
        } 

        /* This would unregister and finalize */
        tet_saf_cleanup(6);
       break;  

    }


     return;

}




void tet_saf_finalize(int sub_test_arg)
{
 SaAmfHandleT dummy_boy;
 tet_infoline("Finalizing with AMF"); 

 switch (sub_test_arg)
 {

 case 1:
        tet_infoline("Test Case ID: AVSV_API_17\n");
        tet_infoline(" Finalizing with valid handle when callbacks are NOT registered \n");
        tet_saf_succ_init(1,0); 
  
       if( saAmfFinalize(gl_TetSafHandle) == SA_AIS_OK)  
       {
         tet_result(TET_PASS);
       }
       else {
             tet_result(TET_FAIL);
       }
      break;
case 2 :     
        tet_infoline("Test Case ID: AVSV_API_18\n");
        tet_infoline(" Finalizing with invalid handle \n");
    
       if( saAmfFinalize(dummy_boy) == SA_AIS_ERR_BAD_HANDLE)  
       {
         tet_result(TET_PASS);
       }
       else {
             tet_result(TET_FAIL);
       }  
      
      break;
case 3 :  
        tet_infoline("Test Case ID: AVSV_API_19\n");
        tet_infoline(" Finalizing with valid handle when callbacks are registered \n");
        tet_saf_succ_init(1,2);

       if( saAmfFinalize(gl_TetSafHandle) == SA_AIS_OK)  
       {
         tet_result(TET_PASS);
       }
       else {
             tet_result(TET_FAIL);
       }  
      break;

case 4 :
       tet_infoline("Test Case ID: AVSV_API_20\n");
       tet_infoline(" Finalizing with valid handle when component are registered \n");
       tet_saf_succ_init(3,2);

       if( saAmfFinalize(gl_TetSafHandle) == SA_AIS_OK)  
       {
         tet_result(TET_PASS);
       }
       else {
             tet_result(TET_FAIL);
       }  

     tet_saf_cleanup(6);
      break;


 case 5 :
       tet_infoline("Test Case ID: AVSV_API_21\n");
       tet_infoline(" Finalizing twice \n");
       tet_saf_succ_init(1,2);

       if( saAmfFinalize(gl_TetSafHandle) == SA_AIS_OK)  
       {
           if( (dummy_boy = saAmfFinalize(gl_TetSafHandle)) == SA_AIS_ERR_BAD_HANDLE)  
           {
               tet_result(TET_PASS);
           }
           else {
                       tet_printf("The ret val from saAmfFinalize is %d\n",dummy_boy);
               tet_result(TET_FAIL);
           }  
       }  

      break;
      
 case 6 :
        tet_infoline("Test Case ID: AVSV_API_22\n");
        tet_infoline(" Finalizing before init \n");
    
           if(( dummy_boy= saAmfFinalize(gl_TetSafHandle)) == SA_AIS_ERR_BAD_HANDLE)  
           {
               tet_result(TET_PASS);
           }
           else {
                       tet_printf("The ret val from saAmfFinalize is %d\n",dummy_boy);
               tet_result(TET_FAIL);
           }  

      break;
 


    } 



}



void tet_saf_health_chk_start(int sub_test_arg)
{

    SaAisErrorT error;
    SaNameT  SaCompName;
    SaAmfHealthcheckKeyT  Healthy;

    /* Hard coding the health-check key  */
    strcpy(Healthy.key,"ABCDEF20");
    Healthy.keyLen=strlen(Healthy.key); 
       
       switch(sub_test_arg) {
       
       case 1:
       tet_infoline("Test Case ID: AVSV_API_41\n");
       tet_infoline(" Starting the Health_check with COMP_INVOKED and non-NULL HC callback"); 
       tet_saf_succ_init(3,2);
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ((error = saAmfHealthcheckStart(
                        gl_TetSafHandle,
                        &SaCompName,
                        &Healthy,
                        SA_AMF_HEALTHCHECK_COMPONENT_INVOKED,
                        SA_AMF_NO_RECOMMENDATION 
                        )) == SA_AIS_OK) 
        {
            tet_result(TET_PASS);
        } 
        else {
            tet_result(TET_FAIL); 
        }      
    }

        /* This would stop health check unreg component and finalize */ 
        tet_saf_cleanup(7); 
     break ; 

     case 2:
        tet_infoline("Test Case ID: AVSV_API_42\n");
        tet_infoline(" Starting the Health_check with COMP_INVOKED and NULL HC callback"); 
    
        tet_saf_succ_init(3,1);
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ((error = saAmfHealthcheckStart(
                        gl_TetSafHandle,
                        &SaCompName,
                        &Healthy,
                        SA_AMF_HEALTHCHECK_COMPONENT_INVOKED,
                        SA_AMF_NO_RECOMMENDATION 
                        )) == SA_AIS_OK) 
        {
            tet_result(TET_PASS);
        } 
        else {
            tet_result(TET_FAIL); 
        }      
    }

        /* This would stop health check , unreg component and finalize */ 
        tet_saf_cleanup(7); 
     break;

    case 3:
           tet_infoline("Test Case ID: AVSV_API_43\n");
           tet_infoline(" Starting the Health_check with AMF_INVOKED and NULL HC callback\n"); 
           tet_saf_succ_init(3,1);

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ((error = saAmfHealthcheckStart(
                        gl_TetSafHandle,
                        &SaCompName,
                        &Healthy,
                        SA_AMF_HEALTHCHECK_AMF_INVOKED,
                        SA_AMF_NO_RECOMMENDATION 
                        )) == SA_AIS_ERR_INIT) 
        {
            tet_result(TET_PASS);
        } 
        else {
            tet_result(TET_FAIL); 
        }      
    }

        /* This would unreg component and finalize */ 
        tet_saf_cleanup(3); 
       break;

   case 4:

       tet_infoline("Test Case ID: AVSV_API_44\n");
       tet_infoline(" Starting the Health_check with AMF_INVOKED and non-NULL HC callback\n"); 
       tet_saf_succ_init(3,2);
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ((error = saAmfHealthcheckStart(
                        gl_TetSafHandle,
                        &SaCompName,
                        &Healthy,
                        SA_AMF_HEALTHCHECK_AMF_INVOKED,
                        SA_AMF_NO_RECOMMENDATION 
                        )) == SA_AIS_OK) 
        {
            tet_result(TET_PASS);
        } 
        else {
            tet_result(TET_FAIL); 
        }      
    }

        /* This would  stop health check unreg component and finalize */ 
        tet_saf_cleanup(7); 
    break;

   case 5:
      tet_infoline("Test Case ID: AVSV_API_45\n");
      tet_infoline(" Starting a Health_check with valid params but without pre-registering the comp"); 
      /* only initialize */
      tet_saf_succ_init(1,2); 

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ((error = saAmfHealthcheckStart(
                        gl_TetSafHandle,
                        &SaCompName,
                        &Healthy,
                        SA_AMF_HEALTHCHECK_AMF_INVOKED,
                        SA_AMF_NO_RECOMMENDATION 
                        )) == SA_AIS_OK) 
        {
            tet_result(TET_PASS);
        } 
        else {
            tet_result(TET_FAIL); 
        }      
    }

        /* This would just finalize */ 
        tet_saf_cleanup(4); 
    break;

 
    case 6:
        tet_infoline("Test Case ID: AVSV_API_46\n");
        tet_infoline(" Starting the Health_check with COMP_INVOKED with invalid component"); 
        tet_saf_succ_init(3,1);
        strcpy((char *)&SaCompName.value,"COMPOUND");
        SaCompName.length = strlen("COMPOUND"); 
        if ((error = saAmfHealthcheckStart(
                        gl_TetSafHandle,
                        &SaCompName,
                        &Healthy,
                        SA_AMF_HEALTHCHECK_COMPONENT_INVOKED,
                        SA_AMF_NO_RECOMMENDATION 
                        )) == SA_AIS_ERR_NOT_EXIST) 
        {
            tet_result(TET_PASS);
        } 
        else {
            tet_result(TET_FAIL); 
        }      

        /* This would unreg component and finalize */ 
        tet_saf_cleanup(6); 
       break;

   case 7:
       tet_infoline("Test Case ID: AVSV_API_47\n");
       tet_infoline(" Starting the Health_check with an invalid Health-Check Key"); 
       tet_saf_succ_init(3,2);
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
             strcpy(Healthy.key,"Extreme");
             Healthy.keyLen=strlen(Healthy.key); 
        if ((error = saAmfHealthcheckStart(
                        gl_TetSafHandle,
                        &SaCompName,
                        &Healthy,
                        SA_AMF_HEALTHCHECK_AMF_INVOKED,
                        SA_AMF_NO_RECOMMENDATION 
                        )) == SA_AIS_ERR_NOT_EXIST) 
        {
            tet_result(TET_PASS);
        } 
        else {
            tet_result(TET_FAIL); 
        }      
    }

        /* This would unreg component and finalize */ 
        tet_saf_cleanup(6); 
    break;    

   case 8:
       tet_infoline("Test Case ID: AVSV_API_48\n");
       tet_infoline(" CAlling the Health_check start twice "); 
       tet_saf_succ_init(3,2);
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ((error = saAmfHealthcheckStart(
                        gl_TetSafHandle,
                        &SaCompName,
                        &Healthy,
                        SA_AMF_HEALTHCHECK_COMPONENT_INVOKED,
                        SA_AMF_NO_RECOMMENDATION
                        )) == SA_AIS_OK) 
        {
        if ((error = saAmfHealthcheckStart(
                        gl_TetSafHandle,
                        &SaCompName,
                        &Healthy,
                        SA_AMF_HEALTHCHECK_COMPONENT_INVOKED,
                        SA_AMF_NO_RECOMMENDATION 
                        )) == SA_AIS_ERR_EXIST) 

                 {
                tet_result(TET_PASS); 
                 }
        } 
        else {
            tet_result(TET_FAIL); 
        }      
    }

        /* This would  stop health check ,unreg component and finalize */ 
        tet_saf_cleanup(7);
       break;

    case 9:
       tet_infoline("Test Case ID: AVSV_API_49\n");
       tet_infoline(" CAlling the Health_check start with invalid handle\n"); 
       tet_saf_succ_init(3,2);
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
             strcpy(Healthy.key,"Extreme");
             Healthy.keyLen=strlen(Healthy.key); 
        if ((error = saAmfHealthcheckStart(
                        100,
                        &SaCompName,
                        &Healthy,
                        SA_AMF_HEALTHCHECK_AMF_INVOKED,
                        SA_AMF_NO_RECOMMENDATION 
                        )) == SA_AIS_ERR_BAD_HANDLE) 
        {
            tet_result(TET_PASS);
        } 
        else {
            tet_result(TET_FAIL); 
        }      
    }

        /* This would unreg component and finalize */ 
        tet_saf_cleanup(6); 
    break;    


     } 
    return;

}




void tet_saf_health_chk_stop (int sub_test_arg)
{
    SaAisErrorT error;
    SaNameT  SaCompName;
    SaAmfHealthcheckKeyT  Healthy;

    /* Hard coding the health-check key  */
    strcpy(Healthy.key,"ABCDEF20");
    Healthy.keyLen=strlen(Healthy.key); 

    switch (sub_test_arg) 
        {
    case 1:
    tet_infoline("Test Case ID: AVSV_API_50");
    tet_infoline("Health Check Stop with valid params for amf invoked HC");
    /* amf invoked hc */
    tet_saf_succ_init(7,2);
    
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ((error = saAmfHealthcheckStop( gl_TetSafHandle, &SaCompName, &Healthy ))
                == SA_AIS_OK)

        {
            tet_result(TET_PASS); 
        }
        else {
            tet_result(TET_FAIL); 
        }  
    }        
    tet_saf_cleanup(6);
    break ; 

    case 2:
        tet_infoline("Test Case ID: AVSV_API_51");
        tet_infoline("Health Check Stop with valid params for comp invoked HC");    
        /* comp invoked hc */
        tet_saf_succ_init(11,2);
    
    
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ((error = saAmfHealthcheckStop( gl_TetSafHandle, &SaCompName, &Healthy ))
                == SA_AIS_OK)

        {
            tet_result(TET_PASS); 
        }
        else {
            tet_result(TET_FAIL); 
        }  
    }        
    tet_saf_cleanup(6);
    break ; 

    case 3:
    
    tet_infoline("Test Case ID: AVSV_API_52\n");
    tet_infoline(" Health Check Stop without Health_check start\n");
    tet_saf_succ_init(3,2);

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ((error = saAmfHealthcheckStop( gl_TetSafHandle, &SaCompName, &Healthy ))
                == SA_AIS_ERR_NOT_EXIST)
        {
            tet_result(TET_PASS); 
        }
        else {
            tet_result(TET_FAIL); 
        }  
    }    
        tet_saf_cleanup(6); 
    break;  

      case 4:
        tet_infoline("Test Case ID: AVSV_API_53\n");
        tet_infoline(" Health check stop with invalid Amf Handle\n");
        tet_saf_succ_init(7,2);
    
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ((error = saAmfHealthcheckStop((SaAmfHandleT)0xabcdefab, &SaCompName, &Healthy ))
                == SA_AIS_ERR_BAD_HANDLE)
        {
            tet_result(TET_PASS); 
        }
        else {
            tet_result(TET_FAIL); 
        }  
    }    
        tet_saf_cleanup(7); 
    break; 

      case 5:
          tet_infoline("Test Case ID: AVSV_API_54");
          tet_infoline("Health Check stop with Invalid Component Name\n");
          tet_saf_succ_init(7,2);
    
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        strcpy((char *)&SaCompName.value,"COMPOUND");
        SaCompName.length = strlen("COMPOUND"); 
        if ((error = saAmfHealthcheckStop((SaAmfHandleT)gl_TetSafHandle, &SaCompName, &Healthy ))
                == SA_AIS_ERR_NOT_EXIST)
        {
            tet_result(TET_PASS); 
        }
        else {
            tet_result(TET_FAIL); 
        }  
    }    
        tet_saf_cleanup(7); 
    break; 

        case 6:
            tet_infoline("Test Case ID: AVSV_API_55");
            tet_infoline(" Health Check Stop without registering the comp \n");
            tet_saf_succ_init(1,1);
    
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ((error = saAmfHealthcheckStop( gl_TetSafHandle, &SaCompName, &Healthy ))
                == SA_AIS_ERR_NOT_EXIST)

        {
            tet_result(TET_PASS); 
        }
        else {
            tet_result(TET_FAIL); 
        }  
    }        
    tet_saf_cleanup(4);
    break ; 


   }
}






void tet_saf_health_chk_confirm (int sub_test_arg)
{

    SaAisErrorT error;
    SaNameT  SaCompName;
    SaAmfHealthcheckKeyT  Healthy;

    /* Hard coding the health-check key  */
    strcpy(Healthy.key,"ABCDEF20");
    Healthy.keyLen=strlen(Healthy.key); 

    switch (sub_test_arg) {
    case 1:
        tet_infoline("Test Case ID: AVSV_API_56");
        tet_infoline("Health Check Confirm with valid inputs");
        tet_saf_succ_init(11,1);

     if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ( (error =saAmfHealthcheckConfirm(
                                    gl_TetSafHandle,
                                    &SaCompName,
                                    &Healthy,
                            SA_AIS_OK)) == SA_AIS_OK  )
        {
              tet_result(TET_PASS);
            }
        else 
        {
          tet_result(TET_FAIL);
            } 
        }
        tet_saf_cleanup(7);
    break;    

    case 2:
        tet_infoline("Test Case ID: AVSV_API_57");
        tet_infoline("Health Check Confirm without starting Health check start ");
        tet_saf_succ_init(3,1);

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ( (error =saAmfHealthcheckConfirm(
                                    gl_TetSafHandle,
                                    &SaCompName,
                                    &Healthy,
                            SA_AIS_OK)) == SA_AIS_ERR_NOT_EXIST)
        {
              tet_result(TET_PASS);
            }
        else 
        {
          tet_result(TET_FAIL);
            } 
        }
        tet_saf_cleanup(6);
    break;    
    
    case 3:
        tet_infoline("Test Case ID: AVSV_API_58"); 
        tet_infoline("Health Check Confirm with invalid component name\n"); 
        tet_saf_succ_init(11,1);
    
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        strcpy((char *)&SaCompName.value,"COMPOUND");
        SaCompName.length = strlen("COMPOUND"); 
        if ( (error =saAmfHealthcheckConfirm(
                                    gl_TetSafHandle,
                                    &SaCompName,
                                    &Healthy,
                            SA_AIS_OK)) == SA_AIS_ERR_NOT_EXIST)
        {
              tet_result(TET_PASS);
            }
        else 
        {
          tet_result(TET_FAIL);
            } 
        }
        tet_saf_cleanup(7);
    break;    

    case 4:
        tet_infoline("Test Case ID: AVSV_API_59"); 
        tet_infoline("Health Check Confirm with invalid Health Check key \n"); 
        tet_saf_succ_init(11,1);
    
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
             strcpy(Healthy.key,"Extreme");
             Healthy.keyLen=strlen(Healthy.key); 
        if ( (error =saAmfHealthcheckConfirm(
                                    gl_TetSafHandle,
                                    &SaCompName,
                                    &Healthy,
                            SA_AIS_OK)) == SA_AIS_ERR_NOT_EXIST)
        {
              tet_result(TET_PASS);
            }
        else 
        {
          tet_result(TET_FAIL);
            } 
        }
        tet_saf_cleanup(7);
    break;    

    case 5:
        tet_infoline("Test Case ID: AVSV_API_60"); 
        tet_infoline(" Health Check Confirm with invalid handle");  
        tet_saf_succ_init(11,1);
    
      if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
     {
        if ( (error =saAmfHealthcheckConfirm(
                                    (SaAmfHandleT) 0xabcdefab,
                                    &SaCompName,
                                    &Healthy,
                             SA_AIS_OK)) == SA_AIS_ERR_BAD_HANDLE)
        {
          tet_print("\nhc_confirm pass = %d\n",error);
          tet_print("comp_name = %s\t health key=%s\n",SaCompName.value, Healthy.key);
              tet_result(TET_PASS);
            }
        else 
        {
          tet_result(TET_FAIL);
          tet_print("\nhc_confirm error = %d\n",error);
          tet_print("comp_name = %s\t health key=%s\n",SaCompName.value, Healthy.key);
            
            } 
         }
        tet_saf_cleanup(7);

    break;

       
      }

}

void tet_saf_errorrpt_spoof(SaAmfRecommendedRecoveryT Reco)
{


    SaAisErrorT error;
    SaNameT  SaCompName;
        SaAmfRecommendedRecoveryT recov;
    SaTimeT   err_time=0;   


     switch (Reco) {
        case SA_AMF_COMPONENT_FAILOVER: 

    recov= SA_AMF_COMPONENT_FAILOVER;

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

          strcpy((char *) SaCompName.value,"safComp=CompT_AFS,safSu=SuT_AFS_NORED,safNode=Node1_TypeSCXB");
          SaCompName.length=strlen((char*)&SaCompName.value);  

  tet_print("The Comp Name FOR The ERROR is %s and the error is %d \n",SaCompName.value,recov);  
        saAmfComponentErrorReport (gl_TetSafHandle,
                &SaCompName,
                err_time, 
                recov,0);
    }

        break; 
 
        case SA_AMF_COMPONENT_RESTART:

    recov= SA_AMF_COMPONENT_RESTART;

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

          strcpy((char *) SaCompName.value,"safComp=CompT_AFSD,safSu=SuT_AFS,safNode=Node6_TypePayload");
          SaCompName.length=strlen((char*)&SaCompName.value);  

  tet_print("The Comp Name FOR The ERROR is %s and the error is %d \n",SaCompName.value,recov);  
        saAmfComponentErrorReport (gl_TetSafHandle,
                &SaCompName,
                err_time, 
                recov,0);
    }

        break;

       case SA_AMF_NODE_FAILOVER:

    recov= SA_AMF_NODE_FAILOVER;

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

  tet_print("The Comp Name FOR The ERROR is %s and the error is %d \n",SaCompName.value,recov);  
        saAmfComponentErrorReport (gl_TetSafHandle,
                &SaCompName,
                err_time, 
                recov,0);
    }

        break;

        case SA_AMF_NODE_SWITCHOVER:

       recov= SA_AMF_NODE_SWITCHOVER;

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

  tet_print("The Comp Name FOR The ERROR is %s and the error is %d \n",SaCompName.value,recov);
        saAmfComponentErrorReport (gl_TetSafHandle,
                &SaCompName,
                err_time,
                recov,0);
    }
         break;

       case SA_AMF_NODE_FAILFAST:

       recov= SA_AMF_NODE_FAILFAST;

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

  tet_print("The Comp Name FOR The ERROR is %s and the error is %d \n",SaCompName.value,recov);
        saAmfComponentErrorReport (gl_TetSafHandle,
                &SaCompName,
                err_time,
                recov,0);
    }
         break;
   

       case SA_AMF_NO_RECOMMENDATION:
        

    recov= SA_AMF_NO_RECOMMENDATION;

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

  tet_print("The Comp Name FOR The ERROR is %s and the error is %d \n",SaCompName.value,recov);  
        saAmfComponentErrorReport (gl_TetSafHandle,
                &SaCompName,
                err_time, 
                recov,0);
    }

        break;
   default:
    break;

      }

}



void tet_saf_errorrpt(SaAmfRecommendedRecoveryT Reco)
{


    SaAisErrorT error;
    SaNameT  SaCompName;
        SaAmfRecommendedRecoveryT recov;
    SaTimeT   err_time=0;   


     switch (Reco) {
        case SA_AMF_COMPONENT_FAILOVER: 

    recov = SA_AMF_COMPONENT_FAILOVER;

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

  tet_print("The Comp Name FOR The ERROR is %s and the error is %d \n",SaCompName.value,recov);  
        saAmfComponentErrorReport (gl_TetSafHandle,
                &SaCompName,
                err_time, 
                recov,0);
    }

        break; 
 
        case SA_AMF_COMPONENT_RESTART:

    recov = SA_AMF_COMPONENT_RESTART;

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

  tet_print("The Comp Name FOR The ERROR is %s and the error is %d \n",SaCompName.value,recov);  
        saAmfComponentErrorReport (gl_TetSafHandle,
                &SaCompName,
                err_time, 
                recov,0);
    }

        break;

       case SA_AMF_NODE_FAILOVER:

    recov= SA_AMF_NODE_FAILOVER;

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

  tet_print("The Comp Name FOR The ERROR is %s and the error is %d \n",SaCompName.value,recov);  
        saAmfComponentErrorReport (gl_TetSafHandle,
                &SaCompName, 
                err_time, 
                recov,0);
    }

        break;
       
       case SA_AMF_NODE_SWITCHOVER:
        
       recov= SA_AMF_NODE_SWITCHOVER;

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

  tet_print("The Comp Name FOR The ERROR is %s and the error is %d \n",SaCompName.value,recov);  
        saAmfComponentErrorReport (gl_TetSafHandle,
                &SaCompName, 
                err_time, 
                recov,0);
    }
         break;

       case SA_AMF_NODE_FAILFAST:
        
       recov= SA_AMF_NODE_FAILFAST;

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

  tet_print("The Comp Name FOR The ERROR is %s and the error is %d \n",SaCompName.value,recov);  
        saAmfComponentErrorReport (gl_TetSafHandle,
                &SaCompName, 
                err_time, 
                recov,0);
    }
         break;

       case SA_AMF_NO_RECOMMENDATION:
        
    recov= SA_AMF_NO_RECOMMENDATION;

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

  tet_print("The Comp Name FOR The ERROR is %s and the error is %d \n",SaCompName.value,recov);  
        saAmfComponentErrorReport (gl_TetSafHandle,
                &SaCompName, 
                err_time, 
                recov,0);
    }

        break;


    default:
    break;
  }
}



void tet_saf_err_cancel()
{
    SaAisErrorT error;
    SaNameT  SaCompName;

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {

          saAmfComponentErrorClear(gl_TetSafHandle, &SaCompName,0);

    }
}


void tet_saf_stopping_complete(SaInvocationT Invocation,SaAisErrorT error)
{
#if 0 
  SaAisErrorT rc;

  if ((rc = (saAmfStoppingComplete (gl_TetSafHandle,
                                    Invocation ,
                                    error))) == SA_AIS_OK)
  {
  
    tet_print("The Stopping Complete has been successfully completed\n");
    tet_result(TET_PASS);

  }

#endif
 }

void tet_saf_ha_state_get(int subtest_arg)
{

   SaAmfHAStateT  ha_state=0;
   extern SaAmfCSIDescriptorT gl_csidesc; 
   SaNameT  csiName = gl_csidesc.csiName;
   SaAisErrorT error;
   SaNameT SaCompName;
   SaAmfHandleT dummy_boy=0;

 
  switch (subtest_arg)
  {

  case 1:
 
    tet_infoline("Test Case ID: AVSV_API_61"); 
    tet_infoline("To Test the HA state get with NULL pointer for the compname");
    tet_saf_succ_init(3,1); 
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
         
         if(( error =   saAmfHAStateGet(gl_TetSafHandle,
                          NULL,
                          &csiName,       
                          &ha_state)) == SA_AIS_ERR_INVALID_PARAM) 
        {
            tet_print("  The ha_state state is %d \n",ha_state);
            tet_result(TET_PASS);

       }
       else 
       {
         tet_print(" The err_val is state is %d \n",error);
         tet_result(TET_FAIL);
       }

     }

   tet_saf_cleanup(6);
   break;

  case 2:
 
    tet_infoline("Test Case ID: AVSV_API_62"); 
    tet_infoline(" calling Ha-state with invalid Handle");
    tet_saf_succ_init(3,1); 
    
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
         
         if(( error =   saAmfHAStateGet(&dummy_boy,
                          &SaCompName,
                          &csiName,       
                          &ha_state)) == SA_AIS_ERR_BAD_HANDLE) 
        {
            tet_print("  The ha_state state is %d \n",ha_state);
            tet_result(TET_PASS);

       }
       else 
       {
         tet_print(" The err_val is state is %d \n",error);
         tet_result(TET_FAIL);
       }

     }

   tet_saf_cleanup(6);
   break;
#if 0
  case 3:
         ha_state = 0;
         tet_infoline("Test Case ID: AVSV_API_63"); 
         tet_infoline(" calling Ha-state with invalid Comp Name");
         tet_saf_succ_init(3,1); 
         
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
         strcpy((char *)&SaCompName.value,"COMPOUND");
        SaCompName.length = strlen("COMPOUND");
          
         if(( error =   saAmfHAStateGet(gl_TetSafHandle,
                          &SaCompName,
                          &csiName,       
                          &ha_state)) == SA_AIS_ERR_NOT_EXIST) 
        {
            tet_print("  The ha_state state is %d \n",ha_state);
            tet_result(TET_PASS);

       }
       else 
       {
         tet_print(" The err_val is state is %d \n",error);
         tet_result(TET_FAIL);
       }

      }
   tet_saf_cleanup(6);
   break;
#endif

    }

}


void tet_comp_capability_get(int sub_test_arg)
{
  
#if 0 
  SaAisErrorT error; 
  SaNameT  SaCompName;
  SaAmfComponentCapabilityModelT cap_mod;
  SaAmfHandleT  dummy_boy;
 


 switch (sub_test_arg)
 {
 case 1:


  if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
  {
   
    if((error ==  saAmfComponentCapabilityModelGet (gl_TetSafHandle,
                                        &SaCompName,
                                        &cap_mod)) == SA_AIS_OK)

    {
      tet_print("The Capability Model is %d \n",cap_mod);
      tet_printf("The Capability Model is %d \n",cap_mod);
      tet_result(TET_PASS);
 
   }
  }

 break;

 case 2:

  if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
  {
   
    if((error ==  saAmfComponentCapabilityModelGet (gl_TetSafHandle,
                                        &SaCompName,
                                        &cap_mod)) == SA_AIS_OK)

    {
      tet_print("The Capability Model is %d \n",cap_mod);
      tet_printf("The Capability Model is %d \n",cap_mod);
      tet_result(TET_PASS);
 
   }

 }

 break;

 case 3: 

  if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
  {
   
    if((error ==  saAmfComponentCapabilityModelGet (gl_TetSafHandle,
                                        NULL,
                                        NULL)) == SA_AIS_OK)

    {
      tet_print("The Capability Model is %d \n",cap_mod);
      tet_printf("The Capability Model is %d \n",cap_mod);
      tet_result(TET_PASS);
 
   }
  
 }

 case 4:

  if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
  {

      if((error ==  saAmfComponentCapabilityModelGet (&dummy_boy,
                      &SaCompName,
                      &cap_mod)) != SA_AIS_OK)

      {
          tet_print("The Capability Model is %d \n",cap_mod);
          tet_printf("The Capability Model is %d \n",cap_mod);
          tet_result(TET_PASS);

      }
  } 

   break;

  }

#endif

}

void tet_saf_pend_oper_get ()
{


 SaAisErrorT error;
 SaNameT  SaCompName;

        if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
        {


        }


}



void tet_saf_pm_start(int sub_test_arg)
{
   SaNameT  SaCompName;
   /*SaNameT dummyName;
   uint32_t pid;*/
   SaAisErrorT error;
   int dummy;
                                                                               
                                                                                
   switch(sub_test_arg)
   {
      case 1:
           tet_infoline("Test Case ID: AVSV_API_23\n");
           tet_infoline(" calling PmStart with all valid params");

           /* initialize with all valid param */
           tet_saf_succ_init(3,1);
                                                                                
          if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
          {
                tet_infoline(" Error getting comp name");
                return;
          }
                                                                                
           if(saAmfPmStart(gl_TetSafHandle, &SaCompName,getpid(),-1,1,SA_AMF_COMPONENT_RESTART) == SA_AIS_OK)
              tet_result(TET_PASS); 
           else
              tet_result(TET_FAIL); 

           saAmfPmStop(gl_TetSafHandle, &SaCompName, SA_AMF_PM_ALL_PROCESSES, getpid(), 1);                                                                     
           tet_saf_cleanup(6);
         break;
      case 2:
           tet_infoline("Test Case ID: AVSV_API_24\n");                                     
           tet_infoline(" calling PmStart with NULL Handle");
           
          /* initialize with all valid param */
           tet_saf_succ_init(3,1);

           if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
          {
                tet_infoline(" Error getting comp name");
                return;
          }


          if(saAmfPmStart(0, &SaCompName,getpid(),0,1,SA_AMF_COMPONENT_RESTART) == SA_AIS_ERR_BAD_HANDLE)
              tet_result(TET_PASS); 
           else
              tet_result(TET_FAIL); 
                                                                                
           tet_saf_cleanup(6); 
         break;
      case 3:
           tet_infoline("Test Case ID: AVSV_API_25\n");
           tet_infoline(" calling PmStart with invalid Handle");
                                                                                
           /* initialize with all valid param */
           tet_saf_succ_init(3,1);

           if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
          {
                tet_infoline(" Error getting comp name");
                return;
          }

            
                                                                                
           if(saAmfPmStart(gl_TetSafHandle + 1, &SaCompName,getpid(),0,1, SA_AMF_COMPONENT_RESTART) ==SA_AIS_ERR_BAD_HANDLE)
              tet_result(TET_PASS);
            else
              tet_result(TET_FAIL);
                                                                                
           tet_saf_cleanup(6);
         break;
     case 4:
           tet_infoline("Test Case ID: AVSV_API_26\n");
           tet_infoline(" calling PmStart with NULL component name");
                                                                                
           /* initialize with all valid param */
           tet_saf_succ_init(3,1);
           if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
          {
                tet_infoline(" Error getting comp name");
                return;
          }

                                                                                
           if(saAmfPmStart(gl_TetSafHandle, 0, getpid(), 0, 1, SA_AMF_COMPONENT_RESTART) == SA_AIS_ERR_INVALID_PARAM)
              tet_result(TET_PASS);
           else
              tet_result(TET_FAIL);
                                                                                
           tet_saf_cleanup(6); 
         break;
      case 5:
           tet_infoline("Test Case ID: AVSV_API_27\n");
           tet_infoline(" calling PmStart with Invalid component name");
                                                                                
           /* initialize with all valid param */
            tet_saf_succ_init(3,1);
                                                                                
           {
              SaNameT  SaCompName;
              strcpy(SaCompName.value,"Invalid_Comp");
              SaCompName.length = strlen("Invalid_Comp");
           if(saAmfPmStart(gl_TetSafHandle, &SaCompName, getpid(), 0, 1,SA_AMF_COMPONENT_RESTART) == SA_AIS_ERR_NOT_EXIST)
              tet_result(TET_PASS);
           else
              tet_result(TET_FAIL);
           }
                                                                                
           tet_saf_cleanup(6);  
         break;
      case 6:
           tet_infoline("Test Case ID: AVSV_API_28\n");
           tet_infoline(" calling PmStart with PID = 0");
                                                                                
           /* initialize with all valid param */
           tet_saf_succ_init(3,1);
           if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
          {
                tet_infoline(" Error getting comp name");
                return;
          }

           if(saAmfPmStart(gl_TetSafHandle, &SaCompName, 0, 0, 1, SA_AMF_COMPONENT_RESTART) == SA_AIS_ERR_INVALID_PARAM)
              tet_result(TET_PASS);
           else
              tet_result(TET_FAIL);
                                                                                
           tet_saf_cleanup(6);  
         break;
      case 7:
           tet_infoline("Test Case ID: AVSV_API_29\n");
           tet_infoline(" calling PmStart with Tree Depth -1");
                                                                                
           /* initialize with all valid param */
           tet_saf_succ_init(3,1);

           if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
          {
                tet_infoline(" Error getting comp name");
                return;
          }
                                                                     
                                                                                
           if((error=saAmfPmStart(gl_TetSafHandle, &SaCompName, getpid(), -1, 1, SA_AMF_COMPONENT_RESTART)) == SA_AIS_OK)
             {
              tet_result(TET_PASS);
             tet_print("\nPM START ERR=%d\n", error);
             }   
           else
            {
             tet_print("\nPM START ERR=%d\n", error);   
              tet_result(TET_FAIL);
            }
           saAmfPmStop(gl_TetSafHandle, &SaCompName, SA_AMF_PM_ALL_PROCESSES, getpid(), 1);                                                                       
           tet_saf_cleanup(6); 
         break;
      case 8:
           tet_infoline("Test Case ID: AVSV_API_30\n");
           tet_infoline(" calling PmStart with Invalid value for PmError");
                                                                                
           tet_saf_succ_init(3,1);

           if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
          {
                tet_infoline(" Error getting comp name");
                return;
          }

           
            {
                                                                                              
           if(saAmfPmStart(gl_TetSafHandle, &SaCompName, getpid(), 0, dummy, SA_AMF_COMPONENT_RESTART) == SA_AIS_ERR_INVALID_PARAM)
              tet_result(TET_PASS);
           else
              tet_result(TET_FAIL);
                                                                                
           }
           tet_saf_cleanup(6); 
         break;

      case 9:
          tet_infoline("Test Case ID: AVSV_API_31\n");
          tet_infoline(" calling PmStart with Invalid recommended recovery");
                                                                                
           /* initialize with all valid param */
           tet_saf_succ_init(3,1);
           if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
          {
                tet_infoline(" Error getting comp name");
                return;
          }

           if(saAmfPmStart(gl_TetSafHandle, &SaCompName, getpid(), 0, 1, 100) == SA_AIS_ERR_INVALID_PARAM)
              tet_result(TET_PASS);
           else
              tet_result(TET_FAIL);
                                                                                
           tet_saf_cleanup(6);
         break;
      
      default:
           tet_infoline(" PmStart with invalid test case");
         break;
    
 }

return;

}


void tet_saf_pm_stop(int sub_test_arg)
{

   SaNameT  SaCompName;
   /*uint32_t pid;*/

                               
                                                                                
   switch(sub_test_arg)
   {
      case 1:
           tet_infoline("Test Case ID: AVSV_API_32\n");
           tet_infoline(" calling PmStop with all valid param");
           /* initialize with all valid param */
           tet_saf_succ_init(3,1);

           if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
           {
               tet_infoline(" Error getting comp name");
               return;
           }
                                                                                
           /* start PM with all valid param */
           
           if(saAmfPmStart(gl_TetSafHandle, &SaCompName,getpid(),0,1,SA_AMF_COMPONENT_RESTART) == SA_AIS_OK)
              tet_infoline("PmStart Sucess");
           else
              tet_infoline("PmStart Failure");

           if(saAmfPmStop(gl_TetSafHandle, &SaCompName, SA_AMF_PM_ALL_PROCESSES, getpid(), 1) == SA_AIS_OK)
              tet_result(TET_PASS);
           else
              tet_result(TET_FAIL);
                                                                                
           tet_saf_cleanup(6);  
                                                                                
         break;
      case 2:
           tet_infoline("Test Case ID: AVSV_API_33\n");                                                                      
           tet_infoline(" calling PmStop with NULL Handle ");
           /* initialize with all valid param */
           tet_saf_succ_init(3,1);


          if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
           {
               tet_infoline(" Error getting comp name");
               return;
           }
 
                                                                                
           /* start PM with all valid param */
       
           if(saAmfPmStart(gl_TetSafHandle, &SaCompName,getpid(),0,1,SA_AMF_COMPONENT_RESTART) == SA_AIS_OK)
              tet_infoline("PmStart Sucess");
           else
              tet_infoline("PmStart Failure");

                                                                                
           if(saAmfPmStop(0, &SaCompName, SA_AMF_PM_ALL_PROCESSES, getpid(), 1) == SA_AIS_ERR_BAD_HANDLE)
              tet_result(TET_PASS);
           else
              tet_result(TET_FAIL);

           saAmfPmStop(gl_TetSafHandle, &SaCompName, SA_AMF_PM_ALL_PROCESSES, getpid(), 1);
           tet_saf_cleanup(6);  
         break;
      case 3:

           tet_infoline("Test Case ID: AVSV_API_34\n");
                                                                      
           tet_infoline(" calling PmStop with Invalid Handle");
           /* initialize with all valid param */
           tet_saf_succ_init(3,1);
           if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
           {
               tet_infoline(" Error getting comp name");
               return;
           }
   
           /* start PM with all valid param */

           if(saAmfPmStart(gl_TetSafHandle, &SaCompName,getpid(),0,1,SA_AMF_COMPONENT_RESTART) == SA_AIS_OK)
              tet_infoline("PmStart Sucess");
           else
              tet_infoline("PmStart Failure");


           if(saAmfPmStop(gl_TetSafHandle + 1, &SaCompName, SA_AMF_PM_ALL_PROCESSES, getpid(), 1) == SA_AIS_ERR_BAD_HANDLE)
              tet_result(TET_PASS);
           else
              tet_result(TET_FAIL);
                                                                                
           saAmfPmStop(gl_TetSafHandle, &SaCompName, SA_AMF_PM_ALL_PROCESSES, getpid(), 1);                                                                               
           tet_saf_cleanup(6);  
                                                                                
         break;
         case 4:
           tet_infoline("Test Case ID: AVSV_API_35\n");                                                                                
           tet_infoline(" calling PmStop with NULL comp name");
           /* initialize with all valid param */
           tet_saf_succ_init(3,1);

           if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
           {
               tet_infoline(" Error getting comp name");
               return;
           }
                                                                     
           /* start PM with all valid param */

           if(saAmfPmStart(gl_TetSafHandle, &SaCompName,getpid(),0,1,SA_AMF_COMPONENT_RESTART) == SA_AIS_OK)
              tet_infoline("PmStart Sucess");
           else
              tet_infoline("PmStart Failure");

                                                                                
           if(saAmfPmStop(gl_TetSafHandle, 0, SA_AMF_PM_ALL_PROCESSES, getpid(), 1) == SA_AIS_ERR_INVALID_PARAM)
              tet_result(TET_PASS);
           else
              tet_result(TET_FAIL);

           saAmfPmStop(gl_TetSafHandle, &SaCompName, SA_AMF_PM_ALL_PROCESSES, getpid(), 1);                                                                                
           tet_saf_cleanup(6);
              break;
      case 5:
           tet_infoline("Test Case ID: AVSV_API_36\n");                                                                                
           tet_infoline(" calling PmStop with Invalid comp name");
           /* initialize with all valid param */
           tet_saf_succ_init(3,1);


           if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
           {
               tet_infoline(" Error getting comp name");
               return;
           }

           /* start PM with all valid param */

           if(saAmfPmStart(gl_TetSafHandle, &SaCompName,getpid(),0,1,SA_AMF_COMPONENT_RESTART) == SA_AIS_OK)
              tet_infoline("PmStart Sucess");
           else
              tet_infoline("PmStart Failure");

           {
              SaNameT  SaCompName;
              strcpy(SaCompName.value,"Invalid_Comp");
              SaCompName.length = strlen("Invalid_Comp");
           if(saAmfPmStop(gl_TetSafHandle, &SaCompName, SA_AMF_PM_ALL_PROCESSES, getpid(), 1) == SA_AIS_ERR_NOT_EXIST)
              tet_result(TET_PASS);
           else
              tet_result(TET_FAIL);
           }
                                                                                
           tet_saf_cleanup(6);  
                                                                                
         break;
      case 6:
           tet_infoline("Test Case ID: AVSV_API_37\n");                                                                     
           tet_infoline(" calling PmStop with Invalid Stop Qual");
           /* initialize with all valid param */
           tet_saf_succ_init(3,1);
           if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
           {
               tet_infoline(" Error getting comp name");
               return;
           }

                                                                                
           /* start PM with all valid param */
           if(saAmfPmStart(gl_TetSafHandle, &SaCompName,getpid(),0,1,SA_AMF_COMPONENT_RESTART) == SA_AIS_OK)
              tet_infoline("PmStart Sucess");
           else
              tet_infoline("PmStart Failure");

           {
              int random = rand();
              while(random == SA_AMF_PM_PROC || random == SA_AMF_PM_PROC_AND_DESCENDENTS || random == SA_AMF_PM_ALL_PROCESSES)
              random =rand();
                                                                                
           if(saAmfPmStop(gl_TetSafHandle, &SaCompName, random, getpid(), 1) == SA_AIS_ERR_INVALID_PARAM)
              tet_result(TET_PASS);
           else
              tet_result(TET_FAIL);
           }
           saAmfPmStop(gl_TetSafHandle, &SaCompName, SA_AMF_PM_ALL_PROCESSES, getpid(), 1);                                                                                
           tet_saf_cleanup(6);
                                                                                
         break;
      case 7:
          tet_infoline("Test Case ID: AVSV_API_38\n");                                                                                  
          tet_infoline(" calling PmStop with PID =0");
           /* initialize with all valid param */
           tet_saf_succ_init(3,1);
          if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
           {
               tet_infoline(" Error getting comp name");
               return;
           }

                                                                                
           /* start PM with all valid param */

           if(saAmfPmStart(gl_TetSafHandle, &SaCompName,getpid(),0,1,SA_AMF_COMPONENT_RESTART) == SA_AIS_OK)
              tet_infoline("PmStart Sucess");
           else
              tet_infoline("PmStart Failure");

                                                                                
           if(saAmfPmStop(gl_TetSafHandle, &SaCompName, SA_AMF_PM_ALL_PROCESSES, 0, 1) == SA_AIS_ERR_INVALID_PARAM)
              tet_result(TET_PASS);
           else
              tet_result(TET_FAIL);
      saAmfPmStop(gl_TetSafHandle, &SaCompName, SA_AMF_PM_ALL_PROCESSES, getpid(), 1);
      tet_saf_cleanup(6);
     break;
      case 8:
           tet_infoline("Test Case ID: AVSV_API_39\n");                                                                                
           tet_infoline(" calling PmStop with Invalid PID ");
           /* initialize with all valid param */
           tet_saf_succ_init(3,1);
           if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
           {
               tet_infoline(" Error getting comp name");
               return;
           }
                                                                     
           /* start PM with all valid param */

           if(saAmfPmStart(gl_TetSafHandle, &SaCompName,getpid(),0,1,SA_AMF_COMPONENT_RESTART) == SA_AIS_OK)
              tet_infoline("PmStart Sucess");
           else
              tet_infoline("PmStart Failure");

                                                                                
           if(saAmfPmStop(gl_TetSafHandle, &SaCompName, SA_AMF_PM_ALL_PROCESSES, rand(), 1) == SA_AIS_ERR_NOT_EXIST)
              tet_result(TET_PASS);
           else
              tet_result(TET_FAIL);
           saAmfPmStop(gl_TetSafHandle, &SaCompName, SA_AMF_PM_ALL_PROCESSES, getpid(), 1);                                                                                 
           tet_saf_cleanup(6);
                                                                                
         break;
      case 9:
            tet_infoline("Test Case ID: AVSV_API_40\n");                                                                                
           tet_infoline(" calling PmStop with Invalid value for PmError");
           /* initialize with all valid param */
           tet_saf_succ_init(3,1);


          if(saAmfComponentNameGet(gl_TetSafHandle,&SaCompName) != SA_AIS_OK)
           {
               tet_infoline(" Error getting comp name");
               return;
           }

                                                                                
           /* start PM with all valid param */

           if(saAmfPmStart(gl_TetSafHandle, &SaCompName,getpid(),0,1,SA_AMF_COMPONENT_RESTART) == SA_AIS_OK)
              tet_infoline("PmStart Sucess");
           else
              tet_infoline("PmStart Failure");

                                                                              
              /* generate a random number which doesnt belong to the window {-1,2} */
              int random = rand();
              while(random  == SA_AMF_PM_NON_ZERO_EXIT || random == SA_AMF_PM_ZERO_EXIT)
                 random =rand();
                                                                                
           if(saAmfPmStop(gl_TetSafHandle, &SaCompName, random , getpid(), 1) == SA_AIS_ERR_INVALID_PARAM)
              tet_result(TET_PASS);
           else
              tet_result(TET_FAIL);

           saAmfPmStop(gl_TetSafHandle, &SaCompName, SA_AMF_PM_ALL_PROCESSES, getpid(), 1);
           tet_saf_cleanup(6);
                                                                                
         break;
      default:
           tet_infoline(" PmStop with invalid test case");
         break;
 }

return;

}
                                                                                                       
void tware_pg_track(int sub_test_arg)
{

    SaAisErrorT error;
    SaNameT trk_csi_name = gl_csidesc.csiName;
    SaUint8T trk_flag = SA_TRACK_CURRENT | SA_TRACK_CHANGES_ONLY;
    SaAmfProtectionGroupNotificationT buf[6];
    SaAmfProtectionGroupNotificationBufferT notif_buff;

    notif_buff.numberOfItems = 6;
    notif_buff.notification = buf;

    if (trk_csi_name.length != 0 )
    {

        if((error =  saAmfProtectionGroupTrack(gl_TetSafHandle,
                        &trk_csi_name,
                        trk_flag,
                        NULL)) != SA_AIS_OK)
        {
            tet_print("\nProtection GROUP TRACKING FAILED with ERR VAL %d !!\n\n",error);
        }
        else {

            tet_print("\nProtection GROUP TRACKING OK !!\n\n");

        }
    }

      /*  tet_print("\nProtection GROUP TRACKING NOT CALLED !!\n\n"); */
    return;


}


int tet_exec_tests(struct svc_testlist *tlp )
{
                                                                                                       
        for (; tlp->testfunc != 0 ; tlp++)
                        (*tlp->testfunc)(tlp->arg);
                                                                                                       
        return(0);
}
                                                                                                     
#if 0
void tware_exec()
{
                                                                                                       
                                                                                                       
 tet_exec_tests(&svc_list);
                                                                                                       
}

#endif

#endif




