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

/*****************************************************************************
..............................................................................
*/


#include <opensaf/ncs_cli.h>
#include <opensaf/mac_papi.h>

uns32 dummycli_cef_load_lib_req(NCS_LIB_REQ_INFO *libreq);
static uns32 dummy_cli_register(NCSCLI_BINDERY *);
static uns32 dummy_cef(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);

/****************************************************************************
  Name          : dummycli_cef_load_lib_req
 
  Description   : This function loads the CLI commands into OpenSAF CLI process
 
  Arguments     : NCS_LIB_REQ_INFO - Library request information     
  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
\*****************************************************************************/
uns32 dummycli_cef_load_lib_req(NCS_LIB_REQ_INFO *libreq)
{
    NCSCLI_BINDERY  i_bindery; 

    if (libreq == NULL)
        return NCSCC_RC_FAILURE; 

    switch (libreq->i_op)
    {
        case NCS_LIB_REQ_CREATE:
        m_NCS_OS_MEMSET(&i_bindery, 0, sizeof(NCSCLI_BINDERY));
        i_bindery.i_cli_hdl = gl_cli_hdl;
        i_bindery.i_mab_hdl = gl_mac_handle; 
        i_bindery.i_req_fnc = ncsmac_mib_request; 
        return dummy_cli_register(&i_bindery);
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
/****************************************************************************
  Name          : dummy_cli_register
 
  Description   : This function registers CLI commands under specified mode
 
  Arguments     : pBindery - CLI Bindery Information pointer 
  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
\*****************************************************************************/
static uns32 dummy_cli_register(NCSCLI_BINDERY *pBindery)
{
   uns32             rc = NCSCC_RC_SUCCESS, idx=0;
   NCSCLI_OP_INFO    info;
   NCSCLI_CMD_LIST   data;

   /* Popilate the commnds along wither CEF */ 
   NCSCLI_CMD_DESC   intf_mode_cmds[] = {
      {
         dummy_cef,
         "shutdown!Shutdown the Interface!"
      },
      {
         dummy_cef,
         "no !Negate a command or set defaults! shutdown!Shutdown the Interface!"
      },
      {
         dummy_cef,
         "set!Set function! logging!Logging! level!Level for logging! \
         {global !Global Logging!} | {node !Per Node Logging! NCSCLI_NUMBER<1..4294967295>!Node Number!}",
         NCSCLI_ADMIN_ACCESS
      },
      {
         dummy_cef,
         "set!Set function! logging!Logging! device!Device for Logging! \
         {global !Global Logging!} | {node !Per Node Logging!NCSCLI_NUMBER<1..4294967295>!Node Number!} | \
         {service !Service Logging! NCSCLI_NUMBER <1..4294967295> !Node ID Number! \
         NCSCLI_NUMBER <1..4294967295> !Service ID Number!}  NCSCLI_STRING!file/buffer/console!",
         NCSCLI_SUPERUSER_ACCESS
      },
      {
         dummy_cef,
         "set!Set function! default!Pre-set defaults! policy!Policy to be set! \
         {node !Per Node Logging!NCSCLI_NUMBER<1..4294967295>!Node Number!} | \
         {service !Service Logging! NCSCLI_NUMBER <1..4294967295> !Node ID Number! \
         NCSCLI_NUMBER <1..255> !Service ID Number!}",
         3
      },
      {
         dummy_cef,
         "configure!Change the configuration! logging!Logging! device!Device to be configured! \
         NCSCLI_NUMBER <1..4294967295>|all !Node ID Number! NCSCLI_NUMBER <1..4294967295>|all !Service ID Number! \
         {file !Output Device to config File! NCSCLI_NUMBER<10..1000000>!File size in KB!} | \
         {buffer !Output device to config Circular Buffer! NCSCLI_NUMBER<10..100000>!Circular Buffer size in KB!} \
         [NCSCLI_STRING! compressed/expanded!]",
         NCSCLI_SUPERUSER_ACCESS
      },
      {
         dummy_cef,
         "configure!Change the configuration! category!Message logging Category! filter!Filter to be set! \
          {NCSCLI_NUMBER <1..4294967295>|all !Node ID Number!} \
          {NCSCLI_NUMBER <1..4294967295>|all !Service ID Number!} \
          {NCSCLI_NUMBER <0..4294967295> !Category Filter Bit Map!}",
          NCSCLI_ADMIN_ACCESS
      },
      {
         dummy_cef,
         "configure!Change the configuration! severity!Message logging Severity! filter!Filter to be set! \
         {NCSCLI_NUMBER <1..4294967295>|all !Node ID Number!} \
         {NCSCLI_NUMBER <1..4294967295>|all !Service ID Number!} \
         {NCSCLI_NUMBER <0..255> !Severity Filter Bit Map!}",
         NCSCLI_VIEWER_ACCESS
      },
      {
         dummy_cef,
         "configure!Change the configuration! number!Number to be set! of!Of! log!Log! files!Files!  \
         NCSCLI_NUMBER <1..255> !Number to be set!"
      }
   };

   info.i_hdl = pBindery->i_cli_hdl;
   info.i_req = NCSCLI_OPREQ_REGISTER;
   info.info.i_register.i_bindery = pBindery;
   info.info.i_register.i_cmdlist = &data;

   /**************************************************************************\
                        Register Config mode commands 
   \*************************************************************************/
   m_NCS_OS_MEMSET(&data, 0, sizeof(NCSCLI_CMD_LIST));
   data.i_node = "root/exec/config";  /* Specify thw absoulte path of the Mode under which 
                                         the commands should get registered */
   data.i_command_mode = "config";    /* Specify the Mode name */
   data.i_access_req = FALSE;         /* Enable the access protection for entering into this 
                                         mode if required */ 
   data.i_access_passwd = 0;            
   data.i_cmd_count = 1;              /* Number of commands registered under this mode */  
   data.i_command_list[0].i_cmdstr = "dummy interface {ethernet@root/exec/config/interface@!ethernet interface!HJCLI_STRING!shelf/slot/port!}";
   data.i_command_list[0].i_cmd_exec_func = dummy_cef; /* Command Execution function */ 
   
   /* Register the command by Invoking the CLI API */
   rc = ncscli_opr_request(&info);  
   if(NCSCC_RC_SUCCESS != rc) return rc;
   printf("\n Registered CLI Commands under CONFIG Mode ...\n");

   /**************************************************************************\
                        Register Interface mode commands 
   \*************************************************************************/
   m_NCS_OS_MEMSET(&data, 0, sizeof(NCSCLI_CMD_LIST));
   data.i_node = "root/exec/config/interface"; 
   data.i_command_mode = "interface";
   data.i_access_req = FALSE;
   data.i_access_passwd = 0;
   data.i_cmd_count = sizeof(intf_mode_cmds) / sizeof(intf_mode_cmds[0]);
   
   for (idx=0; idx<data.i_cmd_count; idx++) {
      data.i_command_list[idx] = intf_mode_cmds[idx];
   }   
   rc = ncscli_opr_request(&info);
   if(NCSCC_RC_SUCCESS != rc) return rc;

   printf("\n Registered CLI Commands under INTERFACE Mode ...\n");
   return rc;
}

/****************************************************************************
  Name          : dummy_cef
 
  Description   : This function is dummy CEF for the registered commands 
 
  Arguments     : arg_list - arg list
                  cef_data - data
  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
\*****************************************************************************/
static uns32 dummy_cef(NCSCLI_ARG_SET *arg, NCSCLI_CEF_DATA *data)
{
   NCSCLI_OP_INFO info;

   printf("\n ... CLI CEF Invoked ...\n");
  
   /* Do the MIB SET/GET request for setting the corresponding MIB variable 
    * for the commnad that got excetued. Make the syncronous MIB request.
   */

   /* One done unlock the CLI for next command. In case the CEF return failure 
    * then unlock is not needsed. Only in case of success CLI done has to be 
    * invoked for unblocking the CLI 
   */ 
   info.i_hdl = data->i_bindery->i_cli_hdl;
   info.i_req = NCSCLI_OPREQ_DONE;
   info.info.i_done.i_status = NCSCC_RC_SUCCESS;    
   
   ncscli_opr_request(&info);   
   printf("\n ... CLI CEF Completed...\n");
   return NCSCC_RC_SUCCESS;
}
