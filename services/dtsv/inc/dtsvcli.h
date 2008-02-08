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

$Header: 
..............................................................................

  DESCRIPTION: Contains definitions for DTSV CLI interface.


******************************************************************************
*/
#ifndef DTSVCLI_H
#define DTSVCLI_H

#define DTSV_MOTV   NCSDTSV_CLI_MIB_FETCH_TIMEOUT  /* defined in dtsv-opt.h */
#define MIB_VERR     "\nMIB returned error code"

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

EXTERN_C uns32 dtsv_cfg_mib_arg(NCSMIB_ARG*        mib,
                                uns32*             index,
                                uns32              index_len,
                                NCSMIB_TBL_ID      tid,
                                uns32              usr_hdl,
                                uns32              ss_hdl,
                                NCSMIB_RSP_FNC     rspfnc);

EXTERN_C uns32 dtsv_set_mib_int(NCSMIB_ARG*        mib,
                                uns32             pid,
                                uns32             r_status,
                                uns32             val,
                                NCSMIB_REQ_FNC     reqfnc,
                                char*             err);

EXTERN_C uns32 dtsv_set_mib_oct(NCSMIB_ARG*        mib,
                                uns32             pid,
                                uns32             r_status,
                                NCSCONTEXT        val,
                                uns16             len,
                                NCSMIB_REQ_FNC     reqfnc,
                                char*             err);

EXTERN_C uns32 dtsv_set_mib_int_val(NCSMIB_ARG       *mib,
                                    uns32           pid,
                                    uns32           val,
                                    NCSMIB_REQ_FNC   reqfnc,
                                    char            *err);

EXTERN_C uns32 dtsv_chk_mib_rsp(char*             err, 
                                NCSMIB_ARG*        arg, 
                                uns32             key);

EXTERN_C uns32 dtsv_cirbuff_setrow(NCSMIB_ARG * mib,
                    NCSMIB_REQ_FNC reqfunc,
                    uns32 node_id, uns32 svc_id,
                    uns32 operation, uns8 device,
                    char    *err);

EXTERN_C uns32 dtsv_get_mib_int(NCSMIB_ARG       *mib,
                       uns32           pid,
                       uns8            *val,
                       NCSMIB_REQ_FNC  reqfnc,
                       char            *err);

EXTERN_C uns32 dtsv_get_mib_oct(NCSMIB_ARG       *mib,
                       uns32           pid,
                       NCSCONTEXT      val,
                       uns32           *len,
                       NCSMIB_REQ_FNC  reqfnc,
                       char            *err);

EXTERN_C uns32 dtsv_send_cmd_mib_req(NCSMIB_ARG  *mib,
                       NCSCLI_CEF_DATA *cef_data,
                       char            *err);

EXTERN_C uns32 dtsv_cli_display(uns32 cli_hdl, char *str);
EXTERN_C uns32 dtsv_cli_done(uns8 *string, uns32 status, uns32 cli_hdl);
EXTERN_C uns32 dtsv_cli_cmd_rsp(NCSMIB_ARG *resp, uns32 cli_hdl);

#define m_RETURN_DTSV_CLI_DONE(st,s,k)   dtsv_cli_done(st,s,k)

#endif 

