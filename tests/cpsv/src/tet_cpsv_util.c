
void tet_run_cpsv_app();
void tet_cpsv_startup(void);
void tet_cpsv_startup(void) 
{

#if (TET_D == 1)
   tet_run_cpd();
#endif

#if (TET_ND == 1)
   tet_run_cpnd();
#endif

#if (TET_A == 1)
   tet_run_cpsv_app();
#endif

}
