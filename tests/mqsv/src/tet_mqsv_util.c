void tet_create_mqd(void);
void tet_create_mqnd(void);
void tet_run_mqsv_app(void);
void tet_mqsv_startup(void);

void tet_mqsv_startup(void) {
#if (TET_D == 1)
   tet_create_mqd();
#endif

#if (TET_ND == 1)
   tet_create_mqnd();
#endif

#if (TET_A == 1)
   tet_run_mqsv_app();
#endif
}
