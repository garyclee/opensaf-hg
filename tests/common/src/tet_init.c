/**********************************************************************************************
#
# modification:
# 2008-11   Jony <jiangnan_hw@huawei.com>  Huawei Technologies CO., LTD.
#   1.  Delete three functions: tware_ncs_startup(), tware_ncs_end(), and tet_svcs_startup(). 
#       Test case tet_startup and tet_cleanup functions are provided in each of the tests. 
#       In this case, developers do not need to modify this file when adding a new test. 
#   2.  If not using the "tetware-patch", delete tet_chill() function and tet_testlist[] array. 
#       Because the array has provided in the test.
#  
************************************************************************************************/

#include "tet_startup.h"
#include "ncs_main_papi.h"
#include "ncssysf_tmr.h"
#include "ncssysfpool.h"

SYS_CFG_INFO sys_cfg;

/* TET function declarations */
#if 0
void tware_ncs_startup(void);
void tware_ncs_end(void);
void tet_chill(int); 
void tet_svcs_startup(void);
void tet_saf_agents_startup(void);

void fill_std_config(char **t_argv);

#ifdef TET_MBCSV
extern void     tet_mbcsv_startup();
#endif
#ifdef TET_EDSV
extern void tet_edsv_startup();
#endif
#ifdef TET_MQSV
extern void tet_mqsv_startup();
#endif
#ifdef TET_GLSV
extern void tet_glsv_startup() ;
#endif
#ifdef TET_CPSV
extern void tet_cpsv_startup() ;
#endif
#ifdef TET_MDS_TDS 
extern void    tet_mds_tds_startup ();
#endif
int fill_syncparameters(int vote);


void tet_avsv_startup(void);


void (*tet_startup)()=tware_ncs_startup;
void (*tet_cleanup)()=tware_ncs_end;
#endif

#define NCS_MAIN_MAX_INPUT 25
#define NCS_MAIN_MAND_PARAMS 7
#define NCS_MAIN_DEF_CLUSTER_ID 1

#define TIMEOUT 10          /* sync time out */
#define TIMEFIVEMINS 300        /* 300 seconds */
int gl_sync_pointnum=1;
                                                                                                                             
typedef struct syncvalues
{
        long syncpointnum;  /* sync point number */
        int *systemids;     /* array of system ids */
        int numofsystemids; /* number of systemids */
        int waittime;       /* system wait time for other systems to sync */
        int vote;           /* TET_SV_YES or TET_SV_NO */
        struct tet_synmsg *message; /* message structure, if info needs to be exchanged during sync */
} SyncParameters;

#ifdef TET_ALL
NCSCONTEXT tet_global_handle;
NCSCONTEXT glsv_handle,cpsv_handle,mqsv_handle,edsv_handle,masv_handle;
void tet_create_test_threads(void);
void tet_create_glsv_thread(void);
void tet_create_mqsv_thread(void);
void tet_create_cpsv_thread(void);
void tet_create_edsv_thread(void);
void tet_create_masv_thread(void);
void tet_run_masv_app(void);
void tet_run_glsv_app();  
void tet_run_mqsv_app();  
void tet_run_cpsv_app();  
void tet_run_edsv_app();
#endif
int tet_remotesync(SyncParameters *syncparam);
/* This would fill the common MDS and node related config */

void fill_std_config (char **t_argv)
{

    uint32_t slot_id = 0 ;
    uint32_t shelf_id = 0;
    uint32_t node_id  =  0 ;
    uint32_t cluster_id = 0;
    uint32_t mds_ifindex = 0;
/*  uint32_t tmp_ctr=6;   warning removal */
    char *tmp_ptr = NULL;
    FILE *f_pid;
    int  rc;


    /* get the environment variables set */

    /* There are no NULL Checks since these 
       are mandatory params that would have 
       to be configured */

    tmp_ptr = (char *) getenv("TET_SLOT_ID"); 
    slot_id = atoi(tmp_ptr);  

    tmp_ptr= (char *) getenv("TET_SHELF_ID");
    shelf_id = atoi(tmp_ptr);

    tmp_ptr = (char *) getenv("TET_CLUSTER_ID");
    cluster_id = atoi(tmp_ptr);

    /* node_id = ((shelf_id <<16) | slot_id); */
    node_id = m_NCS_GET_NODE_ID;

    tmp_ptr = (char *) getenv("TET_MDS_IF_INDEX");
    mds_ifindex = atoi(tmp_ptr); 

    if ( getenv("NCS_ENV_NODE_ID") )
    {
        node_id = atoi(getenv("NCS_ENV_NODE_ID"));
    }

    if ( getenv("SA_AMF_COMPONENT_NAME") )
    {

        printf("\n\nThe  comp name ISSSS %s\n",getenv("SA_AMF_COMPONENT_NAME"));

        if ((f_pid =  fopen(getenv("TET_PID_FILE"),"a")))
        {
            rc =  fprintf(f_pid,"%s: %d\n",getenv("SA_AMF_COMPONENT_NAME"),getpid());
            fclose(f_pid);
        }
        if ( rc <= 0)
        {
            printf("Could not write into the PID file !!!!! \n ");
        }

    }



    sprintf(t_argv[0],"NONE");
    sprintf(t_argv[1],"CLUSTER_ID=%d",cluster_id);
    sprintf(t_argv[2],"SHELF_ID=%d",shelf_id);
    sprintf(t_argv[3],"SLOT_ID=%d",slot_id);
    sprintf(t_argv[4],"NODE_ID=%d",node_id);   
    sprintf(t_argv[5],"PCON_ID=%d",4);
    sprintf(t_argv[6],"MDS_IFINDEX=%d",mds_ifindex);


    printf(" The SHELF_ID is :% d SLOT_ID is : %d  CLUSTER_ID is %d\n",shelf_id,slot_id,cluster_id);
    printf(" NODE_ID is %d  MDS_IFINDEX %d \n",node_id,mds_ifindex);


    return;

}

#if 0
void tware_ncs_startup()
{
    int tmp_ctr = NCS_MAIN_MAND_PARAMS;
    int arg_count=NCS_MAIN_MAND_PARAMS;
    FILE *fp;
    char *t_argv[NCS_MAIN_MAX_INPUT];


    for (tmp_ctr=0;tmp_ctr<NCS_MAIN_MAND_PARAMS;tmp_ctr++)
        t_argv[tmp_ctr] = (char*)malloc(64);  

#if (TET_DDD ==1 ) || (TET_DND == 1)
    sprintf(t_argv[0],"NONE");
    sprintf(t_argv[1],"%s",getenv("TET_CONF_FILE")); 
    ncspvt_load_config_n_startup(t_argc,t_argv); 
#else 
/*  not required to fill in the config info at this time - NODE-ID CHNG*/
/*    fill_std_config(t_argv); */

    /* fill the remaining service configs */

    /*  open the config file */


    fp = fopen(getenv("TET_CONF_FILE"),"r");
    if (fp == NULL)
    {
        printf("No service file name Specified \n");
    }

    /* fill the remaining stuff */
  /* NO CONFIG FILES PASSED IN QUITE A WHILE - COMMENTING */
/*    tet_mainget_svc_enable_info(t_argv,&arg_count,fp) ; */

/* Using the default mode for startup */
    ncs_agents_startup(); 

    tet_svcs_startup(); 

    for (tmp_ctr=0;tmp_ctr<arg_count;tmp_ctr++)
        free(t_argv[tmp_ctr]);

    return;

#endif 

}



void tet_svcs_startup()
{

    sleep(10);

#ifdef TET_AVSV
#ifdef TET_CLM
    tware_clm_succ_init(); 
#else
    tet_avsv_startup (); 
#endif
#endif



#ifdef TET_GLSV
    tet_glsv_startup();
#endif

#ifdef TET_DLSV
    tet_dlsv_startup();
#endif

#ifdef TET_SPSV
    tet_spsv_startup();
#endif

#ifdef TET_MDS_TDS 
    tet_mds_tds_startup ();
#endif

#ifdef TET_LEAP
    tet_leap_startup();
#endif

#ifdef TET_SPSV
    tet_spsv_startup();
#endif

#ifdef TET_MQSV
    tet_mqsv_startup();
#endif

#ifdef TET_CPSV
    tet_cpsv_startup();
#endif

#ifdef TET_MBCSV
    tet_mbcsv_startup();
#endif

#ifdef TET_MDS
    tet_mds_startup();
#endif

#ifdef TET_EDSV
    tet_edsv_startup();
#endif


#ifdef TET_MASV
     tet_masv_startup();
#endif


#ifdef TET_ALL
    tet_saf_agents_startup();
    tet_infoline(" Done with initializing agents, Starting test cases \n");

    /* tet_avsv_startup(); */

    sleep(5);
    /* Create all the saf-test-app threads */
    tet_create_test_threads();

    /* Go to sleep forever. The sub threads would run tests forever */
    while(1)
    {
        tet_infoline(" MAIN THREAD: going to sleep \n");
        sleep(5000);
    }

#endif

    return;

}
#endif

#ifdef TET_ALL

void tet_saf_agents_startup(void)
{
    uint32_t status=NCSCC_RC_SUCCESS;


#if 0 
    status=tet_create_gla();
    tet_create_mqagent();
    tet_create_cpa();
    tet_eda_create();
    /* Commented for the time being, as some of these create** 
       function are void and some are returning different
       return values
     */
    if ((status=tet_create_gla()) != NCSCC_RC_SUCCESS)
    {
        tet_infoline("GLA CREATE FAILED\n");
    }

    if ((status=tet_create_mqagent()) != NCSCC_RC_SUCCESS)
    {
        tet_infoline("MQA CREATE FAILED\n");
    }

    if ((status=tet_create_cpa()) != NCSCC_RC_SUCCESS)
    {
        tet_infoline("CPA CREATE FAILED\n");
    }

    tet_infoline("CPA CREATE SUCCESS\n");

    if ((status=tet_eda_create()) != NCSCC_RC_SUCCESS)
    {
        tet_infoline("EDA CREATE FAILED\n");
    }
#endif

}

void tet_create_test_threads(void)
{
    char *svc_name,*dist_flag;

    svc_name=getenv("TET_SVC_NAME");
    dist_flag=getenv("TET_SVC_DIST");

    if(!svc_name)
    {
        printf("Please fill TET_SVC_NAME in conf.sh\n");
        return; 
    }

    if(0 == (strncmp(svc_name,"glsv",4)))
    {
        if(!dist_flag)
        {
            printf("Please fill TET_SVC_DIST in conf.sh\n");
            return; 
        }
        if(0 == (strncmp(dist_flag,"TRUE",4)))
            tet_run_glsv_dist_cases();
        else
            tet_create_glsv_thread();
        tet_infoline(" CREATED GLSV TEST THREAD \n");
    }
    else if(0 == (strncmp(svc_name,"mqsv",4))) 
    {
        if(!dist_flag)
        {
            printf("Please fill TET_SVC_DIST in conf.sh\n");
            return; 
        }
        if(0 == (strncmp(dist_flag,"TRUE",4)))
            /*  tet_run_mqsv_dist_cases()*/; 
        else
            tet_create_mqsv_thread();
        tet_infoline(" CREATED  MQSV TEST THREAD \n");
    }
    else if(0 == (strncmp(svc_name,"cpsv",4)))
    {
        if(!dist_flag)
        {
            printf("Please fill TET_SVC_DIST in conf.sh\n");
            return; 
        }
        if(0 == (strncmp(dist_flag,"TRUE",4)))
            /* tet_run_cpsv_dist_cases()*/;
        else
            tet_create_cpsv_thread();
        tet_infoline(" CREATED  CPSV TEST THREAD \n");
    }
    else if(0 == (strncmp(svc_name,"edsv",4)))
    {
        if(!dist_flag)
        {
            printf("Please fill TET_SVC_DIST in conf.sh\n");
            return; 
        }
        if(0 == (strncmp(dist_flag,"TRUE",4)))
            tet_run_edsv_dist_cases();
        else
            tet_create_edsv_thread();
        tet_infoline(" CREATED  EDSV TEST THREAD \n");
    }
    else if(0 == (strncmp(svc_name,"masv",4)))
    {
        tet_create_masv_thread();
        tet_infoline(" CREATED  MASV TEST THREAD \n");
    }
    else /* by default, start all threads */
    {
        printf("STARTING ALL TEST THREADS \n");
        tet_create_cpsv_thread();
        tet_create_mqsv_thread();
        tet_create_glsv_thread();
        tet_create_edsv_thread();
        tet_create_masv_thread();
    }
}

void tet_create_glsv_thread(void)
{
    unsigned int rc=NCSCC_RC_SUCCESS;
    /* Create glsv app's thread */
    if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_CREATE((NCS_OS_CB)tet_run_glsv_app,
                    &tet_global_handle,
                    "GLSV_APP",
                    5,
                    NCS_STACKSIZE_MEDIUM,
                    &glsv_handle
                    )))
    {
        tet_infoline(" GLSV TEST THREAD CREATION FAILED\n");
    }

    /* Put the glsv thread in the start state */
    if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_START(glsv_handle)))
    {                            
        tet_infoline(" GLSV TEST THREAD START FAILED\n");
        /* kill the created task */
        m_NCS_TASK_RELEASE(glsv_handle);
    }
}/*end tet_create_glsv_thread */

void tet_create_mqsv_thread(void)
{
    unsigned int rc=NCSCC_RC_SUCCESS;
    /* Create mqsv app's thread */
    if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_CREATE((NCS_OS_CB)tet_run_mqsv_app,
                    &tet_global_handle,
                    "MQSV_APP",
                    5,
                    NCS_STACKSIZE_MEDIUM,
                    &mqsv_handle
                    )))
    {
        tet_infoline(" MQSV TEST THREAD CREATION FAILED\n");
    }

    /* Put the mqsv thread in the start state */
    if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_START(mqsv_handle)))
    {
        tet_infoline(" MQSV TEST THREAD START FAILED\n");
        /* kill the created task */
        m_NCS_TASK_RELEASE(mqsv_handle);
    }
}/*end tet_create_mqsv_thread */

void tet_create_edsv_thread(void)
{
    unsigned int rc=NCSCC_RC_SUCCESS;
    /* Create edsv app's thread */
    if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_CREATE((NCS_OS_CB)tet_run_edsv_app,
                    &tet_global_handle,
                    "EDSV_APP",
                    5,
                    NCS_STACKSIZE_MEDIUM,
                    &edsv_handle
                    )))
    {
        tet_infoline(" EDSV TEST THREAD CREATION FAILED\n");
    }

    /* Put the edsv thread in the start state */
    if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_START(edsv_handle)))
    {
        tet_infoline(" EDSV TEST THREAD START FAILED\n");
        /* kill the created task */
        m_NCS_TASK_RELEASE(edsv_handle);
    }
}/*end tet_create_edsv_thread */

void tet_create_cpsv_thread(void)
{
    unsigned int rc=NCSCC_RC_SUCCESS;
    /* Create cpsv app's thread */
    if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_CREATE((NCS_OS_CB)tet_run_cpsv_app,
                    &tet_global_handle,
                    "CPSV_APP",
                    5,
                    NCS_STACKSIZE_MEDIUM,
                    &cpsv_handle
                    )))
    {
        tet_infoline(" CPSV TEST THREAD CREATION FAILED\n");
    }

    /* Put the cpsv thread in the start state */
    if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_START(cpsv_handle)))
    {
        tet_infoline(" CPSV TEST THREAD START FAILED\n");
        /* kill the created task */
        m_NCS_TASK_RELEASE(cpsv_handle);
    }
}/*end tet_create_cpsv_thread */

void tet_create_masv_thread(void)
{
    unsigned int rc=NCSCC_RC_SUCCESS;
    /* Create MASV's thread */
    if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_CREATE((NCS_OS_CB)tet_run_masv_app,
                    &tet_global_handle,
                    "MASV_APP",
                    5,
                    NCS_STACKSIZE_MEDIUM,
                    &masv_handle
                    )))
    {
        tet_infoline("MASV TEST THREAD CREATION FAILED\n");
    }

    /* Put the MASv thread in the start state */
    if (NCSCC_RC_SUCCESS != (rc = m_NCS_TASK_START(masv_handle)))
    {
        tet_infoline(" MASV TEST THREAD START FAILED\n");
        /* kill the created task */
        m_NCS_TASK_RELEASE(masv_handle);
    }
}/*end tet_create_masv_thread */


void tet_run_masv_app(void)
{
    tet_infoline(" currently nothing to do, but i'll be back \n");
    while(1)
    {
        sleep(500);
    }
}/*end tet_run_masv_app */

#endif /* TET_ALL */

void tware_ncs_end()
{

    tet_infoline(" Ending the agony.. ");
    return;
}

int tet_remotesync(SyncParameters *syncparam)
{
   int rc = -1;
   int num_sys;
   int *sysnames;
   int *sys;
   int i;

   num_sys = tet_remgetlist(&sysnames);
   sys = (int *)malloc((num_sys+1)*sizeof(int));
   for(i=0;i<num_sys;i++)
      sys[i] = sysnames[i];
   sys[num_sys] = tet_remgetsys();
                                                                                                                             
        (void) tet_printf("In tet_remotesync() function ");
                                                                                                                             
        if (syncparam->systemids == NULL)
        {
                syncparam->systemids = sys;
                syncparam->numofsystemids = num_sys;
        }
        if ((syncparam->waittime < 0) || (syncparam->waittime > TIMEFIVEMINS))
                syncparam->waittime = TIMEOUT;
                                                                                                                             
        if (tet_remsync(syncparam->syncpointnum,
                        syncparam->systemids,
                        syncparam->numofsystemids,
                        syncparam->waittime,
                        syncparam->vote,
                        syncparam->message) != 0)
        {
                printf("tet_remsync failed in tet_remotesync()\n");
        }
        else
                rc = 0;
                                                                                                                             
        free(sys);
        return rc;
}

int fill_syncparameters(int vote)
{
   SyncParameters sy_par;
                                                                                                                             
   sy_par.syncpointnum = gl_sync_pointnum;
   sy_par.systemids = NULL;
   sy_par.waittime = TIMEFIVEMINS;
   sy_par.vote = vote;
   sy_par.message = NULL;
                                                                                                                             
   printf(" Waiting for sync %d\n", gl_sync_pointnum);
   gl_sync_pointnum++;
   if(tet_remotesync(&sy_par) == -1)
      return TET_FAIL;
   return TET_PASS;
}

#if (TET_PATCH==1)
void tet_chill(int sub_test_arg) 
{

    tet_result(TET_PASS);

    while(1)
    {
#ifdef TET_AVSV 
#ifdef ND
        sleep(100);
        exit(0);
#else
        sleep(1000000);
#endif
#endif

    }

    remove(getenv("TET_PID_FILE"));   
}

struct tet_testlist tet_testlist[]={

    {tet_chill,300,8},
    {NULL,-1}
};

#endif

