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


#define TET_COMP_INSTANCE "/etc/ncs/comp_instance"
#define TET_TEMP_CI "/etc/ncs/temp"

/********************************************************************************/

int             g_HCount = 0;
int             g_HCount_1 = 0;
int             g_HA_state=0;

tmr_t            g_Timer_ID;
tmr_t            g_Timer_ID_1;

SYSF_MBX        gl_mds_dummy_mbx;
void           *gl_mds_dummy_task_hdl;
SaAmfHandleT    gl_TetSafHandle;
SaAmfCSIDescriptorT gl_csidesc;

SYSF_MBX       g_tet_mbx;

extern struct tet_testlist avsv_list[];
extern struct tet_testlist avsv_list_other[];

int systemid_list[] = {0,1};        /* system IDs to be synced */


/********************************************************************************/

/*
 * Shardool: 29/DEC/2005
 * Added these prototypes otherwise the MVL toolchain will crib..
 */
void tware_avsv_proc_evt(TWARE_EVT *evt);
void tware_evt_send(int evt_val);

/*
* Shardool: renamed, because the function with this name is being defined 
* in amf_test_core.c
*/
void tet_avsv_startup() 
{

uint32_t rc = NCSCC_RC_SUCCESS ; 

#ifdef TET_AVSV 
#ifdef TET_A
    tet_infoline("AMF API Test\n");

        rc = m_NCS_IPC_CREATE(&g_tet_mbx); 
        
        rc = m_NCS_IPC_ATTACH(&g_tet_mbx);

    /* AvSv start function should be included here */

        tet_saf_selobject();
#endif
#endif


}

#ifdef TET_A
/*****************************************************************************/
/* Function Name :  tet_saf_health_chk_callback                              */
/*                                                                           */
/* Description   : This is the callback function for the AMF Invoked         */
/*                 Health checks                                             */
/*                                                                           */
/*                                                                           */
/* Test Proc Ref :                                                           */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

#define CSI_NCS1 "safCsi=Csi_NCS1,safSi=Si_NCS"
#define CSI_PRXY "safCsi=Csi_NCSP,safSi=Si_NCSP"

void
tet_saf_health_chk_callback (SaInvocationT invocation,
        const SaNameT *compName,
        SaAmfHealthcheckKeyT *checkType)
{
  SaAisErrorT error = SA_AIS_OK;
  /*int rc= 100 ;*/
  /* Shardool: hardcoded values for csi name */

/* SaNameT csiName = {28, CSI_NCS1};
 SaNameT csiPrxy = {29, CSI_PRXY};

  SaAmfHAStateT ha_state;*/

  saAmfResponse(gl_TetSafHandle,invocation, error);
  g_HCount++; 
    
  tet_print ("Invoked HC Cbk for the %d \n",g_HCount);
  tet_print("The Comp Name is %s\n",compName->value);  
  tet_printf("The Comp Name is %s\n",compName->value);  
  tet_print("The Healthcheck Key is %s \n",checkType->key); 
#if 1
      if(g_HCount == 50 )
       tware_evt_send(300);
#endif
    return;
 
}



void
tet_saf_CSI_set_callback (SaInvocationT invocation,
                          const SaNameT  *compName,
                          SaAmfHAStateT  haState,
                          SaAmfCSIDescriptorT csidesc)
{
    SaAisErrorT error = SA_AIS_OK;
    SaAmfHAStateT  ha_state;


    tet_print(" Invoked CSI Set Cbk \n");
    saAmfResponse(gl_TetSafHandle,invocation, error);

    tet_printf("The CSI NAME with rcvd params IS %s\n and the HA state %d  \n",csidesc.csiName.value,haState);
    tet_print("The CSI NAME with rcvd params IS %s\n and the HA state %d  \n",csidesc.csiName.value,haState);
    tet_printf(" The CSI FLAGS %d \n", csidesc.csiFlags);
    tet_print(" The CSI FLAGS %d \n", csidesc.csiFlags);

    tet_print(" The arguments to saAmfHAStateGet are \n compname = %s\n csi_name = %s\n HA state  = %d \n", 
                                                                                            compName->value , 
                                                                                         csidesc.csiName.value , 
                                                                                            haState);

    tet_printf(" The CSI NAME IS %s\n and the HA state %d  \n",csidesc.csiName.value,ha_state);

    gl_csidesc = csidesc;
    
    if (csidesc.csiFlags != 4)
    {
        g_HA_state = ha_state;  
    }
    return;

}


void 
tet_saf_CSI_remove_callback (SaInvocationT invocation,
                           const SaNameT *compName,
                           const SaNameT *csiName,
                           SaAmfCSIFlagsT csiFlags)

{
    SaAisErrorT error = SA_AIS_OK;          

    tet_print(" Invoked CSI Remove Set Cbk \n");
    saAmfResponse(gl_TetSafHandle,invocation, error);
}



void 
tet_saf_Comp_Term_callback(SaInvocationT invocation,
                                    const SaNameT *compName)

{

    SaAisErrorT error = SA_AIS_OK;          
    tet_print(" Invoked Comp Terminate  Cbk \n");
    tet_printf(" Invoked Comp Terminate  Cbk \n");
    saAmfResponse(gl_TetSafHandle,invocation, error);
    tware_evt_send(300);
}



void tet_saf_selobject()
{
    /*uint32_t               comp_type;*/
    NCS_SEL_OBJ_SET     all_sel_obj;
    NCS_SEL_OBJ         mbx_fd = m_NCS_IPC_GET_SEL_OBJ(& g_tet_mbx);    
    SaSelectionObjectT  amf_sel_obj;
    SaAisErrorT         amf_error;   
    NCS_SEL_OBJ         ams_ncs_sel_obj;
    NCS_SEL_OBJ         high_sel_obj;
    TWARE_EVT           *evt= NULL;
    /*char                *tmp_ptr = NULL;
    uint32_t               succ_init_option= 3 ;*/
    
#if 1
   /* start  unit test ***/        
     tet_print("AvSv Unit Tests started\n");
     tet_test_start(-1,avsv_list);   

     tet_test_start(-1,avsv_list_other);  

       tet_print("Completed Unit Tests\n");
       sleep(1);  

    /* tet_test_cleanup(); */
   /* completed unit test ***/       
#endif

    tet_saf_succ_init(7,2);  

    m_NCS_SEL_OBJ_ZERO((fd_set *)&all_sel_obj); 
    m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);

    amf_error = saAmfSelectionObjectGet(gl_TetSafHandle, &amf_sel_obj);

    if (amf_error != SA_AIS_OK)
    {     
        tet_infoline("Error getting selection object \n");       
        return;
    }
    m_SET_FD_IN_SEL_OBJ(amf_sel_obj,ams_ncs_sel_obj);
    m_NCS_SEL_OBJ_SET(ams_ncs_sel_obj, &all_sel_obj);

        high_sel_obj = m_GET_HIGHER_SEL_OBJ(ams_ncs_sel_obj,mbx_fd);

    
    while (m_NCS_SEL_OBJ_SELECT(high_sel_obj,&all_sel_obj,0,0,NULL) != -1)
    {      
        /* process all the AMF messages */
        if (m_NCS_SEL_OBJ_ISSET(ams_ncs_sel_obj,&all_sel_obj))
        {
         /* dispatch all the AMF pending function */
/*          amf_error = saAmfDispatch(gl_TetSafHandle, SA_DISPATCH_BLOCKING); 
            amf_error = saAmfDispatch(gl_TetSafHandle, SA_DISPATCH_ONE); 
*/ 
            amf_error = saAmfDispatch(gl_TetSafHandle, SA_DISPATCH_ONE); 
            if (amf_error != SA_AIS_OK)
            {
              tet_print("Dispatch failure\n");
            }
        }
#if 1
         if (m_NCS_SEL_OBJ_ISSET(mbx_fd,&all_sel_obj))
     {
         /* now got the IPC mail box event */
         if ((evt = (TWARE_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(&g_tet_mbx,evt)))
         {
             tware_avsv_proc_evt(evt);
         }  

     }
#endif
             m_NCS_SEL_OBJ_SET(ams_ncs_sel_obj,&all_sel_obj);
             m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);   
        

    }
}

/* Shardool */
void tet_saf_proxied_inst_callback(SaInvocationT invocation, const SaNameT *proxied_name ){
  tet_print("CALLBACK: saAmfProxiedComponentInstantiateCallback\n");
  system("xterm");
  
}

void tet_saf_succ_init(int depth, int fill_cbks)
{

    SaAmfCallbacksT amfCallbacks;
    SaVersionT      amf_version;   
    SaAisErrorT        error;
    /*uint32_t           res = NCSCC_RC_SUCCESS;
    uint32_t           Comp_type;*/
        uint32_t           dummy=1;   
        uint32_t           bit_count=0; 
    SaNameT  SaCompName;
    SaAmfHealthcheckKeyT Healthy;

    /* Shardool */
    SaNameT proxied_comp = {60,"safComp=CompT_NCSPrx,safSu=SuT_NCSPrx,safNode=Node1_TypeSCXB"};
     
    memset(&amfCallbacks, 0, sizeof(SaAmfCallbacksT));

     if ( fill_cbks )
     {
    amfCallbacks.saAmfCSISetCallback        = tet_saf_CSI_set_callback;
        amfCallbacks.saAmfCSIRemoveCallback     = tet_saf_CSI_remove_callback;
        amfCallbacks.saAmfComponentTerminateCallback = tet_saf_Comp_Term_callback;

        /* Shardool: proxy callback */
        amfCallbacks.saAmfProxiedComponentInstantiateCallback  = tet_saf_proxied_inst_callback;

        if (fill_cbks == 2)
       amfCallbacks.saAmfHealthcheckCallback = tet_saf_health_chk_callback;
     }
  
     m_TET_GET_AMF_VER(amf_version);
         tet_print(" SUCC_INIT OPTION IS  %d",depth);

        for(bit_count =0;bit_count < 8 ; bit_count++) {
 
        if ((depth & (dummy << bit_count))) 
        {

        switch( bit_count+1)
        { 
  
         case 1: 
                tet_infoline("Calling Initialize at succ_init ");  
        if  ((error = saAmfInitialize(&gl_TetSafHandle, &amfCallbacks, &amf_version))
                != SA_AIS_OK)

        {
            tet_infoline("saAmfInitialize init failed ");
            return;
        }
        tet_infoline("saAmfInitialize@succ_init success ");
         break;

        case 2: 
                tet_infoline("Calling comp register  at succ_init ");  
        if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
        {
            if ((  error = saAmfComponentRegister(gl_TetSafHandle,&SaCompName,NULL))
                    == SA_AIS_OK)
                tet_infoline("Component registration success \n");
             else
                tet_infoline("Component registration failed \n");
#if 0

                    if( (tware_comp_init_count(&SaCompName,1)) == 0)
                    {
                          tware_comp_init_count(&SaCompName,0);
                    }
                    else {
                           
                          /* increment the instantiate count */
                          tware_comp_init_count(&SaCompName,4);
                    }
#endif
        }
/*               tet_saf_HC_tmr_cbk();   */
        break;

        case 3:
        /* AMF Invoked HC */
        memset(&Healthy, 0, sizeof(SaAmfHealthcheckKeyT));
        strcpy(Healthy.key,"ABCDEF20");
    Healthy.keyLen=strlen(Healthy.key);  
        
        tet_infoline("Calling AMF Health Check  at succ_init\n");  
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ((error = saAmfHealthcheckStart(
                        gl_TetSafHandle,
                        &SaCompName,
                        &Healthy,
                        SA_AMF_HEALTHCHECK_AMF_INVOKED,
                            SA_AMF_COMPONENT_FAILOVER
                        )) != SA_AIS_OK) 
        {
        tet_infoline("Health Check registration failed \n");
        tet_print(" HEALTHCHK_START: AMF_INVOKED FAILURE\n");        
        }
        tet_print(" HEALTHCHK_START: AMF_INVOKED SUCCESS\n");

    }

        break;

        case 4:
        /* Comp Invoked HC */
        memset(&Healthy, 0, sizeof(SaAmfHealthcheckKeyT));
    strcpy(Healthy.key,"ABCDEF20");
    Healthy.keyLen=strlen(Healthy.key); 
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
        tet_infoline("Health Check registration OK for ABCDEF20 \n"); 
                tet_print(" Starting the HC timer  for ABCDEF20 \n");
                tet_saf_HC_tmr_cbk(); 
                }
    }
      
      break; 

      case 5:
      /* One Comp Invoked and one AMF Invoked */ 
    strcpy(Healthy.key,"ABCDEF30");
    Healthy.keyLen=strlen(Healthy.key); 
    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ((error = saAmfHealthcheckStart(
                        gl_TetSafHandle,
                        &SaCompName,
                        &Healthy,
                        SA_AMF_HEALTHCHECK_COMPONENT_INVOKED,
                        SA_AMF_COMPONENT_RESTART
                        )) == SA_AIS_OK) 
        {
        tet_infoline("Health Check registration OK for ABCDEF30 \n");
                tet_print(" Starting the HC timer for ABCDEF30 \n");
                tet_saf_HC_tmr_cbk(); 
                }

    }

        memset(&Healthy, 0, sizeof(SaAmfHealthcheckKeyT));
    strcpy(Healthy.key,"ABCDEF20");
    Healthy.keyLen=strlen(Healthy.key); 
         
         if ((error = saAmfHealthcheckStart(
                                                gl_TetSafHandle,
                                                &SaCompName,
                                                &Healthy,
                                                SA_AMF_HEALTHCHECK_AMF_INVOKED,
                                                SA_AMF_COMPONENT_FAILOVER
                                                )) != SA_AIS_OK)
                {
                tet_infoline("Health Check registration failed \n");
                }
                tet_print("Health Check registration SUCESSS for ABCDEF30 \n");


        break;

        /* 
         * Shardool: added the entire case 
         * Register the process as a proxy component for saCompT=CompT_NCSP
         */ 
        case 6:
          
          tet_infoline("Calling comp register  at succ_init ");  
          if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
            {
              if ((  error = saAmfComponentRegister(gl_TetSafHandle, &proxied_comp, &SaCompName))
                  != SA_AIS_OK)
                {
                  tet_infoline("Component registration failed \n");
                  tet_print("PROXIED_REGISTER: failure\n");
                } else {
                  tet_print("PROXIED_REGISTER: success\n");
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
          
    } /* End switch */

    } /* End if */

   } /* End For */

}

void tet_saf_cleanup(int depth)
{

    SaAisErrorT     error;
    /*uint32_t           Comp_type;*/
        uint32_t           dummy=1;   
        uint32_t           bit_count=0; 
    SaNameT         SaCompName;
    SaAmfHealthcheckKeyT Healthy;

        tet_printf(" Calling the saf cleanup function\n");
         for(bit_count =0;bit_count < 8 ; bit_count++) {
 
        if ((depth & (dummy << bit_count))) 
        {

        /* Each case corresponds to one bit and moves from right to left */
        switch( bit_count+1)
        { 

       case 1: 
        memset(&Healthy, 0, sizeof(SaAmfHealthcheckKeyT));
    strcpy(Healthy.key,"ABCDEF20");
    Healthy.keyLen=strlen(Healthy.key); 

    if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ((error = saAmfHealthcheckStop( gl_TetSafHandle, &SaCompName, &Healthy ))
                == SA_AIS_OK)

        {
            tet_infoline("Stopping Health checks"); 
            tet_print("Stopping Health checks For ABCDEF20"); 
        }

        else {
            tet_infoline("Unable to stop Health Check"); 
            tet_print("Unable to stop Health Check"); 
        }  

    }

       break;

        case 2:

     if((error = saAmfComponentNameGet(gl_TetSafHandle,&SaCompName)) == SA_AIS_OK)
    {
        if ((  error = saAmfComponentUnregister(gl_TetSafHandle,&SaCompName,NULL))
                == SA_AIS_OK)
        { 
            tet_infoline("unregistered the component ");
        }  
        else 
        {
            tet_infoline("Unable to unreg component ");
                        tet_print(" The Un-Reg comp error is %d \n",error);   
                        tet_printf(" The Un-Reg comp error is %d \n",error);   
        } 
    } 

       break;
 
        case 3: 

             tet_infoline("Finalizing with AMF"); 
             saAmfFinalize(gl_TetSafHandle);

        break;

      } /* End if */
      
     } /* End Switch */ 

    } /* End For */
   

}


/* This is based on 10msec timer values */
#define HC_CBK_REFRESH_INTERVAL 400

void tet_saf_HC_tmr_cbk ()
{
      TWARE_EVT *hc_evt;
    
      SaNameT compName;
      int rc=100;
      SaAisErrorT error = SA_AIS_OK;
    

/*    tet_print(" The Timer callback is invoked \n"); */

    /* create the timer */
    if ( g_Timer_ID == 0 )   
    {
        m_NCS_TMR_CREATE(g_Timer_ID,HC_CBK_REFRESH_INTERVAL, 
                tet_saf_HC_tmr_cbk,NULL);    

        m_NCS_TMR_START(g_Timer_ID,HC_CBK_REFRESH_INTERVAL, 
                (TMR_CALLBACK)tet_saf_HC_tmr_cbk,NULL);

         /* sending the confirms the first time */

         tet_saf_health_chk_confirm (6); 
        tet_print("Executing... tet_saf_health_chk_confirm(6)\n ");
        /*         tet_saf_health_chk_confirm (9); */

    tet_print(" The Timer has been created & and a confirm sent for ABCDEF20 \n"); 

    }
    /* stop and start the timer */  
    else 
    {
        m_NCS_TMR_STOP(g_Timer_ID);
        /* start the timer */  
        m_NCS_TMR_START(g_Timer_ID,HC_CBK_REFRESH_INTERVAL, 
                (TMR_CALLBACK )tet_saf_HC_tmr_cbk,NULL);    

        /* send the health check confirm */

            hc_evt = (TWARE_EVT *)     malloc(sizeof(TWARE_EVT));
        hc_evt->type = 100 ;   
        m_NCS_IPC_SEND(&g_tet_mbx, hc_evt,1);   
  
/*        tet_print(" The Timer and a confirm sent for ABCDEF20 \n");*/

    } 

      g_HCount++;
                         
        if((error = saAmfComponentNameGet(gl_TetSafHandle,&compName)) == SA_AIS_OK)
     {                                                 
          tet_print("The Comp Name is %s\n",compName.value);   
    }
     else
           tet_print(" Error in CompNameGet ERR: %d \n",error);


    if (g_HCount == 5)
       {
           if (((rc=strcmp((char *)compName.value,"safComp=CompT_NCS,safSu=SuT_NCS,safNode=Node1_TypeSCXB")) == 0))
           {
               if((tware_comp_init_count(&compName,1) % 5) == 0)
               {
                  tet_print(" Sending Erorr Rpt SA_AMF_COMPONENT_RESTART\n");
                  hc_evt = (TWARE_EVT *)     malloc(sizeof(TWARE_EVT));
                  hc_evt->type = 400 ;
                  m_NCS_IPC_SEND(&g_tet_mbx, hc_evt,1);
               }
               else if ((tware_comp_init_count(&compName,1) % 6) == 0 )
               {  
                  tet_print(" Sending Erorr Rpt SA_AMF_COMPONENT_FAILOVER \n");
                  hc_evt = (TWARE_EVT *)     malloc(sizeof(TWARE_EVT));
                  hc_evt->type = 500 ;
                  m_NCS_IPC_SEND(&g_tet_mbx, hc_evt,1);
               }

           }
                                                                                
       }

      
       if ((g_HCount % 20) == 0)
       {
         if ( g_HA_state != 2)
         {
           tet_print(" Stopping the Heath Check and unreg the component  FFFFFFFFF - ABCDEF20 \n");
           tet_test_cleanup();
        }

/*         tet_saf_cleanup(3); */
      }
      
}


void tware_avsv_proc_evt(TWARE_EVT *evt)

{

   int evt_val = evt->type ; 
   tet_printf(" The EVENT %d HAS ARRIVED\n\n",evt_val);
   switch (evt_val)
   {
 
   case 100:
    
     tet_saf_health_chk_confirm (6);  
     /*tet_saf_health_chk_confirm (9);   */
     break; 

   case 300 : 
      
      tet_saf_errorrpt(SA_AMF_COMPONENT_FAILOVER);   
      free(evt);
      tet_test_cleanup(); 
      exit(0);
     break;

   case 400:
    
      tet_saf_errorrpt(SA_AMF_COMPONENT_RESTART);
      break;
    
   case 500:
         
      tet_saf_errorrpt(SA_AMF_COMPONENT_FAILOVER);
      break;

   default:
       break ;


    }
       
     free(evt);
     return;
}

void tware_evt_send(int evt_val)
{
  TWARE_EVT *Tw_evt;

  Tw_evt = (TWARE_EVT *)malloc(sizeof(TWARE_EVT));

  Tw_evt->type = evt_val ;   

   m_NCS_IPC_SEND(&g_tet_mbx, Tw_evt,1);   

}



int  tware_comp_init_count(SaNameT *SaCompName , int invoked)
{
                                                                                                     
                                                                                                     
 FILE *Fp;
 FILE *Temp_Fp;
 int   inst_count=0;
 char  *comp_name;
 int    ret_c=0;
 int   file_read_count;
 /*int   total_file_read_count = 0;*/
 int   inst_ret=0;
                                                                                                     
                                                                                                     
 comp_name = (char *) malloc(100);
 Fp = fopen(TET_COMP_INSTANCE,"a+");
 Temp_Fp = fopen(TET_TEMP_CI,"w+");

  if ((invoked == 0))
  {
    strcpy(comp_name,(char *)SaCompName->value);
    inst_count = 1;
    fprintf(Fp,"%s %d\n",comp_name,inst_count);
   fclose (Fp);
   fclose (Temp_Fp);
       return 1; 
   }

   while ((file_read_count = fscanf(Fp,"%s %d",comp_name,&inst_count)) != -1)
   {
     if ((ret_c=strcmp((char *)SaCompName->value,comp_name)) == 0)
     {
      inst_ret = inst_count; 
      if ( invoked == 4) 
      {
       inst_count = inst_count +1 ;
        inst_ret = inst_count; 
       }
     } 
    tet_printf(" The compname is %s - %d\n",comp_name,inst_count);
    fprintf(Temp_Fp,"%s %d\n",comp_name,inst_count);
     
   }
   fclose (Fp);
   fclose (Temp_Fp);
   system("cp /etc/ncs/temp /etc/ncs/comp_instance");
   return inst_ret;
}

void tet_avsv_thread_init()
{

        char                *tmp_ptr = NULL;
        uint32_t               succ_init_option= 3 ;

  if((tmp_ptr = (char *) getenv("TET_SUCC_INIT")) != NULL)
        {
           succ_init_option  = atoi(tmp_ptr);  
        }

      tet_saf_succ_init(succ_init_option,2);  


}
 
#ifdef TET_THREAD 
void tet_create_avsv_thread()
{

 pthread_t new_thread;
 uint32_t     thread_id;


 tet_pthread_create(&new_thread, NULL, tet_avsv_thread_init,NULL,-1); 
  

 

}
#endif

#endif



