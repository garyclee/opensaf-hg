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

  MODULE NAME: IFSV_CLI.H

..............................................................................

  DESCRIPTION: Contains definitions for PSR CLI interface.


******************************************************************************
*/

#ifndef IFSV_CLI_H
#define IFSV_CLI_H

#include "ifsv_cli_papi.h"

#define IFSV_DISPLAY_STR_SIZE           150

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            Enums and Data Structure definations

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


typedef struct psr_cef_cmd_data
{
   uns16    gl_cntr; /* Used to maintain the no.of requests we made with MIB */
   uns8     gl_flag; /* response status is stored in this flag */
}IFSV_CEF_CMD_DATA;

/* extracting the port type */
#define m_IFSV_EXTRACT_PORT_TYPE(arg, retVal) {\
    if (m_INTF_COMP_STR (arg, "ethernet") == 0)\
     {\
        retVal = NCS_IFSV_INTF_ETHERNET;\
     } else\
     {\
         retVal = NCS_IFSV_INTF_OTHER;\
     }\
   }

#define m_INTF_COMP_STR(str1,str2) intf_str_comp(str1,str2)

typedef struct  ifindex_spt_info
{
   NCS_IFSV_SPT sspt_info;
   uns8  if_name[IFSV_IF_NAME_SIZE];
   uns32 ifindex;
}NCS_IFSV_SSPT_IF_INDEX_INFO;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            ANSI Function Prototypes

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

uns32 ncsifsv_cli_register(NCSCLI_BINDERY  *i_bindery);

uns32 ncsifsv_cef_load_lib_req(NCS_LIB_REQ_INFO *libreq); 

uns32 ifsv_cef_chng_interface_cmd(NCSCLI_ARG_SET *arg_list, 
                                       NCSCLI_CEF_DATA *cef_data);

uns32 ifsv_cef_conf_intf_shut (NCSCLI_ARG_SET *arg_list, 
                                       NCSCLI_CEF_DATA *cef_data);

uns32 intf_cef_config_ipv4addr(NCSCLI_ARG_SET *arg_list,
                               NCSCLI_CEF_DATA *cef_data);

uns32 intf_str_comp (char *pStr1, char *pStr2);

uns32 ifsv_cef_chng_interface_rsp(struct ncsmib_arg* rsp);

uns32 
ifsv_cef_show_intf_info(NCSCLI_ARG_SET *arg_list,NCSCLI_CEF_DATA *cef_data);


static uns32
ifsv_fill_sspt_display_data(NCSMIB_ARG *p_ncs_mib_arg,
                            NCS_IFSV_SSPT_IF_INDEX_INFO *p_sspt_info_display);

static uns32
ifsv_process_get_row_request(NCSCLI_CEF_DATA *p_cef_data, uns32 shelf, uns32 slot);

static uns32
ifsv_ncs_cli_done(uns32 cli_hdl, uns32 rc);

static uns32
ifsv_ncs_cli_display(uns32 cli_hdl, uns8* str);

void ifsv_intf_display_dump(NCSCLI_CEF_DATA *p_cef_data,
                       NCS_IFSV_SSPT_IF_INDEX_INFO display_info);

#if(NCS_IFSV_BOND_INTF == 1)

uns32
ifsv_cef_show_bind_intf(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);

uns32
ifsv_cef_conf_delete_bind_intf(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);

uns32
ifsv_cef_conf_create_bind_intf(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);

uns32
ifsv_cef_conf_bind_intf(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);

void ifsv_bind_intf_show_intf_info(USRBUF *usrbuf);

uns32 ifsv_cef_goto_interface_mode(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);

#endif

#endif

   
