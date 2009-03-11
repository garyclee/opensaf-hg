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
FILE NAME: VIP_CLI.C
******************************************************************************/
/*
~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~*                             ~*  DESCRIPTION  *~                          *~*
*~*   This file Contains the implementation for "IP Address Virtualization"  *~*
*~*   feature's Command-Line-Interface                                       *~*
~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*

~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~*   vip_cli_register ~~~~~~~~~~~~~~~~~ Function for CLI Registration       *~*
*~*   vip_show_all_CEF ~~~~~~~~~~~~~~~~~ CEF for vip dump                    *~*
*~*   vip_change_mode_vip_CEF ~~~~~~~~ CEF for change_mode to vip mode       *~*
*~*   vip_show_single_entry_CEF ~~~~~~~~ CEF for single entry Show           *~*
*~*   vip_process_get_row_request ~~~~~~ Process for Row Request             *~*
*~*   vip_ncs_cli_display ~~~~~~~~~~~~~~ For Display on CLI                  *~*
*~*   vip_single_entry_check ~~~~~~~~~~~ Check for Single entry Display      *~*
*~*   vip_populate_display_data ~~~~~~~~ Fill the Display DataStructure      *~*
*~*   vip_cli_display_all ~~~~~~~~~~~~~~                                     *~*
*~*   vip_ncs_cli_done ~~~~~~~~~~~~~~~~~ Send Done Operation to CLI module   *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

#if(NCS_VIP == 1)

#include "mab.h"
#include "cli_mem.h"
#include "ncs_cli.h"
#include "vip_cli.h"
#include "vip_mib.h"
#include "vip_mapi.h"
#include "ifsv_log.h"


/*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~*                          *~ MACRO'S Declaration ~*                       *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

#define  VIPCLI_INFORMATION_LEGEND                                  \
"\nVIP Configuration details\n"                                        \
"RANGE : 1 - , Only One IP is Configured in that interface \n"          \
"        O/W - The no of IP Iddresses are mentioned \n"                  \
"IP Address Type : I - Internal IP, E - External IP,\n"          \
"Installed Interface      \n"\/*TBD*/

#define  END_OF_VIPD_DATABASE 100

#define FOREVER 1

/*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~*            *~ Forward Declaration of Static Functions ~*                 *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
static uns32 vip_cli_register(NCSCLI_BINDERY *);
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

uns32 ncsvip_cef_load_lib_req(NCS_LIB_REQ_INFO *libreq);


/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME: ncsvip_cef_load_lib_req                                *~*
*~* DESCRIPTION   :                                                          *~*
*~* ARGUMENTS     : NCS_LIB_REQ_INFO *libreq                                 *~*
*~* RETURNS       : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                        *~*
*~* NOTES         :                                                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
uns32 ncsvip_cef_load_lib_req(NCS_LIB_REQ_INFO *libreq)
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
           return vip_cli_register(&i_bindery);
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

/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME:   vip_cli_register                                     *~*
*~* DESCRIPTION   :                                                          *~*
*~* ARGUMENTS     : NCSCLI_BINDERY *pBindery                                 *~*
*~* RETURNS       : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                        *~*
*~* NOTES         :                                                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
static uns32 
vip_cli_register(NCSCLI_BINDERY *pBindery)
{
   NCSCLI_CMD_LIST data;
   NCSCLI_OP_INFO req;
                                                                                                                             
   memset(&req, 0, sizeof(NCSCLI_OP_INFO));
   req.i_hdl = pBindery->i_cli_hdl;
   req.i_req = NCSCLI_OPREQ_REGISTER;
   req.info.i_register.i_bindery = pBindery;
                                                                                                                             
   data.i_node = "root/exec/config";
   data.i_command_mode = "config";
   data.i_access_req = FALSE;
   data.i_access_passwd = 0;
   data.i_cmd_count = 1;
   data.i_command_list[0].i_cmdstr = "vip-dump !Displays the current Existing Application Virtual IP's Configured!";
   data.i_command_list[0].i_cmd_exec_func = vip_show_all_CEF;
                                                                                                                             
   req.info.i_register.i_cmdlist = &data;
   if(NCSCC_RC_SUCCESS != ncscli_opr_request(&req)) return NCSCC_RC_FAILURE;
                                                                                                                             
   /**************************************************************************\
   *                                                                          *
   *   VIP CLI Top Level Commands                                           *
   *                                                                          *
   \**************************************************************************/
   memset(&data, 0, sizeof(NCSCLI_CMD_LIST));
   data.i_node = "root/exec/config";
   data.i_command_mode = "config";
   data.i_access_req = FALSE;
   data.i_access_passwd = 0;
   data.i_cmd_count = 1;
   data.i_command_list[0].i_cmdstr = "vip !VIP MODE(Give Existing application Name and handle Details to go to VIP Mode This Mode Enables the User to Operate on a single VIP Entry Basis) ! app_name !Character String(0..128)!  NCSCLI_STRING handle !Integer (1...2^32)! NCSCLI_NUMBER <1..4294967295>@root/exec/config/vip@";
   data.i_command_list[0].i_cmd_exec_func = vip_change_mode_vip_CEF;
   req.info.i_register.i_cmdlist = &data;
   if(NCSCC_RC_SUCCESS != ncscli_opr_request(&req)) return NCSCC_RC_FAILURE;
                                                                                                                             
   data.i_node = "root/exec/config/vip";
   data.i_command_mode = "vip";
   data.i_access_req = FALSE;
   data.i_access_passwd = 0;
   data.i_cmd_count = 1;
   data.i_command_list[0].i_cmdstr = "show!Displays Entry Details!";
   data.i_command_list[0].i_cmd_exec_func = vip_show_single_entry_CEF;

   req.info.i_register.i_cmdlist = &data;
   if(NCSCC_RC_SUCCESS != ncscli_opr_request(&req)) 
      return NCSCC_RC_FAILURE;

    return NCSCC_RC_SUCCESS;
}


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
               printf("\n UNABLE TO FETCH VIP DETAILS:::\n");
               printf(" REASON:::");
               printf(" IFD is not RUNNING:::\n");
               m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_CLI, IFSV_VIP_CLI_SHOW_ALL_FAILED);
               break;
        default:
               printf("\n*~*~*~*~*~*~*~*~* END OF VIP DATABASE *~*~*~*~*~*~*~*~*");
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
               printf("\n INVALID VLICATION DETAILS vizz..Application Name or Handle ENTERED BY USER ");
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
    uns8 title_str[200];
    sprintf(title_str, "%2s%10s%10s%10s%10s", "APPLICATION", "HANDLE", "IPPOOL", "IP/MASK", "INTERFACE");

    memset(&ncs_mib_arg, 0, sizeof(NCSMIB_ARG));
    ncsmib_init(&ncs_mib_arg);
    memset(space, 0, sizeof(space));
    ncsmem_aid_init(&ma, space, 1024);

    p_vip_mode_data = (NCS_CLI_MODE_DATA *)p_cef_data->i_subsys->i_cef_mode;

     /*For Top Line To be Displayed*/
     vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
     vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)title_str);
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
                  p_vip_data_display->configured = buff;
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
   int8                    display_str[200];
   int8                    ip_net_str[30]; /*More than 20 for safety*/
   NCS_IPPFX               nworder_dest;   /* an IP Address and IPMask  */
   uns32                   prfx_prn_len;
   uns32                   prn_cursor;

   nworder_dest.ipaddr.info.v4 = vip_data_display.ipaddress;
   nworder_dest.mask_len = vip_data_display.mask_len;

   /* Conversion Of IPAddress into a string*/
   sprintf(ip_net_str, "%d.%d.%d.%d/%d",
      ((uns8*)(&nworder_dest.ipaddr.info.v4))[0], /*This is max 3 chars */
      ((uns8*)(&nworder_dest.ipaddr.info.v4))[1],
      ((uns8*)(&nworder_dest.ipaddr.info.v4))[2],
      ((uns8*)(&nworder_dest.ipaddr.info.v4))[3],
      (nworder_dest.mask_len)); 

   prfx_prn_len = sprintf(display_str, "%2s",vip_data_display.p_service_name);/*1*/
     /*
      * Set the position where next field is to be printed 
      * */
   prn_cursor = prfx_prn_len;

   prn_cursor += sysf_sprintf(display_str + prn_cursor, "%13d ",vip_data_display.handle);/*2*/

   if(vip_data_display.ip_type == 0)/*declare an enum and replace the numbers*/
      prn_cursor += sysf_sprintf(display_str + prn_cursor, "%13s ","INTERNAL");/*3*/
   else
      prn_cursor += sysf_sprintf(display_str + prn_cursor, "%13s ","EXTERNAL");/*3*/

   prn_cursor += sysf_sprintf(display_str + prn_cursor, "%10s ",ip_net_str);/*4&5&6 IpAddress,Mask,Installed Interface*/

   prn_cursor += sprintf(display_str + prn_cursor, "%10s",vip_data_display.installed_intf);/*1*/

   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)"\n");
   vip_ncs_cli_display(p_cef_data->i_bindery->i_cli_hdl, (uns8 *)display_str);

   if(NCSCC_RC_SUCCESS != 
      vip_ncs_cli_done(p_cef_data->i_bindery->i_cli_hdl, NCSCC_RC_SUCCESS))
   {
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_CLI, IFSV_VIP_CLI_DONE_FAILURE);
   }
}

/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME:   vip_single_entry_check.                              *~*
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

