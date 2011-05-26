#if ( (NCS_CPND == 1) && (TET_ND == 1) )
#include "cpsv.h"
#include "cpnd.h"
#include "tet_api.h"

extern uint32_t gl_cpnd_hdl;
void tet_create_cpnd();
void tet_cpnd_show();


void tet_run_cpnd() {

   tet_create_cpnd();
   tet_cpnd_show();

}
   
void tet_cpnd_show() {
   char c;
   while(1) {
      c=getchar();
    switch(c) {
         case 'n':
            printf("Show node information \n");
            break;
         case 'c':
            print_cpnd_cb();
            break;
         case 'e':
            tet_destroy_cpnd();
            break;
         case 'm':
            ncs_mem_whatsout_dump();
            break;
         case 'i':
            ncs_mem_ignore(TRUE);
            break;
         default:
            printf("Codes supported are \n\tn - node info,\n\tc - cpnd cb,\n\tm - mem dump \n\ti - ignore \n\te - exit...\n");
            break;
      }

     if (c == 'e')exit(1);
  }


}

void tet_destroy_cpnd() 
{

   NCS_LIB_REQ_INFO lib_create;
   lib_create.i_op = NCS_LIB_REQ_DESTROY;

   if ( cpnd_lib_req(&lib_create) != NCSCC_RC_SUCCESS)
   {
      printf("CPND lib req failed ... ");
      return;
   }
   printf("CPND destroyed ...");

}

void tet_create_cpnd() 
{
   
   NCS_LIB_REQ_INFO lib_create;
   lib_create.i_op = NCS_LIB_REQ_CREATE;

   if ( cpnd_lib_req(&lib_create) != NCSCC_RC_SUCCESS)
   {
      printf("CPND lib req failed ... ");
      sleep(1);
   }
   printf("CPND lib req Success ....\n");

}


void print_cpnd_cb() {

   CPND_CB           *cb;
   uint32_t              cb_hdl=m_CPND_GET_CB_HDL;
   CPSV_EVT          *cpsv_evt;

   cpsv_evt=m_MMGR_ALLOC_CPSV_EVT(NCS_SERVICE_ID_CPND);

   memset(cpsv_evt,0,sizeof(CPSV_EVT));
   cpsv_evt->type = CPSV_EVT_TYPE_CPND;
   cpsv_evt->info.cpnd.type=CPND_EVT_CB_DUMP;

   cb =  m_CPND_TAKE_CPND_CB;
   m_NCS_IPC_SEND(&cb->cpnd_mbx, cpsv_evt, MDS_SEND_PRIORITY_MEDIUM);
   ncshm_give_hdl(cb_hdl);
}



#endif


