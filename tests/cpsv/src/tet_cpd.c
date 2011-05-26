#if ( (NCS_CPD == 1) && (TET_D == 1) )

#include "cpsv.h"
#include "cpd.h"
#include "tet_api.h"


void tet_create_cpd();
void tet_cpd_dump_cb();

extern uint32_t gl_cpd_hdl;


void tet_run_cpd() {

     tet_create_cpd();

}

void tet_destroy_cpd() {

   NCS_LIB_REQ_INFO lib_create;
   lib_create.i_op = NCS_LIB_REQ_DESTROY;
   char c;

   if ( cpd_lib_req(&lib_create) != NCSCC_RC_SUCCESS)
   {
      printf("CPD lib req failed ... \n");
      sleep(1);
   } else {
      printf("CPD lib req Success ....\n");
   }
}

void tet_create_cpd() 
{
   
   NCS_LIB_REQ_INFO lib_create;
   lib_create.i_op = NCS_LIB_REQ_CREATE;
   char c;

   if ( cpd_lib_req(&lib_create) != NCSCC_RC_SUCCESS)
   {
      printf("CPD lib req failed ... \n");
      sleep(1);
   } else {
      printf("CPD lib req Success ....\n");
   }

   printf(" Enter getcahr to see cpd_cb information \n");
   while(1) {
      c=getchar();
      switch(c) {

         case 'e':
            tet_destroy_cpd();
            exit(1);
            break;
     case 'c':
        tet_cpd_dump_cb();
         break;
         case 'm':
            ncs_mem_whatsout_dump();
            break;
         case 'i':
            ncs_mem_ignore(TRUE);
            break;

         default:
            printf("Codes supported are \n\t c- dump cb,\n\tm - mem,\n\ti - ignore\n\te - exit...\n");
            break;
      }
     if (c == 'e') break;
  }

}


void tet_cpd_dump_cb()
{
   CPD_CB           *cb;
   uint32_t              cb_hdl=m_CPD_GET_CB_HDL;
   CPSV_EVT          *cpsv_evt;

   cpsv_evt=m_MMGR_ALLOC_CPSV_EVT(NCS_SERVICE_ID_CPD);

   memset(cpsv_evt,0,sizeof(CPSV_EVT));
   cpsv_evt->type = CPSV_EVT_TYPE_CPD;
   cpsv_evt->info.cpnd.type=CPD_EVT_CB_DUMP;

   cb =  ncshm_take_hdl(NCS_SERVICE_ID_CPD, cb_hdl);
   m_NCS_IPC_SEND(&cb->cpd_mbx, cpsv_evt, MDS_SEND_PRIORITY_MEDIUM);
   ncshm_give_hdl(cb_hdl);
}



#endif
