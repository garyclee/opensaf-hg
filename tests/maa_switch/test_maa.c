#include <stdio.h>
#include <string.h>
#include "ncsgl_defs.h"
#include "os_defs.h"
#include "ncs_main_papi.h"
#include "ncs_osprm.h"
#include "ncssysf_def.h"
#include "ncs_mtbl.h"
#include "ncs_stack_pub.h"
#include "ncs_mib_pub.h"
#include "mac_papi.h"
/*#include "snmptm_tblid.h" 
#include "saAis.h" */
#if 0
/*argc no.of SwitchOvers can be invoked*/
int main(int argc, char *argv[])
{
   int count=1;
   ncs_agents_startup(NULL,0);
   for(count=1;count<=argv[1];count++)
   {
      if(maa_switch(1)==NCSCC_RC_SUCCESS)
      {
         system("date");
         printf("\nSwitchOver Count = %d\n",count);
         fflush(stdout);
         sleep(180);
      }
      else
      {
         perror("\n FAILURE OF MAA SWITCH\n");
         exit(0);
      }
   }
   return 1;
}
#endif
#if 0
/*argc no.of FailOvers can be invoked*/
int main(int argc, char *argv[])
{
   int count=1,present_active=0,previous_active=0;
   ncs_agents_startup(NULL,0);
   for(count=1;count<=argv[1];count++)
   {
      printf("\n----------------FailOver Count = %d-------------------------\n",count);
      system("date");
      present_active=maa_lck_act_nod(1);
      printf("\n The value of present_active = %d\n",present_active);
      sleep(120);
      printf("\nPassing present_active = %d to Unlock\n",present_active);
      maa_unlck_nod(present_active);
      printf("Wait for 9 Minutes\n");
      fflush(stdout);
      sleep(480);
      previous_active=present_active;
      printf("\n-------------------------------------------------------------\n");
   }
   return 1;
}
void main(int argc, char *argv[])
{
int prev_act=-1;
int state = -1;

  if(argc <= 2)
  {
     printf(" Usage : <lock/unlock> <slot_no>\n");
     return;
  }

 ncs_agents_startup(NULL,0);

 printf("\ngl_mac_handle=%d\n", gl_mac_handle);

  if(!strcmp(argv[1],"lock"))
  {
      printf(" Locking \n\n " );
      prev_act = maa_lck_act_nod(state);
      printf(" Previous Active = %d\n", prev_act);
      return;
  }

  if(!strcmp(argv[1],"unlock"))
  {
      printf(" Unlocking \n\n ");
      prev_act = maa_unlck_nod(atoi(argv[2]));
      printf(" Unlock Return value = %d\n", prev_act);
  }
}
#endif

int main()
{
   uns32 err;
   int act_nod;

   ncs_agents_startup(NULL,0);

   act_nod = maa_get_active_node();
   printf(" Active node is %d", act_nod);

   err = maa_get_status();
}
