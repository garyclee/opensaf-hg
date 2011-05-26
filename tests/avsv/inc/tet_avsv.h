

/* This contains the global definitions that would be used across the
    TET AVSV Test framework */

#ifndef __TET_AVSV_GBL__
#define __TET_AVSV_GBL__

extern SYSF_MBX        gl_mds_dummy_mbx;
extern void *gl_mds_dummy_task_hdl;
extern SaAmfHandleT    gl_TetSafHandle;


extern int systemid_list[];        /* system IDs to sync with */


#endif


struct svc_testlist {
        void (*testfunc)(int);
        int arg;
};


#define tet_print printf
#define TIMEOUT 10

#define m_TET_GET_AMF_VER(amf_ver) amf_ver.releaseCode='B'; amf_ver.majorVersion=0x01; amf_ver.minorVersion=0x01;


typedef struct tware_evt
{
   struct tware_evt        *next;
   uint32_t                  priority;
   uint32_t                  type;  
}TWARE_EVT;



typedef enum {
        TWARE_HC_EVT=1,
        TWARE_HA_EVT=2,
        TWARE_RDNS_EVT=3,
        TWARE_ERR_EVT=4,
        TWARE_CLN_EVT=5
} tet_ncs_evts;

/* these structures would have to be moved to more logical place
    once decided */
                                                                                                                                             
/* MIB Structures for pcode <--> tware usage */
                                                                                                                                             
struct prm_val
{
  union
  {
   int param_val;
   char *cparam_val;
  };
};
                                                                                                                                             
 struct tware_snmp_set
{
  int  table_id;
  int param_id;
  char *instance_id;
  struct prm_val pv;
  int   param_val_len;
};
                                                                                                                                             
struct tware_snmp_get
{
  uint32_t table_id;
  uint32_t param_id;
  char *instance_id;
};

struct snmp_array_map {
   char *array_name;
   void *arr_ptr;
};

void tet_avsv_startup(void);
void tet_saf_proxied_inst_callback(SaInvocationT invocation, const SaNameT *proxied_name );
void tet_avsv_thread_init(void);
void tet_saf_errorrpt_spoof(SaAmfRecommendedRecoveryT Reco);
void tet_saf_stopping_complete(SaInvocationT Invocation,SaAisErrorT error);
void tet_comp_capability_get(int sub_test_arg);
void tet_saf_pend_oper_get(void);
void tware_pg_track(int sub_test_arg);
int tet_exec_tests(struct svc_testlist *tlp );
