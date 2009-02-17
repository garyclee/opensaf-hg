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
 
#define NCS_SYNC_CALL_TIMEOUT  5000
#define  MAX_MIB_BUFF_SIZE  512

int gl_prv_act_node = -1;
void asc_to_int_arr(char *ent_path,uns32 arr[]);

void asc_to_int_arr(char *ent_path,uns32 arr[])
{
   int i= 1;
   int idx_len=0;

   idx_len = strlen(ent_path);
   uns32 dex[idx_len];
   do {
      dex[i] = (unsigned int) ent_path[i-1];
      arr[i] = (unsigned int) ent_path[i-1];
      i++;
   }while(i != (strlen(ent_path)+1));

   dex[0]=idx_len;
   arr[0] = dex[0];
}

uns32 maa_switch(int state)
{
   NCSMIB_ARG  arg;
   NCSMEM_AID  ma;
   int param_value_len = 0; 
   uns8 buffer[MAX_MIB_BUFF_SIZE];

   if(param_value_len >= MAX_MIB_BUFF_SIZE)
   {
      printf("\nString value too long");
      return NCSCC_RC_FAILURE;
   }

   /*
      Put this NCSMIB_ARG structure in start state. Do not have any valid data
      or pointers hanging off the passed structure, as this function will
      overwrite such stuff with null.
   */
   ncsmib_init(&arg);

   /* put the NCSMEM_AID in start state */
   ncsmem_aid_init(&ma, buffer, MAX_MIB_BUFF_SIZE);

   /* Fill in the NCSMIB_ARG structure */
   arg.i_op = NCSMIB_OP_REQ_SET;
   arg.i_tbl_id = NCSMIB_TBL_AVM_SCALAR;
   arg.i_usr_key = gl_mac_handle; /* application can fill their handle here */
   arg.i_mib_key = gl_mac_handle; /* Handle of MAA, used to communicate with MAA */
   arg.i_xch_id = 0;
   arg.i_rsp_fnc = NULL;

   /* Initializing since this got corrupted during MQSV SWIT Testing with junk vals */
   arg.i_idx.i_inst_ids = 0x0;
   arg.i_idx.i_inst_len = 0;  /* Length of the index */

   arg.req.info.set_req.i_param_val.i_param_id = 1  ;
   arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   arg.req.info.set_req.i_param_val.i_length =  4; /*+1 for '/0' char.*/
   arg.req.info.set_req.i_param_val.info.i_int = TRUE;

   /* model a MIB request as synchronous; set timer */
   uns32 retval = ncsmib_sync_request(&arg, ncsmac_mib_request,
                                      NCS_SYNC_CALL_TIMEOUT, &ma);
   if(NCSCC_RC_SUCCESS != retval) /* if ncsmib_sync_request failed , !.e the request not reached application */
   {
      printf("\nncsmib_sync_request failed %d, %d", retval, arg.rsp.i_status);
      return NCSCC_RC_FAILURE;
   }
   else /* if ncsmib_sync_request succeeded and application served the request*/
   {
      if(arg.rsp.i_status != NCSCC_RC_SUCCESS) /* set request is not successfull */
      {
         printf("\nBad i_status %d in ARG", arg.rsp.i_status);
         return NCSCC_RC_FAILURE;
      }
   }

   return NCSCC_RC_SUCCESS;
}

uns32 maa_fail_lock(int node ,int lock_val)
{
   NCSMIB_ARG  arg;
   NCSMEM_AID  ma;
   char enter_path[255];
   int param_value_len = 0; 
   uns8 buffer[MAX_MIB_BUFF_SIZE];
   int count=0;
   uns32 retval;

   if((node != 1) && (node != 14))
   {
      printf("\n FAILOVERS CAN BE DONE ONLY FOR NODES (1 & 14 )\n");
      return NCSCC_RC_FAILURE;
   }

   memset(enter_path,'\0',255);
   sprintf(enter_path,"{{7,%d},{23,2},{65535,0}}",node);
   printf("enter_path = %s",enter_path);

   uns32 index[strlen(enter_path)+1];

   if (param_value_len >= MAX_MIB_BUFF_SIZE)
   {
      printf("\nString value too long");
      return NCSCC_RC_FAILURE;
   }

   /*
      Put this NCSMIB_ARG structure in start state. Do not have any valid data
      or pointers hanging off the passed structure, as this function will
      overwrite such stuff with null.
   */

retry:
   ncsmib_init(&arg);

   /* put the NCSMEM_AID in start state */
   ncsmem_aid_init(&ma, buffer, MAX_MIB_BUFF_SIZE);

   /* Fill in the NCSMIB_ARG structure */
   arg.i_op = NCSMIB_OP_REQ_SET;
   arg.i_tbl_id = NCSMIB_TBL_AVM_ENT_DEPLOYMENT;
   arg.i_usr_key = gl_mac_handle; /* application can fill their handle here */
   arg.i_mib_key = gl_mac_handle; /* Handle of MAA, used to communicate with MAA */
   arg.i_xch_id = 0;
   arg.i_rsp_fnc = NULL;

   /* Fill in the index depending on the INDEX type of the table as defined in the MIB */
   /* In this case the mibi table is indexed by the ipaddress.So index filled as shown below */
   asc_to_int_arr(enter_path,index);

   arg.i_idx.i_inst_ids = index;
   arg.i_idx.i_inst_len = index[0] +1 ;  /* Length of the index */

   arg.req.info.set_req.i_param_val.i_param_id = 11  ;
   arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   arg.req.info.set_req.i_param_val.i_length =  4; /*+1 for '/0' char.*/
   arg.req.info.set_req.i_param_val.info.i_int = lock_val;

   /* model a MIB request as synchronous; set timer */
   retval = ncsmib_sync_request(&arg, ncsmac_mib_request,
                                NCS_SYNC_CALL_TIMEOUT, &ma);
   if (NCSCC_RC_SUCCESS != retval) /* if ncsmib_sync_request failed , !.e the request not reached application */
   {
      printf("\nncsmib_sync_request failed %d, %d", retval, arg.rsp.i_status);
      if(count++ < 10)
      {
         sleep(10);
         goto retry;
      }
      return NCSCC_RC_FAILURE;
   }
   else /* if ncsmib_sync_request succeeded and application served the request*/
   {
      if (arg.rsp.i_status != NCSCC_RC_SUCCESS) /* set request is not successfull */
      {
         printf("\nBad i_status %d in ARG", arg.rsp.i_status);
         if(count++ < 10)
         {
            sleep(10);
            goto retry;
         }
         return NCSCC_RC_FAILURE;
      }
   }
   return NCSCC_RC_SUCCESS;
}

uns32 maa_get_tblentry(NCSMIB_TBL_ID tbl_id, char *index1, char *index2, int *ret_val)
{
   NCSMIB_ARG arg;
   NCSMEM_AID ma;
   int param_value_len = 0; 
   uns8 buffer[MAX_MIB_BUFF_SIZE];
   uns32 index1_len = strlen(index1);
   uns32 index2_len = strlen(index2);

   uns32 index[100];

   if(param_value_len >= MAX_MIB_BUFF_SIZE)
   {
      printf("\nString value too long");
      return NCSCC_RC_FAILURE;
   }

   ncsmib_init(&arg);

   /* put the NCSMEM_AID in start state */
   ncsmem_aid_init(&ma, buffer, MAX_MIB_BUFF_SIZE);

   memset(index,'\0',100*sizeof(uns32));

   /* Fill in the NCSMIB_ARG structure */
   arg.i_op = NCSMIB_OP_REQ_GET;
   arg.i_tbl_id = tbl_id;
   arg.i_usr_key = gl_mac_handle; /* application can fill their handle here */
   arg.i_mib_key = gl_mac_handle; /* Handle of MAA, used to communicate with MAA */
   arg.i_xch_id = 0;
   arg.i_rsp_fnc = NULL;

   /* Fill in the index depending on the INDEX type of the table as defined in the MIB */
   /* In this case the mibi table is indexed by the ipaddress.So index filled as shown below */
   asc_to_int_arr(index1,index);
   asc_to_int_arr(index2,&index[index1_len+1]);

   arg.i_idx.i_inst_ids = index;
   arg.i_idx.i_inst_len = index1_len + index2_len + 2;  /* Length of the index */

   arg.req.info.set_req.i_param_val.i_param_id = 3;
   arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;

   /* model a MIB request as synchronous; set timer */
   uns32 retval = ncsmib_sync_request(&arg, ncsmac_mib_request,
                                      NCS_SYNC_CALL_TIMEOUT, &ma);
   if(NCSCC_RC_SUCCESS != retval) /* if ncsmib_sync_request failed , !.e the request not reached application */
   {
       printf("\nncsmib_sync_request failed %d, %d\n", retval, arg.rsp.i_status);
       return NCSCC_RC_FAILURE;
   }
   else /* if ncsmib_sync_request succeeded and application served the request*/
   {
      if(arg.rsp.i_status != NCSCC_RC_SUCCESS) /* get request is not successfull */
      {
          printf("\nBad i_status %d in ARG\n", arg.rsp.i_status);
          return NCSCC_RC_FAILURE;
      }
   }

   *ret_val = arg.rsp.info.get_rsp.i_param_val.info.i_int;
   return(NCSCC_RC_SUCCESS);
}

void maa_unlck_nod(int prev_act_node)
{
   printf(" \n The Previous Active Node to Be Unlocked %d \n",prev_act_node);

   if((prev_act_node != 1) && (prev_act_node != 14))
   {
      printf(" WHAT THE &@&$ - Give The value returned earlier  \n\n");
      return;
   }

   maa_fail_lock(prev_act_node,3); 
}

int maa_get_status()
{
   char su1_index[]="safSu=SuT_NCS_SCXB,safNode=SC_2_1";
   char si1_index[]="safSi=Si_SCXB";
   char su2_index[]="safSu=SuT_NCS_SCXB,safNode=SC_2_14";
   char si2_index[]="safSi=Si_SCXB";
   int su1_ha_state;
   int su2_ha_state;
   uns32 error;

   error = maa_get_tblentry(NCSMIB_TBL_AVSV_AMF_SU_SI, su1_index, si1_index, &su1_ha_state);
   if(error == NCSCC_RC_SUCCESS)
      printf(" HA state of %s:%s is %d\n", su1_index, si1_index, su1_ha_state);
   else
   {
      printf(" Not able to get the HA state of %s:%s\n", su1_index, si1_index);
      exit(1);
   }

   error = maa_get_tblentry(NCSMIB_TBL_AVSV_AMF_SU_SI, su2_index, si2_index, &su2_ha_state);
   if(error == NCSCC_RC_SUCCESS)
      printf(" HA state of %s:%s is %d\n", su2_index, si2_index, su2_ha_state);
   else
   {
      printf(" Not able to get the HA state of %s:%s\n", su2_index, si2_index);
      exit(1);
   }

   if((su1_ha_state == 1 && su2_ha_state == 2) || (su1_ha_state == 2 && su2_ha_state == 1))
      return NCSCC_RC_SUCCESS;

   exit(1);
}

int maa_get_active_node()
{
   char comp_index[]="safComp=CompT_MAS,safSu=SuT_NCS_SCXB,safNode=SC_2_1";
   char csi_index[]="safCsi=Csi_MAS,safSi=Si_SCXB";
   int comp_ha_state;
   uns32 error;
   int active_node;

   error = maa_get_tblentry(NCSMIB_TBL_AVSV_AMF_COMP_CSI, comp_index, csi_index, &comp_ha_state);
   if(error == NCSCC_RC_SUCCESS)
      printf(" HA state of %s:%s is %d\n", comp_index, csi_index, comp_ha_state);
   else
   {
      printf(" Not able to get the HA state of %s:%s\n", comp_index, csi_index);
      return NCSCC_RC_FAILURE;
   }

   if(comp_ha_state == 2)
      active_node = 14;
   else
      active_node = 1;

   return active_node;  
}

void maa_fail(int state)
{
   int act_node = -1;

   act_node = maa_get_active_node();
   printf("THE ACTIVE SCXB IS NODE %d \n",act_node);
   if((act_node != 1) && (act_node != 14))
   {
      printf(" WHAT THE &@&$ - ACTIVE NODE GET FAILED \n\n");
      return;
   }

   maa_fail_lock(act_node,2);
   printf("\n Gone in  130 Seconds \n");
   sleep(130);
   printf("\n Coming back after  130 Seconds - unlocking @##@$#$#@ \n");
   maa_fail_lock(act_node,3);
}

int maa_lck_act_nod(int state)
{
   int act_node = -1;

   maa_get_status();

   act_node = maa_get_active_node();
   gl_prv_act_node = act_node;
   printf("THE ACTIVE SCXB IS NODE %d \n",act_node);
   if((act_node != 1) && (act_node != 14))
   {
      printf(" WHAT THE &@&$ - ACTIVE NODE GET FAILED \n\n");
      return 0;
   }

   maa_fail_lock(act_node,2);
   return act_node;
}

