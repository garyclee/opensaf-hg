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
  MODULE NAME: SNMPTM_CLI.H

..............................................................................

  DESCRIPTION: Contains CLI definitions for SNMPTM 

******************************************************************************
*/

#ifndef SNMPTM_CLI_H
#define SNMPTM_CLI_H

#if(NCS_SNMPTM_CLI == 1)

typedef struct snmptm_cef_cmd_data
{
  uns32  svc_instance[6]; /* Holds svc_name + pwe_id/instance_id */ 
  uns32  instance_id;
  uns32  tbl_id;
} SNMPTM_CEF_CMD_DATA;

/* SNMPTM CMD-MODE DATA */
#define m_MMGR_ALLOC_SNMPTM_CMD_DATA    malloc(sizeof(SNMPTM_CEF_CMD_DATA))
#define m_MMGR_FREE_SNMPTM_CMD_DATA(p)  free(p)

EXTERN_C void  snmptm_cli_main(int argc, char **argv);
EXTERN_C uns32 snmptm_cli_register(NCSCLI_BINDERY *bindery);
EXTERN_C uns32 snmptm_send_mib_req(NCSMIB_ARG *mib_arg, NCSCLI_CEF_DATA *cef_data);

EXTERN_C uns32 snmptm_cli_done(uns32 cli_hdl, uns32 rc);
EXTERN_C NCSCLI_SUBSYS_CB *snmptmcli_get_subsys_cb(uns32 cli_hdl);

/*======================================================================
                    Call Back Resp Function proto types
=======================================================================*/
EXTERN_C uns32 snmptm_cli_show_resp(NCSMIB_ARG *resp);
EXTERN_C uns32 snmptm_cli_set_resp(NCSMIB_ARG *resp);

/*======================================================================
                    CEF Function proto types
======================================================================= */
EXTERN_C uns32 snmptm_cef_conf_mode(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);
EXTERN_C uns32 snmptm_cef_tbl_create_row(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);
EXTERN_C uns32 snmptm_cef_tblsix_create_row(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);
EXTERN_C uns32 snmptm_cef_tblsix_delete_row(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);
EXTERN_C uns32 snmptm_cef_show_tbl_data(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);
EXTERN_C uns32 snmptm_cef_show_tbls_data(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);
EXTERN_C uns32 snmptm_cef_show_glb_data(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);
EXTERN_C uns32 snmptm_cef_exit(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);

#endif  /* NCS_SNMPTM_CLI */
#endif /* SNMPTM_CLI_H */

