#if ( NCS_MQD == 1) 

#include "tet_mqsv.h"

uns32 mqd_lib_req(NCS_LIB_REQ_INFO *);

void tet_create_mqd() 
{
   NCS_LIB_REQ_INFO lib_create;
   NCS_LIB_REQ_INFO lib_destroy;
   char c;

   lib_create.i_op = NCS_LIB_REQ_CREATE;
   if ( mqd_lib_req(&lib_create) != NCSCC_RC_SUCCESS)
   {
      m_TET_MQSV_PRINTF("MQD lib req failed ... ");
      return;
   }

   m_TET_MQSV_PRINTF("MQD lib req success!! ... ");
  
   m_TET_MQSV_PRINTF(" Enter a character to see mqd mem dump information \n");
   while(1) {
      c=getchar();
      switch(c) {
         case 'e':
            lib_destroy.i_op = NCS_LIB_REQ_DESTROY;
            mqd_lib_req(&lib_destroy);
            exit(1);
            break;
         case 'm':
            tware_mem_dump();
            break;
         case 'i':
            tware_mem_ign();
            break;
         default:
            m_TET_MQSV_PRINTF("Codes supported are \n\tm - mem,\n\ti - ignore\n\te - exit...\n");
            break;
      }
      if (c=='e') break;
  }
}

#endif
