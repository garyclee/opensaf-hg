/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Emerson Network Power
 */

#include <stdio.h>
#include <string>
#ifdef __cplusplus
extern "C" {
#endif
#include "ncsgl_defs.h"
#include "ncs_main_papi.h"
#include "ncssysf_def.h"
#include "ncs_mtbl.h"
#include "ncs_stack_pub.h"
#include "ncs_mib_pub.h"
#include "mac_papi.h"
#include "snmptm_tblid.h"
#include "ncssysf_mem.h"
#include "ncsmib_mem.h"
#ifdef __cplusplus
}
#endif

#include "saAis.h"

const SaTimeT NCS_SYNC_CALL_TIMEOUT = 5000; /*5 seconds.*/

const int MAX_MIB_BUFF_SIZE = 1024;

void ncsmib_pp_new( NCSMIB_ARG* arg);
char* mibpp_opstr_new(uns32 op);
void ncsmib_req_pp_new(NCSMIB_ARG* arg);
void mibpp_row_new(USRBUF* inub);
void mibpp_param_val_new(NCSMIB_PARAM_VAL* val);
void mibpp_inst_new(const uns32* inst, uns32 len);
char*  mibpp_status_new(uns32 status);
const char* const pp_fmat_str_new[4] = { NULL, "Integer", "OctetStr", "Bool"};


uns32 getnextrow_mib_parameter(NCSMIB_TBL_ID, NCSMIB_IDX);
uns32 getrow_mib_parameter(NCSMIB_TBL_ID, NCSMIB_IDX);

/*APIs for getting string and int value params*/
uns32 get_mib_parameter(std::string &param_value, NCSMIB_TBL_ID table_id, 
                       int parameter_id, int physical_board_id)
{
 NCSMIB_ARG  arg;
 NCSMEM_AID  ma;
 uns8 buffer[MAX_MIB_BUFF_SIZE];
 uns32 index[4]; /* Holds the index of the table_id */

        /*
         Put this NCSMIB_ARG structure in start state. Do not have any valid data
         or pointers hanging off the passed structure, as this function will 
         overwrite such stuff with null.
        */
 ncsmib_init(&arg);

        /* put the NCSMEM_AID in start state */
 ncsmem_aid_init(&ma, buffer, MAX_MIB_BUFF_SIZE);

        /* Fill in the NCSMIB_ARG structure */
 arg.i_op = NCSMIB_OP_REQ_GET;
 arg.i_tbl_id = table_id;
 arg.i_usr_key = gl_mac_handle; /* application can fill their handle here */
 arg.i_mib_key = gl_mac_handle;/* Handle of MAA, used to communicate with MAA */ 
 arg.i_rsp_fnc = NULL;

 /* Fill in the index depending on the INDEX type of the table as defined in the MIB*/
        /* In this case the mibi table is indexed by the ipaddress.So index filled as shown below */
        index[0] = 10;
 index[1] = 232;
 index[2] = 92;
 index[3] = 184;
    
 printf("\nINDEX:  %d %d %d %d ", index[0], 
       index[1], index[2], index[3]);
        /* Fill the index info into NCSMIB_ARG structure */
 arg.i_idx.i_inst_ids = index;
 arg.i_idx.i_inst_len = 4; /* Length of the index */
 arg.req.info.get_req.i_param_id = parameter_id;
        
 /*
     model a MIB request as synchronous; set timer
 */
 uns32 retval = ncsmib_sync_request(&arg, ncsmac_mib_request, 
                                  NCS_SYNC_CALL_TIMEOUT, &ma);

 if (NCSCC_RC_SUCCESS != retval)  /* if ncsmib_sync_request failed , !.e the request not reached application */
 {
       printf("\nncsmib_sync_request failed %d, %d", retval, arg.rsp.i_status);
       return NCSCC_RC_FAILURE;
 }
 else  /* if ncsmib_sync_request succeeded and application served the request*/
 {
    if (arg.rsp.i_status != NCSCC_RC_SUCCESS)/* get request is not successfull */ 
    {
       printf("\nBad i_status %d in ARG",arg.rsp.i_status);
       return NCSCC_RC_FAILURE;
    }
    else                                    /* get request is successfull */
    {
        param_value.assign(
                      (char*)arg.rsp.info.get_rsp.i_param_val.info.i_oct,
                      arg.rsp.info.get_rsp.i_param_val.i_length);
        printf("\nLength of i_oct is %d", arg.rsp.info.get_rsp.i_param_val.i_length);
    }
 }

 return NCSCC_RC_SUCCESS;
}

uns32 get_mib_parameter(int &param_value, NCSMIB_TBL_ID table_id, 
    int parameter_id, int physical_board_id)
{
    NCSMIB_ARG  arg;
    NCSMEM_AID  ma;
    uns8 buffer[MAX_MIB_BUFF_SIZE];
    uns32 index[4];
    
    /*
      Put this NCSMIB_ARG structure in start state. Do not have any valid data
      or pointers hanging off the passed structure, as this function will
      overwrite such stuff with null.
    */
    ncsmib_init(&arg);

    /* put the NCSMEM_AID in start state */
    ncsmem_aid_init(&ma, buffer, MAX_MIB_BUFF_SIZE);

    /* Fill in the NCSMIB_ARG structure */
    arg.i_op = NCSMIB_OP_REQ_GET;
    arg.i_tbl_id = table_id;
    arg.i_usr_key = gl_mac_handle; /* application can fill their handle here */
    arg.i_mib_key = gl_mac_handle; /* Handle of MAA, used to communicate with MAA */
    arg.i_xch_id = 0;
    arg.i_rsp_fnc = NULL;

    /* Fill in the index depending on the INDEX type of the table as defined in the MIB*/
    /* In this case the mibi table is indexed by the ipaddress.So index filled as shown below */
    index[0] = 10;
    index[1] = 232;
    index[2] = 92;
    index[3] = 184;

    printf("\nINDEX:  %d %d %d %d ", index[0], index[1], index[2], index[3]);
    /* Fill the index info into NCSMIB_ARG structure */
    arg.i_idx.i_inst_ids = index;
    arg.i_idx.i_inst_len = 4;  /* Length of the index */
    arg.req.info.get_req.i_param_id = parameter_id;

   /*
       model a MIB request as synchronous; set timer
   */
    uns32 retval = ncsmib_sync_request(&arg, ncsmac_mib_request,
        NCS_SYNC_CALL_TIMEOUT, &ma);
    if (NCSCC_RC_SUCCESS != retval) /* if ncsmib_sync_request failed , !.e the request not reached application */
    {
        printf("\nncsmib_sync_request failed %d, %d", retval, arg.rsp.i_status);
        return NCSCC_RC_FAILURE;
    }
    else /* if ncsmib_sync_request succeeded and application served the request*/
    {
            param_value = arg.rsp.info.get_rsp.i_param_val.info.i_int;
    }

    return NCSCC_RC_SUCCESS;
}

/*APIs for setting string and int value params*/
uns32 set_mib_parameter(std::string param_value, NCSMIB_TBL_ID table_id,
 int parameter_id, int physical_board_id)
{
    NCSMIB_ARG  arg;
    NCSMEM_AID  ma;
    char c_string[MAX_MIB_BUFF_SIZE];
    int param_value_len = param_value.length();
    uns8 buffer[MAX_MIB_BUFF_SIZE];
    uns32 index[4];

    if (param_value_len >= MAX_MIB_BUFF_SIZE)
    {
        printf("\nString value too long");
        return NCSCC_RC_FAILURE;
    }

    /*convert the stl string to a c string*/
    param_value.copy(c_string, param_value_len);
    /*NULL terminate it just in case...*/
    c_string[param_value_len] = '\0';

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
    arg.i_tbl_id = table_id;
    arg.i_usr_key = gl_mac_handle; /* application can fill their handle here */
    arg.i_mib_key = gl_mac_handle; /* Handle of MAA, used to communicate with MAA */
    arg.i_xch_id = 0;
    arg.i_rsp_fnc = NULL;

    /* Fill in the index depending on the INDEX type of the table as defined in the MIB */
    /* In this case the mibi table is indexed by the ipaddress.So index filled as shown below */
    index[0] = 10;
    index[1] = 232;
    index[2] = 92;
    index[3] = 184;
    /* Fill the index info into NCSMIB_ARG structure */
    arg.i_idx.i_inst_ids = index;
    arg.i_idx.i_inst_len = 4;  /* Length of the index */

    arg.req.info.set_req.i_param_val.i_param_id = parameter_id;
    arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
    arg.req.info.set_req.i_param_val.i_length = param_value_len + 1; /*+1 for '/0' char.*/
    arg.req.info.set_req.i_param_val.info.i_oct = (uns8*)c_string;

   /*
       model a MIB request as synchronous; set timer
   */
    uns32 retval = ncsmib_sync_request(&arg, ncsmac_mib_request,
        NCS_SYNC_CALL_TIMEOUT, &ma);
    if (NCSCC_RC_SUCCESS != retval) /* if ncsmib_sync_request failed , !.e the request not reached application */
    {
        printf("\nncsmib_sync_request failed %d, %d", retval, arg.rsp.i_status);
        return NCSCC_RC_FAILURE;
    }
    else /* if ncsmib_sync_request succeeded and application served the request*/
    {
        if (arg.rsp.i_status != NCSCC_RC_SUCCESS) /* set request is not successfull */
        {
            printf("\nBad i_status %d in ARG", arg.rsp.i_status);
            return NCSCC_RC_FAILURE;
        }
    }
    return NCSCC_RC_SUCCESS;
}

uns32 set_mib_parameter(int param_value, NCSMIB_TBL_ID table_id,
    int parameter_id, int physical_board_id)
{
    NCSMIB_ARG  arg;
    NCSMEM_AID  ma;
    uns8 buffer[MAX_MIB_BUFF_SIZE];
    uns32 index[4];

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
    arg.i_tbl_id = table_id;
    arg.i_usr_key = gl_mac_handle; /* application can fill their handle here */
    arg.i_mib_key = gl_mac_handle; /* Handle of MAA, used to communicate with MAA */
    arg.i_xch_id = 0;
    arg.i_rsp_fnc = NULL;

    /* Fill in the index depending on the INDEX type of the table as defined in the MIB*/
    /* In this case the mibi table is indexed by the ipaddress.So index filled as shown below */
    index[0] = 10;
    index[1] = 232;
    index[2] = 92;
    index[3] = 184;
    /* Fill the index info into NCSMIB_ARG structure */
    arg.i_idx.i_inst_ids = index;
    arg.i_idx.i_inst_len = 4;  /* Length of the index */

    arg.req.info.set_req.i_param_val.i_param_id = parameter_id;
    arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
    arg.req.info.set_req.i_param_val.i_length = 4; /*Ignored for integers.*/
    arg.req.info.set_req.i_param_val.info.i_int = param_value;

   /*
       model a MIB request as synchronous; set timer
   */
    uns32 retval = ncsmib_sync_request(&arg, ncsmac_mib_request,
        NCS_SYNC_CALL_TIMEOUT, &ma);
    if (NCSCC_RC_SUCCESS != retval) /* if ncsmib_sync_request failed , !.e the request not reached application */
    {
        printf("\nncsmib_sync_request failed %d, %d", retval, arg.rsp.i_status);
        return NCSCC_RC_FAILURE;
    }
    else  /* if ncsmib_sync_request succeeded and application served the request*/
    {
        if (arg.rsp.i_status != NCSCC_RC_SUCCESS) /* set request is not successfull */
        {
            printf("\nBad i_status %d in ARG", arg.rsp.i_status);
            return NCSCC_RC_FAILURE;
        }
    }

    return NCSCC_RC_SUCCESS;
}

int main(void)
{
   NCSMIB_TBL_ID table_id;
   NCSMIB_IDX    i_idx;
   uns32 status;

   /* initiliase the Environment */
   ncs_agents_startup(0,NULL);

   printf("\ngl_mac_handle=%d\n", gl_mac_handle);

   std::string value = "Set";
   int value_int = 100;
             
   /* Set ncsTestTableOneDisplayString to value */
   set_mib_parameter(value, (NCSMIB_TBL_ID)NCSMIB_TBL_SNMPTM_TBLONE,
           1 /* ncsTestTableOneDisplayString */,7);
   value = "NotSet";
   /* Get ncsTestTableOneDisplayString into value */
   get_mib_parameter(value, (NCSMIB_TBL_ID)NCSMIB_TBL_SNMPTM_TBLONE, 
                1/*ncsTestTableOneDisplayString*/, 7);
   printf ("\nncsTestTableOneDisplayString=%s\n", value.c_str());

  
   /* Set ncsTestTableOneDiscrete to value_int */
   set_mib_parameter(value_int, (NCSMIB_TBL_ID)NCSMIB_TBL_SNMPTM_TBLONE,
               8 /* ncsTestTableOneDiscrete */,7);
   value_int = 200;
   /* Get ncsTestTableOneDiscrete into value_int */
   get_mib_parameter(value_int, (NCSMIB_TBL_ID)NCSMIB_TBL_SNMPTM_TBLONE, 
                8 /* ncsTestTableOneDiscrete */, 7);
   printf ("\nncsTestTableOneDiscrete=%d\n", value_int);
   printf("\n");

   /* getnextrow operation */
   for(table_id = NCSMIB_TBL_AVSV_CLM_NODE; table_id <= NCSMIB_TBL_AVSV_END; table_id = (NCSMIB_TBL_ID)((int)table_id + 1))
   {
       printf("\n#############################################################\n");
       printf("\n           Start of Table-Id: %d                    \n",table_id);
       printf("\n#############################################################\n\n");
       i_idx.i_inst_ids = NULL;
       i_idx.i_inst_len = 0;
       status=getnextrow_mib_parameter(table_id,i_idx);
       if(NCSCC_RC_SUCCESS!= status) 
           printf("\ngetnextrow_mib_parameter failed for tabel_id: %d \n",table_id);
       printf("\n#############################################################\n");
       printf("\n             End of Table-Id: %d                    \n",table_id);
       printf("\n#############################################################\n\n");
   }
}
uns32 getnextrow_mib_parameter(NCSMIB_TBL_ID table_id,NCSMIB_IDX i_idx)
{
    NCSMIB_ARG  arg;
    NCSMEM_AID  ma;
    uns8 buffer[MAX_MIB_BUFF_SIZE];
    uns32 status;

    printf("\ngetnextrow_mib_parameter enter for tabel_id: %d",table_id);
    while(1)
    {
        ncsmib_init(&arg);
        ncsmem_aid_init(&ma, buffer, MAX_MIB_BUFF_SIZE);
        arg.i_tbl_id = table_id;
        arg.i_idx = i_idx;
        arg.i_usr_key = gl_mac_handle;
        arg.i_mib_key = gl_mac_handle;
        arg.i_rsp_fnc = NULL;
        arg.i_op = NCSMIB_OP_REQ_NEXTROW;

        uns32 retval = ncsmib_sync_request(&arg, ncsmac_mib_request,
                                          NCS_SYNC_CALL_TIMEOUT, &ma);
        if (NCSCC_RC_SUCCESS != retval)
        {
            printf("\nncsmib_sync_request failed %d, %d", retval, arg.rsp.i_status);
            return NCSCC_RC_FAILURE;
        }
        else
        {
            printf("\nncsmib_sync_request success %d, %d", retval, arg.rsp.i_status);
            if (arg.rsp.i_status != NCSCC_RC_SUCCESS)/* get request is not successfull */
            {
                 return NCSCC_RC_FAILURE;
            }
        }
    
        if(arg.i_op ==  NCSMIB_OP_RSP_NEXTROW) 
        {
             printf("\nprinting the responce of NCSMIB_OP_REQ_NEXTROW (i.e NCSMIB_OP_RSP_NEXTROW)\n");
             NCSMIB_NEXTROW_RSP* rsp  = &arg.rsp.info.nextrow_rsp;
             ncsmib_pp_new(&arg);
             i_idx.i_inst_ids = rsp->i_next.i_inst_ids;
             i_idx.i_inst_len = rsp->i_next.i_inst_len;
             /* issue getrow request based on output of getnextrow */
             status = getrow_mib_parameter(table_id, i_idx);
             if(NCSCC_RC_SUCCESS != status)
             {
                  printf("\ngetrow_mib_parameter failed for tabel_id: %d \n",table_id);
                  return NCSCC_RC_FAILURE;
             }
        }
        printf("\n");
    }
    printf("\ngetnextrow_mib_parameter left for table_id: %d",table_id);
    return NCSCC_RC_SUCCESS;
}
uns32 getrow_mib_parameter(NCSMIB_TBL_ID table_id,NCSMIB_IDX i_idx)
{
    NCSMIB_ARG  arg;
    NCSMEM_AID  ma;
    uns8 buffer[MAX_MIB_BUFF_SIZE];

    printf("\ngetrow_mib_parameter enter for tabel_id: %d",table_id);

     ncsmib_init(&arg);
     ncsmem_aid_init(&ma, buffer, MAX_MIB_BUFF_SIZE);
     arg.i_tbl_id = table_id;
     arg.i_idx = i_idx;
     arg.i_usr_key = gl_mac_handle;
     arg.i_mib_key = gl_mac_handle;
     arg.i_rsp_fnc = NULL;
     arg.i_op = NCSMIB_OP_REQ_GETROW;

    uns32 retval = ncsmib_sync_request(&arg, ncsmac_mib_request,
                                      NCS_SYNC_CALL_TIMEOUT, &ma);
    if (NCSCC_RC_SUCCESS != retval)
    {
        printf("\nncsmib_sync_request failed %d, %d", retval, arg.rsp.i_status);
        return NCSCC_RC_FAILURE;
    }
    else
    {
        printf("\nncsmib_sync_request success %d, %d", retval, arg.rsp.i_status);
        if (arg.rsp.i_status != NCSCC_RC_SUCCESS)/* get request is not successfull */
        {
             return NCSCC_RC_FAILURE;
        }
    }
   
    if(arg.i_op ==  NCSMIB_OP_RSP_GETROW) 
    {
         printf("\nprinting the responce of NCSMIB_OP_REQ_GETROW (i.e NCSMIB_OP_RSP_GETROW)\n");
         ncsmib_pp_new(&arg);
    }

    printf("\ngetrow_mib_parameter left for table_id: %d",table_id);
    return NCSCC_RC_SUCCESS;
}
void ncsmib_pp_new(NCSMIB_ARG* arg)
{
   m_NCS_CONS_PRINTF("=================================\n");
   m_NCS_CONS_PRINTF("MIB_ARG  PP @ address %lx\n", (unsigned long)arg);
   m_NCS_CONS_PRINTF("=================================\n");
   m_NCS_CONS_PRINTF("Op: %s, Tbl Id: %d, Exch Id: %x\n", mibpp_opstr_new(arg->i_op),
                    arg->i_tbl_id, 
                    arg->i_xch_id);
   m_NCS_CONS_PRINTF("User Key: %d\n", arg->i_usr_key);
   m_NCS_CONS_PRINTF("MIB Key: %d\n", arg->i_mib_key);
   m_NCS_CONS_PRINTF("Source: %d\n", arg->i_policy);

   if (m_NCSMIB_ISIT_A_RSP(arg->i_op))
   {
      if (arg->rsp.i_status != 0)
          m_NCS_CONS_PRINTF("Status : %s\n", mibpp_status_new(arg->rsp.i_status));
      if (arg->rsp.i_status == NCSCC_RC_SUCCESS)
      {
         switch (arg->i_op)
         {
         /*****************************************************************************
         The Response Cases
         *****************************************************************************/
         case NCSMIB_OP_RSP_GET  :
         case NCSMIB_OP_RSP_SET  :
         case NCSMIB_OP_RSP_TEST :
            {
               NCSMIB_GET_RSP* rsp = &arg->rsp.info.get_rsp;
               mibpp_inst_new(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len);
               mibpp_param_val_new(&rsp->i_param_val);
               break;
            }

         case NCSMIB_OP_RSP_NEXT :
            {
               NCSMIB_NEXT_RSP* rsp  = &arg->rsp.info.next_rsp;
               mibpp_inst_new(rsp->i_next.i_inst_ids,rsp->i_next.i_inst_len);
               mibpp_param_val_new(&rsp->i_param_val);
               break;
            }

         case NCSMIB_OP_RSP_GETROW :
            {
               NCSMIB_GETROW_RSP* rsp = &arg->rsp.info.getrow_rsp;
               mibpp_inst_new(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len);
               mibpp_row_new(rsp->i_usrbuf);
               break;
            }

         case NCSMIB_OP_RSP_NEXTROW :
            {
               NCSMIB_NEXTROW_RSP* rsp  = &arg->rsp.info.nextrow_rsp;
               mibpp_inst_new(rsp->i_next.i_inst_ids,rsp->i_next.i_inst_len);
               mibpp_row_new(rsp->i_usrbuf);
               break;
            }

         case NCSMIB_OP_RSP_MOVEROW:
            {
               NCSMIB_MOVEROW_RSP* rsp = &arg->rsp.info.moverow_rsp;
               /* m_NCS_CONS_PRINTF("move_to vcard:%u\n",rsp->i_move_to.info.v1.vcard); */
               mibpp_row_new(rsp->i_usrbuf);
               break;
            }

         case NCSMIB_OP_RSP_SETROW:
         case NCSMIB_OP_RSP_SETALLROWS:
         case NCSMIB_OP_RSP_TESTROW:
         case NCSMIB_OP_RSP_REMOVEROWS:
            mibpp_inst_new(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len);               
            break;
         
         case NCSMIB_OP_RSP_CLI:
         case NCSMIB_OP_RSP_CLI_DONE:
            {
               NCSMIB_CLI_RSP* rsp = &arg->rsp.info.cli_rsp;
               mibpp_inst_new(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len);
               if (rsp->o_answer != NULL)
                  mibpp_row_new(rsp->o_answer);
               break;
            }
         default:
            {
               m_NCS_CONS_PRINTF("!!! Invalid Operation Type !!!\n");
               m_LEAP_DBG_SINK_VOID(0);
               return;
            }
         }
      }
      else /* Added to fix the bug 60198 */
      {
          /* Fix for the bug 60198 */
          arg->i_op = (NCSMIB_OP)m_NCSMIB_RSP_TO_REQ(arg->i_op); /* Converting response to corresponding reques op */
          ncsmib_req_pp_new(arg); 
          arg->i_op = (NCSMIB_OP)m_NCSMIB_REQ_TO_RSP(arg->i_op); /* Converting request op to corresponding response op */
      }
   }
   else
   {
       /* code existing in this else part is moved to function ncsmib_req_pp while fixing the bug 60198 */
       ncsmib_req_pp_new(arg);
   }
}
char* mibpp_opstr_new(uns32 op)
{
   switch (op)
   {
   case NCSMIB_OP_RSP_GET     : return "GET Resp";      break;
   case NCSMIB_OP_RSP_SET     : return "SET Resp";      break;
   case NCSMIB_OP_RSP_TEST    : return "TEST Resp";     break;
   case NCSMIB_OP_RSP_NEXT    : return "GET NEXT Resp"; break;
   case NCSMIB_OP_RSP_GETROW  : return "GET ROW Resp";  break;
   case NCSMIB_OP_RSP_NEXTROW : return "NEXT ROW Resp"; break;
   case NCSMIB_OP_RSP_SETROW  : return "SET ROW Resp" ; break;
   case NCSMIB_OP_RSP_TESTROW : return "TEST ROW Resp"; break;
   case NCSMIB_OP_RSP_MOVEROW : return "MOVE ROW Resp"; break;
   case NCSMIB_OP_RSP_GET_INFO: return "GETINFO Resp" ; break;
   case NCSMIB_OP_RSP_REMOVEROWS: return "REMOVE ROWS Resp"; break;
   case NCSMIB_OP_RSP_SETALLROWS: return "SETALL ROWS Resp"; break;
   case NCSMIB_OP_RSP_CLI     : return "CLI Resp";      break;
   case NCSMIB_OP_RSP_CLI_DONE: return "CLI Resp, for wild-card req";      break;

   case NCSMIB_OP_REQ_GET     : return "GET Req";       break;
   case NCSMIB_OP_REQ_NEXT    : return "GET NEXT Req";  break;
   case NCSMIB_OP_REQ_SET     : return "SET Req";       break;
   case NCSMIB_OP_REQ_TEST    : return "TEST Req";      break;
   case NCSMIB_OP_REQ_GETROW  : return "GET ROW Req";   break;
   case NCSMIB_OP_REQ_NEXTROW : return "NEXT ROW Req";  break;
   case NCSMIB_OP_REQ_SETROW  : return "SET ROW Req" ;  break;
   case NCSMIB_OP_REQ_TESTROW : return "TEST ROW Req";  break;
   case NCSMIB_OP_REQ_MOVEROW : return "MOVE ROW Req";  break;
   case NCSMIB_OP_REQ_GET_INFO: return "GETINFO Req" ;  break;
   case NCSMIB_OP_REQ_REMOVEROWS: return "REMOVE ROWS Req"; break;
   case NCSMIB_OP_REQ_SETALLROWS: return "SETALL ROWS Req"; break;
   case NCSMIB_OP_REQ_CLI     : return "CLI Req";      break;
   }
   m_LEAP_DBG_SINK_VOID(0);  
   return "UNKNOWN OP";   /* effectively, the default */
}
char*  mibpp_status_new(uns32 status)
{
   char* ptr = "Unknown MIB Error!!";
   switch (status)
   {
   case NCSCC_RC_SUCCESS:          ptr = "NCSCC_RC_SUCCESS";          break;
   case NCSCC_RC_NO_SUCH_TBL:      ptr = "NCSCC_RC_NO_SUCH_TBL";      break;     
   case NCSCC_RC_NO_OBJECT:        ptr = "NCSCC_RC_NO_OBJECT";        break;        
   case NCSCC_RC_NO_INSTANCE:      ptr = "NCSCC_RC_NO_INSTANCE";      break;      
   case NCSCC_RC_INV_VAL:          ptr = "NCSCC_RC_INV_VAL";          break;          
   case NCSCC_RC_INV_SPECIFIC_VAL: ptr = "NCSCC_RC_INV_SPECIFIC_VAL"; break; 
   case NCSCC_RC_REQ_TIMOUT:       ptr = "NCSCC_RC_REQ_TIMOUT";       break;       
   case NCSCC_RC_FAILURE:          ptr = "NCSCC_RC_FAILURE";          break;
   case NCSCC_RC_NO_ACCESS:        ptr = "NCSCC_RC_NO_ACCESS";        break;
   case NCSCC_RC_NO_CREATION:      ptr = "NCSCC_RC_NO_CREATION";      break;
   }
   return ptr;
}
void mibpp_inst_new(const uns32* inst, uns32 len)
{
   uns32 i;

   m_NCS_CONS_PRINTF("Index vals as int: ");
   if (inst == NULL)
      m_NCS_CONS_PRINTF("NULL\n");
   else
   {
      for (i = 0; i < len; i++)
         m_NCS_CONS_PRINTF(", %d",inst[i]);
      m_NCS_CONS_PRINTF(" \n");
   }

   m_NCS_CONS_PRINTF("Index vals as ascii: ");
   if (inst == NULL)
      m_NCS_CONS_PRINTF("NULL\n");
   else
   {
      for (i = 0; i < len; i++)
         m_NCS_CONS_PRINTF(", %c",inst[i]);
      m_NCS_CONS_PRINTF(" \n");
   }
}
void mibpp_param_val_new(NCSMIB_PARAM_VAL* val)
{
   uns32 i;

   m_NCS_CONS_PRINTF("Param Val: ");
   m_NCS_CONS_PRINTF("Id %d, %s, Val ",val->i_param_id, pp_fmat_str_new[val->i_fmat_id]);
   if (val->i_fmat_id == NCSMIB_FMAT_INT || val->i_fmat_id == NCSMIB_FMAT_BOOL)
      m_NCS_CONS_PRINTF("%d\n",val->info.i_int );

   else if (val->i_fmat_id == NCSMIB_FMAT_OCT)
   {
      for (i = 0; i < val->i_length; i++)
         m_NCS_CONS_PRINTF(", 0x%x ",val->info.i_oct[i]);
      m_NCS_CONS_PRINTF(" \n");
   }
}
void mibpp_row_new(USRBUF* inub)
{
  NCSPARM_AID      pa;
  NCSMIB_PARAM_VAL pv;
  USRBUF*         ub = NULL;

  m_NCS_CONS_PRINTF("ROW STARTS\n");

  if(inub == NULL)
    return;

  ub = m_MMGR_DITTO_BUFR(inub);

  if(ub == NULL)
    return;

  ncsparm_dec_init(&pa,ub);

  while((pa.cnt > 0) && (pa.len > 0))
    {
    ncsparm_dec_parm(&pa,&pv,NULL);
    
    mibpp_param_val_new(&pv);
    
    if((pv.i_fmat_id == NCSMIB_FMAT_OCT) && (pv.info.i_oct != NULL))
      m_MMGR_FREE_MIB_OCT(pv.info.i_oct);
    
    }

  if(ncsparm_dec_done(&pa) != NCSCC_RC_SUCCESS)
    {
    m_LEAP_DBG_SINK_VOID(NCSCC_RC_FAILURE);
    return;
    }

  m_NCS_CONS_PRINTF("ROW ENDS\n");

  return;
}
void ncsmib_req_pp_new(NCSMIB_ARG* arg)
{
      mibpp_inst_new(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len);
      switch (arg->i_op)
      {
      /*****************************************************************************
      The Request Cases
      *****************************************************************************/

      case NCSMIB_OP_REQ_GET  :
      case NCSMIB_OP_REQ_NEXT :
         {
            NCSMIB_GET_REQ* req = &arg->req.info.get_req;
            m_NCS_CONS_PRINTF("Param ID %d\n",req->i_param_id);
            break;
         }

      case NCSMIB_OP_REQ_SET  :
      case NCSMIB_OP_REQ_TEST :
         {
            NCSMIB_SET_REQ* req = &arg->req.info.set_req;
            mibpp_param_val_new(&req->i_param_val);
            break;
         }

      case NCSMIB_OP_REQ_GETROW  :
      case NCSMIB_OP_REQ_NEXTROW :
         {
            NCSMIB_GETROW_REQ* req = &arg->req.info.getrow_req;
            USE(req);
            /* nothing to do here... */ 
            break;
         }

      case NCSMIB_OP_REQ_SETROW  :
      case NCSMIB_OP_REQ_TESTROW :
         {
            NCSMIB_SETROW_REQ* req = &arg->req.info.setrow_req;
            mibpp_row_new(req->i_usrbuf);
            break;
         }

      case NCSMIB_OP_REQ_MOVEROW :
        {
            NCSMIB_MOVEROW_REQ* req = &arg->req.info.moverow_req;
            if (m_NCS_NODE_ID_FROM_MDS_DEST(req->i_move_to) == 0)
            {
                m_NCS_CONS_PRINTF("move_to VDEST:%lld\n",req->i_move_to);
            }
            else
            {
                m_NCS_CONS_PRINTF("move_to ADEST:%lld\n",req->i_move_to);
            }
            mibpp_row_new(req->i_usrbuf);
            break;
        }

      case NCSMIB_OP_REQ_CLI  :
         {
            NCSMIB_CLI_REQ* req = &arg->req.info.cli_req;
            if (req->i_usrbuf != NULL)
               mibpp_row_new(req->i_usrbuf);
            break;
         }

      case NCSMIB_OP_REQ_REMOVEROWS:
      case NCSMIB_OP_REQ_SETALLROWS:
          break;

      default:
         {
            m_NCS_CONS_PRINTF("!!! Invalid Operation Type !!!\n");
            m_LEAP_DBG_SINK_VOID(0);
            return;
         }
      }
}
