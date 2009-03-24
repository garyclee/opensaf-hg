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
 *
 */

/*****************************************************************************
..............................................................................

  MODULE NAME:  AVSVCLI.C

..............................................................................
  DESCRIPTION:

  This module contains functions used by the AVSV Subsystem for Command Line
  Interface.

..............................................................................

 FUNCTIONS INCLUDED in this module:

   dtsv_cef_set_logging_level.........CEF for setting DTSV logging level
***************************************************************************/

#include <config.h>

#include "avsv.h"
#include "mac_papi.h"
#include "ncs_cli.h"
#include "hpl_api.h"

#include <SaHpi.h>
#include "hpl_msg.h"

#define AVSV_BUFFER_LEN   200

#define m_RETURN_AVSV_CLI_DONE(st,s,k)   avsv_cli_done(st,s,k)

#define NCS_CLI_MIB_REQ_TIMEOUT  1000
#define AVM_DEFAULT_HIERARCHY_LVL 2
#define AVM_EXT_HIERARCHY_LVL     3

#define FLUSHIN(c) while(((c = getchar()) != EOF) && (c != '\n'))

uns32
avsv_cef_set_sg_param_values(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);

static uns32 avsv_cli_cmds_reg(NCSCLI_BINDERY *pBindery);

uns32 avsv_config_cef(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);

uns32
avsv_cli_done(uns8 *string, uns32 status, uns32 cli_hdl);

uns32
avsv_cli_display(uns32 cli_hdl, char *str);

uns32
avsv_cli_build_mib_idx(NCSMIB_IDX *mib_idx, char *idx_val, NCSMIB_FMAT_ID format, uns32 cli_hdl);

static uns32
avsv_cli_cfg_mib_arg(NCSMIB_ARG *mib, uns32 *index, uns32 index_len,
                     NCSMIB_TBL_ID  tid, uns32 usr_hdl, uns32 ss_hdl,
                     NCSMIB_REQ_FNC rspfnc);

uns32
avsv_cli_build_and_generate_mibsets(NCSMIB_TBL_ID table_id, uns32 param_id,
                                NCSMIB_IDX *mib_idx, char *val, NCSMIB_FMAT_ID format, NCSMIB_REQ_FNC reqfnc, uns32 cli_hdl);

uns32 ncsavsv_cef_load_lib_req(NCS_LIB_REQ_INFO *libreq);


static uns32
avm_cef_set_ent_adm_req(
                          NCSCLI_ARG_SET  *arg_list, 
                          NCSCLI_CEF_DATA *cef_data
                       );
static uns32
avm_cef_set_adm_switch(
                         NCSCLI_ARG_SET  *arg_list, 
                         NCSCLI_CEF_DATA *cef_data
                      );

/*****************************************************************************
  PROCEDURE NAME: avsv_cli_hisv_init
  DESCRIPTION   : Initialize the HISv client library for use by the CLI.
  ARGUMENTS     :
  RETURNS       : SUCCESS or FAILURE
  NOTES         : CLI needs to lookup entity paths using an HISv API.
*****************************************************************************/
static uns32 avsv_cli_hisv_init() {
   uns32		rc = NCSCC_RC_SUCCESS;
   SaEvtHandleT 	gl_evt_hdl = 0;
   SaEvtCallbacksT 	reg_callback_set;
   NCS_LIB_CREATE 	hisv_create_info;
   SaVersionT 		ver;

   /* Initialize the event subsystem for communication with HISv */
   ver.releaseCode = 'B';
   ver.majorVersion = 0x01;
   ver.minorVersion  = 0x01;
   rc = saEvtInitialize(&gl_evt_hdl, &reg_callback_set, &ver);

   /* Initialize the HPL client-side library */
   rc = hpl_initialize(&hisv_create_info);
   return(rc);
}

/*****************************************************************************
  PROCEDURE NAME: avsv_cli_cmds_reg
  DESCRIPTION   : Registers the AVSV commands with the CLI.
  ARGUMENTS     :
      cli_bind  : pointer to NCSCLI_BINDERY structure containing
                  the information required to interact with the CLI.
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
static uns32 avsv_cli_cmds_reg(NCSCLI_BINDERY *pBindery)
{
   NCSCLI_CMD_LIST data;
   uns32          rc = NCSCC_RC_SUCCESS;
   uns32          idx;
   NCSCLI_OP_INFO req;
   NCSCLI_CMD_DESC avsv_cli_cmds[] =
   {
      {
         avsv_cef_set_sg_param_values,
         "set!set function! NCSCLI_STRING!provide index here  SG,SU,SI or Node name: e.g.- safSg=...,safNode=...! \
         {adminstate!Admin State! {locked|unlocked|shuttingdown} } | {termstate!Terminate State applicabe for su only! {true|false} }",
         NCSCLI_ADMIN_ACCESS
      },
      {
         avm_cef_set_ent_adm_req,
         "reset!Reset the blade! NCSCLI_STRING!Shelf-Id/Slot-Id/Subslot-Id, Subslot-ID optional! operation!Admin Operation! {hardreset|softreset} [NCSCLI_STRING!Entity Types in Hierarchy Shelf-Type/Blade-Type/Blade-Type, Subslot-Id mandates all EntityTypes to be mentioned, otherwise default values for Shelf-type and Blade type in first two levels are 23,7!]",

         NCSCLI_ADMIN_ACCESS
      },
      {
         avm_cef_set_ent_adm_req,
         "admreq!Power down the blade! NCSCLI_STRING!Shelf-Id/Slot-Id/Subslot-Id, Subslot-ID optional! operation!Admin Operation! {shutdown | lock | unlock} [NCSCLI_STRING! Entity Types in Hierarchy Shelf-Type/Blade-Type/Blade-Type, Subslot id mandates all the three types to be mentioned, otherwise default values for first two levels are 23,7!]",
         NCSCLI_ADMIN_ACCESS
      },
      {
         avm_cef_set_adm_switch,
         "admswitch!Switchover System Controller Hosts! ",
         NCSCLI_ADMIN_ACCESS
      } 
   };

   memset(&req, 0, sizeof(NCSCLI_OP_INFO));
   req.i_hdl = pBindery->i_cli_hdl;
   req.i_req = NCSCLI_OPREQ_REGISTER;
   req.info.i_register.i_bindery = pBindery;

   data.i_node = "root/exec/config";
   data.i_command_mode = "config";
   data.i_access_req = FALSE;
   data.i_access_passwd = 0;
   data.i_cmd_count = 1;
   data.i_command_list[0].i_cmdstr = "avsv!Configure AVSV commands!@root/exec/config/avsv@";
   data.i_command_list[0].i_cmd_exec_func = avsv_config_cef;
   data.i_command_list[0].i_cmd_access_level = NCSCLI_ADMIN_ACCESS;

   req.info.i_register.i_cmdlist = &data;
   if(NCSCC_RC_SUCCESS != ncscli_opr_request(&req)) return NCSCC_RC_FAILURE;

   /**************************************************************************\
   *                                                                          *
   *   AVSV CLI Top Level Commands                                            *
   *                                                                          *
   \**************************************************************************/
   memset(&data, 0, sizeof(NCSCLI_CMD_LIST));
   data.i_node = "root/exec/config/avsv";
   data.i_command_mode = "avsv";
   data.i_access_req = FALSE;
   data.i_access_passwd = 0;
   data.i_cmd_count = sizeof(avsv_cli_cmds) / sizeof(avsv_cli_cmds[0]);

   for (idx=0; idx < data.i_cmd_count; idx++)
      data.i_command_list[idx] = avsv_cli_cmds[idx];

   /* Attempt to initialize the HISv client library */
   rc = avsv_cli_hisv_init();
   if (rc == NCSCC_RC_FAILURE) {
      printf("\nWarning: could not initialize HISv hpl_api library\n");
   }

   req.info.i_register.i_cmdlist = &data;
   rc = ncscli_opr_request(&req);

   return rc;
}

uns32 avsv_config_cef(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
   NCSCLI_OP_INFO info;

   info.i_hdl = cef_data->i_bindery->i_cli_hdl;
   info.i_req = NCSCLI_OPREQ_DONE;
   info.info.i_done.i_status = NCSCC_RC_SUCCESS;

   ncscli_opr_request(&info);
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: ncsavsv_cef_load_lib_req
  DESCRIPTION   : Registers the AVSV commands with the CLI.
  ARGUMENTS     :
      libreq    : pointer to NCS_LIB_REQ_INFO structure containing
                  the information required to interact with the CLI.
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 ncsavsv_cef_load_lib_req(NCS_LIB_REQ_INFO *libreq)
{
    NCSCLI_BINDERY  i_bindery;
    if (libreq == NULL)
        return NCSCC_RC_FAILURE;

    switch (libreq->i_op)
    {
        case NCS_LIB_REQ_CREATE:
        memset(&i_bindery, 0, sizeof(NCSCLI_BINDERY));
        i_bindery.i_cli_hdl = gl_cli_hdl;
        i_bindery.i_mab_hdl = gl_mac_handle;
        i_bindery.i_req_fnc = ncsmac_mib_request;
        return avsv_cli_cmds_reg(&i_bindery);
        break;
        case NCS_LIB_REQ_INSTANTIATE:
        case NCS_LIB_REQ_UNINSTANTIATE:
        case NCS_LIB_REQ_DESTROY:
        case NCS_LIB_REQ_TYPE_MAX:
        default:
        break;
    }
    return NCSCC_RC_FAILURE;
}

/*****************************************************************************
  PROCEDURE NAME: avsv_cef_set_sg_param_values
  DESCRIPTION   : CEF for setting parameter values in SG table
  ARGUMENTS     :
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32
avsv_cef_set_sg_param_values(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
   uns32          cli_hdl  = cef_data->i_bindery->i_cli_hdl;
   NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
   NCSCLI_ARG_VAL *index = &arg_list->i_arg_record[1];
   NCSCLI_ARG_VAL *action = &arg_list->i_arg_record[2];
   NCSCLI_ARG_VAL *value = &arg_list->i_arg_record[3];
   uns32          rc = NCSCC_RC_SUCCESS;
   uns8          val[4];
   NCSMIB_IDX        mib_idx;
   NCSMIB_TBL_ID        table_id;
   uns32                param_id;
   
   memset(val, 0, sizeof(val));

   if(strcmp(action->cmd.strval,"adminstate") == 0)
   {
      if(strcmp(value->cmd.strval,"locked") == 0)
      {
         sprintf(val, "%d", 1);
      }
      else if(strcmp(value->cmd.strval,"unlocked") == 0)
      {
         sprintf(val, "%d", 2);
      }
      else if(strcmp(value->cmd.strval,"shuttingdown") == 0)
      {
         sprintf(val, "%d", 3);
      }
      else
      {
         m_RETURN_AVSV_CLI_DONE("\nFAILURE: Invalid state", NCSCC_RC_FAILURE, cli_hdl);
         return NCSCC_RC_FAILURE;
      }
 
      if(strncmp(index->cmd.strval,"safSg=", 6) == 0)
      {
         table_id = NCSMIB_TBL_AVSV_AMF_SG;
         param_id = 10;
      }
      else if((strncmp(index->cmd.strval,"safSu=", 6) == 0) ||
              (strncmp(index->cmd.strval,"safEsu=", 7) == 0))
      {
         table_id = NCSMIB_TBL_AVSV_AMF_SU;
         param_id = 6;
      }
      else if(strncmp(index->cmd.strval,"safSi=", 6) == 0)
      {
         table_id = NCSMIB_TBL_AVSV_AMF_SI;
         param_id = 5;
      }
      else if(strncmp(index->cmd.strval,"safNode=", 8) == 0)
      {  
         table_id = NCSMIB_TBL_AVSV_AMF_NODE;
         param_id = 4;
      }
      else
      {
         m_RETURN_AVSV_CLI_DONE("\nFAILURE: Invalid Index", NCSCC_RC_FAILURE, cli_hdl);
         return NCSCC_RC_FAILURE;
      }
   }
   else if(strcmp(action->cmd.strval,"termstate") == 0)
   {
      if(strcmp(value->cmd.strval,"true") == 0)
      {
         sprintf(val, "%d", 1);
      }
      else if(strcmp(value->cmd.strval,"false") == 0)
      {
         sprintf(val, "%d", 2);
      }
      else
      {
         m_RETURN_AVSV_CLI_DONE("\nFAILURE: Invalid state", NCSCC_RC_FAILURE, cli_hdl);
         return NCSCC_RC_FAILURE;
      }

      if((strncmp(index->cmd.strval,"safSu=", 6) == 0) ||
        (strncmp(index->cmd.strval,"safEsu=", 7) == 0))
      {
         table_id = NCSMIB_TBL_AVSV_NCS_SU;
         param_id = 3;
      }
      else
      {
         m_RETURN_AVSV_CLI_DONE("\nFAILURE: Invalid Index", NCSCC_RC_FAILURE, cli_hdl);
         return NCSCC_RC_FAILURE;
      }
   }
   else
   {
      m_RETURN_AVSV_CLI_DONE("\nFAILURE: Invalid operation", NCSCC_RC_FAILURE, cli_hdl);
      return NCSCC_RC_FAILURE;
   }

   avsv_cli_build_mib_idx(&mib_idx, index->cmd.strval, NCSMIB_FMAT_OCT, cli_hdl);
 
   rc = avsv_cli_build_and_generate_mibsets( table_id, param_id, &mib_idx, val, NCSMIB_FMAT_INT, reqfnc, cli_hdl);

   m_MMGR_FREE_CLI_DEFAULT_VAL(mib_idx.i_inst_ids);

   if(NCSCC_RC_SUCCESS == rc)
   {
      m_RETURN_AVSV_CLI_DONE("\nSUCCESS: State Set Successfully", rc, cli_hdl);
   }
   else
   {
      m_RETURN_AVSV_CLI_DONE("\nFAILURE: State could not set", rc, cli_hdl);
   }

   return rc;
}

/*****************************************************************************
  PROCEDURE NAME: avm_parse_inp
  DESCRIPTION   : This function parses the input
  ARGUMENTS     :
                  arg : ARG list pointer
                  ent_inst : o/p argument
                  count    : o/p argument
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/

static uns32 
avm_parse_inp(NCSCLI_ARG_VAL *arg,
              uns32          *ent_inst,
              uns32          *count)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 loc;
   int8  *start = arg->cmd.strval;
   int8  *end;
   int8  *token_start;
   int8  *token_end;

   if(!arg->cmd.strval)
   {
      *count = 0; 
      return rc;
   }

   *count = -1;
   end    =  start + strlen(arg->cmd.strval) - 1; 
   for(;start && start <= end;)
   {
       loc = 0;
       token_start = start;
       token_end = strpbrk(start, "/"); 
       
       if(token_end == token_start)
       {
          start = token_end + 1;
          continue;
       }
       
       if(!token_end)
       {
          token_end = arg->cmd.strval + strlen(arg->cmd.strval) - 1;
       }else
       {
          token_end--;
       }
     
       for(; token_start <= token_end; token_start++)
       {
          if(((*token_start) >= '0') && ((*token_start) <= '9'))
          {
             loc = loc*10 + (*token_start - '0');
          }else
          {
             return NCSCC_RC_FAILURE;
          }
       }
       
       if(loc == 0)
       {
          return NCSCC_RC_FAILURE;
       }else
       {
          ent_inst[++*count] = loc;
       } 
       start = token_end + 2;
   }
   (*count)++;
   return rc;
}

/*****************************************************************************
  PROCEDURE NAME: avm_constr_inp
  DESCRIPTION   : This function frames the index of Deployment Table 
  ARGUMENTS     :
                  ent_type     : Base Address of array containing Entity Types
                  ent_type_cnt : Number of elements in the array ent_type
                  ent_inst     : Base Address of array containing Entity Instances
                  ent_inst_cnt : Number of elements in the array ent_inst
                  ep           : o/p argument
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
static uns32
avm_constr_ep(
                uns32    *ent_type,
                uns32     ent_type_cnt,
                uns32    *ent_inst,
                uns32     ent_inst_cnt,
                int8     *ep
             )
{
    uns32 rc = NCSCC_RC_SUCCESS;
    uns32 len = 0;
    uns32 ep_flag = 1;  /* 1 = lookup numerical string version for the entity path */
   
    if(ent_type_cnt == ent_inst_cnt)
    {
       if(AVM_DEFAULT_HIERARCHY_LVL == ent_type_cnt)
       {
          len = sprintf(ep, "{{%d,%d},{%d,%d},{%d,%d}}", ent_type[1], ent_inst[1], ent_type[0], ent_inst[0], SAHPI_ENT_ROOT, 0);
       }else
       {
          len = sprintf(ep, "{{%d,%d}{%d,%d},{%d,%d},{%d,%d}}", ent_type[2], ent_inst[2], ent_type[1], ent_inst[1], ent_type[0], ent_inst[0], SAHPI_ENT_ROOT, 0);
       }
    }else
    {
       if(AVM_DEFAULT_HIERARCHY_LVL == ent_inst_cnt)
       {
#ifdef HAVE_HPI_A01
       len = sprintf(ep, "{{%d,%d},{%d,%d},{%d,%d}}", SAHPI_ENT_SYSTEM_BOARD, ent_inst[1], SAHPI_ENT_SYSTEM_CHASSIS, ent_inst[0], SAHPI_ENT_ROOT, 0);
#else
       /* Try to find the correct entity path using the HISv lookup fn - if HISv is available */
       rc = hpl_entity_path_lookup(ep_flag, ent_inst[0], ent_inst[1], ep, EPATH_STRING_SIZE);
       if (rc == NCSCC_RC_SUCCESS) {
          if (strlen(ep) == 0) {
             printf("Error: hpl_entity_path_lookup() did not find the requested entity path\n");
             /* A zero length entity path means that HISv is working - but could not find the entity path */
             rc = NCSCC_RC_FAILURE;
          }
       }
       else {
          /* HISv is not running - so create the entity path using the original hardcoded values */
          len = sprintf(ep, "{{%d,%d},{%d,%d},{%d,%d}}", SAHPI_ENT_PHYSICAL_SLOT, ent_inst[1], SAHPI_ENT_SYSTEM_CHASSIS, ent_inst[0], SAHPI_ENT_ROOT, 0);
          rc = NCSCC_RC_SUCCESS;
       }
#endif
        }
    }
    return rc;
} 

/*****************************************************************************
  PROCEDURE NAME: avm_cef_set_ent_adm_req
  DESCRIPTION   : CEF for setting parameter values in Deployment Table 
  ARGUMENTS     :
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer

  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/

static uns32
avm_cef_set_ent_adm_req(
                          NCSCLI_ARG_SET  *arg_list, 
                          NCSCLI_CEF_DATA *cef_data
                       )
{
    NCSMIB_REQ_FNC     req_fnc = cef_data->i_bindery->i_req_fnc;

    NCSCLI_ARG_VAL    *cmd     = &arg_list->i_arg_record[0];
    NCSCLI_ARG_VAL    *value   = &arg_list->i_arg_record[3];

    uns32              entity_instance[SAHPI_MAX_ENTITY_PATH];
    uns32              entity_type[SAHPI_MAX_ENTITY_PATH];
    uns32              ent_inst_cnt = 0;
    uns32              ent_type_cnt = 0;
    int8               ep[EPATH_STRING_SIZE];
    NCSMIB_IDX         mib_idx;
    NCSMIB_TBL_ID      table_id;
    
    uns32              param_id;
    uns32              rc      = NCSCC_RC_SUCCESS;
    uns32              cli_hdl = cef_data->i_bindery->i_cli_hdl;
    int8               set_val[sizeof(uns32)];
    int8               ans;
    

    memset(entity_instance, '\0',sizeof(uns32) * SAHPI_MAX_ENTITY_PATH);
    memset(entity_type, '\0',sizeof(uns32) * SAHPI_MAX_ENTITY_PATH);
    memset(ep, '\0', sizeof(SaHpiEntityPathT));

    if (NCSCC_RC_SUCCESS != (avm_parse_inp(&arg_list->i_arg_record[1], entity_instance, &ent_inst_cnt)))
    {
       m_RETURN_AVSV_CLI_DONE("\n FAILURE: Invalid Index", NCSCC_RC_FAILURE,  cef_data->i_bindery->i_cli_hdl);
       return NCSCC_RC_FAILURE;
    }

    if (NCSCC_RC_SUCCESS != (avm_parse_inp(&arg_list->i_arg_record[4], entity_type, &ent_type_cnt)))
    {
       m_RETURN_AVSV_CLI_DONE("\n FAILURE: Invalid Index", NCSCC_RC_FAILURE,  cef_data->i_bindery->i_cli_hdl);
       return NCSCC_RC_FAILURE;
    }
   
    if(!(((AVM_DEFAULT_HIERARCHY_LVL == ent_inst_cnt) && ((!ent_type_cnt) || (ent_inst_cnt == ent_type_cnt))) || 
       ((AVM_EXT_HIERARCHY_LVL == ent_inst_cnt) && ((!ent_type_cnt)||(ent_inst_cnt == ent_type_cnt)))))
    {
       m_RETURN_AVSV_CLI_DONE("\n FAILURE: Invalid Index", NCSCC_RC_FAILURE, cef_data->i_bindery->i_cli_hdl);
       return NCSCC_RC_FAILURE;
    }

    /* If subslot has been specified as 0 then it's same as when it's not specified */
    if((AVM_EXT_HIERARCHY_LVL == ent_inst_cnt) && (entity_instance[2] == 0))
    {
       if(ent_type_cnt == ent_inst_cnt)
          ent_type_cnt = AVM_DEFAULT_HIERARCHY_LVL;
       ent_inst_cnt = AVM_DEFAULT_HIERARCHY_LVL;
    }

    if(NCSCC_RC_SUCCESS != (avm_constr_ep(entity_type, ent_type_cnt, entity_instance, ent_inst_cnt, ep)))
    {
       m_RETURN_AVSV_CLI_DONE("\n FAILURE: Invalid Index", NCSCC_RC_FAILURE, cef_data->i_bindery->i_cli_hdl);
       return NCSCC_RC_FAILURE;
    }
     
    table_id = NCSMIB_TBL_AVM_ENT_DEPLOYMENT;

    memset(set_val, 0, sizeof(uns32));
   
    if(!strcmp(cmd->cmd.strval, "reset"))
    {
        param_id = 10;
        if(!strcmp(value->cmd.strval, "softreset"))
        {
           sprintf(set_val, "%d", 1);
        }else if(!strcmp(value->cmd.strval, "hardreset"))
        {
           sprintf(set_val, "%d", 2);
        }else
        {
           m_RETURN_AVSV_CLI_DONE("\n Failure: Invalid State", NCSCC_RC_FAILURE, cli_hdl);
           return NCSCC_RC_FAILURE;
        }
    }else if(!strcmp(cmd->cmd.strval, "admreq"))
    {
        param_id = 11;
        if(!strcmp(value->cmd.strval, "shutdown"))
        {
           sprintf(set_val, "%d", 1);
        }else if(!strcmp(value->cmd.strval, "lock"))
        {
           avsv_cli_display(cli_hdl, "\nWARNING: Lock operation is an abrupt operation. It may result into, node not coming up even after performing unlock operation. The shutdown operation is rather a recommended choice, as it performs the same operation gracefully.\n");

           avsv_cli_display(cli_hdl, "Do you really want to continue with lock operation? - enter Y or y to confirm it");
           ans = getchar(); 
           if((ans=='Y') || (ans=='y'))
              sprintf(set_val, "%d", 2);
           else if(ans=='\n')
           {
              m_RETURN_AVSV_CLI_DONE("\nLock operation has been cancelled", NCSCC_RC_SUCCESS, cli_hdl);
              return NCSCC_RC_SUCCESS; 
           }
           else
           {
              FLUSHIN(ans);
              m_RETURN_AVSV_CLI_DONE("\nLock operation has been cancelled", NCSCC_RC_SUCCESS, cli_hdl);
              return NCSCC_RC_SUCCESS;
           }
           FLUSHIN(ans);
        }else if(!strcmp(value->cmd.strval, "unlock"))
        {
           sprintf(set_val, "%d", 3);
        }else
        {
           m_RETURN_AVSV_CLI_DONE("\n Failure: Invalid State", NCSCC_RC_FAILURE, cli_hdl);
           return NCSCC_RC_FAILURE; 
        }
    }else
    {
        m_RETURN_AVSV_CLI_DONE("\n Failure: Invalid Cmd", NCSCC_RC_FAILURE, cli_hdl);
        return NCSCC_RC_FAILURE; 
    }

    avsv_cli_build_mib_idx(&mib_idx, ep, NCSMIB_FMAT_OCT, cli_hdl);
    
    rc = avsv_cli_build_and_generate_mibsets(table_id, param_id, &mib_idx, set_val, NCSMIB_FMAT_INT, req_fnc, cli_hdl);
    
    m_MMGR_FREE_CLI_DEFAULT_VAL(mib_idx.i_inst_ids);
   
    if(NCSCC_RC_SUCCESS == rc)
    {
       m_RETURN_AVSV_CLI_DONE("\nSuccess: Admin State Set Successfully", rc, cli_hdl);
    }else
    {
       m_RETURN_AVSV_CLI_DONE("\nFailure: Admin State Set Failed", rc, cli_hdl);
    }

    return rc;
}


/*****************************************************************************
  PROCEDURE NAME: avm_cef_set_adm_switch
  DESCRIPTION   : CEF for setting parameter values in Scalar Table
  ARGUMENTS     :
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer

  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/

static uns32
avm_cef_set_adm_switch(
                         NCSCLI_ARG_SET  *arg_list, 
                         NCSCLI_CEF_DATA *cef_data
                      )
{
    NCSMIB_REQ_FNC     req_fnc = cef_data->i_bindery->i_req_fnc;

    NCSCLI_ARG_VAL    *cmd     = &arg_list->i_arg_record[0];

    NCSMIB_IDX         mib_idx;
    NCSMIB_TBL_ID      table_id;
    
    uns32              param_id;
    uns32              rc      = NCSCC_RC_SUCCESS;
    uns32              cli_hdl = cef_data->i_bindery->i_cli_hdl;
    int8               set_val[sizeof(uns32)];
    

    table_id = NCSMIB_TBL_AVM_SCALAR;

    memset(set_val, 0, sizeof(uns32));
   
    if(!strcmp(cmd->cmd.strval, "admswitch"))
    {
        param_id = 1;
        sprintf(set_val, "%d", 1);
    }else
    {
        m_RETURN_AVSV_CLI_DONE("\n Failure: Invalid Cmd", NCSCC_RC_FAILURE, cli_hdl);
        return NCSCC_RC_FAILURE; 
    }

    mib_idx.i_inst_len = 0;
    mib_idx.i_inst_ids = NULL;

    rc = avsv_cli_build_and_generate_mibsets(table_id, param_id, &mib_idx, set_val, NCSMIB_FMAT_INT, req_fnc, cli_hdl);
   
    if(NCSCC_RC_SUCCESS == rc)
    {
       m_RETURN_AVSV_CLI_DONE("\nSuccess: Admin Switchover Set Successfully", rc, cli_hdl);
    }else
    {
       m_RETURN_AVSV_CLI_DONE("\nFailure: Admin Switchover Set Failed", rc, cli_hdl);
    }

    return rc;
}

/*****************************************************************************
  PROCEDURE NAME: avsv_cli_done
  DESCRIPTION   : AVSv CLI done.
  ARGUMENTS     :
                  cli_hdl : CLI handle.
                  str     : string to be printed.
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
/*****************************************************************************
  PROCEDURE NAME: avsv_cli_done
  DESCRIPTION   : AVSv CLI done.
  ARGUMENTS     :
                  cli_hdl : CLI handle.
                  str     : string to be printed.
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32
avsv_cli_done(uns8 *string, uns32 status, uns32 cli_hdl)
{
   NCSCLI_OP_INFO req;
   uns32 rc = NCSCC_RC_SUCCESS;

   if (string)
   {
      avsv_cli_display(cli_hdl, (char *)string);
   }

   memset(&req, 0, sizeof(NCSCLI_OP_INFO));
   req.i_hdl = cli_hdl;
   req.i_req = NCSCLI_OPREQ_DONE;
   req.info.i_done.i_status = status;

   rc = ncscli_opr_request(&req);

   return rc;
}

/*****************************************************************************
  PROCEDURE NAME: avsv_cli_display
  DESCRIPTION   : Function for printing CLI string.
  ARGUMENTS     :
                  cli_hdl : CLI handle.
                  str     : string to be printed.
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32
avsv_cli_display(uns32 cli_hdl, char *str)
{
   NCSCLI_OP_INFO req;
   uns32 rc = NCSCC_RC_SUCCESS;

   memset(&req, 0, sizeof(NCSCLI_OP_INFO));
   req.i_hdl = cli_hdl;
   req.i_req = NCSCLI_OPREQ_DISPLAY;
   req.info.i_display.i_str = (uns8 *)str;

   rc = ncscli_opr_request(&req);

   return rc;
}


/*****************************************************************************
  PROCEDURE NAME: avsv_cli_build_mib_idx
  DESCRIPTION   : Create mib index.
  ARGUMENTS     :

  RETURNS       : SUCCESS or FAILURE
  NOTES: only octet format is supported.
*****************************************************************************/

uns32
avsv_cli_build_mib_idx(NCSMIB_IDX *mib_idx, char *idx_val, NCSMIB_FMAT_ID format, uns32 cli_hdl)
{
   uns32 *ptr = NULL;
   uns32 inst_len=0;
  
  /*mib_idx->i_inst_ids = (uns32 *)malloc(sizeof(idx_val) * sizeof(uns32));*/
  /* +1 for the prefix length in the instance. */
  mib_idx->i_inst_ids = (uns32 *)m_MMGR_ALLOC_CLI_DEFAULT_VAL((strlen(idx_val) + 1) * sizeof(uns32));
  if(mib_idx->i_inst_ids == NULL)
  {
     avsv_cli_display(cli_hdl, "\nAlloc failed");
     return 0;
  }
  ptr = (uns32 *)mib_idx->i_inst_ids;
  inst_len = strlen(idx_val);

  mib_idx->i_inst_len = inst_len+1;
  *ptr++ = inst_len;
  while(inst_len--)
    *ptr++ = (uns32)(*idx_val++);
  
  return 0;
}


/*****************************************************************************
  PROCEDURE NAME: avsv_cli_cfg_mib_arg
  DESCRIPTION   : Populats/Initializes the MIB arg structure.
  ARGUMENTS     :
                  mib      : Pointer to NCSMIB_ARG structure
                  index    : Index of the table
                  index_len: Index length
                  tid      : Table ID
                  usr_hdl  : MIB Key pointer
                  ss_hdl   : Subsystem key pointer
                  rspfnc   : Response function
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/


static uns32
avsv_cli_cfg_mib_arg(NCSMIB_ARG    *mib,
                       uns32        *index,
                       uns32         index_len,
                       NCSMIB_TBL_ID  tid,
                       uns32 usr_hdl,
                       uns32 ss_hdl,
                       NCSMIB_RSP_FNC rspfnc)
{

   ncsmib_init(mib);
  
 /*  memset(mib, 0, sizeof(NCSMIB_ARG)); */
 
   mib->i_idx.i_inst_ids  = index;
   mib->i_idx.i_inst_len  = index_len;
   mib->i_tbl_id          = tid;
   /* The response function, user handle/key and
    * service provider (mab?) key is not encoded at this point
    *
    * Add when necessary
    */

   mib->i_usr_key         = (uns64)usr_hdl;
   mib->i_mib_key         = (uns64)ss_hdl;
   mib->i_rsp_fnc         = rspfnc;

   
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  PROCEDURE NAME: avsv_cli_build_and_generate_mibsets 
  DESCRIPTION   : Send out a Set request not SETROW.
  ARGUMENTS     :

  RETURNS       : SUCCESS or FAILURE
  NOTES:          only int format is supported.
*****************************************************************************/


uns32
avsv_cli_build_and_generate_mibsets(NCSMIB_TBL_ID table_id, uns32 param_id,
                                NCSMIB_IDX *mib_idx, char *val, NCSMIB_FMAT_ID format, NCSMIB_REQ_FNC reqfnc, uns32 cli_hdl)
{
   NCSMIB_ARG  mib_arg;
   uns8        space[1024];
   NCSMEM_AID   mem_aid;
   uns32       status = NCSCC_RC_SUCCESS;

   if(val == NULL)
      return status;

   memset(&mib_arg, 0, sizeof(NCSMIB_ARG));
   memset(space, 0, sizeof(space));
    
   avsv_cli_cfg_mib_arg(&mib_arg, (uns32*)mib_idx->i_inst_ids, mib_idx->i_inst_len, table_id,0,0, NULL);
 
   ncsmem_aid_init(&mem_aid, space, 1024);

   NCSMIB_SET_REQ*       set = &mib_arg.req.info.set_req;
   mib_arg.i_op   = NCSMIB_OP_REQ_SET;
   set->i_param_val.i_fmat_id  = format;

   set->i_param_val.i_param_id = param_id;
   set->i_param_val.i_length = 4;
   set->i_param_val.info.i_int = atoi(val);
   
   /* Fill in the Key  */
   mib_arg.i_mib_key = (uns64)gl_mac_handle;
   mib_arg.i_usr_key = (uns64)gl_mac_handle;
  
   memset(space, 0, sizeof(space));

   /* call the MAB function prototype */
   ncsmib_pp(&mib_arg);

   status = ncsmib_sync_request(&mib_arg, reqfnc,
                                 NCS_CLI_MIB_REQ_TIMEOUT, &mem_aid);
   if (status != NCSCC_RC_SUCCESS)
   {
      /* Log the error */
   }
   else
   {
      /* Sent the request to MAB, and the response is in mib_arg
       *
       */
      if(mib_arg.rsp.add_info_len > 0)
      {
         avsv_cli_display(cli_hdl, mib_arg.rsp.add_info);
      }
      return mib_arg.rsp.i_status;
   }
   return status;
}

