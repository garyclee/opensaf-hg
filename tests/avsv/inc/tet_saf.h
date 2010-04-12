
void
tet_saf_health_chk_callback (SaInvocationT invocation,
        const SaNameT *compName,
        SaAmfHealthcheckKeyT *checkType);
#if 0
void
tet_saf_readiness_state_callback (SaInvocationT invocation,
                                   const SaNameT *compName,
                                   SaAmfReadinessStateT readinessState);
#endif

                          
void tet_saf_CSI_set_callback (SaInvocationT invocation,
                          const SaNameT  *compName,
                          SaAmfHAStateT  haState,
                          SaAmfCSIDescriptorT csidesc);
 

 void 
  tet_saf_CSI_remove_callback (SaInvocationT invocation,
                           const SaNameT *compName,
                           const SaNameT *csiName,
                           SaAmfCSIFlagsT csiFlags);

 void 
 tet_saf_Comp_Term_callback(SaInvocationT invocation,
                                    const SaNameT *compName);




/* Function Prototypes in tet_saf.c */

void tet_saf_initialize (int sub_test_arg);

void tet_saf_comp_register(int sub_test_arg);

void tet_saf_comp_unreg(int sub_test_arg);

void tet_saf_comp_name_get(int sub_test_arg);

void tet_saf_pm_start(int sub_test_arg);
void tet_saf_pm_stop(int sub_test_arg);

void tet_saf_selobject(void);

void tet_saf_finalize(int sub_test_arg);

void tet_saf_pm_start(int sub_test_arg);
void tet_saf_pm_stop(int sub_test_arg);

void tet_saf_succ_init(int depth, int fill_cbks);

void tet_saf_cleanup(int depth);

void tet_saf_health_chk_start(int sub_test_arg);

void tet_saf_health_chk_stop (int sub_test_arg);

void tet_saf_health_chk_confirm (int sub_test_arg);

void tet_saf_Readiness_state_get(int sub_test_arg);

void tet_saf_errorrpt(SaAmfRecommendedRecoveryT);

void tet_saf_ha_state_get(int sub_test_arg);
void tet_saf_err_cancel(void);
    
void tet_saf_HC_tmr_cbk (void);

void tet_avsv_startup();

void tet_avsv_end();


void tet_saf_HC_tmr_cbk_1 ();

int  tware_comp_init_count(SaNameT *SaCompName , int invoked);
