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

  MODULE NAME: DTSVCLI.H

..............................................................................

  DESCRIPTION: Contains definitions for DTSV CLI interface.

******************************************************************************
*/
#ifndef DTSVCLI_H
#define DTSVCLI_H


EXTERN_C uns32 ncsdtsv_cef_load_lib_req(NCS_LIB_REQ_INFO *libreq);

EXTERN_C uns32 dtsv_cef_set_logging_level(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
EXTERN_C uns32 dtsv_cef_reset_logging_level(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
EXTERN_C uns32 dtsv_cef_set_logging_device(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
EXTERN_C uns32 dtsv_cef_reset_logging_device(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
EXTERN_C uns32 dtsv_cef_conf_log_device(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
EXTERN_C uns32 dtsv_cef_enable_logging(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
EXTERN_C uns32 dtsv_cef_disable_logging(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
EXTERN_C uns32 dtsv_cef_cnf_category(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
EXTERN_C uns32 dtsv_cef_cnf_severity(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
/* Added for console(pty) logging*/
EXTERN_C uns32 dtsv_cef_cnf_console(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
/* Adeed for displaying console devices */
EXTERN_C uns32 dtsv_cef_disp_console(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
/* Added for reloading of ASCII_SPEC tables */
EXTERN_C uns32 dtsv_cef_spec_reload(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
EXTERN_C uns32 dtsv_cef_print_config(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
EXTERN_C uns32 dtsv_cef_cnf_num_log_files(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
EXTERN_C uns32 dtsv_cef_enable_sequencing(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
EXTERN_C uns32 dtsv_cef_close_open_files(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);
EXTERN_C uns32 dtsv_cef_disable_sequencing(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
EXTERN_C uns32 dtsv_cef_dump_buff_log(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
EXTERN_C uns32 dtsv_cef_set_dflt_plcy(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data);


#endif
