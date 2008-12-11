#include "stdio.h"
#include "saEvt.h"
#include "ncssysf_def.h"
#include "ncssysf_tsk.h"
#include "hpl_api.h"

/* EVT Handle */
SaEvtHandleT          gl_evt_hdl = 0;

/*
 *  This is a very simple program to show how to call
 *  the HISv client library to shut down a blade.
 *
 *  In this sample code, blade 4 is shutdown, in chassis 2.
 *
 */

int main(int argc, char*argv[])
{
   char 		hpi_entity_path[300];
   NCS_LIB_CREATE 	hisv_create_info;
   int 			rc = 0, hisv_power_res = 0;
   SaEvtCallbacksT	reg_callback_set;
   SaVersionT		ver;

   printf("power_set_main() called...\n");

   /* Initialize the event subsystem for communication with HISv */
   ver.releaseCode = 'B';
   ver.majorVersion = 0x01;
   ver.minorVersion  = 0x01;
   rc = saEvtInitialize(&gl_evt_hdl, &reg_callback_set, &ver);

   /* Initialize the HPL client-side library */
   rc = hpl_initialize(&hisv_create_info);
   /* Allow time for the client library callbacks to initialize */
   sleep(3);

   /* Create an entity path for what blade we want to shutdown */
   sprintf(hpi_entity_path, "{{SYSTEM_BLADE,4},{SYSTEM_CHASSIS,2}}");

   printf("\n***** Shutting down resource: %s\n", hpi_entity_path);

   /* Call the HPL client library to shutdown a blade */
   hisv_power_res = hpl_resource_power_set(2, hpi_entity_path, HISV_RES_POWER_OFF);

   /* Print out the results */
   printf("\n***** hisv_power_res = %d\n", hisv_power_res);
   if (hisv_power_res == 1)
      printf("\n*****          BLADE SUCCESSFULLY SHUTDOWN          *****\n");
   else
      printf("\n*****          BLADE COULD NOT BE SHUTDOWN          *****\n");

   return 0;
}
