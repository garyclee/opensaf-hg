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

  MODULE NAME: PSRCLI.H

..............................................................................

  DESCRIPTION: Contains definitions for PSS CLI interface.


******************************************************************************
*/

#ifndef PSRCLI_H
#define PSRCLI_H

#define PSS_DISPLAY_STR_SIZE           150

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            Enums and Data Structure definations

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


typedef struct pss_cef_cmd_data
{
   uns16    gl_cntr; /* Used to maintain the no.of requests we made with MIB */
   uns8     gl_flag; /* response status is stored in this flag */
}PSS_CEF_CMD_DATA;


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            ANSI Function Prototypes

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

EXTERN_C uns32 pss_cli_mib_resp(NCSMIB_ARG*);
EXTERN_C uns32 pss_cli_cmd_mib_resp_show_profile_clients(NCSMIB_ARG *resp);
EXTERN_C uns32 pss_cli_cmd_mib_resp_dump_profile(NCSMIB_ARG *resp);
EXTERN_C uns32 pss_cli_cmd_mib_resp_set_playback_option_from_xml(NCSMIB_ARG *resp);

EXTERN_C uns32 ncspss_cef_load_lib_req(NCS_LIB_REQ_INFO *libreq); 
EXTERN_C uns32 ncspss_cli_register(NCSCLI_BINDERY*);

EXTERN_C uns32 pss_send_mib_req(NCSMIB_ARG*, NCSCLI_CEF_DATA*);
EXTERN_C uns32 pss_cef_configure_pssv(NCSCLI_ARG_SET*, NCSCLI_CEF_DATA*);
EXTERN_C uns32 pss_cef_list_profiles(NCSCLI_ARG_SET*, NCSCLI_CEF_DATA*);
EXTERN_C uns32 pss_cef_copy_profile(NCSCLI_ARG_SET*, NCSCLI_CEF_DATA*);
EXTERN_C uns32 pss_cef_rename_profile(NCSCLI_ARG_SET*, NCSCLI_CEF_DATA*);
EXTERN_C uns32 pss_cef_delete_profile(NCSCLI_ARG_SET*, NCSCLI_CEF_DATA*);
EXTERN_C uns32 pss_cef_save_profile(NCSCLI_ARG_SET*, NCSCLI_CEF_DATA*);
EXTERN_C uns32 pss_cef_describe_profile(NCSCLI_ARG_SET*, NCSCLI_CEF_DATA*);
EXTERN_C uns32 pss_cef_load_profile(NCSCLI_ARG_SET*, NCSCLI_CEF_DATA*);
EXTERN_C uns32 pss_cef_set_playback_option_from_xml_config(NCSCLI_ARG_SET*,
                                                           NCSCLI_CEF_DATA*);
EXTERN_C uns32 pss_cef_dump_profile(NCSCLI_ARG_SET*, NCSCLI_CEF_DATA*);
EXTERN_C uns32 pss_cef_show_profile_clients(NCSCLI_ARG_SET*, NCSCLI_CEF_DATA*);
EXTERN_C uns32 pss_cef_reload_pssv_lib_conf(NCSCLI_ARG_SET *arg_list,
                                   NCSCLI_CEF_DATA *cef_data);
EXTERN_C uns32 pss_cef_reload_pssv_spcn_list(NCSCLI_ARG_SET *arg_list,
                                   NCSCLI_CEF_DATA *cef_data);
EXTERN_C uns32 pss_cef_replace_current_profile(NCSCLI_ARG_SET *arg_list,
                                   NCSCLI_CEF_DATA *cef_data);
EXTERN_C uns32 pss_send_cmd_mib_req(NCSMIB_ARG *mib_arg,
                                    NCSCLI_CEF_DATA *cef_data);

#endif 

