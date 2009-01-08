#include "tet_startup.h"

void tet_run_gld();
void tet_run_glnd();
void tet_run_glsv_app();
void tet_glsv_startup(void);

void tet_glsv_startup(void) 
{

#if (TET_D == 1)
   tet_run_gld();
#endif

#if (TET_ND == 1)
   tet_run_glnd();
#endif

#if (TET_A == 1)
   tet_run_glsv_app();
#endif

}
