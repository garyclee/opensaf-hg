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

MODULE NAME:  PSRCLI.C
..............................................................................

  DESCRIPTION:

  Source file for IFSV CLI registration function and IFSV CEFs

******************************************************************************
*/

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        Common Include Files.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


#if 1

#include "mab.h"
#include "cli_mem.h"
#include "ncs_cli.h"
#include "ifd.h"

#include "ifd_ir.h"
#include "ifsv_ir_mapi.h"


#if(NCS_VIP == 1)
#include "mab.h"
#include "cli_mem.h"
#include "ncs_cli.h"
#include "vip_cli.h"
#include "vip_mib.h"
#include "vip_mapi.h"
#include "ifsv_log.h"

#define  VIPCLI_INFORMATION_LEGEND                                  \
"\nVIP Configuration details\n"                                        \
"RANGE : 1 - , Only One IP is Configured in that interface \n"          \
"        O/W - The no of IP Iddresses are mentioned \n"                  \
"IP Address Type : I - Internal IP, E - External IP,\n"          \
"Installed Interface      \n"\/*TBD*/

#define  END_OF_VIPD_DATABASE 100

#define FOREVER 1

#endif

#include "ifsv_cli.h"

typedef struct ipxs_data_display_tag
{
    uns32 if_index;
    uns32 ipaddress;
    uns32 mask;
}IPXS_DATA_DISPLAY;

/*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~*            *~ Forward Declaration of Static Functions ~*                 *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
#if(NCS_VIP == 1)
static uns32 vip_show_all_CEF(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
static uns32 vip_change_mode_vip_CEF(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
static uns32 vip_show_single_entry_CEF(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
static uns32 vip_process_get_row_request( NCSCLI_CEF_DATA *p_cef_data,
                                            uns32 *idxs,
                                            uns32 idx_len,
                                            NCS_BOOL single_show);

static uns32 vip_ncs_cli_display(uns32 , uns8* );
static uns32 vip_single_entry_check(uns32 , uns32 *, uns8 *, uns32);
static uns32 vip_populate_display_data(NCSMIB_ARG *, VIP_DATA_DISPLAY *);
static void vip_cli_display_all(NCSCLI_CEF_DATA *, VIP_DATA_DISPLAY );
static uns32 vip_ncs_cli_done(uns32 , uns32 );
#endif

uns32
ipxs_cef_show_ip_info(NCSCLI_ARG_SET *arg_list,NCSCLI_CEF_DATA *cef_data);

static uns32
ipxs_process_get_row_request(NCSCLI_CEF_DATA *p_cef_data, uns32 start, uns32 end);

static uns32
ipxs_populate_display_data(NCSMIB_ARG *p_ncs_mib_arg,
                            IPXS_DATA_DISPLAY *p_ipxs_data_display);

static void
ipxs_cli_display_all(NCSCLI_CEF_DATA *p_cef_data,
                       IPXS_DATA_DISPLAY ipxs_data_display);



static uns32 ifsv_cef_change_mode(NCSCLI_ARG_SET *arg_list,NCSCLI_CEF_DATA *cef_data);
static uns32 ifsv_cli_done(uns32 cli_hdl, uns32 rc);
static uns32 ifsv_cli_display(uns32 cli_hdl, uns8* str);
static uns32 ifsv_cli_modedata_get(uns32 cli_hdl, NCS_IFSV_CLI_MODE_DATA **mode_data);

/****************************************************************************\
*  Name:          ncsifsv_cef_load_lib_req                                   * 
*                                                                            *
*  Description:   To load the CEFs into NCS CLI process                      *
*                                                                            *
*  Arguments:     NCS_LIB_REQ_INFO - Information about the SE API operation  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
*  NOTE:                                                                     * 
\****************************************************************************/
uns32 ncsifsv_cef_load_lib_req(NCS_LIB_REQ_INFO *libreq)
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
        return ncsifsv_cli_register(&i_bindery);
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
  
  PROCEDURE NAME:    ncsifsv_cli_register()

  DESCRIPTION:       This function can be invoked by the AKE to register the 
                     IFSV commands and associated CEFs with CLI.

  ARGUMENTS:
   NCSCLI_BINDERY *   Pointer to NCSCLI_i_bindery Structure
   
  RETURNS:           Success/Failure
   none

  NOTES:        

*****************************************************************************/

uns32 ncsifsv_cli_register(NCSCLI_BINDERY  *i_bindery)
{
   NCSCLI_CMD_LIST        data;
   uns32                  cmd_count = 0;
   NCSCLI_OP_INFO         info;
   
   /************************************************************************
   Netplane Command Language
   
     
       Netplane has defined its own Command Language (NCL)and all the commands
       that are to be registed by the subsystem has to be defined it that format.
       The details of the Language is jotted in Command Language Doc.
       
         Jist of NCL.
         1.  Each commands are to specified as a string ie with " and " quotes.
         2.  Keywords in a command are to be lower case.
         3.  Number are to specified as NCSCLI_NUMBER.
         4.  IP-Address are to specified as NCSCLI_IPADDRESS.
         5   String are to specified as NCSCLI_STRING etc.
         6.  Any thing in [] brace are optional parameters.
         7.  Any thing in {} brace are grouped together.
         8.  Characters declared in ! and ! are treated as help string associated
         with that parameter.
         9.  Any thing declared in % and % are treated as default value associated
         with that parameter.
         
   ************************************************************************/
   
   /* For each command mode register with CLI.
   * IFSV commands belongs to the following command modes 
   * 1. root/exec/config
   * 2. root/exec/config/interfaces
   */
   
   /************** REGISTER COMMANDS IN THE "CONFIG" MODE **************/
   memset(&data, 0, sizeof(NCSCLI_CMD_LIST));
   memset(&info, 0, sizeof(info));

   data.i_node = "root/exec/config";
   data.i_command_mode = "config";
   data.i_access_req = FALSE;        /* Password protected = FALSE */
   data.i_access_passwd = 0;         /* Password function, required 
                                     if password protected is set */

   cmd_count = 0;
   data.i_command_list[cmd_count].i_cmdstr = 
      "interfaces@root/exec/config/interfaces@!Switch to interface configuration!";
   data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
   data.i_command_list[cmd_count++].i_cmd_exec_func = ifsv_cef_change_mode;

   data.i_cmd_count = cmd_count;   /* Number of commands registered. */
   info.i_hdl = i_bindery->i_cli_hdl;
   info.i_req = NCSCLI_OPREQ_REGISTER;
   info.info.i_register.i_bindery = i_bindery;
   info.info.i_register.i_cmdlist = &data;
    
   if(NCSCC_RC_SUCCESS != ncscli_opr_request(&info)) 
      return NCSCC_RC_FAILURE;    


   /**************** Interface mode is set ******************/

   memset(&data, 0, sizeof(NCSCLI_CMD_LIST));
   data.i_node = "root/exec/config/interfaces";  /* Set path for the commands*/
   data.i_command_mode = "interfaces";   /* Set the mode */
   data.i_access_req = FALSE;        /* Password protected = FALSE */
   data.i_access_passwd = 0;         /* Password function, required 
                                   if password protected is set */

   cmd_count = 0;
                                  
   data.i_command_list[cmd_count].i_cmdstr = "bind@root/exec/config/interfaces/bind@!binding interface mode![bind-num <1..30> !binding interface index !NCSCLI_NUMBER]";
   data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
   data.i_command_list[cmd_count++].i_cmd_exec_func = ifsv_cef_conf_bind_intf;

   /* embedding subslot changes */
   data.i_command_list[cmd_count].i_cmdstr = "ethernet@root/exec/config/interfaces/ethernet@!port specific config mode!HJCLI_STRING !shelf/slot/subslot/port!";
   data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
   data.i_command_list[cmd_count++].i_cmd_exec_func = ifsv_cef_chng_interface_cmd;

   data.i_command_list[cmd_count].i_cmdstr = "configure@root/exec/config/interfaces/configure@!interface config!";
   data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
   data.i_command_list[cmd_count++].i_cmd_exec_func = ifsv_cef_change_mode;

   data.i_command_list[cmd_count].i_cmdstr = "show!show ifsv interfaces info! [shelf !Shelf num 1 to 16! NCSCLI_NUMBER<0..16> slot !Slot num 1 to 16! NCSCLI_NUMBER<0..16> subslot !Subslot num 1 to 16! NCSCLI_NUMBER<0..16>]";

   data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
   data.i_command_list[cmd_count++].i_cmd_exec_func = ifsv_cef_show_intf_info;

   data.i_cmd_count = cmd_count;   /* Number of commands registered. */
   info.info.i_register.i_cmdlist = &data;
    
   if(NCSCC_RC_SUCCESS != ncscli_opr_request(&info)) 
      return NCSCC_RC_FAILURE;    


   /* IP SPECIFIC CONFIGURATION */

   memset(&data, 0, sizeof(NCSCLI_CMD_LIST));
   data.i_node = "root/exec/config/interfaces/configure";
   data.i_command_mode = "configure";
   data.i_access_req = FALSE;
   data.i_access_passwd = 0;
   data.i_cmd_count = 2;
   data.i_command_list[0].i_cmdstr = "vip@root/exec/config/interfaces/configure/vip@!virtual ip mode!";
   data.i_command_list[0].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
   data.i_command_list[0].i_cmd_exec_func = ifsv_cef_change_mode;

   data.i_command_list[1].i_cmdstr = "ip@root/exec/config/interfaces/configure/ip@!ip mode!";
   data.i_command_list[1].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
   data.i_command_list[1].i_cmd_exec_func = ifsv_cef_change_mode;

   info.info.i_register.i_cmdlist = &data;
   if(NCSCC_RC_SUCCESS != ncscli_opr_request(&info)) 
             return NCSCC_RC_FAILURE;


#if(NCS_VIP == 1)


   /**************************************************
       ***      VIP  MODE  REGISTRATION       ***
       ***  VIP is layered as below:: 

   |- interfaces
        |-configure 
             | ---> ip
             |---->vip
                    |------>vip-dump
                    |---->appl-config
                                |------->show                ***
   **************************************************/


   data.i_node = "root/exec/config/interfaces/configure/vip";
   data.i_command_mode = "vip";
   data.i_access_req = FALSE;
   data.i_access_passwd = 0;
   data.i_cmd_count = 1;
   data.i_command_list[0].i_cmdstr = "vip-dump!dump vip database!";
   data.i_command_list[0].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
   data.i_command_list[0].i_cmd_exec_func = vip_show_all_CEF;

   info.info.i_register.i_cmdlist = &data;
   if(NCSCC_RC_SUCCESS != ncscli_opr_request(&info)) 
                  return NCSCC_RC_FAILURE;

   memset(&data, 0, sizeof(NCSCLI_CMD_LIST));
   data.i_node = "root/exec/config/interfaces/configure/vip";
   data.i_command_mode = "vip";
   data.i_access_req = FALSE;
   data.i_access_passwd = 0;
   data.i_cmd_count = 1;
   data.i_command_list[0].i_cmdstr = "appl-config !application specific config ! app_name !Character String(Maxlen 128)!  NCSCLI_STRING handle !Integer (1...2^32)! NCSCLI_NUMBER <1..4294967295>@root/exec/config/interfaces/configure/vip/appl-config@";
   data.i_command_list[0].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
   data.i_command_list[0].i_cmd_exec_func = vip_change_mode_vip_CEF;

   info.info.i_register.i_cmdlist = &data;
   if(NCSCC_RC_SUCCESS != ncscli_opr_request(&info)) 
              return NCSCC_RC_FAILURE;

   data.i_node = "root/exec/config/interfaces/configure/vip/appl-config";
   data.i_command_mode = "appl-config";
   data.i_access_req = FALSE;
   data.i_access_passwd = 0;
   data.i_cmd_count = 1;
   data.i_command_list[0].i_cmdstr = "show!Displays Entry Details!";
   data.i_command_list[0].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
   data.i_command_list[0].i_cmd_exec_func = vip_show_single_entry_CEF;

   info.info.i_register.i_cmdlist = &data;
   if(NCSCC_RC_SUCCESS != ncscli_opr_request(&info)) 
          return NCSCC_RC_FAILURE;

#endif/* VIP FLAG ENDS HERE*/
                                  
#if(NCS_IFSV_BOND_INTF == 1)

   /************** REGISTER Commands in "Binding" MODE **************/
   memset(&data, 0, sizeof(NCSCLI_CMD_LIST));
   data.i_node = "root/exec/config/interfaces/bind";  /* Set path for the commands*/
   data.i_command_mode = "bind";   /* Set the mode */
   data.i_access_req = FALSE;        /* Password protected = FALSE */
   
   data.i_access_passwd = 0;         /* Password function, required 
                                        if password protected is set */
   cmd_count = 0;

   /* create [bind <bind-num>] master <ifindex> [slave <ifindex>] */
   data.i_command_list[cmd_count].i_cmdstr = "create!creates a binding interface![bind-num <1..30>!bind-number ! NCSCLI_NUMBER] master!master interface IfIndex!NCSCLI_NUMBER [slave !slave interface index!NCSCLI_NUMBER]";
   data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_ADMIN_ACCESS;
   data.i_command_list[cmd_count++].i_cmd_exec_func = ifsv_cef_conf_create_bind_intf;
   
    /* delete bind <bind-num> */
   data.i_command_list[cmd_count].i_cmdstr = "delete!deletes a binding interface!bind-num <1..30>!bind-number! NCSCLI_NUMBER"; 
   data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_ADMIN_ACCESS;
   data.i_command_list[cmd_count++].i_cmd_exec_func = ifsv_cef_conf_delete_bind_intf;

   /* show */
   data.i_command_list[cmd_count].i_cmdstr = "show!shows the binding interfaces information!"; 
   data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
   data.i_command_list[cmd_count++].i_cmd_exec_func = ifsv_cef_show_bind_intf;
                                                                           
  data.i_cmd_count = cmd_count;   /* Number of commands registered. */
  
  info.info.i_register.i_cmdlist = &data;

  if(NCSCC_RC_SUCCESS != ncscli_opr_request(&info)) 
     return NCSCC_RC_FAILURE;   
#endif
/* IP related commands */
  data.i_node = "root/exec/config/interfaces/configure/ip/";
  data.i_command_mode = "ip";
  data.i_access_req = FALSE;
  data.i_access_passwd = 0;
  data.i_cmd_count = 3;
  data.i_command_list[0].i_cmdstr = "ip !Interface IP Config Commands!\
       address!Configure IPv4 address of an interface!\
       NCSCLI_IPv4 !IPv4-address!\
       mask !Network Mask!\
       NCSCLI_NUMBER <1..32> !Network-Mask-length !\
       interface-index!Interface index of the interface! NCSCLI_NUMBER <1..65535> !OS Index of the interface!";
   data.i_command_list[0].i_cmd_access_level = NCSCLI_ADMIN_ACCESS;
  data.i_command_list[0].i_cmd_exec_func = intf_cef_config_ipv4addr;

  data.i_command_list[1].i_cmdstr = "no ip !Interface IP Config Commands!\
       address!Configure IPv4 address of an interface!\
       NCSCLI_IPv4 !IPv4-address!\
       mask !Network Mask!\
       NCSCLI_NUMBER <1..32> !Network-Mask-length !\
       interface-index!Interface index of the interface! NCSCLI_NUMBER <1..65535> !OS Index of the interface!";
   data.i_command_list[1].i_cmd_access_level = NCSCLI_ADMIN_ACCESS;
  data.i_command_list[1].i_cmd_exec_func = intf_cef_config_ipv4addr;

  data.i_command_list[2].i_cmdstr = "show!show ipaddresses if-index info! [HJCLI_STRING !ifindex range start-end!]";
   data.i_command_list[2].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
  data.i_command_list[2].i_cmd_exec_func = ipxs_cef_show_ip_info;

  info.info.i_register.i_cmdlist = &data;

  if(NCSCC_RC_SUCCESS != ncscli_opr_request(&info)) return NCSCC_RC_FAILURE;

  return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 *
 * Function Name: intf_phase_token
 *
 * Purpose: This is the API to the parse the token.
 *    
 * Arguments: *s:  the string to be parsed
 *            *delim: delimitter to be parsed. 
 *
 * Return Value: HJCC_RC_SUCCESS/error code
 *
 ****************************************************************************/
static int8 * 
intf_phase_token( int8 *s, const int8 *delim)
{
    static int8 *olds = 0;
    int8 *token;

    if (s == 0)
    {
        if (olds == 0)
        {
            return 0;
        } else
            s = olds;
    }

    /* Scan leading delimiters.  */
    s += strspn((char *)s, (const char *)delim);
    if (*s == '\0')
    {
        olds = 0;
        return 0;
    }

    /* Find the end of the token.  */
    token = s;
    s = strpbrk((char *)token, (const char *)delim);
    if (s == 0)
    {
        /* This token finishes the string.  */
        olds = 0;
    }
    else
    {
        /* Terminate the token and make OLDS point past it.  */
        *s = '\0';
        olds = s + 1;
    }
    return token;
}

/****************************************************************************
 *
 * Function Name: intf_extract_slot_port
 *
 * Purpose: extract the solt and port with respect to the port type.
 *    
 * Arguments: *arg:  slot/port string.
 *            portType: port type. 
 *            *pSlotVal: slot value to be output.
 *            *pPortVal: port value to be output.
 *
 * Return Value: NCSCC_RC_SUCCESS/error code
 *
 ****************************************************************************/
static uns32
ifsv_cli_extract_shelf_slot_port (NCSCLI_ARG_VAL *arg, NCS_IFSV_SPT *spt)
{
   int8     *token;
   int8     *token1;
   switch (spt->type)
   {
   case NCS_IFSV_INTF_ETHERNET:
      token     = intf_phase_token( arg->cmd.strval, "/" );
      spt->shelf = atoi(token);
      
      token1    = intf_phase_token( NULL, "/" );
      if (token1 == NULL)
      {
         return (NCSCC_RC_FAILURE);
      }

      spt->slot = atoi(token1);
     
      /* embedding subslot changes */
      token1    = intf_phase_token( NULL, "/" );
      if (token1 == NULL)
      {
         return (NCSCC_RC_FAILURE);
      }

      spt->subslot = atoi(token1);
 
      token1    = intf_phase_token( NULL, "/" );
      if (token1 == NULL)
      {
         return (NCSCC_RC_FAILURE);
      }
      spt->port = atoi(token1);
      break;
       
   default :
      return (NCSCC_RC_FAILURE);
      break;
   }
   return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 *
 * Function Name: intf_str_comp
 *
 * Purpose: Convert the CLI inputed string in to lower case and comapring the 
 *          comparing the string..
 *    
 * Arguments: pStr1:  string to be converted to lower case and to compare.
 *            pStr2:  string to be comapred with.
 *
 * Return Value: zero for equal and nonzero for not equal.
 *
 ****************************************************************************/
uns32 
intf_str_comp (char *pStr1, char *pStr2)
{
   uns32 strLen;
   uns32 tempCounter;

    if (pStr1 == NULL)
    {
        return (1);
    }
   strLen = strlen(pStr1);
   
   for (tempCounter = 0; tempCounter < strLen; tempCounter++)
   {
      /* convert the string to lower case and compare the string*/
      pStr1[tempCounter] = (char)tolower(pStr1[tempCounter]);
   }
   return (strcmp(pStr1, pStr2));
}


/****************************************************************************
 *
 * Function Name: ifsv_cef_change_mode 
 *
 * Purpose: This API will be called by the CLI for "interface" command. 
 *          This API will store the ifndex in the mode information, which
 *          would be used by the command lying under this tree.
 *    
 * Arguments: *arg_list:  the command list.
 *            *cef_data: cef data information.
 *
 * Return Value: NCSCC_RC_SUCCESS/error code
 *
 ****************************************************************************/
uns32 
ifsv_cef_change_mode(NCSCLI_ARG_SET *arg_list,NCSCLI_CEF_DATA *cef_data)
{
   ifsv_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS);

   return (NCSCC_RC_SUCCESS);
}

uns32 
ifsv_cef_show_intf_info(NCSCLI_ARG_SET *arg_list,NCSCLI_CEF_DATA *cef_data)
{
  uns32 shelf = 0;
  uns32 slot = 0;
  uns32 subslot = 0;
  uns32 ret_code;
  /* embedding Subslot changes */
   if((arg_list->i_arg_record[1].cmd.strval!= NULL) &&
      (arg_list->i_arg_record[3].cmd.strval!= NULL) &&
      (arg_list->i_arg_record[5].cmd.strval!= NULL))
   {
       if((strcmp(arg_list->i_arg_record[1].cmd.strval,"shelf") == 0) &&
         (strcmp(arg_list->i_arg_record[3].cmd.strval,"slot") == 0) &&
         (strcmp(arg_list->i_arg_record[5].cmd.strval,"subslot") == 0))
       {
          shelf = (long)arg_list->i_arg_record[2].cmd.strval;
          slot = (long)arg_list->i_arg_record[4].cmd.strval;
          subslot = (long)arg_list->i_arg_record[6].cmd.strval;

          ret_code = ifsv_process_get_row_request(cef_data, shelf, slot, subslot);
          switch(ret_code)
          {
             case NCSCC_RC_NO_INSTANCE:
                   printf("\n :-:-:-:-:-:-:-:-: CouldNot fetch details of this Instance from IFSV Database :-:-:-:-:-:-:-:-:\n");
                   break;
            case NCSCC_RC_FAILURE:
                   printf("\n:-:-:-:-:-:-:-:-: FETCHING IFSV-RECORD DETAILS :-:-:-:-:-:-:-:-:\n");
                   break;
            default:
                   break;
          }
       }
   }
   else
   {
      ret_code = ifsv_process_get_row_request(cef_data, -1, -1, -1);
      switch(ret_code)
      {
         case NCSCC_RC_NO_INSTANCE:
                printf("\n :-:-:-:-:-:-:-:-: CouldNot fetch details of this Instance from IFSV Database :-:-:-:-:-:-:-:-:\n");
                break;
         case NCSCC_RC_FAILURE:
                printf("\n:-:-:-:-:-:-:-:-: Interface Service is Busy Adding/Deleting Records Try <show> Command after some time:-:-:-:-:-:-:-:-:\n");
                break;
         default:
                break;
      }
   }

   ifsv_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS);

   return (NCSCC_RC_SUCCESS);

}

/****************************************************************************
 *
 * Function Name: ifsv_cef_chng_interface_cmd
 *
 * Purpose: This API will be called by the CLI for "interface" command. 
 *          This API will store the ifndex in the mode information, which
 *          would be used by the command lying under this tree.
 *    
 * Arguments: *arg_list:  the command list.
 *            *cef_data: cef data information.
 *
 * Return Value: NCSCC_RC_SUCCESS/error code
 *
 ****************************************************************************/
uns32 
ifsv_cef_chng_interface_cmd(NCSCLI_ARG_SET *arg_list, 
                                       NCSCLI_CEF_DATA *cef_data)
{
   NCS_IFSV_CLI_MODE_DATA   *mode_data=NULL;
   uns32                portType;
   uns32                rc;
   NCSMIB_ARG           ncsmib_arg;
   uns8                 space[2048];
   NCSMEM_AID           ma;
   uns32                xch_id = 1;
   uns32                inst_ids[IFSV_IFMAPTBL_INST_LEN];
 

   mode_data = m_MMGR_ALLOC_CLI_DEFAULT_VAL(sizeof(NCS_IFSV_CLI_MODE_DATA));

   if(!mode_data)
   {
      ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"no memory\n");
      return NCSCC_RC_FAILURE;
   }
 
   m_IFSV_EXTRACT_PORT_TYPE(arg_list->i_arg_record[0].cmd.strval,portType);

   mode_data->spt.type = portType;

   rc = ifsv_cli_extract_shelf_slot_port(&arg_list->i_arg_record[1], 
                                                           &mode_data->spt);
   if(rc != NCSCC_RC_SUCCESS)
   {
      /* embedding subslot changes */
      ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"Incorrect shelf/slot/subslot/port format \n");
      m_MMGR_FREE_CLI_DEFAULT_VAL(mode_data);
      return rc;
   }

   inst_ids[0] = mode_data->spt.shelf;
   inst_ids[1] = mode_data->spt.slot;
   /* embedding subslot changes */
   inst_ids[2] = mode_data->spt.subslot;
   inst_ids[3] = mode_data->spt.port;
   inst_ids[4] = mode_data->spt.type;
   inst_ids[5] = NCS_IFSV_SUBSCR_EXT;

   /* Initialize the MIB arg structure */
   memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&ncsmib_arg);
   ncsmem_aid_init(&ma, space, sizeof(space));

   ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
   ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
   ncsmib_arg.i_op      = NCSMIB_OP_REQ_GET;
   ncsmib_arg.i_rsp_fnc = NULL;
   ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
   ncsmib_arg.i_idx.i_inst_len = IFSV_IFMAPTBL_INST_LEN;
   ncsmib_arg.i_xch_id  = xch_id++;
   ncsmib_arg.i_tbl_id  = NCSMIB_TBL_IFSV_IFMAPTBL;
   ncsmib_arg.req.info.get_req.i_param_id = ncsIfsvIfMapEntryIfIndex_ID;

   rc = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 1000, &ma);
   if((rc != NCSCC_RC_SUCCESS) || 
      (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS))
   {
      ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\nFailed \n");
      m_MMGR_FREE_CLI_DEFAULT_VAL(mode_data);
     if(rc != NCSCC_RC_SUCCESS)
        return rc;
     else
       return ncsmib_arg.rsp.i_status;
   }

   mode_data->ifindex = ncsmib_arg.rsp.info.get_rsp.i_param_val.info.i_int;

   cef_data->i_subsys->i_cef_mode = (void *)mode_data;

   ifsv_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS);

   return (NCSCC_RC_SUCCESS);
}



/*****************************************************************************
  PROCEDURE NAME:   ifsv_cef_conf_intf_shut

  DESCRIPTION: CEF function to shut-down the interface/tunnel.
    
  PARAMETERS:
    
  RETURNS:
    
*****************************************************************************/ 
uns32 ifsv_cef_conf_intf_shut (NCSCLI_ARG_SET *arg_list, 
                                       NCSCLI_CEF_DATA *cef_data)
{
   NCS_IFSV_CLI_MODE_DATA    *mode_data=NULL;
   NCSMIB_ARG           ncsmib_arg;
   uns8                 space[2048];
   NCSMEM_AID           ma;
   uns32                xch_id = 1;
   uns32                inst_ids[IFSV_IFINDEX_INST_LEN];
   uns32                pos=0;
   uns32                admin_status, rc;

   if(strcmp(arg_list->i_arg_record[pos].cmd.strval,"no") == 0)
   {
      admin_status = NCS_STATUS_UP;  /* Set to UP */
   }
   else
      admin_status = NCS_STATUS_DOWN; /* Set to Down */

   ifsv_cli_modedata_get(cef_data->i_bindery->i_cli_hdl, &mode_data);

   inst_ids[0] = mode_data->ifindex;;

   /* Initialize the MIB arg structure */
   memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&ncsmib_arg);
   ncsmem_aid_init(&ma, space, sizeof(space));

   ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
   ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
   ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
   ncsmib_arg.i_rsp_fnc = NULL;
   ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
   ncsmib_arg.i_idx.i_inst_len = IFSV_IFINDEX_INST_LEN;
   ncsmib_arg.i_xch_id  = xch_id++;
   ncsmib_arg.i_tbl_id  = NCSMIB_TBL_IFSV_IFTBL;
   ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ifAdminStatus_ID;
   ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   ncsmib_arg.req.info.set_req.i_param_val.i_length = sizeof(admin_status);
   ncsmib_arg.req.info.set_req.i_param_val.info.i_int = admin_status;

   rc = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 1000, &ma);
   if((rc != NCSCC_RC_SUCCESS) || 
      (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS))
   {
      ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n Command Failed \n");
     if(rc != NCSCC_RC_SUCCESS)
        return rc;
     else
       return ncsmib_arg.rsp.i_status;
   }
   
   ifsv_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS);
   return (NCSCC_RC_SUCCESS);
}


static uns32 ifsv_cli_done(uns32 cli_hdl, uns32 rc)
{
   NCSCLI_OP_INFO info;
   
   info.i_hdl = cli_hdl;
   info.i_req = NCSCLI_OPREQ_DONE;
   info.info.i_done.i_status = rc;    
   
   ncscli_opr_request(&info);   
   return NCSCC_RC_SUCCESS;

}

static uns32 ifsv_cli_display(uns32 cli_hdl, uns8* str)
{
   NCSCLI_OP_INFO info;
   
   info.i_hdl = cli_hdl;
   info.i_req = NCSCLI_OPREQ_DISPLAY;
   info.info.i_display.i_str = str;    
   
   ncscli_opr_request(&info);   
   return NCSCC_RC_SUCCESS;
}

static uns32 ifsv_cli_modedata_get(uns32 cli_hdl, NCS_IFSV_CLI_MODE_DATA **mode_data)
{
   NCSCLI_OP_INFO info;
   
   info.i_hdl = cli_hdl;
   info.i_req = NCSCLI_OPREQ_GET_MODE;
   info.info.i_mode.i_lvl  = 0;    
   
   ncscli_opr_request(&info); 
   
   *mode_data = info.info.i_mode.o_data;

   return NCSCC_RC_SUCCESS;
}

uns32 ncs_ifsv_cli_get_ifindex_from_spt(uns32            i_mib_key, 
                                       uns32             i_usr_key,  
                                       NCSMIB_REQ_FNC   reqfnc, 
                                       NCS_IFSV_SPT     *i_spt, 
                                       NCS_IFSV_IFINDEX *o_ifindex)
{
   uns32                rc;
   NCSMIB_ARG           ncsmib_arg;
   uns8                 space[2048];
   NCSMEM_AID           ma;
   uns32                xch_id = 1;
   uns32                inst_ids[IFSV_IFMAPTBL_INST_LEN];
 

   inst_ids[0] = i_spt->shelf;
   inst_ids[1] = i_spt->slot;
   /* embedding subslot changes */
   inst_ids[2] = i_spt->subslot;
   inst_ids[3] = i_spt->port;
   inst_ids[4] = i_spt->type;

   /* Initialize the MIB arg structure */
   memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&ncsmib_arg);
   ncsmem_aid_init(&ma, space, sizeof(space));

   ncsmib_arg.i_mib_key = i_mib_key;
   ncsmib_arg.i_usr_key = i_usr_key;
   ncsmib_arg.i_op      = NCSMIB_OP_REQ_GET;
   ncsmib_arg.i_rsp_fnc = NULL;
   ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
   ncsmib_arg.i_idx.i_inst_len = IFSV_IFMAPTBL_INST_LEN;
   ncsmib_arg.i_xch_id  = xch_id++;
   ncsmib_arg.i_tbl_id  = NCSMIB_TBL_IFSV_IFMAPTBL;
   ncsmib_arg.req.info.get_req.i_param_id = ncsIfsvIfMapEntryIfIndex_ID;

   rc = ncsmib_sync_request(&ncsmib_arg, reqfnc, 1000, &ma);
   if((rc != NCSCC_RC_SUCCESS) || 
      (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS))
   {
     if(rc != NCSCC_RC_SUCCESS)
        return rc;
     else
       return ncsmib_arg.rsp.i_status;
   }

   *o_ifindex = ncsmib_arg.rsp.info.get_rsp.i_param_val.info.i_int;
   return rc;
}

uns32 ncs_ifsv_cli_get_ippfx_from_spt(uns32            i_mib_key,
                                     uns32            i_usr_key,
                                     NCSMIB_REQ_FNC   reqfnc,
                                     NCS_IFSV_SPT     *i_spt, 
                                     NCS_IPPFX        **o_ippfx, 
                                     uns32            *pfx_cnt)
{
   uns32                rc;
   NCSMIB_ARG           ncsmib_arg;
   uns8                 space[2048];
   NCSMEM_AID           ma;
   uns32                xch_id = 1;
   uns32                inst_ids[IFSV_IFMAPTBL_INST_LEN];

   inst_ids[0] = i_spt->shelf;
   inst_ids[1] = i_spt->slot;
   /* embedding subslot changes */
   inst_ids[2] = i_spt->subslot;
   inst_ids[3] = i_spt->port;
   inst_ids[4] = i_spt->type;

   /* Initialize the MIB arg structure */
   memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&ncsmib_arg);
   ncsmem_aid_init(&ma, space, sizeof(space));

   ncsmib_arg.i_mib_key = i_mib_key;
   ncsmib_arg.i_usr_key = i_usr_key;
   ncsmib_arg.i_op      = NCSMIB_OP_REQ_GET;
   ncsmib_arg.i_rsp_fnc = NULL;
   ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
   ncsmib_arg.i_idx.i_inst_len = IFSV_IFMAPTBL_INST_LEN;
   ncsmib_arg.i_xch_id  = xch_id++;
   ncsmib_arg.i_tbl_id  = NCSMIB_TBL_IFSV_IFMAPTBL;
   ncsmib_arg.req.info.get_req.i_param_id = ncsIfsvIfMapEntryIfIndex_ID;

   rc = ncsmib_sync_request(&ncsmib_arg, reqfnc, 1000, &ma);
   if((rc != NCSCC_RC_SUCCESS) ||
      (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS))
   {
     if(rc != NCSCC_RC_SUCCESS)
        return rc;
     else
       return ncsmib_arg.rsp.i_status;
   }
   inst_ids[0] = (NCS_IFSV_IFINDEX) ncsmib_arg.rsp.info.get_rsp.i_param_val.info.i_int;
   
      /* Initialize the MIB arg structure */
   memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&ncsmib_arg);
   ncsmem_aid_init(&ma, space, sizeof(space));

   ncsmib_arg.i_mib_key = i_mib_key;
   ncsmib_arg.i_usr_key = i_usr_key;
   ncsmib_arg.i_op      = NCSMIB_OP_REQ_GET;
   ncsmib_arg.i_rsp_fnc = NULL;
   ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
   ncsmib_arg.i_idx.i_inst_len = IFSV_IFINDEX_INST_LEN;
   ncsmib_arg.i_xch_id  = xch_id++;
   ncsmib_arg.i_tbl_id  = NCSMIB_TBL_IPXS_IFIPTBL;
   ncsmib_arg.req.info.get_req.i_param_id = ncsIfsvIfIpAddr_ID;

   rc = ncsmib_sync_request(&ncsmib_arg, reqfnc, 1000, &ma);
   if((rc != NCSCC_RC_SUCCESS) ||
      (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS))
   {
     if(rc != NCSCC_RC_SUCCESS)
        return rc;
     else
       return ncsmib_arg.rsp.i_status;
   }

   /* Fill the ippfx from resp */
   if(ncsmib_arg.rsp.info.get_rsp.i_param_val.i_length)
   {
      uns32 count = 0, temp = 0;

      /* Right now we are getting only one (IPv4) address in this
         get */
      *o_ippfx = m_MMGR_ALLOC_NCSCLI_OPAQUE(sizeof(NCS_IPPFX));
      if(*o_ippfx == NULL)
         return NCSCC_RC_FAILURE;
      memset((*o_ippfx), 0, sizeof(NCS_IPPFX));
      (*o_ippfx)->ipaddr.type = NCS_IP_ADDR_TYPE_IPV4;

      for(count=0; count<4 ; count++)
      {
         temp = (ncsmib_arg.rsp.info.get_rsp.i_param_val.info.i_oct[count]) << ((4 - (count+1)) * 8);
         (*o_ippfx)->ipaddr.info.v4 = (*o_ippfx)->ipaddr.info.v4 | temp;
      }
      *pfx_cnt = 1;   
   }
   return rc;
}


/****************************************************************************
 *
 * Function Name: ifsv_cef_conf_bind_intf
 *
 * Purpose: This API will be called by the CLI for "interface" command. 
 *          This API will store the ifndex in the mode information, which
 *          would be used by the command lying under this tree.
 *    
 * Arguments: *arg_list:  the command list.
 *            *cef_data: cef data information.
 *
 * Return Value: NCSCC_RC_SUCCESS/error code
 *
 ****************************************************************************/
uns32 
ifsv_cef_conf_bind_intf(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
   NCS_IFSV_CLI_MODE_DATA   *mode_data=NULL;
   int32                bind_num = -1;
   uns32                rc;
   NCSMIB_ARG           ncsmib_arg;
   uns8                 space[2048];
   NCSMEM_AID           ma;
   uns32                xch_id = 1;
   uns32                inst_ids[1];
 
   mode_data = m_MMGR_ALLOC_CLI_DEFAULT_VAL(sizeof(NCS_IFSV_CLI_MODE_DATA));
   if(!mode_data)
   {
      ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"no memory\n");
      return NCSCC_RC_FAILURE;
   }
  
   if(arg_list->i_pos_value != 0)
   { 
   if(!strcmp(arg_list->i_arg_record[1].cmd.strval,"bind-num")) 
   {
      bind_num = arg_list->i_arg_record[2].cmd.intval;
    

   inst_ids[0] = bind_num;

   mode_data->spt.port  = bind_num;
   mode_data->spt.shelf = IFSV_BINDING_SHELF_ID;
   mode_data->spt.slot  = IFSV_BINDING_SLOT_ID;
   /* embedding subslot changes */
   mode_data->spt.subslot  = IFSV_BINDING_SUBSLOT_ID;
   mode_data->spt.type  = NCS_IFSV_INTF_BINDING;
   mode_data->spt.subscr_scope = NCS_IFSV_SUBSCR_INT;
   
   /* Initialize the MIB arg structure */
   memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&ncsmib_arg);
   ncsmem_aid_init(&ma, space, sizeof(space));

   ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
   ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
   ncsmib_arg.i_op      = NCSMIB_OP_REQ_GET;
   ncsmib_arg.i_rsp_fnc = NULL;
   ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
   ncsmib_arg.i_idx.i_inst_len = 1;
   ncsmib_arg.i_xch_id  = xch_id++;
   ncsmib_arg.i_tbl_id  = NCSMIB_TBL_IFSV_BIND_IFTBL;
   ncsmib_arg.req.info.get_req.i_param_id = ncsIfsvBindIfBindIfIndex_ID;

   rc = ncsmib_sync_request(&ncsmib_arg, ncsmac_mib_request, 1000, &ma);
   if((rc != NCSCC_RC_SUCCESS) || 
      (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS))
   {
      ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\nFailed \n");
      if(rc != NCSCC_RC_SUCCESS)
         return rc;
      else
        return ncsmib_arg.rsp.i_status;
   }

   mode_data->ifindex = ncsmib_arg.rsp.info.get_rsp.i_param_val.info.i_int;

      }
   }
   else {
       mode_data->ifindex = 0xff;
   }

   cef_data->i_subsys->i_cef_mode = (void *)mode_data;

   ifsv_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS);

   return (NCSCC_RC_SUCCESS);
}


/****************************************************************************
 *
 * Function Name: ifsv_cef_conf_create_bind_intf 
 *
 * Purpose: This API will be called by the CLI for "interface" command. 
 *          This API will store the ifndex in the mode information, which
 *          would be used by the command lying under this tree.
 *    
 * Arguments: *arg_list:  the command list.
 *            *cef_data: cef data information.
 *
 * Return Value: NCSCC_RC_SUCCESS/error code
 *
 ****************************************************************************/
uns32
ifsv_cef_conf_create_bind_intf(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
   NCS_IFSV_CLI_MODE_DATA    *mode_data=NULL;
   NCSMIB_ARG      ncsmib_arg;
   uns8                 space[2048];
   NCSMEM_AID      ma;
   uns32                xch_id = 1;
   uns32                inst_ids[IFSV_IFINDEX_INST_LEN];
   uns32                pos;
   uns32                rc;
   int32                 bind_num = -1,master_ifindex = -1,slave_ifindex = -1;
   NCSPARM_AID             rsp_pa;
   NCSMIB_PARAM_VAL        param_val;


   /* create [bind <bind-num>] master <ifindex> [slave <ifindex>] */
   pos = 1;
   if(strcmp(arg_list->i_arg_record[pos].cmd.strval,"bind-num") == 0)
   {
        pos++;
        bind_num = arg_list->i_arg_record[pos].cmd.intval;
        pos ++;
   }

   if(strcmp(arg_list->i_arg_record[pos].cmd.strval,"master") == 0)
   {
        pos++;
        master_ifindex = arg_list->i_arg_record[pos].cmd.intval;
        pos ++;
   }

   if((arg_list->i_pos_value & 0x60) == 0x60)
   {
      if(strcmp(arg_list->i_arg_record[pos].cmd.strval,"slave") == 0)
      {
           pos++;
           slave_ifindex = arg_list->i_arg_record[pos].cmd.intval;
           pos ++;
      }
   }

   ifsv_cli_modedata_get(cef_data->i_bindery->i_cli_hdl, &mode_data);

   /* If bind_num is not given, then use the bind_num of the mode.. */
   if(bind_num < 0) 
   {
          if(mode_data->ifindex == 0xff)
          {
            /* This means that neither the mode is associated to bind_num. not it is given in command */
              ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n Command Failed. Specify the bind number.  \n");
               return NCSCC_RC_FAILURE;
           }
      else {   
         inst_ids[0] = mode_data->spt.port;
      }
   }
   else
   {
      inst_ids[0] = bind_num;
   }     

   /* Initialize the MIB arg structure */
   memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&ncsmib_arg);
   ncsmem_aid_init(&ma, space, sizeof(space));

   ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
   ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
   ncsmib_arg.i_op      = NCSMIB_OP_REQ_SETROW;
   ncsmib_arg.i_rsp_fnc = NULL;
   ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
   ncsmib_arg.i_idx.i_inst_len = 1;
   ncsmib_arg.i_xch_id  = xch_id++;
   ncsmib_arg.i_tbl_id  = NCSMIB_TBL_IFSV_BIND_IFTBL;


   ncsparm_enc_init(&rsp_pa);
   memset(&param_val, 0 , sizeof(NCSMIB_PARAM_VAL));

   /* No need of Rowstatus. Set the master and slave ifindices */
   param_val.i_fmat_id = NCSMIB_FMAT_INT;
   param_val.i_param_id = ncsIfsvBindIfMasterIfIndex_ID;
   param_val.info.i_int = master_ifindex;
   param_val.i_length = 1;
   ncsparm_enc_param(&rsp_pa, &param_val);

   if(slave_ifindex >= 0) {     
      param_val.i_fmat_id = NCSMIB_FMAT_INT;
      param_val.i_param_id = ncsIfsvBindIfSlaveIfIndex_ID;
      param_val.info.i_int = slave_ifindex;
      param_val.i_length = 1;
      ncsparm_enc_param(&rsp_pa, &param_val);
   }

   ncsmib_arg.req.info.setrow_req.i_usrbuf = ncsparm_enc_done(&rsp_pa);

   rc = ncsmib_sync_request(&ncsmib_arg, ncsmac_mib_request, 1000, &ma);
   if((rc != NCSCC_RC_SUCCESS) || 
      (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS))
   {
      ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n Command Failed \n");
     if(rc != NCSCC_RC_SUCCESS)
        return rc;
     else
       return ncsmib_arg.rsp.i_status;
   }
   
   ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n Command Successful \n");
   ifsv_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS);
   return (NCSCC_RC_SUCCESS);

}


/****************************************************************************
 *
 * Function Name: ifsv_cef_conf_bind_intf
 *
 * Purpose: This API will be called by the CLI for "interface" command. 
 *          This API will store the ifndex in the mode information, which
 *          would be used by the command lying under this tree.
 *    
 * Arguments: *arg_list:  the command list.
 *            *cef_data: cef data information.
 *
 * Return Value: NCSCC_RC_SUCCESS/error code
 *
 ****************************************************************************/
uns32
ifsv_cef_conf_delete_bind_intf(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
   NCSMIB_ARG      ncsmib_arg;
   uns8                 space[2048];
   NCSMEM_AID      ma;
   uns32                xch_id = 1;
   uns32                inst_ids[IFSV_IFINDEX_INST_LEN];
   uns32                rc;
   int32                 bind_num = -1;


   /* delete bind <bind-num> */
   if(strcmp(arg_list->i_arg_record[1].cmd.strval,"bind-num") == 0)
   {
        bind_num = arg_list->i_arg_record[2].cmd.intval;
   }

   inst_ids[0] = bind_num;

   /* Initialize the MIB arg structure */
   memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&ncsmib_arg);
   ncsmem_aid_init(&ma, space, sizeof(space));

   ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
   ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
   ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
   ncsmib_arg.i_rsp_fnc = NULL;
   ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
   ncsmib_arg.i_idx.i_inst_len = 1;
   ncsmib_arg.i_xch_id  = xch_id++;
   ncsmib_arg.i_tbl_id  = NCSMIB_TBL_IFSV_BIND_IFTBL;
   ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsIfsvBindIfBindRowStatus_ID;
   ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
   ncsmib_arg.req.info.set_req.i_param_val.i_length = 1;
   ncsmib_arg.req.info.set_req.i_param_val.info.i_int = NCSMIB_ROWSTATUS_DESTROY;
   
   rc = ncsmib_sync_request(&ncsmib_arg, ncsmac_mib_request, 1000, &ma);
   if((rc != NCSCC_RC_SUCCESS) || 
      (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS))
   {
      ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n Command Failed \n");
     if(rc != NCSCC_RC_SUCCESS)
        return rc;
     else
       return ncsmib_arg.rsp.i_status;
   }
   
   ifsv_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS);
   return (NCSCC_RC_SUCCESS);

}

 /* show */   
/****************************************************************************
 *
 * Function Name: ifsv_cef_conf_bind_intf
 *
 * Purpose: This API will be called by the CLI for "interface" command. 
 *          This API will store the ifndex in the mode information, which
 *          would be used by the command lying under this tree.
 *    
 * Arguments: *arg_list:  the command list.
 *            *cef_data: cef data information.
 *
 * Return Value: NCSCC_RC_SUCCESS/error code
 *
 ****************************************************************************/
uns32
ifsv_cef_show_bind_intf(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
   NCS_IFSV_CLI_MODE_DATA    *mode_data=NULL;
   NCSMIB_ARG      ncsmib_arg;
   uns8                 space[2048];
   NCSMEM_AID      ma;
   uns32                xch_id = 1;
   uns32                inst_ids[IFSV_IFINDEX_INST_LEN];
   uns32                rc;
   int32                 bind_num = -1;
   NCS_BOOL          row_show_succ = FALSE;

   ifsv_cli_modedata_get(cef_data->i_bindery->i_cli_hdl, &mode_data);

   /* If bind_num is not given, then use the bind_num of the mode.. */
   bind_num = mode_data->spt.port;

   /* Initialize the MIB arg structure */
   memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
   memset(space, 0, 2048);
   ncsmib_init(&ncsmib_arg);
   ncsmem_aid_init(&ma, space, sizeof(space));

     
   if(bind_num < 0) {
        inst_ids[0] = 0;
        ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
        ncsmib_arg.i_idx.i_inst_len = 1;   
     
       do {
          ncsmib_arg.i_op = NCSMIB_OP_REQ_NEXTROW;
          ncsmib_arg.i_tbl_id = NCSMIB_TBL_IFSV_BIND_IFTBL;
          ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
          ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
          ncsmib_arg.i_xch_id  = xch_id++;
          ncsmib_arg.i_rsp_fnc = NULL;
   
          rc = ncsmib_sync_request(&ncsmib_arg, ncsmac_mib_request, 1000, &ma);
          if((rc != NCSCC_RC_SUCCESS) || 
             (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS))
         {
             if(row_show_succ == TRUE) 
             {
                ifsv_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS);
                return NCSCC_RC_SUCCESS;
             }      
             ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n Command Failed \n");
             ifsv_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
             if(rc != NCSCC_RC_SUCCESS)
                return rc;
             else
               return ncsmib_arg.rsp.i_status;
         }
         else {
             row_show_succ = TRUE;
         }
         ifsv_bind_intf_show_intf_info(ncsmib_arg.rsp.info.nextrow_rsp.i_usrbuf);

         bind_num = ncsmib_arg.rsp.info.nextrow_rsp.i_next.i_inst_ids[0];

         memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
         memset(space, 0, 2048);
         ncsmib_init(&ncsmib_arg);
         ncsmem_aid_init(&ma, space, sizeof(space));    



         /* Show all the rows of the table */   
         inst_ids[0] = bind_num;
         ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
         ncsmib_arg.i_idx.i_inst_len = 1;
   }while(1);
   
      }
   else
      {
        /* Show only the row with bind_num as index */
        ncsmib_arg.i_op      = NCSMIB_OP_REQ_GETROW;
        ncsmib_arg.i_tbl_id = NCSMIB_TBL_IFSV_BIND_IFTBL;
        ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
        ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
        ncsmib_arg.i_xch_id  = xch_id++;
        ncsmib_arg.i_rsp_fnc = NULL;
        inst_ids[0] = bind_num;
        ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)&inst_ids;
        ncsmib_arg.i_idx.i_inst_len = 1;   

          rc = ncsmib_sync_request(&ncsmib_arg, ncsmac_mib_request, 1000, &ma);
          if((rc != NCSCC_RC_SUCCESS) || 
             (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS))
         {
             ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n Command Failed \n");
             ifsv_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
             if(rc != NCSCC_RC_SUCCESS)
                return rc;
             else
               return ncsmib_arg.rsp.i_status;
         }
         ifsv_bind_intf_show_intf_info(ncsmib_arg.rsp.info.getrow_rsp.i_usrbuf);
      }
   
   ifsv_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS);
   return (NCSCC_RC_SUCCESS);

}
       

void ifsv_bind_intf_show_intf_info(USRBUF *usrbuf)
{
  USRBUF *ub = NULL;
  uns32 param_cnt;
  NCSPARM_AID rsp_pa;
  NCSMIB_PARAM_VAL param_val;
  uns8 space[1024];
  NCSMEM_AID ma;


  ub = m_MMGR_DITTO_BUFR(usrbuf);
  if(ub == NULL)
     return;

  memset(space, 0, sizeof(space));
  ncsmem_aid_init(&ma, space, 1024);

  param_cnt = ncsparm_dec_init(&rsp_pa,ub);
  printf("\n*********************************\n");
  while(param_cnt != 0)
  {
    if (ncsparm_dec_parm(&rsp_pa,&param_val,&ma) != NCSCC_RC_SUCCESS)
              break;
     switch (param_val.i_param_id)
     {
           case ncsIfsvBindIfBindPortNum_ID:
           {
               printf("\nBIND Port #%d\n\n\n",param_val.info.i_int);
           }
           break;

           case ncsIfsvBindIfBindIfIndex_ID:
           {
               printf("BIND IfIndex %d\n\n",param_val.info.i_int);
           }
           break;

           case ncsIfsvBindIfMasterIfIndex_ID:
           {
               printf("Master IfIndex %d\n\n",param_val.info.i_int);
           }
           break;

           case ncsIfsvBindIfSlaveIfIndex_ID:
           {
               printf("Slave IfIndex %d\n\n",param_val.info.i_int);
           }
           break;

           case ncsIfsvBindIfBindRowStatus_ID:
               if(param_val.info.i_int == NCSMIB_ROWSTATUS_ACTIVE)
               {
                 printf("Admin Status : UP\n\n");
               }
               else
               {
                 printf("Admin Status : DOWN\n\n");
               }
           break;
     }
     param_cnt--;
  }
  printf("*********************************\n");
  ncsparm_dec_done(&rsp_pa);
}


/****************************************************************************
 *
 * Function Name: intf_cef_config_ipv4addr
 *
 * Purpose: This API will be called by the CLI for "ip address" command. 
 *          This API will help configure IPv4 address for a specified interface.
 *    
 * Arguments: *arg_list:  the command list.
 *            *cef_data: cef data information.
 *
 * Return Value: NCSCC_RC_SUCCESS/error code
 *
 ****************************************************************************/
 uns32 intf_cef_config_ipv4addr(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
    NCSMIB_ARG           ncsmib_arg;
    uns8                 space[2048];
    NCSMEM_AID           ma;
    NCS_IP_ADDR          ipaddr;
    NCS_IPV4_ADDR        v4addr, lcl_mask;
    uns32                xch_id = 1, rel_offset = 0, ifindex = 0;
    uns32                retval = NCSCC_RC_SUCCESS, i, ret_val = NCSCC_RC_SUCCESS;
    uns32                inst_ids[IPXS_IPENTRY_INST_MAX_LEN], mask_len = 0;

    if(strcmp(arg_list->i_arg_record[0].cmd.strval, "no") == 0)
    {
       rel_offset = 1;
    }

    /* Verify the keywords first. */
    if((strcmp(arg_list->i_arg_record[rel_offset].cmd.strval, "ip") != 0) &&
       (strcmp(arg_list->i_arg_record[rel_offset + 1].cmd.strval, "address") != 0) &&
       (strcmp(arg_list->i_arg_record[rel_offset + 3].cmd.strval, "mask") != 0) &&
       (strcmp(arg_list->i_arg_record[rel_offset + 5].cmd.strval, "interface-index") != 0))
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "Wrong command usage\n\n");
       goto end;
    }

    v4addr = arg_list->i_arg_record[rel_offset + 2].cmd.intval; /* Host order */
    mask_len = arg_list->i_arg_record[rel_offset + 4].cmd.intval; /* Host order */
    if(mask_len > 32)
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "Wrong mask length specified\n\n");
       goto end;
    }
    ifindex = arg_list->i_arg_record[rel_offset + 6].cmd.intval;

    memset(&inst_ids, '\0', sizeof(inst_ids));
    inst_ids[0] = NCS_IP_ADDR_TYPE_IPV4;
    lcl_mask = 0xff000000;
    for(i = 0; i < 4; i++)
    {
       inst_ids[i + 1] = (v4addr & lcl_mask) >> ((4 - (i+1)) * 8); /* Verify this. */
       lcl_mask = (lcl_mask) >> 8;
    }

    /* Set the address mask length first. */
    memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
    memset(&ipaddr, 0, sizeof(ipaddr));
    ncsmib_init(&ncsmib_arg);
    ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
    ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
    ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
    ncsmib_arg.i_rsp_fnc = NULL;
    ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
    ncsmib_arg.i_idx.i_inst_len = IPXS_IPENTRY_INST_MIN_LEN;
    ncsmib_arg.i_xch_id  = xch_id++;
    ncsmib_arg.i_tbl_id  = NCSMIB_TBL_IPXS_IPTBL;
    ncsmem_aid_init(&ma, space, sizeof(space));



    if(rel_offset == 1)
    {
       /* This is "no ip address" command. So, address has to be deleted. */
       ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsIfsvIpEntRowStatus_ID;
       ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
       ncsmib_arg.req.info.set_req.i_param_val.i_length = 4;
       ncsmib_arg.req.info.set_req.i_param_val.info.i_int = NCS_ROW_DESTROY;
       retval = ncsmib_sync_request(&ncsmib_arg, ncsmac_mib_request, 1000, &ma);
       if (retval != NCSCC_RC_SUCCESS)
       {
          ret_val = m_MAB_DBG_SINK(retval);
          ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "IPXS IPIPTBL MIB Request failed for RowStatus destroy\n");
          goto end;
       }
       ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
       if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
       {
          retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
          ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "Invalid MIB Response received for RowStatus destroy\n");
          goto end;
       }
       if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
       {
          retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
          ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "MIB Request failed for RowStatus destroy\n");
          goto end;
       }
       return ifsv_cli_done(cef_data->i_bindery->i_cli_hdl, retval);    
    }

    /* Create Row for this entry */
    ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsIfsvIpEntRowStatus_ID;
    ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
    ncsmib_arg.req.info.set_req.i_param_val.i_length = 4;
    ncsmib_arg.req.info.set_req.i_param_val.info.i_int = NCS_ROW_CREATE_AND_WAIT;

    retval = ncsmib_sync_request(&ncsmib_arg, ncsmac_mib_request, 1000, &ma);

    if (retval != NCSCC_RC_SUCCESS)
    {
          ret_val = m_MAB_DBG_SINK(retval);
          ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "IPXS IPIPTBL MIB Request failed for RowStatus create and wait\n");
          goto end;
    }
    ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
    if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
    {
          retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
          ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "Invalid MIB Response received for RowStatus create and wait\n");
          goto end;
    }
    if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
    {
          retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
          ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "MIB Request failed for RowStatus create and wait\n");
          goto end;
    }


    memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
    memset(&ipaddr, 0, sizeof(ipaddr));
    ncsmib_init(&ncsmib_arg);
    ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
    ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
    ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
    ncsmib_arg.i_rsp_fnc = NULL;
    ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
    ncsmib_arg.i_idx.i_inst_len = IPXS_IPENTRY_INST_MIN_LEN;
    ncsmib_arg.i_xch_id  = xch_id++;
    ncsmib_arg.i_tbl_id  = NCSMIB_TBL_IPXS_IPTBL;
    ncsmem_aid_init(&ma, space, sizeof(space));
    ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsIfsvIpEntAddrMask_ID;
    ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
    ncsmib_arg.req.info.set_req.i_param_val.i_length = 1;
    ncsmib_arg.req.info.set_req.i_param_val.info.i_int = mask_len;
    retval = ncsmib_sync_request(&ncsmib_arg, ncsmac_mib_request, 1000, &ma);
    if (retval != NCSCC_RC_SUCCESS)
    {
       ret_val = m_MAB_DBG_SINK(retval);
       ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "IPXS IPIPTBL MIB Request failed for address mask length\n");
       goto end;
    }
    ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
    if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "Invalid MIB Response received for address mask length\n");
       goto end;
    }
    if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "MIB Request failed for address mask length\n");
       goto end;
    }

    memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
    memset(&ipaddr, 0, sizeof(ipaddr));
    ncsmib_init(&ncsmib_arg);
    ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
    ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
    ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
    ncsmib_arg.i_rsp_fnc = NULL;
    ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
    ncsmib_arg.i_idx.i_inst_len = IPXS_IPENTRY_INST_MIN_LEN;
    ncsmib_arg.i_xch_id  = xch_id++;
    ncsmib_arg.i_tbl_id  = NCSMIB_TBL_IPXS_IPTBL;
    ncsmem_aid_init(&ma, space, sizeof(space));
    ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsIfsvIpEntIfIndex_ID;
    ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
    ncsmib_arg.req.info.set_req.i_param_val.i_length = 4;
    ncsmib_arg.req.info.set_req.i_param_val.info.i_int = ifindex;
    retval = ncsmib_sync_request(&ncsmib_arg, ncsmac_mib_request, 1000, &ma);
    if (retval != NCSCC_RC_SUCCESS)
    {
       ret_val = m_MAB_DBG_SINK(retval);
       ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "IPXS IPIPTBL MIB Request failed for interface index\n");
       goto end;
    }
    ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
    if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "Invalid MIB Response received for interface index\n");
       goto end;
    }
    
    if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "MIB Request failed for interface index\n");
       goto end;
    }

    /* Create Row for this entry */
    memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
    memset(&ipaddr, 0, sizeof(ipaddr));
    ncsmib_init(&ncsmib_arg);
    ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
    ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
    ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
    ncsmib_arg.i_rsp_fnc = NULL;
    ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
    ncsmib_arg.i_idx.i_inst_len = IPXS_IPENTRY_INST_MIN_LEN;
    ncsmib_arg.i_xch_id  = xch_id++;
    ncsmib_arg.i_tbl_id  = NCSMIB_TBL_IPXS_IPTBL;
    ncsmem_aid_init(&ma, space, sizeof(space));
    ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsIfsvIpEntRowStatus_ID;
    ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
    ncsmib_arg.req.info.set_req.i_param_val.i_length = 4;
    ncsmib_arg.req.info.set_req.i_param_val.info.i_int = NCS_ROW_ACTIVE;

    retval = ncsmib_sync_request(&ncsmib_arg, ncsmac_mib_request, 1000, &ma);

    if (retval != NCSCC_RC_SUCCESS)
    {
          ret_val = m_MAB_DBG_SINK(retval);
          ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "IPXS IPIPTBL MIB Request failed for RowStatus active\n");
          goto end;
    }
    ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
    if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
    {
          retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
          ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "Invalid MIB Response received for RowStatus active\n");
          goto end;
    }
    if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
    {
          retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
          ifsv_cli_display(cef_data->i_bindery->i_cli_hdl, "MIB Request failed for RowStatus active\n");
          goto end;
    }
end:

    return ifsv_cli_done(cef_data->i_bindery->i_cli_hdl, retval);    
}

#if(NCS_VIP == 1)

/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME: vip_show_all_CEF                                       *~*
*~* DESCRIPTION   : This Function is the CommandExecutionFunction for the    *~*
*~*                 command "vip dump"                                     *~*
*~* ARGUMENTS     : NCSCLI_ARG_SET *p_arg_list                               *~*
*~*               : NCSCLI_CEF_DATA *p_cef_data                              *~*
*~* RETURNS       : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                        *~*
*~* NOTES         :                                                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

uns32
vip_show_all_CEF(NCSCLI_ARG_SET *p_arg_list, NCSCLI_CEF_DATA *p_cef_data)
{
    uns32 ret_code;
    /* For this CEF MIB INDEX is NULL and Index Len is Passed as 0*/
    ret_code = vip_process_get_row_request(p_cef_data, NULL, 0, FALSE);

     switch(ret_code)
     {
        case NCSCC_RC_NO_INSTANCE:
               printf("\n INVALID VLICATION DETAILS vizz..Application Name or Handle ENTERED BY USER ");
               break;
        case IFSV_VIP_NO_RECORDS_IN_VIP_DATABASE:
               printf("\nCURRENTLY NO RECORDS EXIST IN VIP DATABASE");
               break;
        case NCSCC_RC_FAILURE:
               printf("\n Interface Service is Busy Adding/Deleting Records Try <vipdump> Command after some time:::\n");
               m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_CLI, IFSV_VIP_CLI_SHOW_ALL_FAILED);
               break;
        default:
               break;
     }

     if(NCSCC_RC_SUCCESS != vip_ncs_cli_done(p_cef_data->i_bindery->i_cli_hdl,
                                               NCSCC_RC_SUCCESS))
     {
        m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_CLI, IFSV_VIP_CLI_DONE_FAILURE);
        return NCSCC_RC_FAILURE;
     } 

    return NCSCC_RC_SUCCESS;
}


/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME: vip_show_single_entry_CEF                              *~*
*~* DESCRIPTION   : This Function is the CommandExecutionFunction for the    *~*
*~*                 command "vip show"                                     *~*
*~* ARGUMENTS     : NCSCLI_ARG_SET *p_arg_list                               *~*
*~*               : NCSCLI_CEF_DATA *p_cef_data                              *~*
*~* RETURNS       : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                        *~*
*~* NOTES         :                                                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
uns32
vip_show_single_entry_CEF(NCSCLI_ARG_SET *p_arg_list,
                            NCSCLI_CEF_DATA *p_cef_data)
{
     NCS_CLI_MODE_DATA *p_vip_mode_data;
 
     uns32 slen_ctr;
     uns32 tot_len_cnt = 0;
     uns32 idxs[128];
 
     uns32 ret_code;
 
     p_vip_mode_data = (NCS_CLI_MODE_DATA *)p_cef_data->i_subsys->i_cef_mode;
 
 
     idxs[tot_len_cnt++] = strlen(p_vip_mode_data->vipApplName);
     for(slen_ctr=0; slen_ctr < idxs[0];tot_len_cnt++, slen_ctr++)
     {
         idxs[tot_len_cnt] = p_vip_mode_data->vipApplName[slen_ctr];/* Index: 1*/
     }
     /* Index: 2*/
     idxs[tot_len_cnt] = p_vip_mode_data->poolHdl;

     /*For IP Address Value*/
     /* Index: 3*/
     idxs[++tot_len_cnt] = 0;
     idxs[++tot_len_cnt] = 0;
     idxs[++tot_len_cnt] = 0;
     idxs[++tot_len_cnt] = 0;

     /*For IP MAsk Value*/
     idxs[++tot_len_cnt] = 0;/* Index: 4*/
     
     /*For Interface Name Length*/
     idxs[++tot_len_cnt] = 1;

     /*For Interface Name Value*/
     /* Index: 5*/
     idxs[++tot_len_cnt]  = 0;

     /* IPType */ 
     /* Index: 6*/
     idxs[++tot_len_cnt] = 0;

     tot_len_cnt++;


     ret_code = vip_process_get_row_request(p_cef_data, idxs, tot_len_cnt, TRUE);
     switch(ret_code)
     {
        case NCSCC_RC_NO_INSTANCE:
               printf("\n INVALID APPLICATION DETAILS vizz..Application Name or Handle ENTERED BY USER ");
               break;
        case IFSV_VIP_NO_RECORDS_IN_VIP_DATABASE:
               printf("\n CURRENTLY NO RECORDS FOUND IN VIP DATABASE ");
               break;
        default:
               m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_CLI, IFSV_VIP_CLI_SHOW_ALL_FAILED);
     }

     if(NCSCC_RC_SUCCESS != 
        vip_ncs_cli_done(p_cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS))
     {
         m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_CLI, IFSV_VIP_SINGLE_ENTRY_SHOW_CEF_FAILED);
         return NCSCC_RC_FAILURE;
     }

     return NCSCC_RC_SUCCESS;
}


/*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME: vip_change_mode_vip_CEF                               *~*
*~* DESCRIPTION   : This Function is the CommandExecutionFunction for         *~*
*~*                 changing the mode to "vip" mode.                        *~*
*~* ARGUMENTS     : NCSCLI_ARG_SET *p_arg_list                                *~*
*~*               : NCSCLI_CEF_DATA *p_cef_data                               *~*
*~* RETURNS       : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                         *~*
*~* NOTES         :                                                           *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

uns32
vip_change_mode_vip_CEF(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *p_cef_data)
{
   NCS_CLI_MODE_DATA *p_vip_mode_data;
   NCSMIB_ARG ncs_mib_arg;
   uns8 space[1024];
   NCSMEM_AID ma;
   uns32 slen_ctr;
   uns32 tot_len_cnt = 0;
   uns32 idxs[128];
   uns32 xch_id = 1;
   uns32 rc;
   uns32 rsp_status;

   memset(&ncs_mib_arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&ncs_mib_arg);
   memset(space, 0, sizeof(space));
   ncsmem_aid_init(&ma, space, 1024);

   p_vip_mode_data = (NCS_CLI_MODE_DATA *)
                     m_MMGR_ALLOC_NCSCLI_OPAQUE(sizeof(NCS_CLI_MODE_DATA));
   strcpy(p_vip_mode_data->vipApplName, arg_list->i_arg_record[2].cmd.strval);
   p_vip_mode_data->poolHdl = arg_list->i_arg_record[4].cmd.intval;


   p_cef_data->i_subsys->i_cef_mode = (void *)p_vip_mode_data;

   if(strlen(p_vip_mode_data->vipApplName) == 0)  
   {
      m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_CLI, IFSV_VIP_CLI_INVALID_APPL_NAME_INPUT_TO_CLI);
      return NCSCC_RC_FAILURE;
   }
   if( p_vip_mode_data->poolHdl <= 0)
   {
      m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_CLI, IFSV_VIP_CLI_INVALID_HANDLE_INPUT_TO_CLI);
      return NCSCC_RC_FAILURE;
   }

    idxs[tot_len_cnt++] = strlen(p_vip_mode_data->vipApplName);
    for(slen_ctr=0; slen_ctr < idxs[0];tot_len_cnt++, slen_ctr++)
    {
        idxs[tot_len_cnt] = p_vip_mode_data->vipApplName[slen_ctr];/* Index: 1*/
    }
    /* Index: 2*/
    idxs[tot_len_cnt] = p_vip_mode_data->poolHdl;

    /*For IP Address Value*/
    /* Index: 3*/
    idxs[++tot_len_cnt] = 0;
    idxs[++tot_len_cnt] = 0;
    idxs[++tot_len_cnt] = 0;
    idxs[++tot_len_cnt] = 0;

    /*For IP MAsk Value*/
    idxs[++tot_len_cnt] = 0;/* Index: 4*/
     
    /*For Interface Name Length*/
    idxs[++tot_len_cnt] = 1;

    /*For Interface Name Value*/
    /* Index: 5*/
    idxs[++tot_len_cnt]  = 0;

    /* IPType */ 
    /* Index: 6*/
    idxs[++tot_len_cnt] = 0;

    tot_len_cnt++;

    ncs_mib_arg.i_idx.i_inst_ids = idxs;
    ncs_mib_arg.i_idx.i_inst_len = tot_len_cnt;

    ncs_mib_arg.i_op = NCSMIB_OP_REQ_NEXTROW;
    ncs_mib_arg.i_tbl_id = NCSMIB_TBL_IFSV_VIPTBL;
    ncs_mib_arg.i_usr_key = p_cef_data->i_bindery->i_cli_hdl;
    ncs_mib_arg.i_mib_key = p_cef_data->i_bindery->i_mab_hdl;
    ncs_mib_arg.i_xch_id  = xch_id++;
    ncs_mib_arg.i_rsp_fnc = NULL;
  
    rc = ncsmib_sync_request(&ncs_mib_arg,
                             ncsmac_mib_request,
                             1000,
                             &ma);
    rsp_status = ncs_mib_arg.rsp.i_status;
    if((rc != NCSCC_RC_SUCCESS) || (ncs_mib_arg.rsp.i_status != NCSCC_RC_SUCCESS))
    {
        vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n INVALID INPUTS::THE INPUTS SHOULD BE ONE OF THE ENTRIES IN vip-dump command \n");

        if(rc != NCSCC_RC_SUCCESS)
           return rc;
     }

    if (vip_single_entry_check(
           ncs_mib_arg.rsp.info.nextrow_rsp.i_next.i_inst_len,
           (uns32 *)ncs_mib_arg.rsp.info.nextrow_rsp.i_next.i_inst_ids,
           p_vip_mode_data->vipApplName,
           p_vip_mode_data->poolHdl)
                 == NCSCC_RC_SUCCESS)
    {
        m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_CLI, IFSV_VIP_CLI_MODE_CHANGE_CEF_SUCCESS);
        if(NCSCC_RC_SUCCESS != 
           vip_ncs_cli_done(p_cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS))
        {
            m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_CLI, IFSV_VIP_CLI_DONE_FAILURE);
            return NCSCC_RC_FAILURE;
        }
        return NCSCC_RC_SUCCESS;
    }
    else
    {
        return NCSCC_RC_FAILURE;
    }

}

/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME: vip_process_get_row_request                            *~*
*~* DESCRIPTION   : This function is commonly used by both the CEF's used    *~*
*~*                   to fetch the rows based on the single_show FLAG        *~*
*~*                   1. if it is True, then this function is used to fetch  *~*
*~*                   a single row and correspondingly calls the display     *~*
*~*                   function to display it.                                *~*
*~*                   2. if it is FALSE, then this function is used to fetch *~*
*~*                   and dislay all the rows(vip entries) and display them. *~*
*~* ARGUMENTS     : NCSMIB_ARG *p_ncs_mib_arg                                *~*
*~*               : NCSCLI_CEF_DATA *p_cef_data                              *~*
*~*               : NCS_BOOL single_show                                     *~*
*~* RETURNS       : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                        *~*
*~* NOTES         :                                                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
static uns32
vip_process_get_row_request( NCSCLI_CEF_DATA *p_cef_data,
                               uns32 *idxs,
                               uns32 idx_len,
                               NCS_BOOL single_show)
{
    NCS_CLI_MODE_DATA *p_vip_mode_data;
    VIP_DATA_DISPLAY vip_data_display;
    uns32 xch_id = 1;
    uns32 rc;
    uns32 rsp_status;
    uns32 sec_idxs[128];
    uns32 sec_idx_len;

    NCSMIB_ARG ncs_mib_arg;
    uns8 space[1024];
    NCSMEM_AID ma;

    NCS_BOOL records_exist = FALSE;

    memset(&ncs_mib_arg, 0, sizeof(NCSMIB_ARG));
    ncsmib_init(&ncs_mib_arg);
    memset(space, 0, sizeof(space));
    ncsmem_aid_init(&ma, space, 1024);

    p_vip_mode_data = (NCS_CLI_MODE_DATA *)p_cef_data->i_subsys->i_cef_mode;

     /*For Top Line To be Displayed*/
     vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
     /*Populating the mib arg structure*/

    ncs_mib_arg.i_idx.i_inst_ids = idxs;
    ncs_mib_arg.i_idx.i_inst_len = idx_len;

    do
    {
       ncs_mib_arg.i_op = NCSMIB_OP_REQ_NEXTROW;
       ncs_mib_arg.i_tbl_id = NCSMIB_TBL_IFSV_VIPTBL;
       ncs_mib_arg.i_usr_key = p_cef_data->i_bindery->i_cli_hdl;
       ncs_mib_arg.i_mib_key = p_cef_data->i_bindery->i_mab_hdl;
       ncs_mib_arg.i_xch_id  = xch_id++;
       ncs_mib_arg.i_rsp_fnc = NULL;
  
       rc = ncsmib_sync_request(&ncs_mib_arg,
                               ncsmac_mib_request,
                               1000,
                               &ma);
       rsp_status = ncs_mib_arg.rsp.i_status;
       if((rc != NCSCC_RC_SUCCESS) || (ncs_mib_arg.rsp.i_status != NCSCC_RC_SUCCESS))
       {
          if(records_exist == TRUE)
             return NCSCC_RC_SUCCESS;
          /*vip_cli_display_all(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\nFailed \n");*/
          if(rc != NCSCC_RC_SUCCESS)
             return rc;
          else
             return ncs_mib_arg.rsp.i_status;
       }
       else 
          records_exist = TRUE;


       if (single_show == TRUE)
       {
          if (vip_single_entry_check(
               ncs_mib_arg.rsp.info.nextrow_rsp.i_next.i_inst_len,
               (uns32 *)ncs_mib_arg.rsp.info.nextrow_rsp.i_next.i_inst_ids,
               p_vip_mode_data->vipApplName,
               p_vip_mode_data->poolHdl)
                    == NCSCC_RC_SUCCESS)
          {
             /*
              ~*~*~*~**~*~**~*~**~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
              In this case the get row request will be called depending on the 
              no of IPAddresses & Interface Name's present for that VIPD Entry.
              ~*~*~*~**~*~**~*~**~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
             */
             m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_CLI, IFSV_VIP_CLI_SINGLE_ENTRY_DISPLAY);
 
             if(NCSCC_RC_SUCCESS != 
                vip_populate_display_data(&ncs_mib_arg, &vip_data_display))
               return NCSCC_RC_FAILURE;

             vip_cli_display_all(p_cef_data, vip_data_display);
             /*
               In this case break is not required as the getnext request fails 
               for that particular request, the flow comes out of the while loop
             */
          }
          else
          {
             /*
              ~*~*~*~**~*~**~*~**~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
              In this case the user given Inputs are Invalid, as it is not found 
              in the VIP Database Invalid CLI INPUT error. 
              ~*~*~*~**~*~**~*~**~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
             */
             /* 
              Print saying No Instance*/
              return NCSCC_RC_FAILURE;
          }
       }
       else
       {
       /*
        *~*~*~**~*~**~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
        In this case the get row request will be called repeatitively untill 
        all the rows in the vipDatabase are displayed From first row to last one
        *~*~*~**~*~**~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
       */
           m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_CLI, IFSV_VIP_CLI_DISPLAY_ALL_ENTRIES);
           if(NCSCC_RC_SUCCESS != 
               vip_populate_display_data(&ncs_mib_arg, &vip_data_display))
               return NCSCC_RC_FAILURE;
           /*
            * Call The DIsplay Routine......
           */
           vip_cli_display_all(p_cef_data, vip_data_display);
       }
 
       /*
        * For Processing the Next Row ........
       */
       sec_idx_len = ncs_mib_arg.rsp.info.nextrow_rsp.i_next.i_inst_len;
       memcpy(sec_idxs,ncs_mib_arg.rsp.info.nextrow_rsp.i_next.i_inst_ids,
              sec_idx_len*sizeof(uns32));


       memset(&ncs_mib_arg, 0, sizeof(NCSMIB_ARG));
       ncsmib_init(&ncs_mib_arg);
       memset(space, 0, sizeof(space));
       ncsmem_aid_init(&ma, space, 1024);

       ncs_mib_arg.i_idx.i_inst_ids =  sec_idxs;
       ncs_mib_arg.i_idx.i_inst_len = sec_idx_len;
     }while(FOREVER);

     return NCSCC_RC_SUCCESS;
}

/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME: vip_populate_display_data                              *~*
*~* DESCRIPTION   : This function populates the structure used for           *~*
*~*                 Displaying data obtained from the VIP database.          *~*
*~* ARGUMENTS     : NCSMIB_ARG *p_ncs_mib_arg                                *~*
*~*               : VIP_DATA_DISPLAY *p_vip_data_display                 *~*
*~* RETURNS       : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                        *~*
*~* NOTES         :                                                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
static uns32
vip_populate_display_data(NCSMIB_ARG *p_ncs_mib_arg,
                            VIP_DATA_DISPLAY *p_vip_data_display)
{
   uns32 *idxs;
   uns32 idx_len;
   uns32 idx_cnt;
   uns32 service_len;
   uns32 interface_len;
   uns32 slen_ctr;
   uns32 ilen_ctr;
   uns32 buff;
   uns32 param_cnt;
   NCSPARM_AID rsp_pa;
   NCSMIB_PARAM_VAL param_val;
   uns8 space[1024];
   NCSMEM_AID ma;
   USRBUF *ub = NULL;

   if(p_ncs_mib_arg->rsp.info.nextrow_rsp.i_usrbuf == NULL)
      return NCSCC_RC_FAILURE;
   ub = m_MMGR_DITTO_BUFR(p_ncs_mib_arg->rsp.info.nextrow_rsp.i_usrbuf);
   if(ub == NULL)
     return NCSCC_RC_FAILURE;



   memset(space, 0, sizeof(space));
   ncsmem_aid_init(&ma, space, 1024);

   memset(p_vip_data_display, 0, sizeof(VIP_DATA_DISPLAY));


   idx_len = p_ncs_mib_arg->rsp.info.nextrow_rsp.i_next.i_inst_len;
   idxs = (uns32 *)p_ncs_mib_arg->rsp.info.nextrow_rsp.i_next.i_inst_ids;

   if(idx_len > 0)
   {
       service_len = idxs[0];
       p_vip_data_display->p_service_name =
                     (uns8 *)m_MMGR_ALLOC_NCSCLI_OPAQUE(service_len + 1);
       /*INDEX  ::1 */
       for(slen_ctr=0, idx_cnt = 1; slen_ctr<service_len; slen_ctr++, idx_cnt++)
       {
             p_vip_data_display->p_service_name[slen_ctr] = (uns8)idxs[idx_cnt];
       }
       p_vip_data_display->p_service_name[slen_ctr] = '\0';

       /*INDEX  ::2 */
       p_vip_data_display->handle = idxs[idx_cnt++];/*2*/
       /*
        * IPAddress Extraction "3"
       * */
       /*INDEX  ::3 */
       ((uns8 *)(&p_vip_data_display->ipaddress))[0] = (uns8)idxs[idx_cnt++];
       ((uns8 *)(&p_vip_data_display->ipaddress))[1] = (uns8)idxs[idx_cnt++];
       ((uns8 *)(&p_vip_data_display->ipaddress))[2] = (uns8)idxs[idx_cnt++];
       ((uns8 *)(&p_vip_data_display->ipaddress))[3] = (uns8)idxs[idx_cnt++];
       /*INDEX  ::4 */
       p_vip_data_display->mask_len     = (uns8)idxs[idx_cnt++];/*5*/

       /*INDEX  ::5 */
       interface_len = idxs[idx_cnt++];
       p_vip_data_display->p_interface_name =
                                 (uns8 *)m_MMGR_ALLOC_NCSCLI_OPAQUE(interface_len + 1);
       for(ilen_ctr=0; ilen_ctr<interface_len; ilen_ctr++)
       {
          p_vip_data_display->p_interface_name[ilen_ctr] = (uns8)idxs[idx_cnt++];
       }
       p_vip_data_display->p_interface_name[ilen_ctr] = '\0';/*6*/

       /*INDEX  ::6 */
       p_vip_data_display->ip_type = idxs[idx_cnt++];/*3*/
 
       /*
        * Populate the Userbuff Information into the DATA Destructure declared at the TOP
       */
       param_cnt = 
          ncsparm_dec_init(&rsp_pa,ub);
       while(0 != param_cnt)
       {
            if (ncsparm_dec_parm(&rsp_pa,&param_val,&ma) != NCSCC_RC_SUCCESS)
               break;
            switch (param_val.i_param_id)
            {
               case ncsVIPEntryType_ID:
               {
                  buff = param_val.info.i_int;
                  p_vip_data_display->configured = m_NCS_OS_NTOHS(buff);
               }
               break;

               case ncsVIPInstalledInterfaceName_ID:
               {
                   memcpy(p_vip_data_display->installed_intf, 
                   param_val.info.i_oct, 
                   param_val.i_length);
               }
               break;

               case ncsVIPIPRange_ID:
               {
                   buff = param_val.info.i_int;
                   p_vip_data_display->ip_range = buff;
               }
               break;

               case ncsVIPOperStatus_ID:
               {
                   buff = param_val.info.i_int;
                   p_vip_data_display->opr_status = buff;
               }
               break;

               case ncsVIPRowStatus_ID:
               {
                  buff = param_val.info.i_int;
               }
               break;

               default:
               break;
           }/*Switch*/
    
           param_cnt--;
       }/*while*/
    }
    else
    {
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_CLI, IFSV_VIP_CLI_INVALID_INDEX_RECEIVED);
       return NCSCC_RC_FAILURE;
    }

    ncsparm_dec_done(&rsp_pa);


    return NCSCC_RC_SUCCESS;
}

/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME:   vip_cli_display_all                                  *~*
*~* DESCRIPTION   :   This function Displays data on Command prompt.         *~*
*~* ARGUMENTS     :   NCSCLI_CEF_DATA *p_cef_data                            *~*
*~*               :   VIP_DATA_DISPLAY vip_data_display                  *~*
*~* RETURNS       :   NONE                                                   *~*
*~* NOTES         :                                                          *~*
*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
static void
vip_cli_display_all(NCSCLI_CEF_DATA *p_cef_data,
                       VIP_DATA_DISPLAY vip_data_display)
{
   int8                    ip_net_str[30]; /*More than 20 for safety*/
   NCS_IPPFX               nworder_dest;   /* an IP Address and IPMask  */
   uns8 ip_addr_str[60]={'\0'};
   uns8 hdl_str[20]={'\0'};
   uns8 app_name[150]={'\0'};
   uns8 app_handle[60]={'\0'};
   uns8 ip_pool_type[60]={'\0'};
   uns8 installed_intf[128]={'\0'};
   uns8 entry_status[60]={'\0'};
   uns8 end_of_vip_entry[128]={'\0'};

   sprintf (end_of_vip_entry, "%s", "\n==================================================================================================\n");

   nworder_dest.ipaddr.info.v4 = vip_data_display.ipaddress;
   nworder_dest.mask_len = vip_data_display.mask_len;

   /* Conversion Of IPAddress into a string*/
   sprintf(ip_net_str, "%d.%d.%d.%d/%d",
      ((uns8*)(&nworder_dest.ipaddr.info.v4))[0], /*This is max 3 chars */
      ((uns8*)(&nworder_dest.ipaddr.info.v4))[1],
      ((uns8*)(&nworder_dest.ipaddr.info.v4))[2],
      ((uns8*)(&nworder_dest.ipaddr.info.v4))[3],
      (nworder_dest.mask_len));
   strcat(ip_addr_str, "IpAddress/Masklen   :  ");
   strcat(ip_addr_str,ip_net_str);

      strcat(app_name, "Application         :  ");
   strcat(app_name,vip_data_display.p_service_name);
 
      strcat(app_handle, "Handle              :  ");
   sprintf(hdl_str, "%d", vip_data_display.handle);
   strcat(app_handle, hdl_str);

      strcat(ip_pool_type, "IpType              :  ");
   if(vip_data_display.ip_type == 0)/*declare an enum and replace the numbers*/
      strcat(ip_pool_type,"INTERNAL");
   else
      strcat(ip_pool_type,"EXTERNAL");

   strcat(installed_intf, "Interface           :  ");
      strcat(installed_intf,vip_data_display.installed_intf);

   strcat(entry_status, "EntryStatus         :  ");
   if (( m_IFSV_IS_VIP_ENTRY_STALE(vip_data_display.configured)) != TRUE)
   {
      strcat(entry_status,"ACTIVE-VIP");
   }
   else
      strcat(entry_status,"STALE-VIP");

   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)end_of_vip_entry);
  
   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)app_name);
   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)ip_addr_str);
   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)app_handle);
   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)ip_pool_type);
   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)installed_intf);
   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)entry_status);
   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");

   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)end_of_vip_entry);
   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");


   if(NCSCC_RC_SUCCESS != 
      vip_ncs_cli_done(p_cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS))
   {
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_CLI, IFSV_VIP_CLI_DONE_FAILURE);
   }
}

/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME:   vip_single_entry_check.                                *~*
*~* DESCRIPTION   :   This Function is used to check whether the information *~*
*~*                   obtained from CLI is valid or not.                     *~*
*~* ARGUMENTS     :   uns32 idx_len                                          *~*  
*~*               :   uns32 *idxs                                            *~*  
*~*               :   uns8 *p_given_service_name                             *~*  
*~*               :   uns32 given_handle                                     *~*  
*~* RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                        *~*
*~* NOTES         :                                                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

uns32
vip_single_entry_check(uns32 idx_len,
                         uns32 *idxs,
                         uns8 *p_given_service_name, 
                         uns32 given_handle)
{
   VIP_DATA_DISPLAY *p_vip_data_display;
   uns32 slen_ctr;
   uns32 service_len;

   if(idx_len > 0)
   {
      service_len = idxs[0];
      p_vip_data_display->p_service_name =
                    (uns8 *)m_MMGR_ALLOC_NCSCLI_OPAQUE(service_len + 1);
      /* Free at the end */
      for(slen_ctr=0; slen_ctr<service_len; slen_ctr++)
      {
         p_vip_data_display->p_service_name[slen_ctr] = 
                          (uns8)idxs[slen_ctr+1];
      }
      p_vip_data_display->p_service_name[slen_ctr] = '\0';
      p_vip_data_display->handle = idxs[++slen_ctr];

      if((strcmp(p_vip_data_display->p_service_name, 
                 p_given_service_name) == 0)
          &&(p_vip_data_display->handle == given_handle))
      {
         return NCSCC_RC_SUCCESS;
      }
   }
   return NCSCC_RC_FAILURE;
}
/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME: vip_ncs_cli_display                                    *~*
*~* DESCRIPTION   :                                                          *~*
*~* ARGUMENTS     : uns32 cli_hdl                                            *~*
*~*               : uns8* str                                                *~*
*~* RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                        *~*
*~* NOTES         :                                                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
static uns32 
vip_ncs_cli_display(uns32 cli_hdl, uns8* str)
{
   NCSCLI_OP_INFO info;
                            
   info.i_hdl = cli_hdl;
   info.i_req = NCSCLI_OPREQ_DISPLAY;
   info.info.i_display.i_str = str;
                                     
   if(ncscli_opr_request(&info) != NCSCC_RC_SUCCESS)
   {
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}
/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME:   vip_ncs_cli_done                                     *~*
*~* DESCRIPTION   :                                                          *~*
*~* ARGUMENTS     : uns32 cli_hdl                                            *~*
*~*               : uns32 rc                                                 *~*
*~* RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                        *~*
*~* NOTES         :                                                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
                                                                                                                             
static uns32 
vip_ncs_cli_done(uns32 cli_hdl, uns32 rc)
{
   NCSCLI_OP_INFO info;
                                     
   info.i_hdl = cli_hdl;
   info.i_req = NCSCLI_OPREQ_DONE;
   info.info.i_done.i_status = rc;
                                     
   if(ncscli_opr_request(&info) != NCSCC_RC_SUCCESS)
   {
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}

#endif

/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME: ifsv_process_get_row_request                             *~*
*~* DESCRIPTION   : This function is commonly used by both the CEF's used    *~*
*~* ARGUMENTS     : NCSCLI_CEF_DATA *p_cef_data                              *~*
*~* RETURNS       : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                        *~*
*~* NOTES         :                                                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/


static uns32
ifsv_process_get_row_request(NCSCLI_CEF_DATA *p_cef_data, uns32 shelf, uns32 slot, uns32 subslot)
{
    NCS_IFSV_CLI_MODE_DATA *p_ifsv_mode_data;
    uns32 xch_id = 1;
    uns32 rc;
    uns32 rsp_status;
    uns32 sec_idxs[128];
    uns32 sec_idx_len;
    NCSMIB_ARG ncs_mib_arg;
    uns8 space[1024];
    NCSMEM_AID ma;
    NCS_IFSV_SSPT_IF_INDEX_INFO sspt_info;
    uns8 title_str[200];

    /* embedding subslot changes */
    sprintf(title_str, "%-10.8s%-10.7s%-15.10s%-10.7s%-10.9s%-15.11s%7.7s%10.7s", "SHELF-ID", "SLOT-ID", "SUBSLOT_ID", "PORT-ID", "TYPE-INFO", "SUBSCR-INFO", "IFINDEX", "IF-NAME");

    memset(&ncs_mib_arg, 0, sizeof(NCSMIB_ARG));
    ncsmib_init(&ncs_mib_arg);
    memset(space, 0, sizeof(space));
    ncsmem_aid_init(&ma, space, 1024);


    p_ifsv_mode_data = (NCS_IFSV_CLI_MODE_DATA *)p_cef_data->i_subsys->i_cef_mode;

     /*For Top Line To be Displayed*/
     ifsv_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
     ifsv_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)title_str);
     /*Populating the mib arg structure*/

    ncs_mib_arg.i_idx.i_inst_ids = NULL;
    ncs_mib_arg.i_idx.i_inst_len = 0;

    do
    {
       ncs_mib_arg.i_op = NCSMIB_OP_REQ_NEXTROW;
       ncs_mib_arg.i_tbl_id = NCSMIB_TBL_IFSV_IFMAPTBL;
       ncs_mib_arg.i_usr_key = p_cef_data->i_bindery->i_cli_hdl;
       ncs_mib_arg.i_mib_key = p_cef_data->i_bindery->i_mab_hdl;
       ncs_mib_arg.i_xch_id  = xch_id++;
       ncs_mib_arg.i_rsp_fnc = NULL;
  
       rc = ncsmib_sync_request(&ncs_mib_arg,
                               ncsmac_mib_request,
                               1000,
                               &ma);
       rsp_status = ncs_mib_arg.rsp.i_status;
       if((rc != NCSCC_RC_SUCCESS) || (ncs_mib_arg.rsp.i_status != NCSCC_RC_SUCCESS))
       {
          if((rc != NCSCC_RC_SUCCESS)|| (ncs_mib_arg.rsp.i_status == NCSCC_RC_NO_INSTANCE))
             return rc;
       }

       sec_idx_len = ncs_mib_arg.rsp.info.nextrow_rsp.i_next.i_inst_len;
       memcpy(sec_idxs,ncs_mib_arg.rsp.info.nextrow_rsp.i_next.i_inst_ids,
              sec_idx_len*sizeof(uns32));

       memset(&sspt_info, 0, sizeof(NCS_IFSV_SSPT_IF_INDEX_INFO));

       sspt_info.sspt_info.shelf = sec_idxs[0];
       sspt_info.sspt_info.slot = sec_idxs[1];
       /* embedding subslot changes */
       sspt_info.sspt_info.subslot = sec_idxs[2];
       sspt_info.sspt_info.port = sec_idxs[3];
       sspt_info.sspt_info.type = sec_idxs[4];
       sspt_info.sspt_info.subscr_scope = sec_idxs[5];

       if(NCSCC_RC_SUCCESS != ifsv_fill_sspt_display_data(&ncs_mib_arg, &sspt_info))
          return NCSCC_RC_FAILURE;

       if((shelf == -1) && (slot == -1) && (subslot == -1))
       {
         ifsv_intf_display_dump(p_cef_data, sspt_info);
       }
       else
       {
         /* embedding subslot changes */
         if((sspt_info.sspt_info.shelf == shelf) &&
            (sspt_info.sspt_info.slot == slot) &&
            (sspt_info.sspt_info.subslot == subslot))
         ifsv_intf_display_dump(p_cef_data, sspt_info);
       }

       /* * For Processing the Next Row ........ */
       memset(&ncs_mib_arg, 0, sizeof(NCSMIB_ARG));
       ncsmib_init(&ncs_mib_arg);
       memset(space, 0, sizeof(space));
       ncsmem_aid_init(&ma, space, 1024);

       ncs_mib_arg.i_idx.i_inst_ids =  sec_idxs;
       ncs_mib_arg.i_idx.i_inst_len = sec_idx_len;
     }while(FOREVER);

     return NCSCC_RC_SUCCESS;
}


void ifsv_intf_display_dump(NCSCLI_CEF_DATA *p_cef_data,
                       NCS_IFSV_SSPT_IF_INDEX_INFO display_info)
{
   int8                    display_str[90];
   uns32                   prn_cursor=0;
   
   prn_cursor += sprintf(display_str + prn_cursor, "%-10.2d",display_info.sspt_info.shelf);/*1*/
   prn_cursor += sprintf(display_str + prn_cursor, "%-10.2d",display_info.sspt_info.slot);/*2*/
   /* embedding subslot changes */
   prn_cursor += sprintf(display_str + prn_cursor, "%-15.2d",display_info.sspt_info.subslot);/*3*/
   prn_cursor += sprintf(display_str + prn_cursor, "%-10.2d",display_info.sspt_info.port);/*4*/
   prn_cursor += sprintf(display_str + prn_cursor, "%-10.2d",display_info.sspt_info.type);/*5*/
   if(display_info.sspt_info.subscr_scope == NCS_IFSV_SUBSCR_EXT)
      prn_cursor += sprintf(display_str + prn_cursor, "%-15.11s","EXTERNAL");/*6*/
   else
      if(display_info.sspt_info.subscr_scope == NCS_IFSV_SUBSCR_INT)
         prn_cursor += sprintf(display_str + prn_cursor, "%-15.11s","INTERNAL");/*6*/
   prn_cursor += sprintf(display_str + prn_cursor, "%-10d",display_info.ifindex);/*7*/
   prn_cursor += sprintf(display_str + prn_cursor, "%-10s",display_info.if_name);/*8*/

   ifsv_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
   ifsv_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);


   ifsv_ncs_cli_done(p_cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS);
}


static uns32
ifsv_fill_sspt_display_data(NCSMIB_ARG *p_ncs_mib_arg,
                            NCS_IFSV_SSPT_IF_INDEX_INFO *p_sspt_info_display)
{
   uns32 param_cnt;
   uns32 buff;
   NCSPARM_AID rsp_pa;
   NCSMIB_PARAM_VAL param_val;
   uns8 space[1024];
   NCSMEM_AID ma;
   USRBUF *ub = NULL;

   if(p_ncs_mib_arg->rsp.info.nextrow_rsp.i_usrbuf == NULL)
      return NCSCC_RC_FAILURE;
   ub = m_MMGR_DITTO_BUFR(p_ncs_mib_arg->rsp.info.nextrow_rsp.i_usrbuf);
   if(ub == NULL)
     return NCSCC_RC_FAILURE;

   memset(space, 0, sizeof(space));
   ncsmem_aid_init(&ma, space, 1024);


    param_cnt = ncsparm_dec_init(&rsp_pa,ub);
    while(0 != param_cnt)
    {
         if (ncsparm_dec_parm(&rsp_pa,&param_val,&ma) != NCSCC_RC_SUCCESS)
            break;
         switch (param_val.i_param_id)
         {
            case ncsIfsvIfMapEntryIfIndex_ID:
            {
               buff = param_val.info.i_int;
               p_sspt_info_display->ifindex = buff;
            }
            break;

            case ncsIfsvIfMapEntryIfInfo_ID:
            {
                memcpy(p_sspt_info_display->if_name,
                                param_val.info.i_oct,
                                param_val.i_length);

            }
            break;

            default:
               break;
         }
         param_cnt--;
    }

    ncsparm_dec_done(&rsp_pa);

    return NCSCC_RC_SUCCESS;
}


static uns32
ifsv_ncs_cli_done(uns32 cli_hdl, uns32 rc)
{
   NCSCLI_OP_INFO info;

   info.i_hdl = cli_hdl;
   info.i_req = NCSCLI_OPREQ_DONE;
   info.info.i_done.i_status = rc;

   if(ncscli_opr_request(&info) != NCSCC_RC_SUCCESS)
   {
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}

static uns32
ifsv_ncs_cli_display(uns32 cli_hdl, uns8* str)
{
   NCSCLI_OP_INFO info;

   info.i_hdl = cli_hdl;
   info.i_req = NCSCLI_OPREQ_DISPLAY;
   info.info.i_display.i_str = str;

   if(ncscli_opr_request(&info) != NCSCC_RC_SUCCESS)
   {
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}


uns32
ipxs_cef_show_ip_info(NCSCLI_ARG_SET *arg_list,NCSCLI_CEF_DATA *cef_data)
{
   uns32 if_id_start;
   uns32 if_id_last;
   int8     *token;
   int8     *token1;

   NCSCLI_ARG_VAL *arg =&arg_list->i_arg_record[1];


   if(arg_list->i_arg_record[1].cmd.strval != NULL) 
   {
          token     = intf_phase_token( arg->cmd.strval, "-" );
          if_id_start = atoi(token);
         
          token1    = intf_phase_token( NULL, "-" );
          if (token1 == NULL)
          {
             return (NCSCC_RC_FAILURE);
          }
          if_id_last = atoi(token1);

          if(NCSCC_RC_SUCCESS != ipxs_process_get_row_request(cef_data, if_id_start, if_id_last))
             return NCSCC_RC_FAILURE;
   }
   else if(NCSCC_RC_SUCCESS != ipxs_process_get_row_request(cef_data, -1, -1))
     return NCSCC_RC_FAILURE;

   ifsv_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS);

   return (NCSCC_RC_SUCCESS);

}


static uns32
ipxs_process_get_row_request(NCSCLI_CEF_DATA *p_cef_data, uns32 range_start, uns32 range_end)
{
    NCS_IFSV_CLI_MODE_DATA *p_ipxs_mode_data;
    uns32 xch_id = 1;
    uns32 rc;
    uns32 rsp_status;
    uns32 sec_idxs[128]={0};
    uns32 sec_idx_len=0;
    NCSMIB_ARG ncs_mib_arg;
    uns8 space[1024];
    NCSMEM_AID ma;
    IPXS_DATA_DISPLAY ipxs_data_display;
    uns8 title_str[80];
    uns8 format_str[80];


    sprintf(title_str, "%-10.10s%-20.15s", "Ifindex","IpAddress/Mask");
    sprintf(format_str, "%s", "-----------------------");


    memset(&ncs_mib_arg, 0, sizeof(NCSMIB_ARG));
    ncsmib_init(&ncs_mib_arg);
    memset(space, 0, sizeof(space));
    ncsmem_aid_init(&ma, space, 1024);


    p_ipxs_mode_data = (NCS_IFSV_CLI_MODE_DATA *)p_cef_data->i_subsys->i_cef_mode;

     /*For Top Line To be Displayed*/
     ifsv_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
     ifsv_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)title_str);
     ifsv_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
     ifsv_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)format_str);
     /*Populating the mib arg structure*/

    ncs_mib_arg.i_idx.i_inst_ids = NULL;
    ncs_mib_arg.i_idx.i_inst_len = 0;

    do
    {
       ncs_mib_arg.i_op = NCSMIB_OP_REQ_NEXTROW;
       ncs_mib_arg.i_tbl_id = NCSMIB_TBL_IPXS_IPTBL;
       ncs_mib_arg.i_usr_key = p_cef_data->i_bindery->i_cli_hdl;
       ncs_mib_arg.i_mib_key = p_cef_data->i_bindery->i_mab_hdl;
       ncs_mib_arg.i_xch_id  = xch_id++;
       ncs_mib_arg.i_rsp_fnc = NULL;

       rc = ncsmib_sync_request(&ncs_mib_arg,
                               ncsmac_mib_request,
                               1000,
                               &ma);
       rsp_status = ncs_mib_arg.rsp.i_status;
       if((rc != NCSCC_RC_SUCCESS) || (ncs_mib_arg.rsp.i_status != NCSCC_RC_SUCCESS))
       {
          if((rc != NCSCC_RC_SUCCESS)|| (ncs_mib_arg.rsp.i_status == NCSCC_RC_NO_INSTANCE))
             return rc;
       }

       sec_idx_len = ncs_mib_arg.rsp.info.nextrow_rsp.i_next.i_inst_len;
       memcpy(sec_idxs,ncs_mib_arg.rsp.info.nextrow_rsp.i_next.i_inst_ids,
              sec_idx_len*sizeof(uns32));

       memset(&ipxs_data_display, 0, sizeof(IPXS_DATA_DISPLAY));

/*
       ~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
       *~ Fill the ipxs display data structure, one is ip address which we get
       *~ from MIB index and other is MIB parameter, which needs to be fetched
       ~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
       if(NCSCC_RC_SUCCESS != ipxs_populate_display_data(&ncs_mib_arg, &ipxs_data_display))
          return NCSCC_RC_FAILURE;


       if((range_start == -1) && (range_end == -1))
       {
          ipxs_cli_display_all(p_cef_data,ipxs_data_display);
       }
       else
       {
         if((ipxs_data_display.if_index >= range_start) && (ipxs_data_display.if_index <= range_end) )
          ipxs_cli_display_all(p_cef_data,ipxs_data_display);
       }


       /* * For Processing the Next Row ........ */
       memset(&ncs_mib_arg, 0, sizeof(NCSMIB_ARG));
       ncsmib_init(&ncs_mib_arg);
       memset(space, 0, sizeof(space));
       ncsmem_aid_init(&ma, space, 1024);

       ncs_mib_arg.i_idx.i_inst_ids =  sec_idxs;
       ncs_mib_arg.i_idx.i_inst_len = sec_idx_len;
     }while(FOREVER);

     return NCSCC_RC_SUCCESS;
}

/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME: ipxs_populate_display_data                               *~*
*~* DESCRIPTION   : This function populates the structure used for           *~*
*~*                 Displaying data obtained from the VIP database.          *~*
*~* ARGUMENTS     : NCSMIB_ARG *p_ncs_mib_arg                                *~*
*~*               : IPXS_DATA_DISPLAY *p_ipxs_data_display                   *~*
*~* RETURNS       : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                        *~*
*~* NOTES         :                                                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
static uns32
ipxs_populate_display_data(NCSMIB_ARG *p_ncs_mib_arg,
                            IPXS_DATA_DISPLAY *p_ipxs_data_display)
{
   uns32 *idxs;
   uns32 idx_len;
   uns32 buff;
   uns32 param_cnt;
   NCSPARM_AID rsp_pa;
   NCSMIB_PARAM_VAL param_val;
   uns8 space[1024];
   NCSMEM_AID ma;
   USRBUF *ub = NULL;


   if(p_ncs_mib_arg->rsp.info.nextrow_rsp.i_usrbuf == NULL)
      return NCSCC_RC_FAILURE;
   ub = m_MMGR_DITTO_BUFR(p_ncs_mib_arg->rsp.info.nextrow_rsp.i_usrbuf);
   if(ub == NULL)
     return NCSCC_RC_FAILURE;



   memset(space, 0, sizeof(space));
   ncsmem_aid_init(&ma, space, 1024);

   memset(p_ipxs_data_display, 0, sizeof(IPXS_DATA_DISPLAY));


   idx_len = p_ncs_mib_arg->rsp.info.nextrow_rsp.i_next.i_inst_len;
   idxs = (uns32 *)p_ncs_mib_arg->rsp.info.nextrow_rsp.i_next.i_inst_ids;

   if(idx_len > 0)
   {
       ((uns8 *)(&p_ipxs_data_display->ipaddress))[0] = (uns8)idxs[1];
       ((uns8 *)(&p_ipxs_data_display->ipaddress))[1] = (uns8)idxs[2];
       ((uns8 *)(&p_ipxs_data_display->ipaddress))[2] = (uns8)idxs[3];
       ((uns8 *)(&p_ipxs_data_display->ipaddress))[3] = (uns8)idxs[4];

       /*
        * Populate the Userbuff Information into the DATA Destructure declared at the TOP
       */
       param_cnt =
          ncsparm_dec_init(&rsp_pa,ub);
       while(0 != param_cnt)
       {
            if (ncsparm_dec_parm(&rsp_pa,&param_val,&ma) != NCSCC_RC_SUCCESS)
               break;
            switch (param_val.i_param_id)
            {
               case ncsIfsvIpEntIfIndex_ID:
               {
                  buff = param_val.info.i_int;
                  p_ipxs_data_display->if_index = buff;
               }
               break;
               case ncsIfsvIpEntAddrMask_ID:
               {
                  buff = param_val.info.i_int;
                  p_ipxs_data_display->mask = m_NCS_OS_NTOHL(buff);
               }
               break;

               default:
               break;
           }/*Switch*/

           param_cnt--;
       }/*while*/
    }
    else
    {
       return NCSCC_RC_FAILURE;
    }

    ncsparm_dec_done(&rsp_pa);

    return NCSCC_RC_SUCCESS;
}

static void
ipxs_cli_display_all(NCSCLI_CEF_DATA *p_cef_data,
                       IPXS_DATA_DISPLAY ipxs_data_display)
{
   int8                    display_str[200];
   int8                    ip_net_str[30]; /*More than 20 for safety*/
   uns32                   prfx_prn_len;
   uns32                   prn_cursor;

   /* Conversion Of IPAddress into a string*/
   sprintf(ip_net_str, "%d.%d.%d.%d/%d",
      ((uns8*)(&ipxs_data_display.ipaddress))[0], /*This is max 3 chars */
      ((uns8*)(&ipxs_data_display.ipaddress))[1],
      ((uns8*)(&ipxs_data_display.ipaddress))[2],
      ((uns8*)(&ipxs_data_display.ipaddress))[3],
      (ipxs_data_display.mask));

   prfx_prn_len = sprintf(display_str, "%-10d",ipxs_data_display.if_index);/*1*/
     /*
      * Set the position where next field is to be printed
      * */
   prn_cursor = prfx_prn_len;

   prn_cursor += sprintf(display_str + prn_cursor, "%-20.20s ",ip_net_str);

   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
   if(NCSCC_RC_SUCCESS !=
      vip_ncs_cli_done(p_cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS))
   {
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_CLI, IFSV_VIP_CLI_DONE_FAILURE);
   }
}

#else

extern int dummy; 
#endif /** NCS_MAB ==1 , NCS_PSR ==1 , NCS_CLI ==1 **/
