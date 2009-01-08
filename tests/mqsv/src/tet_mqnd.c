#if (NCS_MQND == 1)

#include "tet_mqsv.h"

uns32 mqnd_lib_req(NCS_LIB_REQ_INFO *);

void tet_create_mqnd() 
{
   NCS_LIB_REQ_INFO lib_create;
   NCS_LIB_REQ_INFO lib_destroy;
   char c;

   lib_create.i_op = NCS_LIB_REQ_CREATE;

   if ( mqnd_lib_req(&lib_create) != NCSCC_RC_SUCCESS)
   {
      m_TET_MQSV_PRINTF("MQND lib req failed ... ");
      sleep(1);
   }

   m_TET_MQSV_PRINTF(" MQND lib req Succeeds...");

   m_TET_MQSV_PRINTF(" Enter a character to see mqnd mem dump information \n");
   while(1) {
      c=getchar();
      switch(c) {
         case 'e':
            lib_destroy.i_op = NCS_LIB_REQ_DESTROY;
            mqnd_lib_req(&lib_destroy);
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
