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

  Source file for PSR CLI registration function and PSR CEFs

******************************************************************************
*/

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        Common Include Files.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#include "mab.h"
#include "ncs_cli.h"
#include "psrcli.h"
#include "psr_mapi.h"
#include "psr_api.h"
#include "psr_papi.h"
#include "psr_pvt.h"

static uns32 pss_cli_done(uns32 cli_hdl, uns32 rc);
static uns32 pss_cli_display(uns32 cli_hdl, uns8* str);


/****************************************************************************\
*  Name:          ncspss_cef_load_lib_req                                    * 
*                                                                            *
*  Description:   To load the CEFs into NCS CLI process                      *
*                                                                            *
*  Arguments:     NCS_LIB_REQ_INFO - Information about the SE API operation  *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
*  NOTE:                                                                     * 
\****************************************************************************/
uns32 ncspss_cef_load_lib_req(NCS_LIB_REQ_INFO *libreq)
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
        return ncspss_cli_register(&i_bindery);
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
  
  PROCEDURE NAME:    ncspss_cli_register()

  DESCRIPTION:       This function can be invoked by the AKE to register the 
                     PSR commands and associated CEFs with CLI.

  ARGUMENTS:
   NCSCLI_BINDERY *   Pointer to NCSCLI_i_bindery Structure
   
  RETURNS:           Success/Failure
   none

  NOTES:        

*****************************************************************************/

uns32 ncspss_cli_register(NCSCLI_BINDERY  *i_bindery)
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
     * PSS commands belongs to 4 command modes 
     * 1. Exec Mode
     * 2. Config Mode   
     */
   
    /* REGISTER THE EXEC MODE COMMANDS */
    memset(&data, 0, sizeof(NCSCLI_CMD_LIST));

    data.i_node = "root/exec/";
    data.i_command_mode = "exec";
    data.i_access_req = FALSE;
    data.i_access_passwd = 0;


    /************** REGISTER COMMANDS IN THE "CONFIG" MODE **************/
    data.i_node = "root/exec/config";
    data.i_command_mode = "config";
    data.i_access_req = FALSE;        /* Password protected = FALSE */
    data.i_access_passwd = 0;         /* Password function, required 
                                         if password protected is set */
    cmd_count = 0;
    data.i_command_list[cmd_count].i_cmdstr = "pssv!Configure Persistent Store-Restore (PSSv) service! \
                                  @root/exec/config/pss@";
    data.i_command_list[cmd_count++].i_cmd_exec_func = pss_cef_configure_pssv;
    data.i_cmd_count = cmd_count;   /* Number of commands registered. */

    info.i_hdl = i_bindery->i_cli_hdl;
    info.i_req = NCSCLI_OPREQ_REGISTER;
    info.info.i_register.i_bindery = i_bindery;
    info.info.i_register.i_cmdlist = &data;
    if(NCSCC_RC_SUCCESS != ncscli_opr_request(&info)) return NCSCC_RC_FAILURE;    
    

    /************** REGISTER COMMANDS IN THE "configure" MODE **************/
    memset(&data, 0, sizeof(NCSCLI_CMD_LIST));
    data.i_node = "root/exec/config/pss";  /* Set path for the commands*/
    data.i_command_mode = "pssv";   /* Set the mode */
    data.i_access_req = FALSE;        /* Password protected = FALSE */
    data.i_access_passwd = 0;         /* Password function, required 
                                         if password protected is set */
    cmd_count = 0;

    /* List command */
    data.i_command_list[cmd_count].i_cmdstr = "list !Display! \
                                               profiles!Profile Details!";
    data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
    data.i_command_list[cmd_count++].i_cmd_exec_func = pss_cef_list_profiles;

    /* Copy Profile command */
    data.i_command_list[cmd_count].i_cmdstr = "copy !Copy Profile! \
                                   profile !Keyword!\
                                   NCSCLI_STRING !Existing Profile! \
                                   NCSCLI_STRING !New Profile!";
    data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_ADMIN_ACCESS;
    data.i_command_list[cmd_count++].i_cmd_exec_func = pss_cef_copy_profile;

    /* Rename Profile command */
    data.i_command_list[cmd_count].i_cmdstr = "rename !Rename Profile! \
                                   profile !Keyword!\
                                   NCSCLI_STRING !Existing Profile! \
                                   NCSCLI_STRING !New Profile Name!";
    data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_ADMIN_ACCESS;
    data.i_command_list[cmd_count++].i_cmd_exec_func = pss_cef_rename_profile;

    /* Delete Profile command */
    data.i_command_list[cmd_count].i_cmdstr = "delete !Delete Profile! \
                                               profile !Keyword!\
                                               NCSCLI_STRING !Profile Name!";
    data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_ADMIN_ACCESS;
    data.i_command_list[cmd_count++].i_cmd_exec_func = pss_cef_delete_profile;

    /* Save Profile command */
    data.i_command_list[cmd_count].i_cmdstr = 
                    "save !Save Profile to Current Config or a named profile! \
                     profile !Keyword!\
                     [NCSCLI_STRING !Profile Name!]";
    data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_ADMIN_ACCESS;
    data.i_command_list[cmd_count++].i_cmd_exec_func = pss_cef_save_profile;

    /* Describe Profile command */
    data.i_command_list[cmd_count].i_cmdstr = "describe !Describe Profile! \
                                   profile !Keyword!\
                                   NCSCLI_STRING !Profile Name!\
                                   NCSCLI_WILDCARD !Profile Description!";
    data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_ADMIN_ACCESS;
    data.i_command_list[cmd_count++].i_cmd_exec_func = pss_cef_describe_profile;

    /* Load Profile command */
    data.i_command_list[cmd_count].i_cmdstr = "load !Load Profile! \
                                               profile !Keyword!\
                                               NCSCLI_STRING !Profile Name!";
    data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_ADMIN_ACCESS;
    data.i_command_list[cmd_count++].i_cmd_exec_func = pss_cef_load_profile;

    /* Replace Profile command */
    data.i_command_list[cmd_count].i_cmdstr = "replace-current-profile !Replace Current Profile!\
                                               NCSCLI_STRING !Alternate Profile Name!";
    data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_ADMIN_ACCESS;
    data.i_command_list[cmd_count++].i_cmd_exec_func = pss_cef_replace_current_profile;

    /* set-playback-option-from-xml-config command */
    data.i_command_list[cmd_count].i_cmdstr = 
            "set !Set!  playback-option-from-xml-config !Keyword!\
                              NCSCLI_STRING !PCN name, upto 255 characters!";
    data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_ADMIN_ACCESS;
    data.i_command_list[cmd_count++].i_cmd_exec_func = 
                        pss_cef_set_playback_option_from_xml_config;

    /* dump profile command */
    data.i_command_list[cmd_count].i_cmdstr = 
            "dump!Dump MIB entries of a specified client in the profile!  profile !Keyword!\
                    NCSCLI_STRING !profile-name! \
                    NCSCLI_STRING !Log-file-name(absolute path, where to dump entries)!\
                    NCSCLI_STRING !PCN-name, upto 255 characters!";
    data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
    data.i_command_list[cmd_count++].i_cmd_exec_func = 
                        pss_cef_dump_profile;

    /* show profile-clients command */
    data.i_command_list[cmd_count].i_cmdstr = 
            "show!Show registered clients in the profile!  profile-clients !Keyword!\
                              NCSCLI_STRING !profile-name!";
    data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
    data.i_command_list[cmd_count++].i_cmd_exec_func = 
                        pss_cef_show_profile_clients;

    /* reload pssv_lib_conf config file command */
    data.i_command_list[cmd_count].i_cmdstr = 
            "reload!Reload the config file!  pssv_lib_conf-config !Keyword!";
    data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_ADMIN_ACCESS;
    data.i_command_list[cmd_count++].i_cmd_exec_func = 
                        pss_cef_reload_pssv_lib_conf;

    /* reload pssv_spcn_list config file command */
    data.i_command_list[cmd_count].i_cmdstr = 
            "reload!Reload the config file!  pssv_spcn_list-config !Keyword!";
    data.i_command_list[cmd_count].i_cmd_access_level = NCSCLI_ADMIN_ACCESS;
    data.i_command_list[cmd_count++].i_cmd_exec_func = 
                        pss_cef_reload_pssv_spcn_list;

    data.i_cmd_count = cmd_count;   /* Number of commands registered. */

    info.info.i_register.i_cmdlist = &data;
    if(NCSCC_RC_SUCCESS != ncscli_opr_request(&info)) return NCSCC_RC_FAILURE;   

    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME: pss_cef_configure_pssv

  DESCRIPTION   : Tells the CLI to enter configure-pssv mode
    
  ARGUMENTS     :
      arg_list  : Arguments list given in command
      i_cef_data: command related information

  RETURNS: 
      NCSCC_RC_SUCCESS
      NCSCC_RC_FAILURE

  NOTES:

*****************************************************************************/
uns32 pss_cef_configure_pssv(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *data)
{
   return pss_cli_done(data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS);   
}

static uns32 pss_cli_done(uns32 cli_hdl, uns32 rc)
{
   NCSCLI_OP_INFO info;
   
   info.i_hdl = cli_hdl;
   info.i_req = NCSCLI_OPREQ_DONE;
   info.info.i_done.i_status = rc;    
   
   ncscli_opr_request(&info);   
   return NCSCC_RC_SUCCESS;

}

static uns32 pss_cli_display(uns32 cli_hdl, uns8* str)
{
   NCSCLI_OP_INFO info;
   
   info.i_hdl = cli_hdl;
   info.i_req = NCSCLI_OPREQ_DISPLAY;
   info.info.i_display.i_str = str;    
   
   ncscli_opr_request(&info);   
   return NCSCC_RC_SUCCESS;

}

/*****************************************************************************

  PROCEDURE NAME: pss_cef_set_playback_option_from_xml_config

  DESCRIPTION   : Sends the MIB requests to set the XML playback option for SPCN
  ARGUMENTS     :
      arg_list  : Argument list given in command.  
      cef_data  : command related data. 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cef_set_playback_option_from_xml_config(NCSCLI_ARG_SET *arg_list,
                                                  NCSCLI_CEF_DATA *cef_data)
{
    NCSMIB_ARG           mib_arg;
    NCS_UBAID            lcl_uba;
    uns8                 *buff_ptr = NULL;
    char                 *str_ptr = NULL;
    uns32                str_len = 0;
    
    memset(&mib_arg, '\0', sizeof(mib_arg));
    
    mib_arg.i_op = NCSMIB_OP_REQ_CLI;    /* Operation type */
    mib_arg.i_tbl_id = NCSMIB_TBL_PSR_CMD; /* CMD-TBL-ID */
    mib_arg.i_rsp_fnc = pss_cli_cmd_mib_resp_set_playback_option_from_xml;
    mib_arg.i_idx.i_inst_ids = NULL;   /* inst_id framed from SNMPTM svc_name & instance_id */
    mib_arg.i_idx.i_inst_len = 0; 
    ncsstack_init(&mib_arg.stack, NCSMIB_STACK_SIZE);
    
    mib_arg.req.info.cli_req.i_cmnd_id = PSS_CMD_TBL_CMD_NUM_SET_XML_OPTION;  /* Command id */
    mib_arg.req.info.cli_req.i_wild_card = FALSE; /* set to TRUE for wild-card req */

    memset(&lcl_uba, '\0', sizeof(lcl_uba));
    if(ncs_enc_init_space(&lcl_uba) != NCSCC_RC_SUCCESS)
    {
      m_NCS_CONS_PRINTF("\n\npss_cef_set_playback_option_from_xml_config(): USRBUF malloc failed");
      pss_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
    }
    m_BUFR_STUFF_OWNER(lcl_uba.start);

    str_ptr = arg_list->i_arg_record[2].cmd.strval;
    str_len = strlen(str_ptr);
    if(str_len > 255)
    {
      m_NCS_CONS_PRINTF("\n\npss_cef_set_playback_option_from_xml_config(): PCN length more than 255 characters....");
      pss_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      m_MMGR_FREE_BUFR_LIST(lcl_uba.start);
      return NCSCC_RC_FAILURE;
    }
    buff_ptr = ncs_enc_reserve_space(&lcl_uba, sizeof(uns16));
    if (buff_ptr == NULL)
    {
      m_NCS_CONS_PRINTF("\n\npss_cef_set_playback_option_from_xml_config(): reserve space for uns16 failed");
      pss_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      m_MMGR_FREE_BUFR_LIST(lcl_uba.start);
      return NCSCC_RC_FAILURE;
    }
    ncs_encode_16bit(&buff_ptr, (uns16)str_len);
    ncs_enc_claim_space(&lcl_uba, sizeof(uns16));

    if(ncs_encode_n_octets_in_uba(&lcl_uba, str_ptr, str_len) != NCSCC_RC_SUCCESS)
    {
      m_NCS_CONS_PRINTF("\n\npss_cef_set_playback_option_from_xml_config(): encode_n_octets_in_uba for the PCN name failed");
      pss_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      m_MMGR_FREE_BUFR_LIST(lcl_uba.start);
      return NCSCC_RC_FAILURE;
    }

    mib_arg.req.info.cli_req.i_usrbuf = lcl_uba.start;
    
    /* send the request */
    if ((pss_send_cmd_mib_req(&mib_arg, cef_data)) == NCSCC_RC_FAILURE)
        return NCSCC_RC_FAILURE;
    
    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME: pss_cef_dump_profile

  DESCRIPTION   : Sends the MIB requests to dump the profile table entries
                  for a specified client.

  ARGUMENTS     :
      arg_list  : Argument list given in command.  
      cef_data  : command related data. 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cef_dump_profile(NCSCLI_ARG_SET *arg_list,
                                                  NCSCLI_CEF_DATA *cef_data)
{
    NCSMIB_ARG           mib_arg;
    FILE                 *fh = NULL;
    NCS_UBAID            lcl_uba;
    uns8                 *buff_ptr = NULL;
    char                 *str_ptr = NULL, *file_name = NULL, *pcn_name = NULL;
    uns32                str_len = 0, file_str_len = 0, pcn_str_len = 0;
    
    memset(&mib_arg, '\0', sizeof(mib_arg));
    
    mib_arg.i_op = NCSMIB_OP_REQ_CLI;    /* Operation type */
    mib_arg.i_tbl_id = NCSMIB_TBL_PSR_CMD; /* CMD-TBL-ID */
    mib_arg.i_rsp_fnc = pss_cli_cmd_mib_resp_dump_profile;
    mib_arg.i_idx.i_inst_ids = NULL;   /* inst_id framed from SNMPTM svc_name & instance_id */
    mib_arg.i_idx.i_inst_len = 0; 
    ncsstack_init(&mib_arg.stack, NCSMIB_STACK_SIZE);
    
    mib_arg.req.info.cli_req.i_cmnd_id = PSS_CMD_TBL_CMD_NUM_DISPLAY_MIB;  /* Command id */
    mib_arg.req.info.cli_req.i_wild_card = FALSE; /* set to TRUE for wild-card req */

    str_ptr = arg_list->i_arg_record[2].cmd.strval;
    file_name = arg_list->i_arg_record[3].cmd.strval;
    pcn_name = arg_list->i_arg_record[4].cmd.strval;
    str_len = strlen(str_ptr);
    file_str_len = strlen(file_name);
    pcn_str_len = strlen(pcn_name);

    fh = sysf_fopen(file_name, "a+");
    if(fh == NULL)
    {
       m_NCS_CONS_PRINTF("\n\npss_cef_dump_profile(): Can't open file %s in append mode...\n\n", str_ptr);
       pss_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
       return NCSCC_RC_FAILURE;
    }
    sysf_fclose(fh);

    memset(&lcl_uba, '\0', sizeof(lcl_uba));
    if(ncs_enc_init_space(&lcl_uba) != NCSCC_RC_SUCCESS)
    {
      m_NCS_CONS_PRINTF("\n\npss_cef_dump_profile(): USRBUF malloc failed");
      pss_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
    }
    m_BUFR_STUFF_OWNER(lcl_uba.start);

    buff_ptr = ncs_enc_reserve_space(&lcl_uba, sizeof(uns16));
    if (buff_ptr == NULL)
    {
      m_NCS_CONS_PRINTF("\n\npss_cef_dump_profile(): reserve space for uns16 failed");
      pss_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      m_MMGR_FREE_BUFR_LIST(lcl_uba.start);
      return NCSCC_RC_FAILURE;
    }
    ncs_encode_16bit(&buff_ptr, (uns16)str_len);
    ncs_enc_claim_space(&lcl_uba, sizeof(uns16));

    if(ncs_encode_n_octets_in_uba(&lcl_uba, str_ptr, str_len) != NCSCC_RC_SUCCESS)
    {
      m_NCS_CONS_PRINTF("\n\npss_cef_dump_profile(): encode_n_octets_in_uba for the Profile name failed");
      pss_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      m_MMGR_FREE_BUFR_LIST(lcl_uba.start);
      return NCSCC_RC_FAILURE;
    }

    buff_ptr = ncs_enc_reserve_space(&lcl_uba, sizeof(uns16));
    if (buff_ptr == NULL)
    {
      m_NCS_CONS_PRINTF("\n\npss_cef_dump_profile(): reserve space for uns16 failed");
      pss_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      m_MMGR_FREE_BUFR_LIST(lcl_uba.start);
      return NCSCC_RC_FAILURE;
    }
    ncs_encode_16bit(&buff_ptr, (uns16)file_str_len);
    ncs_enc_claim_space(&lcl_uba, sizeof(uns16));

    if(ncs_encode_n_octets_in_uba(&lcl_uba, file_name, file_str_len) != NCSCC_RC_SUCCESS)
    {
      m_NCS_CONS_PRINTF("\n\npss_cef_dump_profile(): encode_n_octets_in_uba for the LOG file name failed");
      pss_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      m_MMGR_FREE_BUFR_LIST(lcl_uba.start);
      return NCSCC_RC_FAILURE;
    }

    buff_ptr = ncs_enc_reserve_space(&lcl_uba, sizeof(uns16));
    if (buff_ptr == NULL)
    {
      m_NCS_CONS_PRINTF("\n\npss_cef_dump_profile(): reserve space for uns16 failed");
      pss_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      m_MMGR_FREE_BUFR_LIST(lcl_uba.start);
      return NCSCC_RC_FAILURE;
    }
    ncs_encode_16bit(&buff_ptr, (uns16)pcn_str_len);
    ncs_enc_claim_space(&lcl_uba, sizeof(uns16));

    if(ncs_encode_n_octets_in_uba(&lcl_uba, pcn_name, pcn_str_len) != NCSCC_RC_SUCCESS)
    {
      m_NCS_CONS_PRINTF("\n\npss_cef_dump_profile(): encode_n_octets_in_uba for the PCN name failed");
      pss_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      m_MMGR_FREE_BUFR_LIST(lcl_uba.start);
      return NCSCC_RC_FAILURE;
    }

    mib_arg.req.info.cli_req.i_usrbuf = lcl_uba.start;

    /* send the request */
    if ((pss_send_cmd_mib_req(&mib_arg, cef_data)) == NCSCC_RC_FAILURE)
    {
        m_MMGR_FREE_BUFR_LIST(lcl_uba.start);
        return NCSCC_RC_FAILURE;
    }

    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME: pss_cef_show_profile_clients

  DESCRIPTION   : Sends the MIB requests to dump the clients present in a profile. 

  ARGUMENTS     :
      arg_list  : Argument list given in command.  
      cef_data  : command related data. 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cef_show_profile_clients(NCSCLI_ARG_SET *arg_list,
                                   NCSCLI_CEF_DATA *cef_data)
{
    NCSMIB_ARG           mib_arg;
    NCS_UBAID            lcl_uba;
    uns8                 *buff_ptr = NULL;
    char                 *str_ptr = NULL;
    uns32                str_len = 0;
    
    memset(&mib_arg, '\0', sizeof(mib_arg));
    mib_arg.i_op = NCSMIB_OP_REQ_CLI;    /* Operation type */
    mib_arg.i_tbl_id = NCSMIB_TBL_PSR_CMD; /* CMD-TBL-ID */
    mib_arg.i_rsp_fnc = pss_cli_cmd_mib_resp_show_profile_clients;
    mib_arg.i_idx.i_inst_ids = NULL;   /* inst_id framed from SNMPTM svc_name & instance_id */
    mib_arg.i_idx.i_inst_len = 0; 
    ncsstack_init(&mib_arg.stack, NCSMIB_STACK_SIZE);
    
    mib_arg.req.info.cli_req.i_cmnd_id = PSS_CMD_TBL_CMD_NUM_DISPLAY_PROFILE;  /* Command id */
    mib_arg.req.info.cli_req.i_wild_card = FALSE; /* set to TRUE for wild-card req */

    memset(&lcl_uba, '\0', sizeof(lcl_uba));
    if(ncs_enc_init_space(&lcl_uba) != NCSCC_RC_SUCCESS)
    {
      m_NCS_CONS_PRINTF("\n\npss_cef_show_profile_clients(): USRBUF malloc failed");
      pss_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
    }
    m_BUFR_STUFF_OWNER(lcl_uba.start);

    str_ptr = arg_list->i_arg_record[2].cmd.strval;
    str_len = strlen(str_ptr) + 1;
    buff_ptr = ncs_enc_reserve_space(&lcl_uba, sizeof(uns16));
    if (buff_ptr == NULL)
    {
      m_NCS_CONS_PRINTF("\n\npss_cef_show_profile_clients(): reserve space for uns16 failed");
      pss_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
    }
    ncs_encode_16bit(&buff_ptr, (uns16)str_len);
    ncs_enc_claim_space(&lcl_uba, sizeof(uns16));

    if(ncs_encode_n_octets_in_uba(&lcl_uba, str_ptr, str_len) != NCSCC_RC_SUCCESS)
    {
      m_NCS_CONS_PRINTF("\n\npss_cef_show_profile_clients(): encode_n_octets_in_uba for the profile-name failed");
      pss_cli_done(cef_data->i_bindery->i_cli_hdl, NCSCC_RC_FAILURE);
      return NCSCC_RC_FAILURE;
    }

    mib_arg.req.info.cli_req.i_usrbuf = lcl_uba.start;
    
    /* send the request */
    if ((pss_send_cmd_mib_req(&mib_arg, cef_data)) == NCSCC_RC_FAILURE)
        return NCSCC_RC_FAILURE;
    
    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME: pss_cef_reload_pssv_lib_conf

  DESCRIPTION   : Sends the request to reload the OSAF_SYSCONFDIR/pssv_lib_conf
                  configuration file.

  ARGUMENTS     :
      arg_list  : Argument list given in command.  
      cef_data  : command related data. 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cef_reload_pssv_lib_conf(NCSCLI_ARG_SET *arg_list,
                                   NCSCLI_CEF_DATA *cef_data)
{
    NCSMIB_ARG            ncsmib_arg;
    uns8                 space[2048];
    NCSMEM_AID            ma;
    uns32                xch_id = 1;
    uns32                retval = NCSCC_RC_SUCCESS, ret_val;
    char                 display_str[PSS_DISPLAY_STR_SIZE+1];
    uns32                inst_ids[NCS_PSS_MAX_PROFILE_NAME+1];

    /* Initialize the MIB arg structure */
    memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
    ncsmib_init(&ncsmib_arg);
    ncsmem_aid_init(&ma, space, sizeof(space));

    pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");

    /* Trigger the reload operation */
    ncsmib_init(&ncsmib_arg);
    ncsmem_aid_init(&ma, space, sizeof(space));
    ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
    ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
    ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
    ncsmib_arg.i_rsp_fnc = NULL;
    ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
    ncsmib_arg.i_idx.i_inst_len = 0;
    ncsmib_arg.i_xch_id  = xch_id++;
    ncsmib_arg.i_tbl_id  = NCSMIB_SCLR_PSR_TRIGGER;
    ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
    ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvTriggerOperation_ID;
    ncsmib_arg.req.info.set_req.i_param_val.info.i_int = ncsPSSvTriggerReloadLibConf;
    retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 10000, &ma);
    if (retval != NCSCC_RC_SUCCESS)
    {
       ret_val = m_MAB_DBG_SINK(retval);
       sysf_sprintf(display_str, "MIB Request failed\n");
       pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
       goto end;
    }

    if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       sysf_sprintf(display_str, "MIB Request failed\n");
       pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
       goto end;
    }

    if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       sysf_sprintf(display_str, "MIB Request failed\n");
       pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
       goto end;
    }
    sysf_sprintf(display_str, "Reload pssv_lib_conf operation success\n");
    pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);

end:

    return pss_cli_done(cef_data->i_bindery->i_cli_hdl, retval);  
}

/*****************************************************************************

  PROCEDURE NAME: pss_cef_reload_pssv_spcn_list

  DESCRIPTION   : Sends the request to reload the OSAF_LOCALSTATEDIR/pssv_spcn_list
                  configuration file.

  ARGUMENTS     :
      arg_list  : Argument list given in command.  
      cef_data  : command related data. 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cef_reload_pssv_spcn_list(NCSCLI_ARG_SET *arg_list,
                                   NCSCLI_CEF_DATA *cef_data)
{
    NCSMIB_ARG            ncsmib_arg;
    uns8                 space[2048];
    NCSMEM_AID            ma;
    uns32                xch_id = 1;
    uns32                retval = NCSCC_RC_SUCCESS, ret_val;
    char                 display_str[PSS_DISPLAY_STR_SIZE+1];
    uns32                inst_ids[NCS_PSS_MAX_PROFILE_NAME+1];

    /* Initialize the MIB arg structure */
    memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
    ncsmib_init(&ncsmib_arg);
    ncsmem_aid_init(&ma, space, sizeof(space));

    pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");

    /* Trigger the reload operation */
    ncsmib_init(&ncsmib_arg);
    ncsmem_aid_init(&ma, space, sizeof(space));
    ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
    ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
    ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
    ncsmib_arg.i_rsp_fnc = NULL;
    ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
    ncsmib_arg.i_idx.i_inst_len = 0;
    ncsmib_arg.i_xch_id  = xch_id++;
    ncsmib_arg.i_tbl_id  = NCSMIB_SCLR_PSR_TRIGGER;
    ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
    ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvTriggerOperation_ID;
    ncsmib_arg.req.info.set_req.i_param_val.info.i_int = ncsPSSvTriggerReloadSpcnList;
    retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 10000, &ma);
    if (retval != NCSCC_RC_SUCCESS)
    {
       ret_val = m_MAB_DBG_SINK(retval);
       sysf_sprintf(display_str, "MIB Request failed\n");
       pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
       goto end;
    }

    if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       sysf_sprintf(display_str, "MIB Request failed\n");
       pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
       goto end;
    }

    if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       sysf_sprintf(display_str, "MIB Request failed\n");
       pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
       goto end;
    }
    sysf_sprintf(display_str, "Reload pssv_spcn_list operation success\n");
    pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);

end:

    return pss_cli_done(cef_data->i_bindery->i_cli_hdl, retval);  
}

/*****************************************************************************

  PROCEDURE NAME: pss_cli_mib_resp

  DESCRIPTION   : Response function for PSS Profile MIB CLI commands
  ARGUMENTS     :
      resp      : response structure pointer, of type NCSMIB_ARG 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cli_mib_resp(NCSMIB_ARG *resp)
{
    /* Get the pointer for the command data in CLI control block */
    if (resp->i_usr_key == 0)
        return NCSCC_RC_FAILURE;
    
    switch(resp->rsp.i_status)
    {
    case NCSCC_RC_SUCCESS:
        m_NCS_CONS_PRINTF("\nCLI Command Failed");
        pss_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);

    default:
        m_NCS_CONS_PRINTF("\nCLI Command Failed");
        pss_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
        break;
    }

    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME: pss_cli_cmd_mib_resp_show_profile_clients

  DESCRIPTION   : Response function for PSS CLI command "show profile-clients"
  ARGUMENTS     :
      resp      : response structure pointer, of type NCSMIB_ARG 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cli_cmd_mib_resp_show_profile_clients(NCSMIB_ARG *resp)
{
    uns8 rsp_completed = FALSE;
    
    /* Get the pointer for the command data in CLI control block */
    if (resp->i_usr_key == 0)
        return NCSCC_RC_FAILURE;
    
    /* For a wild-card request, close the CLI session once it receives the 
       response with NCSMIB_OP_RSP_CLI_DONE operation. For an Exact request,
       close the CLI session once it receives the response with o_partial flag
       set to TRUE */ 
    if ((resp->i_op == NCSMIB_OP_RSP_CLI_DONE) ||
        ((resp->i_op == NCSMIB_OP_RSP_CLI) && 
        (resp->rsp.info.cli_rsp.o_partial == FALSE)))
        rsp_completed = TRUE;
    
    switch(resp->rsp.i_status)
    {
    case NCSCC_RC_FAILURE:
        m_NCS_CONS_PRINTF("\nThe CLI command show profile-clients failed...");
        pss_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
        return NCSCC_RC_SUCCESS;
        break;
        
    case NCSCC_RC_SUCCESS:
        {
            USRBUF   *buff =m_MMGR_DITTO_BUFR(resp->rsp.info.cli_rsp.o_answer); 
            uns32    buff_len = 0;
            uns8     *buff_ptr = NULL;
            int8     pcn[NCSMIB_PCN_LENGTH_MAX];
            uns16    pwe_cnt = 0, pwe_id = 0, pcn_cnt = 0, str_len = 0, j = 0, k = 0;
            NCS_UBAID lcl_uba; 
            
            if(resp->rsp.info.cli_rsp.i_cmnd_id == PSS_CMD_TBL_CMD_NUM_DISPLAY_PROFILE)
            {
               /* Decode the USRBUF, and display the clients information to the user. */
               buff_len = m_MMGR_LINK_DATA_LEN(buff);
               if(buff_len == 0)
               {
                  m_NCS_CONS_PRINTF("\nNo clients found for the profile...");
                  pss_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
                  return NCSCC_RC_FAILURE;
               } 
               memset(&pcn, '\0', sizeof(pcn));
               memset(&lcl_uba, '\0', sizeof(lcl_uba));
               ncs_dec_init_space(&lcl_uba, buff);

               buff_ptr = ncs_dec_flatten_space(&lcl_uba, (uns8*)&pwe_cnt, sizeof(uns16));
               if(buff_ptr == NULL)
               {
                  m_NCS_CONS_PRINTF("\npayload decode failed...");
                  pss_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
                  return NCSCC_RC_FAILURE;
               } 
               pwe_cnt = ncs_decode_16bit(&buff_ptr);
               ncs_dec_skip_space(&lcl_uba, sizeof(uns16));

               m_NCS_CONS_PRINTF("\n\n***Registered clients are shown below :***\n");

               for(j = 0; j < pwe_cnt; j++)
               { 
                  buff_ptr = ncs_dec_flatten_space(&lcl_uba, (uns8*)&pwe_id, sizeof(uns16));
                  if(buff_ptr == NULL)
                  {
                     m_NCS_CONS_PRINTF("\npayload decode failed...");
                     pss_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
                     return NCSCC_RC_FAILURE;
                  } 
                  pwe_id = ncs_decode_16bit(&buff_ptr);
                  ncs_dec_skip_space(&lcl_uba, sizeof(uns16));
                  m_NCS_CONS_PRINTF("\n\t PWE_ID = %d\n", pwe_id);  

                  buff_ptr = ncs_dec_flatten_space(&lcl_uba, (uns8*)&pcn_cnt, sizeof(uns16));
                  if(buff_ptr == NULL)
                  {
                     m_NCS_CONS_PRINTF("\npayload decode failed...");
                     pss_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
                     return NCSCC_RC_FAILURE;
                  } 
                  pcn_cnt = ncs_decode_16bit(&buff_ptr);
                  ncs_dec_skip_space(&lcl_uba, sizeof(uns16));

                  for(k = 0; k < pcn_cnt; k++)
                  { 
                     buff_ptr = ncs_dec_flatten_space(&lcl_uba, (uns8*)&str_len, sizeof(uns16));
                     if(buff_ptr == NULL)
                     {
                        m_NCS_CONS_PRINTF("\npayload decode failed...");
                        pss_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
                        return NCSCC_RC_FAILURE;
                     } 
                     str_len = ncs_decode_16bit(&buff_ptr);
                     ncs_dec_skip_space(&lcl_uba, sizeof(uns16));
                     if(ncs_decode_n_octets_from_uba(&lcl_uba, (char*)&pcn, str_len) != NCSCC_RC_SUCCESS)
                     {
                        m_NCS_CONS_PRINTF("\npayload decode failed...");
                        pss_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
                        return NCSCC_RC_FAILURE;
                     }
                     m_NCS_CONS_PRINTF("\t\t - %s\n", (char*)&pcn);  
                  } 
               } 
            }
       }
       break;
       
   default:
       break;
   }
   /* Request processing is completes */
   if (rsp_completed == TRUE)
   {
       pss_cli_done(resp->i_usr_key, NCSCC_RC_SUCCESS);
   }
   
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME: pss_cli_cmd_mib_resp_set_playback_option_from_xml

  DESCRIPTION   : Response function for PSS CLI command
                  "set playback-option-from-xml-config"
  ARGUMENTS     :
      resp      : response structure pointer, of type NCSMIB_ARG 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cli_cmd_mib_resp_set_playback_option_from_xml(NCSMIB_ARG *resp)
{
    uns8 rsp_completed = FALSE;
    
    /* Get the pointer for the command data in CLI control block */
    if (resp->i_usr_key == 0)
        return NCSCC_RC_FAILURE;
    
    /* For a wild-card request, close the CLI session once it receives the 
       response with NCSMIB_OP_RSP_CLI_DONE operation. For an Exact request,
       close the CLI session once it receives the response with o_partial flag
       set to TRUE */ 
    if ((resp->i_op == NCSMIB_OP_RSP_CLI_DONE) ||
        ((resp->i_op == NCSMIB_OP_RSP_CLI) && 
        (resp->rsp.info.cli_rsp.o_partial == FALSE)))
        rsp_completed = TRUE;

    switch(resp->rsp.i_status)
    {
    case NCSCC_RC_FAILURE:
        m_NCS_CONS_PRINTF("\nFailed to update SPCN entry in " OSAF_LOCALSTATEDIR "pssv_spcn_list file...\n");
        pss_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
        return NCSCC_RC_SUCCESS;
        break;
        
    case NCSCC_RC_SUCCESS:
        m_NCS_CONS_PRINTF("\nSPCN Entry updated in " OSAF_LOCALSTATEDIR "pssv_spcn_list file...\n");
        break;
       
    default:
        break;
    }
    /* Request processing is completes */
    if (rsp_completed == TRUE)
    {
       pss_cli_done(resp->i_usr_key, NCSCC_RC_SUCCESS);
    }

    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME: pss_cli_cmd_mib_resp_dump_profile

  DESCRIPTION   : Response function for PSS CLI command
                  "dump profile"
  ARGUMENTS     :
      resp      : response structure pointer, of type NCSMIB_ARG

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cli_cmd_mib_resp_dump_profile(NCSMIB_ARG *resp)
{
    uns8 rsp_completed = FALSE;

    /* Get the pointer for the command data in CLI control block */
    if (resp->i_usr_key == 0)
        return NCSCC_RC_FAILURE;

    /* For a wild-card request, close the CLI session once it receives the
       response with NCSMIB_OP_RSP_CLI_DONE operation. For an Exact request,
       close the CLI session once it receives the response with o_partial flag
       set to TRUE */
    if ((resp->i_op == NCSMIB_OP_RSP_CLI_DONE) ||
        ((resp->i_op == NCSMIB_OP_RSP_CLI) &&
        (resp->rsp.info.cli_rsp.o_partial == FALSE)))
        rsp_completed = TRUE;

    switch(resp->rsp.i_status)
    {
    case NCSCC_RC_FAILURE:
        m_NCS_CONS_PRINTF("\nFailed to dump Profile information into file...\n");
        pss_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
        return NCSCC_RC_SUCCESS;
        break;

    case NCSCC_RC_SUCCESS:
        if(resp->rsp.info.cli_rsp.i_cmnd_id == PSS_CMD_TBL_CMD_NUM_DISPLAY_MIB)
        {
           m_NCS_CONS_PRINTF("\nProfile information dumped into file...\n");
           pss_cli_done(resp->i_usr_key, NCSCC_RC_SUCCESS);
        }
        break;

    case NCSCC_RC_NO_INSTANCE:
        if(resp->rsp.info.cli_rsp.i_cmnd_id == PSS_CMD_TBL_CMD_NUM_DISPLAY_MIB)
        {
           m_NCS_CONS_PRINTF("\nClient information not found for the specified profile...\n");
           pss_cli_done(resp->i_usr_key, NCSCC_RC_SUCCESS);
        }
        break;

    case NCSCC_RC_INV_VAL:
        m_NCS_CONS_PRINTF("\nInvalid profile specified...\n");
        pss_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
        break;

    case NCSCC_RC_NOSUCHNAME:
        m_NCS_CONS_PRINTF("\nNo MIBs recorded for this PCN...\n");
        pss_cli_done(resp->i_usr_key, NCSCC_RC_FAILURE);
        break;

    default:
        pss_cli_done(resp->i_usr_key, NCSCC_RC_SUCCESS);
        break;
    }

    return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  pss_send_cmd_mib_req
  
  Description   :  This function registers all the snmptm commands with the
                   CLI.
 
  Arguments     :  bindery  - CLI bindery data
    
  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes         : 
****************************************************************************/
uns32 pss_send_cmd_mib_req(NCSMIB_ARG *mib_arg, NCSCLI_CEF_DATA *cef_data)
{
   if (cef_data == NULL)
      return NCSCC_RC_FAILURE;

   mib_arg->i_usr_key = cef_data->i_bindery->i_cli_hdl;
   mib_arg->i_mib_key = cef_data->i_bindery->i_mab_hdl;

   if (ncsmib_timed_request(mib_arg, ncsmac_mib_request, 100)
       != NCSCC_RC_SUCCESS)
      return NCSCC_RC_FAILURE;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME: pss_cef_list_profiles

  DESCRIPTION   : Sends the MIB requests to get the profile table entries
  ARGUMENTS     :
      arg_list  : Argument list given in command.  
      cef_data  : command related data. 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cef_list_profiles(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
    NCSMIB_ARG           ncsmib_arg;
    uns8                 space[2048];
    NCSMEM_AID            ma;
    uns32                num_inst_ids = 0, xch_id = 1;
    uns32                retval = NCSCC_RC_SUCCESS, i, ret_val;
    uns32                inst_ids[NCS_PSS_MAX_PROFILE_NAME+1];
    char                 display_str[PSS_DISPLAY_STR_SIZE+1];
    char                 profile_name[NCS_PSS_MAX_PROFILE_NAME+1];


    /* Initialize the MIB arg structure */
    memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
    memset(&inst_ids, 0, sizeof(inst_ids));
    ncsmib_init(&ncsmib_arg);

    if(strcmp(arg_list->i_arg_record[0].cmd.strval, "no"))
    {
        ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
        ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
        ncsmib_arg.i_op      = NCSMIB_OP_REQ_NEXT;
        ncsmib_arg.i_rsp_fnc = NULL;
        ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
        ncsmib_arg.i_idx.i_inst_len = 0;
        ncsmib_arg.req.info.next_req.i_param_id = ncsPSSvProfileDesc_ID;
        ncsmib_arg.i_xch_id  = xch_id++;
        ncsmib_arg.i_tbl_id  = NCSMIB_TBL_PSR_PROFILES;
        ncsmem_aid_init(&ma, space, sizeof(space));
        retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 1000, &ma);
        if (retval == NCSCC_RC_NO_INSTANCE)
           goto end;
        else if (retval != NCSCC_RC_SUCCESS)
        {
            ret_val = m_MAB_DBG_SINK(retval);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
        while (TRUE)
        {
            if (ncsmib_arg.i_op != NCSMIB_OP_RSP_NEXT)
            {
                retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                sysf_sprintf(display_str, "Invalid MIB Response failed\n");
                pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
                goto end;
            }

            if(ncsmib_arg.rsp.i_status == NCSCC_RC_NO_INSTANCE)
                goto end;
            if(ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
            {
                retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                sysf_sprintf(display_str, "MIB Request failed\n");
                pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
                goto end;
            }

            num_inst_ids = ncsmib_arg.rsp.info.next_rsp.i_next.i_inst_len;
            if ((num_inst_ids == 0) || (num_inst_ids == 1))
                goto end;

            memcpy(inst_ids, ncsmib_arg.rsp.info.next_rsp.i_next.i_inst_ids,
                        num_inst_ids * sizeof(uns32));
            for (i = 1; i < num_inst_ids; i++)
                profile_name[i-1] = (char) inst_ids[i];
            profile_name[num_inst_ids-1] = '\0';
            sysf_sprintf(display_str, "%s\n", profile_name);
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);

            ncsmib_init(&ncsmib_arg);
            ncsmem_aid_init(&ma, space, sizeof(space));

            ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
            ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
            ncsmib_arg.i_op      = NCSMIB_OP_REQ_NEXT;
            ncsmib_arg.i_rsp_fnc = NULL;
            ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
            ncsmib_arg.i_idx.i_inst_len = num_inst_ids;
            ncsmib_arg.req.info.next_req.i_param_id = ncsPSSvProfileDesc_ID;
            ncsmib_arg.i_xch_id  = xch_id++;
            ncsmib_arg.i_tbl_id  = NCSMIB_TBL_PSR_PROFILES;
            retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 1000, &ma);
            if (retval == NCSCC_RC_NO_INSTANCE)
               goto end;
            else if (retval != NCSCC_RC_SUCCESS)
            {
                ret_val = m_MAB_DBG_SINK(retval);
                sysf_sprintf(display_str, "MIB Request failed\n");
                pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
                goto end;
            }
        } /* while (TRUE) */
    }

end:

    return pss_cli_done(cef_data->i_bindery->i_cli_hdl, retval);    
}


/*****************************************************************************

  PROCEDURE NAME: pss_cef_copy_profile

  DESCRIPTION   : Sends the MIB requests for copying an existing profile into
                  a new profile.
  ARGUMENTS     :
      arg_list  : Argument list given in command.  
      cef_data  : command related data. 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cef_copy_profile(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
   NCSMIB_ARG            ncsmib_arg;
   uns8                 space[2048];
   NCSMEM_AID            ma;
   uns32                xch_id = 1;
   uns32                retval = NCSCC_RC_SUCCESS, ret_val;
   uns32                inst_ids[NCS_PSS_MAX_PROFILE_NAME+1];
   char                 display_str[PSS_DISPLAY_STR_SIZE+1];
   
   /* Initialize the MIB arg structure */
   memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&ncsmib_arg);
   
   pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
   if(strcmp(arg_list->i_arg_record[0].cmd.strval, "no"))
   {
      /* Set the scalars. First set the ExistingProfile name*/
      ncsmib_init(&ncsmib_arg);
      ncsmem_aid_init(&ma, space, sizeof(space));
      
      ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
      ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
      ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
      ncsmib_arg.i_rsp_fnc = NULL;
      ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
      ncsmib_arg.i_idx.i_inst_len = 0;
      ncsmib_arg.i_xch_id  = xch_id++;
      ncsmib_arg.i_tbl_id  = NCSMIB_SCLR_PSR_TRIGGER;
      ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
      ncsmib_arg.req.info.set_req.i_param_val.i_length = (uns16)(strlen(arg_list->i_arg_record[2].cmd.strval) + 1);
      ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvExistingProfile_ID;
      ncsmib_arg.req.info.set_req.i_param_val.info.i_oct = (uns8 *)arg_list->i_arg_record[2].cmd.strval;
      retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 1000, &ma);
      if (retval != NCSCC_RC_SUCCESS)
      {
         ret_val = m_MAB_DBG_SINK(retval);
         sysf_sprintf(display_str, "MIB Request failed\n");
         pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
         goto end;
      }
      
      if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
      {
         retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         sysf_sprintf(display_str, "MIB Request failed\n");
         pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
         goto end;
      }
      
      if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
      {
         retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         sysf_sprintf(display_str, "MIB Request failed\n");
         pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
         goto end;
      }
      
      /* Now set the NewProfile name */
      ncsmib_init(&ncsmib_arg);
      ncsmem_aid_init(&ma, space, sizeof(space));
      
      ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
      ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
      ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
      ncsmib_arg.i_rsp_fnc = NULL;
      ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
      ncsmib_arg.i_idx.i_inst_len = 0;
      ncsmib_arg.i_xch_id  = xch_id++;
      ncsmib_arg.i_tbl_id  = NCSMIB_SCLR_PSR_TRIGGER;
      ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
      ncsmib_arg.req.info.set_req.i_param_val.i_length = (uns16)(strlen(arg_list->i_arg_record[3].cmd.strval) + 1);
      ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvNewProfile_ID;
      ncsmib_arg.req.info.set_req.i_param_val.info.i_oct = (uns8 *)arg_list->i_arg_record[3].cmd.strval;
      retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 1000, &ma);
      if (retval != NCSCC_RC_SUCCESS)
      {
         ret_val = m_MAB_DBG_SINK(retval);
         sysf_sprintf(display_str, "MIB Request failed\n");
         pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
         goto end;
      }
      
      if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
      {
         retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         sysf_sprintf(display_str, "MIB Request failed\n");
         pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
         goto end;
      }
      
      if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
      {
         retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         sysf_sprintf(display_str, "MIB Request failed\n");
         pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
         goto end;
      }
      
      /* Now trigger the copy operation */
      ncsmib_init(&ncsmib_arg);
      ncsmem_aid_init(&ma, space, sizeof(space));
      
      ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
      ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
      ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
      ncsmib_arg.i_rsp_fnc = NULL;
      ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
      ncsmib_arg.i_idx.i_inst_len = 0;
      ncsmib_arg.i_xch_id  = xch_id++;
      ncsmib_arg.i_tbl_id  = NCSMIB_SCLR_PSR_TRIGGER;
      ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
      ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvTriggerOperation_ID;
      ncsmib_arg.req.info.set_req.i_param_val.info.i_int = ncsPSSvTriggerCopy;
      retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 1000, &ma);
      if (retval != NCSCC_RC_SUCCESS)
      {
         ret_val = m_MAB_DBG_SINK(retval);
         sysf_sprintf(display_str, "MIB Request failed\n");
         pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
         goto end;
      }
      
      if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
      {
         retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         sysf_sprintf(display_str, "MIB Request failed\n");
         pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
         goto end;
      }
      
      if (ncsmib_arg.rsp.i_status == NCSCC_RC_INV_VAL)
      {
         retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         sysf_sprintf(display_str, "Invalid profile name combination specified\n");
         pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
         goto end;
      }
      if (ncsmib_arg.rsp.i_status == NCSCC_RC_NO_INSTANCE)
      {
         retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         sysf_sprintf(display_str, "No such profile\n");
         pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
         goto end;
      }
      
      if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
      {
         retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
         sysf_sprintf(display_str, "MIB Request failed\n");
         pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
         goto end;
      }
      sysf_sprintf(display_str, "copy profile success\n");
      pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
    }
    
end:
    return pss_cli_done(cef_data->i_bindery->i_cli_hdl, retval);  
    
}

/*****************************************************************************

  PROCEDURE NAME: pss_cef_rename_profile

  DESCRIPTION   : Sends the MIB requests for renaming an existing profile 
  ARGUMENTS     :
      arg_list  : Argument list given in command.  
      cef_data  : command related data. 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cef_rename_profile(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
    NCSMIB_ARG            ncsmib_arg;
    uns8                 space[2048];
    NCSMEM_AID            ma;
    uns32                xch_id = 1;
    uns32                retval = NCSCC_RC_SUCCESS, ret_val;
    char                 display_str[PSS_DISPLAY_STR_SIZE+1];
    uns32                inst_ids[NCS_PSS_MAX_PROFILE_NAME+1];

    /* Initialize the MIB arg structure */
    memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
    ncsmib_init(&ncsmib_arg);

    pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
    if(strcmp(arg_list->i_arg_record[0].cmd.strval, "no"))
    {
        /* Set the scalars. First set the ExistingProfile name*/
        ncsmib_init(&ncsmib_arg);
        ncsmem_aid_init(&ma, space, sizeof(space));

        ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
        ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
        ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
        ncsmib_arg.i_rsp_fnc = NULL;
        ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
        ncsmib_arg.i_idx.i_inst_len = 0;
        ncsmib_arg.i_xch_id  = xch_id++;
        ncsmib_arg.i_tbl_id  = NCSMIB_SCLR_PSR_TRIGGER;
        ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
        ncsmib_arg.req.info.set_req.i_param_val.i_length = (uns16)(strlen(arg_list->i_arg_record[2].cmd.strval) + 1);
        ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvExistingProfile_ID;
        ncsmib_arg.req.info.set_req.i_param_val.info.i_oct = (uns8 *)arg_list->i_arg_record[2].cmd.strval;
        retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 1000, &ma);
        if (retval != NCSCC_RC_SUCCESS)
        {
            ret_val = m_MAB_DBG_SINK(retval);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        /* Now set the NewProfile name */
        ncsmib_init(&ncsmib_arg);
        ncsmem_aid_init(&ma, space, sizeof(space));

        ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
        ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
        ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
        ncsmib_arg.i_rsp_fnc = NULL;
        ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
        ncsmib_arg.i_idx.i_inst_len = 0;
        ncsmib_arg.i_xch_id  = xch_id++;
        ncsmib_arg.i_tbl_id  = NCSMIB_SCLR_PSR_TRIGGER;
        ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
        ncsmib_arg.req.info.set_req.i_param_val.i_length = (uns16)(strlen(arg_list->i_arg_record[3].cmd.strval) + 1);
        ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvNewProfile_ID;
        ncsmib_arg.req.info.set_req.i_param_val.info.i_oct = (uns8 *)arg_list->i_arg_record[3].cmd.strval;
        retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 1000, &ma);
        if (retval != NCSCC_RC_SUCCESS)
        {
            ret_val = m_MAB_DBG_SINK(retval);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        /* Now trigger the copy operation */
        ncsmib_init(&ncsmib_arg);
        ncsmem_aid_init(&ma, space, sizeof(space));

        ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
        ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
        ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
        ncsmib_arg.i_rsp_fnc = NULL;
        ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
        ncsmib_arg.i_idx.i_inst_len = 0;
        ncsmib_arg.i_xch_id  = xch_id++;
        ncsmib_arg.i_tbl_id  = NCSMIB_SCLR_PSR_TRIGGER;
        ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
        ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvTriggerOperation_ID;
        ncsmib_arg.req.info.set_req.i_param_val.info.i_int = ncsPSSvTriggerRename;
        retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 1000, &ma);
        if (retval != NCSCC_RC_SUCCESS)
        {
            ret_val = m_MAB_DBG_SINK(retval);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.rsp.i_status == NCSCC_RC_INV_VAL)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "Invalid profile name combination specified\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }
        if (ncsmib_arg.rsp.i_status == NCSCC_RC_NO_INSTANCE)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "Source profile does not exist\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }
        if (ncsmib_arg.rsp.i_status == NCSCC_RC_NO_OBJECT)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "Destination profile already exists\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }
        sysf_sprintf(display_str, "rename profile success\n");
        pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
    }

end:

    return pss_cli_done(cef_data->i_bindery->i_cli_hdl, retval);  
}


/*****************************************************************************

  PROCEDURE NAME: pss_cef_delete_profile

  DESCRIPTION   : Sends the MIB requests for deleting an existing profile 
  ARGUMENTS     :
      arg_list  : Argument list given in command.  
      cef_data  : command related data. 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cef_delete_profile(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
    NCSMIB_ARG            ncsmib_arg;
    uns8                 space[2048];
    NCSMEM_AID            ma;
    uns32                xch_id = 1, i;
    uns32                retval = NCSCC_RC_SUCCESS, ret_val;
    char                 display_str[PSS_DISPLAY_STR_SIZE+1];
    uns32                inst_ids[NCS_PSS_MAX_PROFILE_NAME+1];
    uns32                num_inst_ids = 0;

    /* Initialize the MIB arg structure */
    memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
    ncsmib_init(&ncsmib_arg);

    pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
    if(strcmp(arg_list->i_arg_record[0].cmd.strval, "no"))
    {
        /* Now trigger the copy operation */
        ncsmib_init(&ncsmib_arg);
        ncsmem_aid_init(&ma, space, sizeof(space));

        ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
        ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
        ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
        ncsmib_arg.i_rsp_fnc = NULL;
        num_inst_ids        = strlen(arg_list->i_arg_record[2].cmd.strval) + 1;
        for (i = 1; i < num_inst_ids; i++)
            inst_ids[i] = arg_list->i_arg_record[2].cmd.strval[i - 1];
        inst_ids[0] = num_inst_ids - 1;
        ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
        ncsmib_arg.i_idx.i_inst_len = num_inst_ids;
        ncsmib_arg.i_xch_id  = xch_id++;
        ncsmib_arg.i_tbl_id  = NCSMIB_TBL_PSR_PROFILES;
        ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
        ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvProfileTableStatus_ID;
        ncsmib_arg.req.info.set_req.i_param_val.info.i_int = NCSMIB_ROWSTATUS_DESTROY;
        retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 1000, &ma);
        if (retval != NCSCC_RC_SUCCESS)
        {
            ret_val = m_MAB_DBG_SINK(retval);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.rsp.i_status == NCSCC_RC_INV_VAL)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "Invalid profile name specified\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.rsp.i_status == NCSCC_RC_NO_INSTANCE)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "No such profile\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }
        sysf_sprintf(display_str, "delete profile success\n");
        pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);

    }
end:

    return pss_cli_done(cef_data->i_bindery->i_cli_hdl, retval);  
}


/*****************************************************************************

  PROCEDURE NAME: pss_cef_save_profile

  DESCRIPTION   : Sends the MIB requests for saving the current system 
                  configuration either to the Current Configuration profile or
                  a new named profile
  ARGUMENTS     :
      arg_list  : Argument list given in command.  
      cef_data  : command related data. 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cef_save_profile(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
    NCSMIB_ARG            ncsmib_arg;
    uns8                 space[2048];
    NCSMEM_AID            ma;
    uns32                xch_id = 1, i;
    uns32                retval = NCSCC_RC_SUCCESS, ret_val;
    char                 display_str[PSS_DISPLAY_STR_SIZE+1];
    uns32                inst_ids[NCS_PSS_MAX_PROFILE_NAME+1];
    uns32                num_inst_ids = 0;
    int8 *               profile_name = NULL;

    if (arg_list->i_pos_value & 0x00000004)
        profile_name = arg_list->i_arg_record[2].cmd.strval;
    /* Initialize the MIB arg structure */
    memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
    ncsmib_init(&ncsmib_arg);

    pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
    if(strcmp(arg_list->i_arg_record[0].cmd.strval, "no"))
    {
        /* Now trigger the copy operation */
        ncsmib_init(&ncsmib_arg);
        ncsmem_aid_init(&ma, space, sizeof(space));

        ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
        ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
        ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
        ncsmib_arg.i_rsp_fnc = pss_cli_mib_resp;
        ncsmib_arg.i_xch_id  = xch_id++;
        if (profile_name != NULL)
        {
            num_inst_ids = strlen(profile_name) + 1;
            for (i = 1; i < num_inst_ids; i++)
                inst_ids[i] = arg_list->i_arg_record[2].cmd.strval[i - 1];
            inst_ids[0] = num_inst_ids - 1;
            ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
            ncsmib_arg.i_idx.i_inst_len = num_inst_ids;
            ncsmib_arg.i_tbl_id  = NCSMIB_TBL_PSR_PROFILES;
            ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
            ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvProfileTableStatus_ID;
            ncsmib_arg.req.info.set_req.i_param_val.info.i_int = NCSMIB_ROWSTATUS_CREATE_GO;
        }
        else
        {
            ncsmib_arg.i_idx.i_inst_len = 0;
            ncsmib_arg.i_tbl_id  = NCSMIB_SCLR_PSR_TRIGGER;
            ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
            ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvTriggerOperation_ID;
            ncsmib_arg.req.info.set_req.i_param_val.info.i_int = ncsPSSvTriggerSave;
        }

        retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 1000, &ma);
        if (retval != NCSCC_RC_SUCCESS)
        {
            ret_val = m_MAB_DBG_SINK(retval);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "MIB Request failed...response is not of SET type\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl,(uns8 *) display_str);
            goto end;
        }

        if (ncsmib_arg.rsp.i_status == NCSCC_RC_INV_VAL)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "Invalid operation specified...\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }
        if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "MIB Request failed...response is not success\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }
        sysf_sprintf(display_str, "save profile success\n");
        pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
    }
end:

    return pss_cli_done(cef_data->i_bindery->i_cli_hdl, retval);  
}


/*****************************************************************************

  PROCEDURE NAME: pss_cef_describe_profile

  DESCRIPTION   : Sends the MIB requests for either viewing or setting the 
                  description for an existing profile.
  ARGUMENTS     :
      arg_list  : Argument list given in command.  
      cef_data  : command related data. 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cef_describe_profile(NCSCLI_ARG_SET *arg_list, 
                               NCSCLI_CEF_DATA *cef_data)
{
    NCSMIB_ARG            ncsmib_arg;
    uns8                 space[2048];
    NCSMEM_AID            ma;
    uns32                xch_id = 1;
    uns32                retval = NCSCC_RC_SUCCESS, ret_val;
    char                 display_str[PSS_DISPLAY_STR_SIZE+1];
    uns32                inst_ids[NCS_PSS_MAX_PROFILE_NAME+1];
    uns32                num_inst_ids, i;
    int8 *               profile_desc = NULL;
    int8 *               profile_name;

    /* Initialize the MIB arg structure */
    memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));

    pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
    if(strcmp(arg_list->i_arg_record[0].cmd.strval, "no"))
    {
        if (arg_list->i_pos_value & 0x00000008)
            profile_desc = arg_list->i_arg_record[3].cmd.strval;

        profile_name = arg_list->i_arg_record[2].cmd.strval;
        /* Set the scalars. First set the ExistingProfile name*/
        ncsmib_init(&ncsmib_arg);
        ncsmem_aid_init(&ma, space, sizeof(space));

        ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
        ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
        ncsmib_arg.i_rsp_fnc = NULL;
        num_inst_ids = strlen(profile_name) + 1;
        for (i = 1; i < num_inst_ids; i++)
            inst_ids[i] = profile_name[i - 1];
        inst_ids[0] = num_inst_ids - 1;
        ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
        ncsmib_arg.i_idx.i_inst_len = num_inst_ids;
        ncsmib_arg.i_xch_id  = xch_id++;

        if (profile_desc != NULL)
        {
            ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
            ncsmib_arg.i_tbl_id  = NCSMIB_TBL_PSR_PROFILES;
            ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
            ncsmib_arg.req.info.set_req.i_param_val.i_length = (uns16)(strlen(profile_desc) + 1);
            ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvProfileDesc_ID;
            ncsmib_arg.req.info.set_req.i_param_val.info.i_oct = (uns8 *)profile_desc;
        }
        else
        {
            ncsmib_arg.i_op      = NCSMIB_OP_REQ_GET;
            ncsmib_arg.i_tbl_id  = NCSMIB_TBL_PSR_PROFILES;
            ncsmib_arg.req.info.get_req.i_param_id = ncsPSSvProfileDesc_ID;
        }
        retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 1000, &ma);
        if (retval != NCSCC_RC_SUCCESS)
        {
            ret_val = m_MAB_DBG_SINK(retval);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (profile_desc != NULL)
        {
            if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
            {
                retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                sysf_sprintf(display_str, "MIB Request failed\n");
                pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
                goto end;
            }

            if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
            {
                retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                sysf_sprintf(display_str, "MIB Request failed\n");
                pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
                goto end;
            }

            sysf_sprintf(display_str, "\nProfile description set success: %s\n", profile_desc);
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            retval = NCSCC_RC_SUCCESS;
            goto end;
        } /* if (profile_desc != NULL) */
        else
        {
            if (ncsmib_arg.i_op != NCSMIB_OP_RSP_GET)
            {
                retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                sysf_sprintf(display_str, "MIB Request failed\n");
                pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
                goto end;
            }

            if (ncsmib_arg.rsp.i_status == NCSCC_RC_NO_INSTANCE)
            {
                retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                sysf_sprintf(display_str, "No such profile\n");
                pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
                goto end;
            }

            if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
            {
                retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                sysf_sprintf(display_str, "Description not set\n");
                pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
                goto end;
            }
            if(ncsmib_arg.rsp.info.get_rsp.i_param_val.i_length == 0)
            {
               sysf_sprintf(display_str, "\nDescription not set\n");
               pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
               goto end;
            }
            {
               char prdesc[256];

               memset(&prdesc, '\0', sizeof(prdesc));
               profile_desc = (int8 *)ncsmib_arg.rsp.info.get_rsp.i_param_val.info.i_oct;
               strncpy(&prdesc, profile_desc, ncsmib_arg.rsp.info.get_rsp.i_param_val.i_length);
               prdesc[255] = '\0'; /* Preventive null termination */
               sysf_sprintf(display_str, "\nProfile description: %s\n", (char*)&prdesc);
               pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            }
            goto end;
        }
    }

end:

    return pss_cli_done(cef_data->i_bindery->i_cli_hdl, retval);  
}


/*****************************************************************************

  PROCEDURE NAME: pss_cef_replace_current_profile

  DESCRIPTION   : Sends the MIB requests for replace the current profile with
                  an alternate configuration profile in the persistent store
  ARGUMENTS     :
      arg_list  : Argument list given in command.  
      cef_data  : command related data. 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cef_replace_current_profile(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
    NCSMIB_ARG            ncsmib_arg;
    uns8                 space[2048];
    NCSMEM_AID            ma;
    uns32                xch_id = 1;
    uns32                retval = NCSCC_RC_SUCCESS, ret_val;
    char                 display_str[PSS_DISPLAY_STR_SIZE+1];
    uns32                inst_ids[NCS_PSS_MAX_PROFILE_NAME+1];

    /* Initialize the MIB arg structure */
    memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
    ncsmib_init(&ncsmib_arg);

    pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
    /* Set the scalars. First set the ExistingProfile name*/
    ncsmib_init(&ncsmib_arg);
    ncsmem_aid_init(&ma, space, sizeof(space));

    ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
    ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
    ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
    ncsmib_arg.i_rsp_fnc = NULL;
    ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
    ncsmib_arg.i_idx.i_inst_len = 0;
    ncsmib_arg.i_xch_id  = xch_id++;
    ncsmib_arg.i_tbl_id  = NCSMIB_SCLR_PSR_TRIGGER;
    ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
    ncsmib_arg.req.info.set_req.i_param_val.i_length = (uns16)(strlen(arg_list->i_arg_record[1].cmd.strval) + 1);
    ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvExistingProfile_ID;
    ncsmib_arg.req.info.set_req.i_param_val.info.i_oct = (uns8 *)arg_list->i_arg_record[1].cmd.strval;
    retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 1000, &ma);
    if (retval != NCSCC_RC_SUCCESS)
    {
       ret_val = m_MAB_DBG_SINK(retval);
       sysf_sprintf(display_str, "MIB Request failed\n");
       pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
       goto end;
    }
    if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       sysf_sprintf(display_str, "MIB Request failed\n");
       pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
       goto end;
    }

    if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       sysf_sprintf(display_str, "MIB Request failed\n");
       pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
       goto end;
    }

    /* Now trigger the replace operation */
    ncsmib_init(&ncsmib_arg);
    ncsmem_aid_init(&ma, space, sizeof(space));
    ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
    ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
    ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
    ncsmib_arg.i_rsp_fnc = NULL;
    ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
    ncsmib_arg.i_idx.i_inst_len = 0;
    ncsmib_arg.i_xch_id  = xch_id++;
    ncsmib_arg.i_tbl_id  = NCSMIB_SCLR_PSR_TRIGGER;
    ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
    ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvTriggerOperation_ID;
    ncsmib_arg.req.info.set_req.i_param_val.info.i_int = ncsPSSvTriggerReplace;
    retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 10000, &ma);
    if (retval != NCSCC_RC_SUCCESS)
    {
       ret_val = m_MAB_DBG_SINK(retval);
       sysf_sprintf(display_str, "MIB Request failed\n");
       pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
       goto end;
    }

    if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       sysf_sprintf(display_str, "MIB Request failed\n");
       pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
       goto end;
    }

    if (ncsmib_arg.rsp.i_status == NCSCC_RC_NO_INSTANCE)
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       sysf_sprintf(display_str, "No such profile\n");
       pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
       goto end;
    }

    if (ncsmib_arg.rsp.i_status == NCSCC_RC_INV_VAL)
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       sysf_sprintf(display_str, "Invalid alternate profile specified\n");
       pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
       goto end;
    }

    if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
    {
       retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
       sysf_sprintf(display_str, "MIB Request failed\n");
       pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
       goto end;
    }
    sysf_sprintf(display_str, "Replace operation success\n");
    pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);

end:

    return pss_cli_done(cef_data->i_bindery->i_cli_hdl, retval);  
}

/*****************************************************************************

  PROCEDURE NAME: pss_cef_load_profile

  DESCRIPTION   : Sends the MIB requests for loading a profile from the 
                  persistent store
  ARGUMENTS     :
      arg_list  : Argument list given in command.  
      cef_data  : command related data. 

  RETURNS       : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

  NOTES         :

*****************************************************************************/
uns32 pss_cef_load_profile(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
    NCSMIB_ARG            ncsmib_arg;
    uns8                 space[2048];
    NCSMEM_AID            ma;
    uns32                xch_id = 1;
    uns32                retval = NCSCC_RC_SUCCESS, ret_val;
    char                 display_str[PSS_DISPLAY_STR_SIZE+1];
    uns32                inst_ids[NCS_PSS_MAX_PROFILE_NAME+1];

    /* Initialize the MIB arg structure */
    memset(&ncsmib_arg, 0, sizeof(NCSMIB_ARG));
    ncsmib_init(&ncsmib_arg);

    pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
    if(strcmp(arg_list->i_arg_record[0].cmd.strval, "no"))
    {
        /* Set the scalars. First set the ExistingProfile name*/
        ncsmib_init(&ncsmib_arg);
        ncsmem_aid_init(&ma, space, sizeof(space));

        ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
        ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
        ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
        ncsmib_arg.i_rsp_fnc = NULL;
        ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
        ncsmib_arg.i_idx.i_inst_len = 0;
        ncsmib_arg.i_xch_id  = xch_id++;
        ncsmib_arg.i_tbl_id  = NCSMIB_SCLR_PSR_TRIGGER;
        ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
        ncsmib_arg.req.info.set_req.i_param_val.i_length = (uns16)(strlen(arg_list->i_arg_record[2].cmd.strval) + 1);
        ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvExistingProfile_ID;
        ncsmib_arg.req.info.set_req.i_param_val.info.i_oct = (uns8 *)arg_list->i_arg_record[2].cmd.strval;
        retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 1000, &ma);
        if (retval != NCSCC_RC_SUCCESS)
        {
            ret_val = m_MAB_DBG_SINK(retval);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        /* Now trigger the copy operation */
        ncsmib_init(&ncsmib_arg);
        ncsmem_aid_init(&ma, space, sizeof(space));

        ncsmib_arg.i_mib_key = cef_data->i_bindery->i_mab_hdl;
        ncsmib_arg.i_usr_key = cef_data->i_bindery->i_cli_hdl;
        ncsmib_arg.i_op      = NCSMIB_OP_REQ_SET;
        ncsmib_arg.i_rsp_fnc = NULL;
        ncsmib_arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
        ncsmib_arg.i_idx.i_inst_len = 0;
        ncsmib_arg.i_xch_id  = xch_id++;
        ncsmib_arg.i_tbl_id  = NCSMIB_SCLR_PSR_TRIGGER;
        ncsmib_arg.req.info.set_req.i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
        ncsmib_arg.req.info.set_req.i_param_val.i_param_id = ncsPSSvTriggerOperation_ID;
        ncsmib_arg.req.info.set_req.i_param_val.info.i_int = ncsPSSvTriggerLoad;
        retval = ncsmib_sync_request(&ncsmib_arg, cef_data->i_bindery->i_req_fnc, 10000, &ma);
        if (retval != NCSCC_RC_SUCCESS)
        {
            ret_val = m_MAB_DBG_SINK(retval);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.i_op != NCSMIB_OP_RSP_SET)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.rsp.i_status == NCSCC_RC_NO_INSTANCE)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "No such profile\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.rsp.i_status == NCSCC_RC_INV_VAL)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "Invalid alternate profile specified\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }

        if (ncsmib_arg.rsp.i_status != NCSCC_RC_SUCCESS)
        {
            retval = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            sysf_sprintf(display_str, "MIB Request failed\n");
            pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
            goto end;
        }
        sysf_sprintf(display_str, "Load operation success\n");
        pss_cli_display(cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);
    }

end:

    return pss_cli_done(cef_data->i_bindery->i_cli_hdl, retval);  
}

