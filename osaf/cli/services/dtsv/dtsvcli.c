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

#include <configmake.h>

/*****************************************************************************
..............................................................................

  MODULE NAME:  DTSVCLI.C 

..............................................................................
  DESCRIPTION:
    
  This module contains functions used by the DTSV Subsystem for Command Line
  Interface.
  
..............................................................................

  FUNCTIONS INCLUDED in this module:

   ncsdtsv_cli_register................Routine to register DTSV CLI commands
   dtsv_cef_set_logging_level.........CEF for setting DTSV logging level
   dtsv_cef_reset_logging_level.......CEF for re-setting DTSV logging level
   dtsv_cef_set_logging_device........CEF for setting the logging device
   dtsv_cef_reset_logging_device......CEF for re-setting the logging device
   dtsv_cef_conf_log_device...........CEF for setting the logging device parameters
   dtsv_cef_enable_logging............CEF for enabling the logging  
   dtsv_cef_disable_logging...........CEF for disabling the logging
   dtsv_cef_cnf_category..............CEF for configuring the category bit map
   dtsv_cef_cnf_severity..............CEF for configuring the severity bit map
   dtsv_cef_cnf_num_log_files.........CEF for configuring the number of log files
   dtsv_cef_enable_sequencing.........CEF for enabling message sequencing
   dtsv_cef_disable_sequencing........CEF for disabling message sequencing
   dtsv_cef_dump_buff_log.............CEF for dumping the log stored in circular buffer
***************************************************************************/

#include "dts.h"
#include "mac_papi.h"

#define DTSV_BUFFER_LEN   200
#define CAT_FILTER_SET_ALL_VAL 0xFFFFFFFF
#define CAT_FILTER_NOSET_ALL_VAL 0

static uns32 dtsv_cli_cmds_reg(NCSCLI_BINDERY *pBindery);
uns32 dtsv_config_cef(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);

/*****************************************************************************
  PROCEDURE NAME: dtsv_cli_cmds_reg
  DESCRIPTION   : Registers the DTSV commands with the CLI.
  ARGUMENTS     :
      cli_bind  : pointer to NCSCLI_BINDERY structure containing
                  the information required to interact with the CLI.
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
static uns32 dtsv_cli_cmds_reg(NCSCLI_BINDERY *pBindery)
{
	NCSCLI_CMD_LIST data;
	uns32 rc = NCSCC_RC_SUCCESS;
	uns32 idx;
	NCSCLI_OP_INFO req;
	NCSCLI_CMD_DESC dtsv_cli_cmds[] = {
		{
		 dtsv_cef_set_logging_level,
		 "set!Set function! logging!Logging! level!Level for logging! \
         {global !Global Logging!} | {node !Per Node Logging! NCSCLI_NUMBER<1..4294967295>!Node Number!}",
		 NCSCLI_ADMIN_ACCESS},
		{
		 dtsv_cef_reset_logging_level,
		 "no !Negate the command! set!Set function! logging!Logging! level!Level for logging! \
         {global !Global Logging!} | {node !Per Node Logging!NCSCLI_NUMBER<1..4294967295>!Node Number!}",
		 NCSCLI_ADMIN_ACCESS},
		{
		 dtsv_cef_set_logging_device,
		 "set!Set function! logging!Logging! device!Device for Logging! \
         {global !Global Logging!} | {node !Per Node Logging!NCSCLI_NUMBER<1..4294967295>!Node Number!} | \
         {service !Service Logging! NCSCLI_NUMBER <1..4294967295> !Node ID Number! \
         NCSCLI_NUMBER <1..4294967295> !Service ID Number!}  NCSCLI_STRING!file/buffer/console!",
		 NCSCLI_ADMIN_ACCESS},
		{
		 dtsv_cef_reset_logging_device,
		 "no !Negate the command! set!Set function! logging!Logging! device!Device for Logging! \
         {global !Global Logging!} | {node !Per Node Logging!NCSCLI_NUMBER<1..4294967295>!Node Number!} | \
         {service !Service Logging! NCSCLI_NUMBER <1..4294967295> !Node ID Number! \
         NCSCLI_NUMBER <1..4294967295> !Service ID Number!}  NCSCLI_STRING!file/buffer/console!",
		 NCSCLI_ADMIN_ACCESS},
		{
		 dtsv_cef_conf_log_device,
		 "configure!Change the configuration! logging!Logging! device!Device to be configured! \
         NCSCLI_NUMBER <1..4294967295>|all !Node ID Number! NCSCLI_NUMBER <1..4294967295>|all !Service ID Number! \
         {file !Output Device to config File! NCSCLI_NUMBER<100..1000000>!File size in KB!} | \
         {buffer !Output device to config Circular Buffer! NCSCLI_NUMBER<10..100000>!Circular Buffer size in KB!} \
         [NCSCLI_STRING! compressed/expanded!]",
		 NCSCLI_ADMIN_ACCESS},
		{
		 dtsv_cef_enable_logging,
		 "enable!Enables! logging!Logging! \
         {NCSCLI_NUMBER <1..4294967295>|all !Node ID Number!} \
         {NCSCLI_NUMBER <1..4294967295>|all !Service ID Number!}",
		 NCSCLI_ADMIN_ACCESS},
		{
		 dtsv_cef_disable_logging,
		 "disable!Disables! logging!Logging! \
         {NCSCLI_NUMBER <1..4294967295>|all !Node ID Number!} \
         {NCSCLI_NUMBER <1..4294967295>|all !Service ID Number!}",
		 NCSCLI_ADMIN_ACCESS},
		{
		 dtsv_cef_cnf_category,
		 "configure!Change the configuration! category!Message logging Category! filter!Filter to be set! \
         {NCSCLI_NUMBER <1..4294967295>|all !Node ID Number!} \
         {NCSCLI_NUMBER <1..4294967295>|all !Service ID Number!} \
         {set !To set the bit!| noset !To reset the bit!} \
         {NCSCLI_NUMBER <0..31>!specify the bit to set/reset! \
                        |all !set/reset all the bits!}",
		 NCSCLI_ADMIN_ACCESS},
		{
		 dtsv_cef_cnf_severity,
		 "configure!Change the configuration! severity!Message logging Severity! filter!Filter to be set! \
         {NCSCLI_NUMBER <1..4294967295>|all !Node ID Number!} \
         {NCSCLI_NUMBER <1..4294967295>|all !Service ID Number!} \
         {emergency !Enable logs of NCSFL_SEV_EMERGENCY only!} | \
         {alert !Enable logs of NCSFL_SEV_ALERT & above!} | \
         {critical !Enable logs of NCSFL_SEV_CRITICAL & above!} | \
         {error !Enable logs of NCSFL_SEV_ERROR & above!} | \
         {warning !Enable logs of NCSFL_SEV_WARNING & above!} | \
         {notice !Enable logs of NCSFL_SEV_NOTICE & above!} | \
         {info   !Enable logs of NCSFL_SEV_INFO & above!} | \
         {debug  !Enable logs of NCSFL_SEV_DEBUG & above!} | \
         {none   !Disables all logs!}",
		 NCSCLI_ADMIN_ACCESS},
		{
		 dtsv_cef_cnf_num_log_files,
		 "configure!Change the configuration! number!Number to be set! of!Of! log!Log! files!Files!  \
         NCSCLI_NUMBER <1..255> !Number to be set!",
		 NCSCLI_ADMIN_ACCESS},
		/* Smik - Adding CLI command for adding/removing console devices */
		{
		 dtsv_cef_cnf_console,
		 "configure!Change the configuration! \
         console!console device when logging is set to console! \
         {NCSCLI_NUMBER <1..4294967295>|all !Node ID Number!} \
         {NCSCLI_NUMBER <1..4294967295>|all !Service ID Number!} \
         {add!add a console device! \
         NCSCLI_STRING!device string, eg. /dev/tty4 or /dev/pts/1 etc.! \
         {{notice !Enable console logging for NOTICE logs!} | \
         {warning !Enable console logging for WARNING logs!} | \
         {error !Enable console logging for ERROR logs!} | \
         {critical !Enable console logging for CRITICAL logs!} | \
         {alert !Enable console logging for ALERT logs!} | \
         {emergency !Enable console logging for EMERGENCY logs!} | \
         {all !Enable console logging for all logs except INFO & DEBUG!}} \
         } | \
         {rmv !remove a configured console! \
         {NCSCLI_STRING!device string, eg. /dev/tty4 or /dev/pts/1 etc.! | \
         all !Removes all configured consoles!}}",
		 NCSCLI_ADMIN_ACCESS},
		/* Smik - Adding CLI command for displaying console devices */
		{
		 dtsv_cef_disp_console,
		 "display!Displays current configuration! \
         console!console devices associated! \
         {NCSCLI_NUMBER <1..4294967295>|all !Node ID Number!} \
         {NCSCLI_NUMBER <1..4294967295>|all !Service ID Number!}"},
		{
		 dtsv_cef_print_config,
		 "print !Prints! \
           config !current config data for DTS, file stored in " OSAF_LOCALSTATEDIR "log/DTS_<date>.config!"},
		{
		 dtsv_cef_close_open_files,
		 "close!Closes ! files! log files used for logging!",
		 NCSCLI_ADMIN_ACCESS},
		{
		 dtsv_cef_enable_sequencing,
		 "enable!Enables! sequencing!Message Sequencing!",
		 NCSCLI_ADMIN_ACCESS},
		{
		 dtsv_cef_disable_sequencing,
		 "disable!Disables! sequencing!Message Sequencing!",
		 NCSCLI_ADMIN_ACCESS},
		{
		 dtsv_cef_dump_buff_log,
		 "dump!Copy! buffer!In Memory Circular Buffer! log!Log! \
         {NCSCLI_NUMBER <1..4294967295>|all !Node ID Number!} \
         {NCSCLI_NUMBER <1..4294967295>|all !Service ID Number!} \
         {NCSCLI_NUMBER <1..255> !Action to be taken! NCSCLI_STRING!file/console!}"},
		{
		 dtsv_cef_set_dflt_plcy,
		 "set!Set function! default!Pre-set defaults! policy!Policy to be set! \
          {node !Per Node Logging!NCSCLI_NUMBER<1..4294967295>!Node Number!} | \
          {service !Service Logging! NCSCLI_NUMBER <1..4294967295> !Node ID Number! \
          NCSCLI_NUMBER <1..4294967295> !Service ID Number!}",
		 NCSCLI_ADMIN_ACCESS}
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
	data.i_command_list[0].i_cmdstr = "dtsv!Configure DTSV commands!@root/exec/config/dtsv@";
	data.i_command_list[0].i_cmd_exec_func = dtsv_config_cef;
	data.i_command_list[0].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;

	req.info.i_register.i_cmdlist = &data;
	if (NCSCC_RC_SUCCESS != ncscli_opr_request(&req))
		return NCSCC_RC_FAILURE;

   /**************************************************************************\
   *                                                                          *
   *   DTSV CLI Top Level Commands                                            *
   *                                                                          *
   \**************************************************************************/
	memset(&data, 0, sizeof(NCSCLI_CMD_LIST));
	data.i_node = "root/exec/config/dtsv";
	data.i_command_mode = "dtsv";
	data.i_access_req = FALSE;
	data.i_access_passwd = 0;
	data.i_cmd_count = sizeof(dtsv_cli_cmds) / sizeof(dtsv_cli_cmds[0]);

	for (idx = 0; idx < data.i_cmd_count; idx++)
		data.i_command_list[idx] = dtsv_cli_cmds[idx];

	req.info.i_register.i_cmdlist = &data;
	rc = ncscli_opr_request(&req);
	return rc;
}

uns32 dtsv_config_cef(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSCLI_OP_INFO info;

	info.i_hdl = cef_data->i_bindery->i_cli_hdl;
	info.i_req = NCSCLI_OPREQ_DONE;
	info.info.i_done.i_status = NCSCC_RC_SUCCESS;

	ncscli_opr_request(&info);
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: ncsdtsv_cef_load_lib_req
  DESCRIPTION   : Registers the DTSV commands with the CLI.
  ARGUMENTS     :
      libreq    : pointer to NCS_LIB_REQ_INFO structure containing
                  the information required to interact with the CLI.
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 ncsdtsv_cef_load_lib_req(NCS_LIB_REQ_INFO *libreq)
{
	NCSCLI_BINDERY i_bindery;
	if (libreq == NULL)
		return NCSCC_RC_FAILURE;

	switch (libreq->i_op) {
	case NCS_LIB_REQ_CREATE:
		memset(&i_bindery, 0, sizeof(NCSCLI_BINDERY));
		i_bindery.i_cli_hdl = gl_cli_hdl;
		i_bindery.i_mab_hdl = gl_mac_handle;
		i_bindery.i_req_fnc = ncsmac_mib_request;
		return dtsv_cli_cmds_reg(&i_bindery);
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
  PROCEDURE NAME: dtsv_cef_set_logging_level
  DESCRIPTION   : CEF for setting DTSV logging level
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_set_logging_level(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{

	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 dtsv_hdl = cef_data->i_bindery->i_mab_hdl;
	NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
	NCSCLI_ARG_VAL *pArgv = &arg_list->i_arg_record[3];
	uns32 rc = NCSCC_RC_SUCCESS, oid = 0;
	uns8 get_value;

	if (strcmp(pArgv->cmd.strval, "global") == 0) {
		dtsv_cfg_mib_arg(&mib, &oid, GLOBAL_INST_ID_LEN, NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

		rc = dtsv_set_mib_int(&mib, ncsDtsvGlobalMessageLogging_ID, 0, NCS_SNMP_TRUE, reqfnc, MIB_VERR);

	} else if (strcmp(pArgv->cmd.strval, "node") == 0) {
		pArgv = &arg_list->i_arg_record[4];

		oid = pArgv->cmd.intval;
		dtsv_cfg_mib_arg(&mib, &oid, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);

		if ((rc =
		     dtsv_get_mib_int(&mib, ncsDtsvNodeMessageLogging_ID, &get_value, reqfnc,
				      MIB_VERR)) == NCSCC_RC_SUCCESS) {
			dtsv_cfg_mib_arg(&mib, &oid, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);
			if (get_value == 0)
				rc = dtsv_set_mib_int(&mib, ncsDtsvNodeMessageLogging_ID, ncsDtsvNodeRowStatus_ID,
						      NCS_SNMP_TRUE, reqfnc, MIB_VERR);
			else
				rc = dtsv_set_mib_int_val(&mib, ncsDtsvNodeMessageLogging_ID, NCS_SNMP_TRUE, reqfnc,
							  MIB_VERR);
		}
	} else
		rc = NCSCC_RC_FAILURE;

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV logging level failed");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_reset_logging_level
  DESCRIPTION   : CEF for re-setting DTSV logging level
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_reset_logging_level(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 dtsv_hdl = cef_data->i_bindery->i_mab_hdl;
	NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
	NCSCLI_ARG_VAL *pArgv = &arg_list->i_arg_record[4];
	uns32 rc = NCSCC_RC_SUCCESS, oid = 0;
	uns8 get_value;

	if (strcmp(pArgv->cmd.strval, "global") == 0) {
		dtsv_cfg_mib_arg(&mib, &oid, GLOBAL_INST_ID_LEN, NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

		rc = dtsv_set_mib_int(&mib, ncsDtsvGlobalMessageLogging_ID, 0, NCS_SNMP_FALSE, reqfnc, MIB_VERR);

	} else if (strcmp(pArgv->cmd.strval, "node") == 0) {
		pArgv = &arg_list->i_arg_record[5];

		oid = pArgv->cmd.intval;
		dtsv_cfg_mib_arg(&mib, &oid, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);

		if ((rc =
		     dtsv_get_mib_int(&mib, ncsDtsvNodeMessageLogging_ID, &get_value, reqfnc,
				      MIB_VERR)) == NCSCC_RC_SUCCESS) {
			dtsv_cfg_mib_arg(&mib, &oid, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);
			if (get_value == 0)
				rc = dtsv_set_mib_int(&mib, ncsDtsvNodeMessageLogging_ID, ncsDtsvNodeRowStatus_ID,
						      NCS_SNMP_FALSE, reqfnc, MIB_VERR);
			else
				rc = dtsv_set_mib_int_val(&mib, ncsDtsvNodeMessageLogging_ID, NCS_SNMP_FALSE, reqfnc,
							  MIB_VERR);
		}
	} else
		rc = NCSCC_RC_FAILURE;

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV logging level re-set failed");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_set_logging_device
  DESCRIPTION   : CEF for setting the logging device
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_set_logging_device(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 dtsv_hdl = cef_data->i_bindery->i_mab_hdl;
	NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
	NCSCLI_ARG_VAL *pArgv = &arg_list->i_arg_record[3];
	uns32 rc = NCSCC_RC_SUCCESS, oids[SVC_INST_ID_LEN];
	uns8 device, get_val;
	uns32 length;

	if (strcmp(pArgv->cmd.strval, "global") == 0) {
		dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN, NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

		if (dtsv_get_mib_oct(&mib, ncsDtsvGlobalLogDevice_ID,
				     &get_val, &length, reqfnc, MIB_VERR) != NCSCC_RC_FAILURE) {

			pArgv = &arg_list->i_arg_record[4];

			dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN,
					 NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

			if (strcmp(pArgv->cmd.strval, "file") == 0)
				device = (uns8)(LOG_FILE | get_val);
			else if (strcmp(pArgv->cmd.strval, "buffer") == 0)
				device = (uns8)(CIRCULAR_BUFFER | get_val);
			else if (strcmp(pArgv->cmd.strval, "console") == 0)
				device = (uns8)(OUTPUT_CONSOLE | get_val);
			else {
				dtsv_cli_display(cli_hdl, "\nDTSV log device specified is incorrect");
				return NCSCC_RC_FAILURE;
			}

			rc = dtsv_set_mib_oct(&mib, ncsDtsvGlobalLogDevice_ID, 0, &device, 1, reqfnc, MIB_VERR);
		}

	} else if (strcmp(pArgv->cmd.strval, "node") == 0) {
		pArgv = &arg_list->i_arg_record[4];

		oids[0] = pArgv->cmd.intval;

		dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);

		rc = dtsv_get_mib_oct(&mib, ncsDtsvNodeLogDevice_ID, &get_val, &length, reqfnc, MIB_VERR);
		if (rc != NCSCC_RC_FAILURE) {
			dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);

			pArgv = &arg_list->i_arg_record[5];

			if (strcmp(pArgv->cmd.strval, "file") == 0)
				device = (uns8)(LOG_FILE | get_val);
			else if (strcmp(pArgv->cmd.strval, "buffer") == 0)
				device = (uns8)(CIRCULAR_BUFFER | get_val);
			else if (strcmp(pArgv->cmd.strval, "console") == 0)
				device = (uns8)(OUTPUT_CONSOLE | get_val);
			else {
				dtsv_cli_display(cli_hdl, "\nDTSV log device specified is incorrect");
				return NCSCC_RC_FAILURE;
			}

			if (rc == NCSCC_RC_NO_INSTANCE)
				rc = dtsv_set_mib_oct(&mib, ncsDtsvNodeLogDevice_ID, ncsDtsvNodeRowStatus_ID,
						      &device, 1, reqfnc, MIB_VERR);
			else
				rc = dtsv_set_mib_oct(&mib, ncsDtsvNodeLogDevice_ID, 0, &device, 1, reqfnc, MIB_VERR);
		}

	} else if (strcmp(pArgv->cmd.strval, "service") == 0) {
		pArgv = &arg_list->i_arg_record[4];
		oids[0] = pArgv->cmd.intval;

		pArgv = &arg_list->i_arg_record[5];
		oids[1] = pArgv->cmd.intval;

		dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN, NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl, dtsv_hdl, NULL);

		rc = dtsv_get_mib_oct(&mib, ncsDtsvServiceLogDevice_ID, &get_val, &length, reqfnc, MIB_VERR);
		if (rc != NCSCC_RC_FAILURE) {

			dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN,
					 NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl, dtsv_hdl, NULL);

			pArgv = &arg_list->i_arg_record[6];

			if (strcmp(pArgv->cmd.strval, "file") == 0)
				device = (uns8)(LOG_FILE | get_val);
			else if (strcmp(pArgv->cmd.strval, "buffer") == 0)
				device = (uns8)(CIRCULAR_BUFFER | get_val);
			else if (strcmp(pArgv->cmd.strval, "console") == 0)
				device = (uns8)(OUTPUT_CONSOLE | get_val);
			else {
				dtsv_cli_display(cli_hdl, "\nDTSV log device specified is incorrect");
				return NCSCC_RC_FAILURE;
			}

			if (rc == NCSCC_RC_NO_INSTANCE)
				rc = dtsv_set_mib_oct(&mib, ncsDtsvServiceLogDevice_ID, ncsDtsvServiceRowStatus_ID,
						      &device, 1, reqfnc, MIB_VERR);
			else
				rc = dtsv_set_mib_oct(&mib, ncsDtsvServiceLogDevice_ID, 0, &device, 1, reqfnc,
						      MIB_VERR);
		}

	} else
		rc = NCSCC_RC_FAILURE;

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV log device set failed");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_reset_logging_device
  DESCRIPTION   : CEF for re-setting the logging device
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_reset_logging_device(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 dtsv_hdl = cef_data->i_bindery->i_mab_hdl;
	NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
	NCSCLI_ARG_VAL *pArgv = &arg_list->i_arg_record[4];
	uns32 rc = NCSCC_RC_SUCCESS, oids[SVC_INST_ID_LEN];
	uns8 device, get_val;
	uns32 length;

	if (strcmp(pArgv->cmd.strval, "global") == 0) {
		dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN, NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

		if (dtsv_get_mib_oct(&mib, ncsDtsvGlobalLogDevice_ID,
				     &get_val, &length, reqfnc, MIB_VERR) != NCSCC_RC_FAILURE) {

			pArgv = &arg_list->i_arg_record[5];

			dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN,
					 NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

			if (strcmp(pArgv->cmd.strval, "file") == 0)
				device = (uns8)((~LOG_FILE) & get_val);
			else if (strcmp(pArgv->cmd.strval, "buffer") == 0)
				device = (uns8)((~CIRCULAR_BUFFER) & get_val);
			else if (strcmp(pArgv->cmd.strval, "console") == 0)
				device = (uns8)((~OUTPUT_CONSOLE) & get_val);
			else {
				dtsv_cli_display(cli_hdl, "\nDTSV log device specified is incorrect");
				return NCSCC_RC_FAILURE;
			}

			rc = dtsv_set_mib_oct(&mib, ncsDtsvGlobalLogDevice_ID, 0, &device, 1, reqfnc, MIB_VERR);
		}

	} else if (strcmp(pArgv->cmd.strval, "node") == 0) {
		pArgv = &arg_list->i_arg_record[5];

		oids[0] = pArgv->cmd.intval;

		dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);

		rc = dtsv_get_mib_oct(&mib, ncsDtsvNodeLogDevice_ID, &get_val, &length, reqfnc, MIB_VERR);
		if (rc != NCSCC_RC_FAILURE) {

			dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);

			pArgv = &arg_list->i_arg_record[6];

			if (strcmp(pArgv->cmd.strval, "file") == 0)
				device = (uns8)((~LOG_FILE) & get_val);
			else if (strcmp(pArgv->cmd.strval, "buffer") == 0)
				device = (uns8)((~CIRCULAR_BUFFER) & get_val);
			else if (strcmp(pArgv->cmd.strval, "console") == 0)
				device = (uns8)((~OUTPUT_CONSOLE) & get_val);
			else {
				dtsv_cli_display(cli_hdl, "\nDTSV log device specified is incorrect");
				return NCSCC_RC_FAILURE;
			}

			if (rc == NCSCC_RC_NO_INSTANCE)
				rc = dtsv_set_mib_oct(&mib, ncsDtsvNodeLogDevice_ID, ncsDtsvNodeRowStatus_ID,
						      &device, 1, reqfnc, MIB_VERR);
			else
				rc = dtsv_set_mib_oct(&mib, ncsDtsvNodeLogDevice_ID, 0, &device, 1, reqfnc, MIB_VERR);
		}

	} else if (strcmp(pArgv->cmd.strval, "service") == 0) {
		pArgv = &arg_list->i_arg_record[5];
		oids[0] = pArgv->cmd.intval;

		pArgv = &arg_list->i_arg_record[6];
		oids[1] = pArgv->cmd.intval;

		dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN, NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl, dtsv_hdl, NULL);

		rc = dtsv_get_mib_oct(&mib, ncsDtsvServiceLogDevice_ID, &get_val, &length, reqfnc, MIB_VERR);
		if (rc != NCSCC_RC_FAILURE) {

			dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN,
					 NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl, dtsv_hdl, NULL);

			pArgv = &arg_list->i_arg_record[7];

			if (strcmp(pArgv->cmd.strval, "file") == 0)
				device = (uns8)((~LOG_FILE) & get_val);
			else if (strcmp(pArgv->cmd.strval, "buffer") == 0)
				device = (uns8)((~CIRCULAR_BUFFER) & get_val);
			else if (strcmp(pArgv->cmd.strval, "console") == 0)
				device = (uns8)((~OUTPUT_CONSOLE) & get_val);
			else {
				dtsv_cli_display(cli_hdl, "\nDTSV log device specified is incorrect");
				return NCSCC_RC_FAILURE;
			}

			if (rc == NCSCC_RC_NO_INSTANCE)
				rc = dtsv_set_mib_oct(&mib, ncsDtsvServiceLogDevice_ID, ncsDtsvServiceRowStatus_ID,
						      &device, 1, reqfnc, MIB_VERR);
			else
				rc = dtsv_set_mib_oct(&mib, ncsDtsvServiceLogDevice_ID, 0, &device, 1, reqfnc,
						      MIB_VERR);
		}

	} else
		rc = NCSCC_RC_FAILURE;

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV log device re-set failed");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_conf_log_device
  DESCRIPTION   : CEF for setting the logging device parameters
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_conf_log_device(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 dtsv_hdl = cef_data->i_bindery->i_mab_hdl;
	NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
	NCSCLI_ARG_VAL *pArgv = &arg_list->i_arg_record[3];
	uns32 rc = NCSCC_RC_SUCCESS, oids[SVC_INST_ID_LEN];
	uns32 size, format;
	uns8 get_value;

	if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
		pArgv = &arg_list->i_arg_record[4];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			pArgv = &arg_list->i_arg_record[5];

			dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN,
					 NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

			if (strcmp(pArgv->cmd.strval, "file") == 0) {
				pArgv = &arg_list->i_arg_record[6];

				size = pArgv->cmd.intval;

				if ((rc = dtsv_set_mib_int(&mib, ncsDtsvGlobalLogFileSize_ID, 0,
							   size, reqfnc, MIB_VERR)) != NCSCC_RC_SUCCESS) {
					dtsv_cli_display(cli_hdl, "\nDTSV: global file size configuration failed");
					return rc;
				}

				if (arg_list->i_pos_value & 0x80) {
					pArgv = &arg_list->i_arg_record[7];

					if (strcmp(pArgv->cmd.strval, "compressed") == 0)
						format = COMPRESSED_FORMAT;
					else
						format = EXPANDED_FORMAT;

					dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN,
							 NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

					if ((rc = dtsv_set_mib_int(&mib, ncsDtsvGlobalFileLogCompFormat_ID, 0,
								   format, reqfnc, MIB_VERR)) != NCSCC_RC_SUCCESS) {
						dtsv_cli_display(cli_hdl,
								 "\nDTSV: global file log format configuration failed");
						return rc;
					}
				}
			} else if (strcmp(pArgv->cmd.strval, "buffer") == 0) {
				pArgv = &arg_list->i_arg_record[6];

				size = pArgv->cmd.intval;

				if ((rc = dtsv_set_mib_int(&mib, ncsDtsvGlobalCircularBuffSize_ID, 0,
							   size, reqfnc, MIB_VERR)) != NCSCC_RC_SUCCESS) {
					dtsv_cli_display(cli_hdl, "\nDTSV: global buffer size configuration failed");
					return rc;
				}

				if (arg_list->i_pos_value & 0x80) {
					pArgv = &arg_list->i_arg_record[7];

					if (strcmp(pArgv->cmd.strval, "compressed") == 0)
						format = COMPRESSED_FORMAT;
					else
						format = EXPANDED_FORMAT;

					dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN,
							 NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

					if ((rc = dtsv_set_mib_int(&mib, ncsDtsvGlobalCirBuffCompFormat_ID, 0,
								   format, reqfnc, MIB_VERR)) != NCSCC_RC_SUCCESS) {
						dtsv_cli_display(cli_hdl,
								 "\nDTSV: global buffer log format configuration failed");
						return rc;
					}
				}
			} else
				return NCSCC_RC_FAILURE;
		} else {
			/* Not Supported; Return Error */
			dtsv_cli_display(cli_hdl, "\nCommand arguments not supported");
			return NCSCC_RC_FAILURE;
		}
	} else {
		oids[0] = pArgv->cmd.intval;
		pArgv = &arg_list->i_arg_record[4];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			pArgv = &arg_list->i_arg_record[5];

			dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);

			if (strcmp(pArgv->cmd.strval, "file") == 0) {
				pArgv = &arg_list->i_arg_record[6];

				size = pArgv->cmd.intval;

				if ((rc =
				     dtsv_get_mib_int(&mib, ncsDtsvNodeLogFileSize_ID, &get_value, reqfnc,
						      MIB_VERR)) == NCSCC_RC_SUCCESS) {
					dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl,
							 dtsv_hdl, NULL);
					if (get_value == 0) {
						if ((rc =
						     dtsv_set_mib_int(&mib, ncsDtsvNodeLogFileSize_ID,
								      ncsDtsvNodeRowStatus_ID, size, reqfnc,
								      MIB_VERR)) != NCSCC_RC_SUCCESS) {
							dtsv_cli_display(cli_hdl,
									 "\nDTSV: node file size configuration failed");
							return rc;
						}
					} else {
						if ((rc =
						     dtsv_set_mib_int_val(&mib, ncsDtsvNodeLogFileSize_ID, size, reqfnc,
									  MIB_VERR)) != NCSCC_RC_SUCCESS) {
							dtsv_cli_display(cli_hdl,
									 "\nDTSV: node file size configuration failed");
							return rc;
						}
					}
				} else
					return rc;

				if (arg_list->i_pos_value & 0x80) {
					pArgv = &arg_list->i_arg_record[7];

					if (strcmp(pArgv->cmd.strval, "compressed") == 0)
						format = COMPRESSED_FORMAT;
					else
						format = EXPANDED_FORMAT;

					dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN,
							 NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);

					if ((rc =
					     dtsv_get_mib_int(&mib, ncsDtsvNodeFileLogCompFormat_ID, &get_value, reqfnc,
							      MIB_VERR)) == NCSCC_RC_SUCCESS) {
						dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE,
								 cli_hdl, dtsv_hdl, NULL);
						if (get_value == 0) {
							if ((rc =
							     dtsv_set_mib_int(&mib, ncsDtsvNodeFileLogCompFormat_ID,
									      ncsDtsvNodeRowStatus_ID, format, reqfnc,
									      MIB_VERR)) != NCSCC_RC_SUCCESS) {
								dtsv_cli_display(cli_hdl,
										 "\nDTSV: node file log format configuration failed");
								return rc;
							}
						} else {
							if ((rc =
							     dtsv_set_mib_int_val(&mib, ncsDtsvNodeFileLogCompFormat_ID,
										  format, reqfnc,
										  MIB_VERR)) != NCSCC_RC_SUCCESS) {
								dtsv_cli_display(cli_hdl,
										 "\nDTSV: node file log format configuration failed");
								return rc;
							}
						}
					} else
						return rc;

				}
			} else if (strcmp(pArgv->cmd.strval, "buffer") == 0) {
				pArgv = &arg_list->i_arg_record[6];

				size = pArgv->cmd.intval;

				if ((rc =
				     dtsv_get_mib_int(&mib, ncsDtsvNodeCircularBuffSize_ID, &get_value, reqfnc,
						      MIB_VERR)) == NCSCC_RC_SUCCESS) {
					dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl,
							 dtsv_hdl, NULL);
					if (get_value == 0) {
						if ((rc =
						     dtsv_set_mib_int(&mib, ncsDtsvNodeCircularBuffSize_ID,
								      ncsDtsvNodeRowStatus_ID, size, reqfnc,
								      MIB_VERR)) != NCSCC_RC_SUCCESS) {
							dtsv_cli_display(cli_hdl,
									 "\nDTSV: node buffer size configuration failed");
							return rc;
						}
					} else {
						if ((rc =
						     dtsv_set_mib_int_val(&mib, ncsDtsvNodeCircularBuffSize_ID, size,
									  reqfnc, MIB_VERR)) != NCSCC_RC_SUCCESS) {
							dtsv_cli_display(cli_hdl,
									 "\nDTSV: node buffer size configuration failed");
							return rc;
						}
					}
				} else
					return rc;

				if (arg_list->i_pos_value & 0x80) {
					pArgv = &arg_list->i_arg_record[7];

					if (strcmp(pArgv->cmd.strval, "compressed") == 0)
						format = COMPRESSED_FORMAT;
					else
						format = EXPANDED_FORMAT;

					dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN,
							 NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);

					if ((rc =
					     dtsv_get_mib_int(&mib, ncsDtsvNodeCirBuffCompFormat_ID, &get_value, reqfnc,
							      MIB_VERR)) == NCSCC_RC_SUCCESS) {
						dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE,
								 cli_hdl, dtsv_hdl, NULL);
						if (get_value == 0) {
							if ((rc =
							     dtsv_set_mib_int(&mib, ncsDtsvNodeCirBuffCompFormat_ID,
									      ncsDtsvNodeRowStatus_ID, format, reqfnc,
									      MIB_VERR)) != NCSCC_RC_SUCCESS) {
								dtsv_cli_display(cli_hdl,
										 "\nDTSV: node buffer log format configuration failed");
								return rc;
							}
						} else {
							if ((rc =
							     dtsv_set_mib_int_val(&mib, ncsDtsvNodeCirBuffCompFormat_ID,
										  format, reqfnc,
										  MIB_VERR)) != NCSCC_RC_SUCCESS) {
								dtsv_cli_display(cli_hdl,
										 "\nDTSV: node buffer log format configuration failed");
								return rc;
							}
						}
					} else
						return rc;

				}
			} else
				return NCSCC_RC_FAILURE;
		} else {
			/* Service per node Logging */
			oids[1] = pArgv->cmd.intval;

			pArgv = &arg_list->i_arg_record[5];

			dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN,
					 NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl, dtsv_hdl, NULL);

			if (strcmp(pArgv->cmd.strval, "file") == 0) {
				pArgv = &arg_list->i_arg_record[6];

				size = pArgv->cmd.intval;

				if ((rc =
				     dtsv_get_mib_int(&mib, ncsDtsvServiceLogFileSize_ID, &get_value, reqfnc,
						      MIB_VERR)) == NCSCC_RC_SUCCESS) {
					dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN, NCSMIB_TBL_DTSV_SVC_PER_NODE,
							 cli_hdl, dtsv_hdl, NULL);
					if (get_value == 0) {
						if ((rc =
						     dtsv_set_mib_int(&mib, ncsDtsvServiceLogFileSize_ID,
								      ncsDtsvServiceRowStatus_ID, size, reqfnc,
								      MIB_VERR)) != NCSCC_RC_SUCCESS) {
							dtsv_cli_display(cli_hdl,
									 "\nDTSV: service file size configuration failed");
							return rc;
						}
					} else {
						if ((rc =
						     dtsv_set_mib_int_val(&mib, ncsDtsvServiceLogFileSize_ID, size,
									  reqfnc, MIB_VERR)) != NCSCC_RC_SUCCESS) {
							dtsv_cli_display(cli_hdl,
									 "\nDTSV: service file size configuration failed");
							return rc;
						}
					}
				} else
					return rc;

				if (arg_list->i_pos_value & 0x80) {
					pArgv = &arg_list->i_arg_record[7];

					if (strcmp(pArgv->cmd.strval, "compressed") == 0)
						format = COMPRESSED_FORMAT;
					else
						format = EXPANDED_FORMAT;

					dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN,
							 NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl, dtsv_hdl, NULL);

					if ((rc =
					     dtsv_get_mib_int(&mib, ncsDtsvServiceFileLogCompFormat_ID, &get_value,
							      reqfnc, MIB_VERR)) == NCSCC_RC_SUCCESS) {
						dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN,
								 NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl, dtsv_hdl, NULL);
						if (get_value == 0) {
							if ((rc =
							     dtsv_set_mib_int(&mib, ncsDtsvServiceFileLogCompFormat_ID,
									      ncsDtsvServiceRowStatus_ID, format,
									      reqfnc, MIB_VERR)) != NCSCC_RC_SUCCESS) {
								dtsv_cli_display(cli_hdl,
										 "\nDTSV: service file log format configuration failed");
								return rc;
							}
						} else {
							if ((rc =
							     dtsv_set_mib_int_val(&mib,
										  ncsDtsvServiceFileLogCompFormat_ID,
										  format, reqfnc,
										  MIB_VERR)) != NCSCC_RC_SUCCESS) {
								dtsv_cli_display(cli_hdl,
										 "\nDTSV: service file log format configuration failed");
								return rc;
							}
						}
					} else
						return rc;

				}
			} else if (strcmp(pArgv->cmd.strval, "buffer") == 0) {
				pArgv = &arg_list->i_arg_record[6];

				size = pArgv->cmd.intval;

				if ((rc =
				     dtsv_get_mib_int(&mib, ncsDtsvServiceCircularBuffSize_ID, &get_value, reqfnc,
						      MIB_VERR)) == NCSCC_RC_SUCCESS) {
					dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN, NCSMIB_TBL_DTSV_SVC_PER_NODE,
							 cli_hdl, dtsv_hdl, NULL);
					if (get_value == 0) {
						if ((rc =
						     dtsv_set_mib_int(&mib, ncsDtsvServiceCircularBuffSize_ID,
								      ncsDtsvServiceRowStatus_ID, size, reqfnc,
								      MIB_VERR)) != NCSCC_RC_SUCCESS) {
							dtsv_cli_display(cli_hdl,
									 "\nDTSV: service buffer size configuration failed");
							return rc;
						}
					} else {
						if ((rc =
						     dtsv_set_mib_int_val(&mib, ncsDtsvServiceCircularBuffSize_ID, size,
									  reqfnc, MIB_VERR)) != NCSCC_RC_SUCCESS) {
							dtsv_cli_display(cli_hdl,
									 "\nDTSV: service buffer size configuration failed");
							return rc;
						}
					}
				} else
					return rc;

				if (arg_list->i_pos_value & 0x80) {
					pArgv = &arg_list->i_arg_record[7];

					if (strcmp(pArgv->cmd.strval, "compressed") == 0)
						format = COMPRESSED_FORMAT;
					else
						format = EXPANDED_FORMAT;

					dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN,
							 NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl, dtsv_hdl, NULL);

					if ((rc =
					     dtsv_get_mib_int(&mib, ncsDtsvServiceCirBuffCompFormat_ID, &get_value,
							      reqfnc, MIB_VERR)) == NCSCC_RC_SUCCESS) {
						dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN,
								 NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl, dtsv_hdl, NULL);
						if (get_value == 0) {
							if ((rc =
							     dtsv_set_mib_int(&mib, ncsDtsvServiceCirBuffCompFormat_ID,
									      ncsDtsvServiceRowStatus_ID, format,
									      reqfnc, MIB_VERR)) != NCSCC_RC_SUCCESS) {
								dtsv_cli_display(cli_hdl,
										 "\nDTSV: service buffer log format configuration failed");
								return rc;
							}
						} else {
							if ((rc =
							     dtsv_set_mib_int_val(&mib,
										  ncsDtsvServiceCirBuffCompFormat_ID,
										  format, reqfnc,
										  MIB_VERR)) != NCSCC_RC_SUCCESS) {
								dtsv_cli_display(cli_hdl,
										 "\nDTSV: service buffer log format configuration failed");
								return rc;
							}
						}
					} else
						return rc;

				}
			} else
				return NCSCC_RC_FAILURE;
		}
	}

	m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_enable_logging
  DESCRIPTION   : CEF for enabling the logging
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_enable_logging(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 dtsv_hdl = cef_data->i_bindery->i_mab_hdl;
	NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
	NCSCLI_ARG_VAL *pArgv = &arg_list->i_arg_record[2];
	uns32 rc = NCSCC_RC_SUCCESS, oids[SVC_INST_ID_LEN];
	uns8 get_value;

	if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
		pArgv = &arg_list->i_arg_record[3];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			/* Enable global logging */
			dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN,
					 NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

			rc = dtsv_set_mib_int(&mib, ncsDtsvGlobalLoggingState_ID, 0, NCS_SNMP_TRUE, reqfnc, MIB_VERR);
		} else {
			/* Not Supported; Return Error */
			dtsv_cli_display(cli_hdl, "\nCommand arguments not supported");
			return NCSCC_RC_FAILURE;
		}
	} else {
		oids[0] = pArgv->cmd.intval;
		pArgv = &arg_list->i_arg_record[3];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			/* Enable Node logging */
			dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);

			if ((rc =
			     dtsv_get_mib_int(&mib, ncsDtsvNodeMessageLogging_ID, &get_value, reqfnc,
					      MIB_VERR) == NCSCC_RC_SUCCESS)) {
				dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl,
						 NULL);
				if (get_value == 0)
					rc = dtsv_set_mib_int(&mib, ncsDtsvNodeLoggingState_ID, ncsDtsvNodeRowStatus_ID,
							      NCS_SNMP_TRUE, reqfnc, MIB_VERR);
				else
					rc = dtsv_set_mib_int_val(&mib, ncsDtsvNodeLoggingState_ID, NCS_SNMP_TRUE,
								  reqfnc, MIB_VERR);
			}

		} else {
			/* Service per node Logging */
			oids[1] = pArgv->cmd.intval;

			dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN,
					 NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl, dtsv_hdl, NULL);

			if ((rc =
			     dtsv_get_mib_int(&mib, ncsDtsvServiceLoggingState_ID, &get_value, reqfnc,
					      MIB_VERR) == NCSCC_RC_SUCCESS)) {
				dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN, NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl,
						 dtsv_hdl, NULL);
				if (get_value == 0)
					rc = dtsv_set_mib_int(&mib, ncsDtsvServiceLoggingState_ID,
							      ncsDtsvServiceRowStatus_ID, NCS_SNMP_TRUE, reqfnc,
							      MIB_VERR);
				else
					rc = dtsv_set_mib_int_val(&mib, ncsDtsvServiceLoggingState_ID, NCS_SNMP_TRUE,
								  reqfnc, MIB_VERR);
			}
		}
	}

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV logging enable failed");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;

}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_disable_logging
  DESCRIPTION   : CEF for disabling the logging
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_disable_logging(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 dtsv_hdl = cef_data->i_bindery->i_mab_hdl;
	NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
	NCSCLI_ARG_VAL *pArgv = &arg_list->i_arg_record[2];
	uns32 rc = NCSCC_RC_SUCCESS, oids[SVC_INST_ID_LEN];
	uns8 get_value;

	if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
		pArgv = &arg_list->i_arg_record[3];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			/* Enable global logging */
			dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN,
					 NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

			rc = dtsv_set_mib_int(&mib, ncsDtsvGlobalLoggingState_ID, 0, NCS_SNMP_FALSE, reqfnc, MIB_VERR);
		} else {
			/* Not Supported; Return Error */
			dtsv_cli_display(cli_hdl, "\nCommand arguments not supported");
			return NCSCC_RC_FAILURE;
		}
	} else {
		oids[0] = pArgv->cmd.intval;
		pArgv = &arg_list->i_arg_record[3];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			/* Enable Node logging */
			dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);

			if ((rc =
			     dtsv_get_mib_int(&mib, ncsDtsvNodeLoggingState_ID, &get_value, reqfnc,
					      MIB_VERR) == NCSCC_RC_SUCCESS)) {
				dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl,
						 NULL);
				if (get_value == 0)
					rc = dtsv_set_mib_int(&mib, ncsDtsvNodeLoggingState_ID, ncsDtsvNodeRowStatus_ID,
							      NCS_SNMP_FALSE, reqfnc, MIB_VERR);
				else
					rc = dtsv_set_mib_int_val(&mib, ncsDtsvNodeLoggingState_ID, NCS_SNMP_FALSE,
								  reqfnc, MIB_VERR);
			}
		} else {
			/* Service per node Logging */
			oids[1] = pArgv->cmd.intval;

			dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN,
					 NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl, dtsv_hdl, NULL);

			if ((rc =
			     dtsv_get_mib_int(&mib, ncsDtsvServiceLoggingState_ID, &get_value, reqfnc,
					      MIB_VERR) == NCSCC_RC_SUCCESS)) {
				dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN, NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl,
						 dtsv_hdl, NULL);
				if (get_value == 0)
					rc = dtsv_set_mib_int(&mib, ncsDtsvServiceLoggingState_ID,
							      ncsDtsvServiceRowStatus_ID, NCS_SNMP_FALSE, reqfnc,
							      MIB_VERR);
				else
					rc = dtsv_set_mib_int_val(&mib, ncsDtsvServiceLoggingState_ID, NCS_SNMP_FALSE,
								  reqfnc, MIB_VERR);
			}
		}
	}

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV logging disable failed");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;

}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_cnf_category
  DESCRIPTION   : CEF for configuring the category bit map
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_cnf_category(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 dtsv_hdl = cef_data->i_bindery->i_mab_hdl;
	NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
	NCSCLI_ARG_VAL *pArgv = &arg_list->i_arg_record[3];
	uns32 rc = NCSCC_RC_SUCCESS, oids[SVC_INST_ID_LEN];
	uns32 bit_map, length, get_val = 0;
	NCS_BOOL bit_set;

	if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
		pArgv = &arg_list->i_arg_record[4];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			/* Enable global logging */
			dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN,
					 NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

			pArgv = &arg_list->i_arg_record[5];
			if (strcmp(pArgv->cmd.strval, "set") == 0)
				bit_set = TRUE;
			else
				bit_set = FALSE;

			pArgv = &arg_list->i_arg_record[6];
			if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
				/* All bits are selected */
				bit_map = (bit_set) ? CAT_FILTER_SET_ALL_VAL : CAT_FILTER_NOSET_ALL_VAL;
			} else {
				/* A particular bit has been set */
				/* First get the current value of category bit */
				if (dtsv_get_mib_oct
				    (&mib, ncsDtsvGlobalCategoryBitMap_ID, &get_val, &length, reqfnc,
				     MIB_VERR) != NCSCC_RC_FAILURE) {
					get_val = ntohl(get_val);
					dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN, NCSMIB_TBL_DTSV_SCLR_GLOBAL,
							 cli_hdl, dtsv_hdl, NULL);
					m_DTS_CLI_CAT_FLTR(get_val, bit_set, pArgv->cmd.intval, bit_map);
				}	/*end of if */
			}	/*end of else */
			bit_map = htonl(bit_map);
			rc = dtsv_set_mib_oct(&mib, ncsDtsvGlobalCategoryBitMap_ID, 0, &bit_map, 4, reqfnc, MIB_VERR);
		}		/*end of if i_arg_record[4] == NCSCLI_KEYWORD */
	}
	/*end of if i_arg_record[3] == NCSCLI_KEYWORD */
	else {
		oids[0] = pArgv->cmd.intval;
		pArgv = &arg_list->i_arg_record[4];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			/* Enable Node Logging */
			dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);

			pArgv = &arg_list->i_arg_record[5];
			if (strcmp(pArgv->cmd.strval, "set") == 0)
				bit_set = TRUE;
			else
				bit_set = FALSE;

			pArgv = &arg_list->i_arg_record[6];
			if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
				/* All bits are selected */
				bit_map = (bit_set) ? CAT_FILTER_SET_ALL_VAL : CAT_FILTER_NOSET_ALL_VAL;
				bit_map = htonl(bit_map);
				rc = dtsv_get_mib_oct(&mib, ncsDtsvNodeCategoryBitMap_ID, &get_val, &length, reqfnc,
						      MIB_VERR);
				if (rc != NCSCC_RC_FAILURE) {
					get_val = ntohl(get_val);
					dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl,
							 dtsv_hdl, NULL);
					/* Check return code for instance created */
					if (rc == NCSCC_RC_NO_INSTANCE)
						rc = dtsv_set_mib_oct(&mib, ncsDtsvNodeCategoryBitMap_ID,
								      ncsDtsvNodeRowStatus_ID, (uns8 *)&bit_map, 4,
								      reqfnc, MIB_VERR);
					else
						rc = dtsv_set_mib_oct(&mib, ncsDtsvNodeCategoryBitMap_ID, 0,
								      (uns8 *)&bit_map, 4, reqfnc, MIB_VERR);
				}
			} else {
				/* A particular bit has been set */
				/* First get the current value of category bit */
				rc = dtsv_get_mib_oct(&mib, ncsDtsvNodeCategoryBitMap_ID, &get_val, &length, reqfnc,
						      MIB_VERR);
				if (rc != NCSCC_RC_FAILURE) {
					get_val = ntohl(get_val);
					dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl,
							 dtsv_hdl, NULL);
					m_DTS_CLI_CAT_FLTR(get_val, bit_set, pArgv->cmd.intval, bit_map);
					bit_map = htonl(bit_map);
					if (rc == NCSCC_RC_NO_INSTANCE)
						rc = dtsv_set_mib_oct(&mib, ncsDtsvNodeCategoryBitMap_ID,
								      ncsDtsvNodeRowStatus_ID, (uns8 *)&bit_map, 4,
								      reqfnc, MIB_VERR);
					else
						rc = dtsv_set_mib_oct(&mib, ncsDtsvNodeCategoryBitMap_ID, 0,
								      (uns8 *)&bit_map, 4, reqfnc, MIB_VERR);
				}	/*end of if */
			}	/*end of else */
		} /*end for Node change */
		else {
			/* Enable Service Per Node Logging */
			oids[1] = pArgv->cmd.intval;
			dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN,
					 NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl, dtsv_hdl, NULL);

			pArgv = &arg_list->i_arg_record[5];
			if (strcmp(pArgv->cmd.strval, "set") == 0)
				bit_set = TRUE;
			else
				bit_set = FALSE;

			pArgv = &arg_list->i_arg_record[6];
			if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
				/* All bits are selected */
				bit_map = (bit_set) ? CAT_FILTER_SET_ALL_VAL : CAT_FILTER_NOSET_ALL_VAL;
				bit_map = htonl(bit_map);
				rc = dtsv_get_mib_oct(&mib, ncsDtsvServiceCategoryBitMap_ID, &get_val, &length, reqfnc,
						      MIB_VERR);
				if (rc != NCSCC_RC_FAILURE) {
					get_val = ntohl(get_val);
					dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN, NCSMIB_TBL_DTSV_SVC_PER_NODE,
							 cli_hdl, dtsv_hdl, NULL);
					if (rc == NCSCC_RC_NO_INSTANCE)
						rc = dtsv_set_mib_oct(&mib, ncsDtsvServiceCategoryBitMap_ID,
								      ncsDtsvServiceRowStatus_ID, (uns8 *)&bit_map, 4,
								      reqfnc, MIB_VERR);
					else
						rc = dtsv_set_mib_oct(&mib, ncsDtsvServiceCategoryBitMap_ID, 0,
								      (uns8 *)&bit_map, 4, reqfnc, MIB_VERR);
				}
			} else {
				/* A particular bit has been set */
				/* First get the current value of category bit */
				rc = dtsv_get_mib_oct(&mib, ncsDtsvServiceCategoryBitMap_ID, &get_val, &length, reqfnc,
						      MIB_VERR);
				if (rc != NCSCC_RC_FAILURE) {
					get_val = ntohl(get_val);
					dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN, NCSMIB_TBL_DTSV_SVC_PER_NODE,
							 cli_hdl, dtsv_hdl, NULL);
					m_DTS_CLI_CAT_FLTR(get_val, bit_set, pArgv->cmd.intval, bit_map);
					bit_map = htonl(bit_map);
					if (rc == NCSCC_RC_NO_INSTANCE)
						rc = dtsv_set_mib_oct(&mib, ncsDtsvServiceCategoryBitMap_ID,
								      ncsDtsvServiceRowStatus_ID, (uns8 *)&bit_map, 4,
								      reqfnc, MIB_VERR);
					else
						rc = dtsv_set_mib_oct(&mib, ncsDtsvServiceCategoryBitMap_ID, 0,
								      (uns8 *)&bit_map, 4, reqfnc, MIB_VERR);
				}	/*end of if */
			}	/*end of else */
		}		/* end of per service logging */

	}			/* end of else for i_arg_record[3] == NCSCLI_KEYWORD */
	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV configure category bit map failed");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_cnf_severity
  DESCRIPTION   : CEF for configuring the severity bit map
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_cnf_severity(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 dtsv_hdl = cef_data->i_bindery->i_mab_hdl;
	NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
	NCSCLI_ARG_VAL *pArgv = &arg_list->i_arg_record[3];
	uns32 rc = NCSCC_RC_SUCCESS, oids[SVC_INST_ID_LEN];
	uns32 length;
	uns8 get_val, bit_map;

	if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
		pArgv = &arg_list->i_arg_record[4];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			/* Enable global logging */
			dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN,
					 NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

			pArgv = &arg_list->i_arg_record[5];
			m_DTS_CLI_SEV_FLTR(pArgv, bit_map);

			rc = dtsv_set_mib_oct(&mib, ncsDtsvGlobalSeverityBitMap_ID, 0,
					      (uns8 *)&bit_map, 1, reqfnc, MIB_VERR);
		} else {
			/* Not Supported; Return Error */
			dtsv_cli_display(cli_hdl, "\nCommand arguments not supported");
			return NCSCC_RC_FAILURE;
		}
	} else {
		oids[0] = pArgv->cmd.intval;
		pArgv = &arg_list->i_arg_record[4];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			/* Enable Node logging */
			dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);

			pArgv = &arg_list->i_arg_record[5];
			m_DTS_CLI_SEV_FLTR(pArgv, bit_map);
			rc = dtsv_get_mib_oct(&mib, ncsDtsvNodeSeverityBitMap_ID, &get_val, &length, reqfnc, MIB_VERR);
			if (rc != NCSCC_RC_FAILURE) {
				dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl,
						 NULL);
				if (rc == NCSCC_RC_NO_INSTANCE)
					rc = dtsv_set_mib_oct(&mib, ncsDtsvNodeSeverityBitMap_ID,
							      ncsDtsvNodeRowStatus_ID, (uns8 *)&bit_map, 1, reqfnc,
							      MIB_VERR);
				else
					rc = dtsv_set_mib_oct(&mib, ncsDtsvNodeSeverityBitMap_ID, 0, (uns8 *)&bit_map,
							      1, reqfnc, MIB_VERR);
			}
		} else {
			/* Service per node Logging */
			oids[1] = pArgv->cmd.intval;

			dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN,
					 NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl, dtsv_hdl, NULL);

			pArgv = &arg_list->i_arg_record[5];
			m_DTS_CLI_SEV_FLTR(pArgv, bit_map);
			rc = dtsv_get_mib_oct(&mib, ncsDtsvServiceSeverityBitMap_ID, &get_val, &length, reqfnc,
					      MIB_VERR);
			if (rc != NCSCC_RC_FAILURE) {
				dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN, NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl,
						 dtsv_hdl, NULL);
				if (rc == NCSCC_RC_NO_INSTANCE)
					rc = dtsv_set_mib_oct(&mib, ncsDtsvServiceSeverityBitMap_ID,
							      ncsDtsvServiceRowStatus_ID, (uns8 *)&bit_map, 1, reqfnc,
							      MIB_VERR);
				else
					rc = dtsv_set_mib_oct(&mib, ncsDtsvServiceSeverityBitMap_ID, 0,
							      (uns8 *)&bit_map, 1, reqfnc, MIB_VERR);
			}

		}
	}

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV configure severity bit map failed");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_cnf_console
  DESCRIPTION   : CEF for configuring the console device
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_cnf_console(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	NCSCLI_ARG_VAL *pArgv = &arg_list->i_arg_record[2];
	uns32 rc = NCSCC_RC_SUCCESS, oids[SVC_INST_ID_LEN];
	NCS_UBAID cli_uba;
	uns8 *buff_ptr = NULL;
	char *cons_str = NULL;
	uns8 str_len = 0, bit_map = 0;	/* For configuring console severity */

	memset(&mib, '\0', sizeof(mib));

	/* Console printing - CLI will send a MASv CLI cmd to DTS */
	mib.i_op = NCSMIB_OP_REQ_CLI;	/* Operation type */
	mib.i_tbl_id = NCSMIB_TBL_DTSV_CMD;	/* CMD-TBL-ID */
	mib.i_rsp_fnc = NULL;
	mib.i_idx.i_inst_ids = NULL;
	mib.i_idx.i_inst_len = 0;
	ncsstack_init(&mib.stack, NCSMIB_STACK_SIZE);

	mib.req.info.cli_req.i_wild_card = FALSE;	/* set to TRUE for wild-card req */

	/* Initialize the userbuf */
	memset(&cli_uba, '\0', sizeof(cli_uba));
	if (ncs_enc_init_space(&cli_uba) != NCSCC_RC_SUCCESS) {
		dtsv_cli_display(cli_hdl, "\nUserbuf init failed");
		return NCSCC_RC_FAILURE;
	}
	m_BUFR_STUFF_OWNER(cli_uba.start);

	/* Parse the CLI CEF data */
	if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
		pArgv = &arg_list->i_arg_record[3];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			/* configuring global console device */
			pArgv = &arg_list->i_arg_record[4];

			if (strcmp(pArgv->cmd.strval, "add") == 0) {
				/* add Global console device */
				mib.req.info.cli_req.i_cmnd_id = dtsvAddGlobalConsole;
				/* retrieve the console device string */
				cons_str = arg_list->i_arg_record[5].cmd.strval;
				str_len = strlen(cons_str);
				/* Console device string shouldn't be more than 20 chars */
				if (str_len > DTS_CONS_DEV_MAX) {
					dtsv_cli_display(cli_hdl,
							 "\nSpecified console device exceeding 20 chars. Specify console correctly");
					return NCSCC_RC_FAILURE;
				}

				/* retrieve the severity specified */
				pArgv = &arg_list->i_arg_record[6];
				m_DTS_CLI_CONF_CONS(pArgv, bit_map);
				/* Encode the add console data */
				buff_ptr = ncs_enc_reserve_space(&cli_uba, DTSV_ADD_GBL_CONS_SIZE);
				if (buff_ptr == NULL) {
					dtsv_cli_display(cli_hdl, "\nReserve space failed");
					return NCSCC_RC_FAILURE;
				}

				ncs_encode_8bit(&buff_ptr, bit_map);
				ncs_encode_8bit(&buff_ptr, str_len);
				ncs_enc_claim_space(&cli_uba, DTSV_ADD_GBL_CONS_SIZE);

				if (ncs_encode_n_octets_in_uba(&cli_uba, cons_str, str_len) != NCSCC_RC_SUCCESS) {
					dtsv_cli_display(cli_hdl, "\nencode_n_octets_in_uba for device string failed");
					return NCSCC_RC_FAILURE;
				}

				/* Add uba to req's userbuf */
				mib.req.info.cli_req.i_usrbuf = cli_uba.start;

				/* send the request */
				rc = dtsv_send_cmd_mib_req(&mib, cef_data, MIB_VERR);
			} /*end of add global console */
			else if (strcmp(pArgv->cmd.strval, "rmv") == 0) {
				/* remove global console device */
				mib.req.info.cli_req.i_cmnd_id = dtsvRmvGlobalConsole;
				cons_str = arg_list->i_arg_record[5].cmd.strval;
				str_len = strlen(cons_str);
				/* Console device string shouldn't be more than 20 chars */
				if (str_len > DTS_CONS_DEV_MAX) {
					dtsv_cli_display(cli_hdl,
							 "\nSpecified console device exceeding 20 chars. Specify console correctly");
					return NCSCC_RC_FAILURE;
				}

				/* Encode the remove console data */
				buff_ptr = ncs_enc_reserve_space(&cli_uba, DTSV_RMV_GBL_CONS_SIZE);
				if (buff_ptr == NULL) {
					dtsv_cli_display(cli_hdl, "\nReserve space failed");
					return NCSCC_RC_FAILURE;
				}

				ncs_encode_8bit(&buff_ptr, str_len);
				ncs_enc_claim_space(&cli_uba, DTSV_RMV_GBL_CONS_SIZE);

				if (ncs_encode_n_octets_in_uba(&cli_uba, cons_str, str_len) != NCSCC_RC_SUCCESS) {
					dtsv_cli_display(cli_hdl, "\nencode_n_octets_in_uba for device string failed");
					return NCSCC_RC_FAILURE;
				}

				/* Add uba to req's userbuf */
				mib.req.info.cli_req.i_usrbuf = cli_uba.start;

				/* send the request */
				rc = dtsv_send_cmd_mib_req(&mib, cef_data, MIB_VERR);
			}	/*end of rmv global console */
		}
	} else {
		oids[0] = pArgv->cmd.intval;
		pArgv = &arg_list->i_arg_record[3];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			/* configuring node console device */
			pArgv = &arg_list->i_arg_record[4];

			if (strcmp(pArgv->cmd.strval, "add") == 0) {
				/* add Node console device */
				mib.req.info.cli_req.i_cmnd_id = dtsvAddNodeConsole;
				/* retrieve the console device string */
				cons_str = arg_list->i_arg_record[5].cmd.strval;
				str_len = strlen(cons_str);
				/* Console device string shouldn't be more than 20 chars */
				if (str_len > DTS_CONS_DEV_MAX) {
					dtsv_cli_display(cli_hdl,
							 "\nSpecified console device exceeding 20 chars. Specify console correctly");
					return NCSCC_RC_FAILURE;
				}

				/* retrieve the severity specified */
				pArgv = &arg_list->i_arg_record[6];
				m_DTS_CLI_CONF_CONS(pArgv, bit_map);

				/* Encode the add console data */
				buff_ptr = ncs_enc_reserve_space(&cli_uba, DTSV_ADD_NODE_CONS_SIZE);
				if (buff_ptr == NULL) {
					dtsv_cli_display(cli_hdl, "\nReserve space failed");
					return NCSCC_RC_FAILURE;
				}

				ncs_encode_8bit(&buff_ptr, bit_map);
				ncs_encode_32bit(&buff_ptr, oids[0]);
				ncs_encode_8bit(&buff_ptr, str_len);
				ncs_enc_claim_space(&cli_uba, DTSV_ADD_NODE_CONS_SIZE);

				if (ncs_encode_n_octets_in_uba(&cli_uba, cons_str, str_len) != NCSCC_RC_SUCCESS) {
					dtsv_cli_display(cli_hdl, "\nencode_n_octets_in_uba for device string failed");
					return NCSCC_RC_FAILURE;
				}

				/* Add uba to req's userbuf */
				mib.req.info.cli_req.i_usrbuf = cli_uba.start;

				/* send the request */
				rc = dtsv_send_cmd_mib_req(&mib, cef_data, MIB_VERR);
			} /*end of add node console */
			else if (strcmp(pArgv->cmd.strval, "rmv") == 0) {
				/* remove Node console device */
				mib.req.info.cli_req.i_cmnd_id = dtsvRmvNodeConsole;
				cons_str = arg_list->i_arg_record[5].cmd.strval;
				str_len = strlen(cons_str);
				/* Console device string shouldn't be more than 20 chars */
				if (str_len > DTS_CONS_DEV_MAX) {
					dtsv_cli_display(cli_hdl,
							 "\nSpecified console device exceeding 20 chars. Specify console correctly");
					return NCSCC_RC_FAILURE;
				}

				/* Encode the remove console data */
				buff_ptr = ncs_enc_reserve_space(&cli_uba, DTSV_RMV_NODE_CONS_SIZE);
				if (buff_ptr == NULL) {
					dtsv_cli_display(cli_hdl, "\nReserve space failed");
					return NCSCC_RC_FAILURE;
				}

				ncs_encode_32bit(&buff_ptr, oids[0]);
				ncs_encode_8bit(&buff_ptr, str_len);
				ncs_enc_claim_space(&cli_uba, DTSV_RMV_NODE_CONS_SIZE);

				if (ncs_encode_n_octets_in_uba(&cli_uba, cons_str, str_len) != NCSCC_RC_SUCCESS) {
					dtsv_cli_display(cli_hdl, "\nencode_n_octets_in_uba for device string failed");
					return NCSCC_RC_FAILURE;
				}

				/* Add uba to req's userbuf */
				mib.req.info.cli_req.i_usrbuf = cli_uba.start;

				/* send the request */
				rc = dtsv_send_cmd_mib_req(&mib, cef_data, MIB_VERR);
			}	/*end of rmv node console */
		} else {
			/* configuring service console device */
			oids[1] = pArgv->cmd.intval;
			pArgv = &arg_list->i_arg_record[4];

			if (strcmp(pArgv->cmd.strval, "add") == 0) {
				/* add Service console device */
				mib.req.info.cli_req.i_cmnd_id = dtsvAddSvcConsole;
				/* retrieve the console device string */
				cons_str = arg_list->i_arg_record[5].cmd.strval;
				str_len = strlen(cons_str);
				/* Console device string shouldn't be more than 20 chars */
				if (str_len > DTS_CONS_DEV_MAX) {
					dtsv_cli_display(cli_hdl,
							 "\nSpecified console device exceeding 20 chars. Specify console correctly");
					return NCSCC_RC_FAILURE;
				}

				/* retrieve the severity specified */
				pArgv = &arg_list->i_arg_record[6];
				m_DTS_CLI_CONF_CONS(pArgv, bit_map);
				/* Encode the add console data */
				buff_ptr = ncs_enc_reserve_space(&cli_uba, DTSV_ADD_SVC_CONS_SIZE);
				if (buff_ptr == NULL) {
					dtsv_cli_display(cli_hdl, "\nReserve space failed");
					return NCSCC_RC_FAILURE;
				}

				ncs_encode_8bit(&buff_ptr, bit_map);
				ncs_encode_32bit(&buff_ptr, oids[0]);
				ncs_encode_32bit(&buff_ptr, oids[1]);
				ncs_encode_8bit(&buff_ptr, str_len);
				ncs_enc_claim_space(&cli_uba, DTSV_ADD_SVC_CONS_SIZE);

				if (ncs_encode_n_octets_in_uba(&cli_uba, cons_str, str_len) != NCSCC_RC_SUCCESS) {
					dtsv_cli_display(cli_hdl, "\nencode_n_octets_in_uba for device string failed");
					return NCSCC_RC_FAILURE;
				}

				/* Add uba to req's userbuf */
				mib.req.info.cli_req.i_usrbuf = cli_uba.start;

				/* send the request */
				rc = dtsv_send_cmd_mib_req(&mib, cef_data, MIB_VERR);
			} /*end of add svc console */
			else if (strcmp(pArgv->cmd.strval, "rmv") == 0) {
				/* remove Service console device */
				mib.req.info.cli_req.i_cmnd_id = dtsvRmvSvcConsole;
				cons_str = arg_list->i_arg_record[5].cmd.strval;
				str_len = strlen(cons_str);
				/* Console device string shouldn't be more than 20 chars */
				if (str_len > DTS_CONS_DEV_MAX) {
					dtsv_cli_display(cli_hdl,
							 "\nSpecified console device exceeding 20 chars. Specify console correctly");
					return NCSCC_RC_FAILURE;
				}

				/* Encode the remove console data */
				buff_ptr = ncs_enc_reserve_space(&cli_uba, DTSV_RMV_SVC_CONS_SIZE);
				if (buff_ptr == NULL) {
					dtsv_cli_display(cli_hdl, "\nReserve space failed");
					return NCSCC_RC_FAILURE;
				}

				ncs_encode_32bit(&buff_ptr, oids[0]);
				ncs_encode_32bit(&buff_ptr, oids[1]);
				ncs_encode_8bit(&buff_ptr, str_len);
				ncs_enc_claim_space(&cli_uba, DTSV_RMV_SVC_CONS_SIZE);

				if (ncs_encode_n_octets_in_uba(&cli_uba, cons_str, str_len) != NCSCC_RC_SUCCESS) {
					dtsv_cli_display(cli_hdl, "\nencode_n_octets_in_uba for device string failed");
					return NCSCC_RC_FAILURE;
				}

				/* Add uba to req's userbuf */
				mib.req.info.cli_req.i_usrbuf = cli_uba.start;

				/* send the request */
				rc = dtsv_send_cmd_mib_req(&mib, cef_data, MIB_VERR);
			}	/*end of rmv svc console */
		}
	}

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV configure console device failed");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;

}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_disp_console
  DESCRIPTION   : CEF for displaying the console devices currently asscociated 
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_disp_console(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	NCSCLI_ARG_VAL *pArgv = &arg_list->i_arg_record[2];
	uns32 rc = NCSCC_RC_SUCCESS, oids[SVC_INST_ID_LEN];
	NCS_UBAID cli_uba;
	uns8 *buff_ptr = NULL;

	memset(&mib, '\0', sizeof(mib));

	/* Displaying console  devices - CLI will send a MASv CLI cmd to DTS */
	mib.i_op = NCSMIB_OP_REQ_CLI;	/* Operation type */
	mib.i_tbl_id = NCSMIB_TBL_DTSV_CMD;	/* CMD-TBL-ID */
	mib.i_rsp_fnc = (NCSMIB_RSP_FNC)dtsv_cli_cmd_rsp;
	mib.i_idx.i_inst_ids = NULL;
	mib.i_idx.i_inst_len = 0;
	ncsstack_init(&mib.stack, NCSMIB_STACK_SIZE);

	mib.req.info.cli_req.i_wild_card = FALSE;	/* set to TRUE for wild-card req */

	/* Initialize userbuf */
	memset(&cli_uba, '\0', sizeof(cli_uba));
	if (ncs_enc_init_space(&cli_uba) != NCSCC_RC_SUCCESS) {
		dtsv_cli_display(cli_hdl, "\nUserbuf init failed");
		return NCSCC_RC_FAILURE;
	}
	m_BUFR_STUFF_OWNER(cli_uba.start);

	/* Parse the CLI CEF data */
	if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
		pArgv = &arg_list->i_arg_record[3];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			/* display global console device */
			mib.req.info.cli_req.i_cmnd_id = dtsvDispGlobalConsole;
			mib.req.info.cli_req.i_usrbuf = NULL;
			/* send the request */
			rc = dtsv_send_cmd_mib_req(&mib, cef_data, MIB_VERR);
		}
	} /*end of display global level consoles */
	else {
		oids[0] = pArgv->cmd.intval;
		pArgv = &arg_list->i_arg_record[3];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			/* display node console device */
			mib.req.info.cli_req.i_cmnd_id = dtsvDispNodeConsole;

			/* Encode node_id */
			buff_ptr = ncs_enc_reserve_space(&cli_uba, sizeof(uns32));
			if (buff_ptr == NULL) {
				dtsv_cli_display(cli_hdl, "\nReserve space failed");
				return NCSCC_RC_FAILURE;
			}

			ncs_encode_32bit(&buff_ptr, oids[0]);
			ncs_enc_claim_space(&cli_uba, sizeof(uns32));

			/* Add uba to req's userbuf */
			mib.req.info.cli_req.i_usrbuf = cli_uba.start;

			/* send the request */
			rc = dtsv_send_cmd_mib_req(&mib, cef_data, MIB_VERR);
		} /*end of display node level console devices */
		else {
			/* display service console device */
			oids[1] = pArgv->cmd.intval;
			mib.req.info.cli_req.i_cmnd_id = dtsvDispSvcConsole;

			/* Encode node_id */
			buff_ptr = ncs_enc_reserve_space(&cli_uba, 2 * sizeof(uns32));
			if (buff_ptr == NULL) {
				dtsv_cli_display(cli_hdl, "\nReserve space failed");
				return NCSCC_RC_FAILURE;
			}

			ncs_encode_32bit(&buff_ptr, oids[0]);
			ncs_encode_32bit(&buff_ptr, oids[1]);
			ncs_enc_claim_space(&cli_uba, 2 * sizeof(uns32));

			/* Add uba to req's userbuf */
			mib.req.info.cli_req.i_usrbuf = cli_uba.start;

			/* send the request */
			rc = dtsv_send_cmd_mib_req(&mib, cef_data, MIB_VERR);
		}		/*end of display service level console devices */
	}

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV display console devices failed");
		return rc;
	} else {
		rc = dtsv_cli_cmd_rsp(&mib, cli_hdl);
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return rc;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_spec_reload
  DESCRIPTION   : CEF for reloading ASCII_SPEC libraries
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_spec_reload(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 rc = NCSCC_RC_SUCCESS;

	memset(&mib, '\0', sizeof(mib));

	/* ASCII_SPEC reload - CLI will send a MASv CLI cmd to DTS */
	mib.i_op = NCSMIB_OP_REQ_CLI;	/* Operation type */
	mib.i_tbl_id = NCSMIB_TBL_DTSV_CMD;	/* CMD-TBL-ID */
	mib.i_rsp_fnc = NULL;
	mib.i_idx.i_inst_ids = NULL;
	mib.i_idx.i_inst_len = 0;
	ncsstack_init(&mib.stack, NCSMIB_STACK_SIZE);

	mib.req.info.cli_req.i_wild_card = FALSE;	/* set to TRUE for wild-card req */

	/* Set command id */
	mib.req.info.cli_req.i_cmnd_id = dtsvAsciiSpecReload;
	/* Set the USERBUF to NULL */
	mib.req.info.cli_req.i_usrbuf = NULL;

	/* send the request */
	rc = dtsv_send_cmd_mib_req(&mib, cef_data, MIB_VERR);

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV reload ASCII_SPEC tables failed");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_print_config 
  DESCRIPTION   : CEF for printing current config for DTS. 
  ARGUMENTS     :
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_print_config(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 rc = NCSCC_RC_SUCCESS;

	memset(&mib, '\0', sizeof(mib));

	/* CLI will send a MASv CLI cmd to DTS for printing current config */
	mib.i_op = NCSMIB_OP_REQ_CLI;	/* Operation type */
	mib.i_tbl_id = NCSMIB_TBL_DTSV_CMD;	/* CMD-TBL-ID */
	mib.i_rsp_fnc = NULL;
	mib.i_idx.i_inst_ids = NULL;
	mib.i_idx.i_inst_len = 0;
	ncsstack_init(&mib.stack, NCSMIB_STACK_SIZE);

	mib.req.info.cli_req.i_wild_card = FALSE;	/* set to TRUE for wild-card req */

	/* Set command id */
	mib.req.info.cli_req.i_cmnd_id = dtsvPrintCurrConfig;
	/* Set the USERBUF to NULL */
	mib.req.info.cli_req.i_usrbuf = NULL;

	/* send the request */
	rc = dtsv_send_cmd_mib_req(&mib, cef_data, MIB_VERR);

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV print current config for DTS failed");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_cnf_num_log_files
  DESCRIPTION   : CEF for configuring the number of log files
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_cnf_num_log_files(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 dtsv_hdl = cef_data->i_bindery->i_mab_hdl;
	NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
	NCSCLI_ARG_VAL *pArgv = &arg_list->i_arg_record[5];
	uns32 rc = NCSCC_RC_SUCCESS, oids[SVC_INST_ID_LEN];

	dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN, NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

	rc = dtsv_set_mib_int(&mib, ncsDtsvGlobalNumOfLogFiles_ID, 0, pArgv->cmd.intval, reqfnc, MIB_VERR);

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV configure number of log files failed");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_enable_sequencing
  DESCRIPTION   : CEF for enabling message sequencing.
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_enable_sequencing(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 dtsv_hdl = cef_data->i_bindery->i_mab_hdl;
	NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
	uns32 rc = NCSCC_RC_SUCCESS, oids[SVC_INST_ID_LEN];

	dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN, NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

	rc = dtsv_set_mib_int(&mib, ncsDtsvGlobalLogMsgSequencing_ID, 0, NCS_SNMP_TRUE, reqfnc, MIB_VERR);

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV sequencing enable failed");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_disable_sequencing
  DESCRIPTION   : CEF for disabling message sequencing.
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_disable_sequencing(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 dtsv_hdl = cef_data->i_bindery->i_mab_hdl;
	NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
	uns32 rc = NCSCC_RC_SUCCESS, oids[SVC_INST_ID_LEN];

	dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN, NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

	rc = dtsv_set_mib_int(&mib, ncsDtsvGlobalLogMsgSequencing_ID, 0, NCS_SNMP_FALSE, reqfnc, MIB_VERR);

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV sequencing disable failed");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_close_open_files
  DESCRIPTION   : CEF for closing all the open files currently being used 
                  for logging. DTS then will start logging into new log files.
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_close_open_files(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 dtsv_hdl = cef_data->i_bindery->i_mab_hdl;
	NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
	uns32 rc = NCSCC_RC_SUCCESS, oids[SVC_INST_ID_LEN];

	dtsv_cfg_mib_arg(&mib, oids, GLOBAL_INST_ID_LEN, NCSMIB_TBL_DTSV_SCLR_GLOBAL, cli_hdl, dtsv_hdl, NULL);

	rc = dtsv_set_mib_int(&mib, ncsDtsvGlobalCloseOpenFiles_ID, 0, NCS_SNMP_TRUE, reqfnc, MIB_VERR);

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV: Fail to to close all the files.");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_dump_buff_log
  DESCRIPTION   : CEF for dumping the log stored in circular buffer.
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_dump_buff_log(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 dtsv_hdl = cef_data->i_bindery->i_mab_hdl;
	NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
	NCSCLI_ARG_VAL *pArgv = &arg_list->i_arg_record[3];
	uns32 rc = NCSCC_RC_SUCCESS, oids[SVC_INST_ID_LEN];
	uns8 device;
	uns32 operation, node_id, svc_id;

	oids[0] = 1;
	oids[1] = 1;
	dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN, NCSMIB_TBL_DTSV_CIRBUFF_OP, cli_hdl, dtsv_hdl, NULL);

	if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
		pArgv = &arg_list->i_arg_record[4];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			/* Dump log from global buffer */
			pArgv = &arg_list->i_arg_record[5];
			operation = pArgv->cmd.intval;

			pArgv = &arg_list->i_arg_record[6];
			if (strcmp(pArgv->cmd.strval, "file") == 0) {
				device = LOG_FILE;
			} else
				device = OUTPUT_CONSOLE;

			rc = dtsv_cirbuff_setrow(&mib, reqfnc, 0, 0, operation, device, MIB_VERR);
		} else {
			/* Not Supported; Return Error */
			dtsv_cli_display(cli_hdl, "\nCommand arguments not supported");
			return NCSCC_RC_FAILURE;
		}
	} else {
		node_id = pArgv->cmd.intval;
		pArgv = &arg_list->i_arg_record[4];

		if (pArgv->i_arg_type == NCSCLI_KEYWORD) {
			/* Dump log from per node buffer */
			pArgv = &arg_list->i_arg_record[5];
			operation = pArgv->cmd.intval;

			pArgv = &arg_list->i_arg_record[6];
			if (strcmp(pArgv->cmd.strval, "file") == 0) {
				device = LOG_FILE;
			} else
				device = OUTPUT_CONSOLE;

			rc = dtsv_cirbuff_setrow(&mib, reqfnc, node_id, 0, operation, device, MIB_VERR);
		} else {
			svc_id = pArgv->cmd.intval;

			/* Dump log from service per node buffer */
			pArgv = &arg_list->i_arg_record[5];
			operation = pArgv->cmd.intval;

			pArgv = &arg_list->i_arg_record[6];
			if (strcmp(pArgv->cmd.strval, "file") == 0) {
				device = LOG_FILE;
			} else
				device = OUTPUT_CONSOLE;

			rc = dtsv_cirbuff_setrow(&mib, reqfnc, node_id, svc_id, operation, device, MIB_VERR);
		}
	}

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV - Unable to dump buffer log");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;

}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cef_dump_buff_log
  DESCRIPTION   : CEF Setting Policy to default. This CEF destroyes the row.
  ARGUMENTS     : 
                  arg_list : ARG list pointer
                  cef_data : CEF data pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cef_set_dflt_plcy(NCSCLI_ARG_SET *arg_list, NCSCLI_CEF_DATA *cef_data)
{
	NCSMIB_ARG mib;
	uns32 cli_hdl = cef_data->i_bindery->i_cli_hdl;
	uns32 dtsv_hdl = cef_data->i_bindery->i_mab_hdl;
	NCSMIB_REQ_FNC reqfnc = cef_data->i_bindery->i_req_fnc;
	NCSCLI_ARG_VAL *pArgv = &arg_list->i_arg_record[3];
	uns32 rc = NCSCC_RC_SUCCESS, oids[SVC_INST_ID_LEN];

	if (strcmp(pArgv->cmd.strval, "node") == 0) {
		pArgv = &arg_list->i_arg_record[4];

		oids[0] = pArgv->cmd.intval;

		dtsv_cfg_mib_arg(&mib, oids, NODE_INST_ID_LEN, NCSMIB_TBL_DTSV_NODE, cli_hdl, dtsv_hdl, NULL);

		rc = dtsv_set_mib_int_val(&mib, ncsDtsvNodeRowStatus_ID, NCSMIB_ROWSTATUS_DESTROY, reqfnc, MIB_VERR);

	} else if (strcmp(pArgv->cmd.strval, "service") == 0) {
		pArgv = &arg_list->i_arg_record[4];
		oids[0] = pArgv->cmd.intval;

		pArgv = &arg_list->i_arg_record[5];
		oids[1] = pArgv->cmd.intval;

		dtsv_cfg_mib_arg(&mib, oids, SVC_INST_ID_LEN, NCSMIB_TBL_DTSV_SVC_PER_NODE, cli_hdl, dtsv_hdl, NULL);

		rc = dtsv_set_mib_int_val(&mib, ncsDtsvServiceRowStatus_ID, NCSMIB_ROWSTATUS_DESTROY, reqfnc, MIB_VERR);

	} else
		rc = NCSCC_RC_FAILURE;

	if (NCSCC_RC_SUCCESS != rc) {
		dtsv_cli_display(cli_hdl, "\nDTSV log device set failed");
		return rc;
	} else {
		m_RETURN_DTSV_CLI_DONE(0, rc, cli_hdl);
	}

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cirbuff_setrow
  DESCRIPTION   : Function for printing CLI string.
  ARGUMENTS     : 
                  mib     : Mib arg.
                  reqfunc : Request function
              node_id : Node ID
              svc_id  : Service ID
              operation : Operation to be carried out
              device  : Output device.
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32
dtsv_cirbuff_setrow(NCSMIB_ARG *mib,
		    NCSMIB_REQ_FNC reqfunc, uns32 node_id, uns32 svc_id, uns32 operation, uns8 device, char *err)
{
	NCSPARM_AID rsp_pa;
	NCSMIB_PARAM_VAL param_val;
	NCSMEM_AID mem_aid;
	uns8 space[100];
	int paid;

	ncsparm_enc_init(&rsp_pa);
	ncsmem_aid_init(&mem_aid, space, 1024);
	memset(&param_val, 0, sizeof(NCSMIB_PARAM_VAL));

	mib->i_op = NCSMIB_OP_REQ_SETROW;

	param_val.i_fmat_id = NCSMIB_FMAT_INT;

	for (paid = dtsvCirbuffOpIndexNode_Id; paid < dtsvCirBuffOpMax_Id; paid++) {
		param_val.i_param_id = paid;

		if (paid == dtsvCirbuffOpIndexNode_Id) {
			param_val.i_length = 4;
			param_val.info.i_int = node_id;
		} else if (paid == dtsvCirbuffOpIndexService_Id) {
			param_val.i_length = 1;
			param_val.info.i_int = svc_id;
		} else if (paid == dtsvCirbuffOpOperation) {
			param_val.i_length = 1;
			param_val.info.i_int = operation;
		} else if (paid == dtsvCirbuffOpDevice) {
			param_val.i_length = 1;
			param_val.info.i_int = device;
		} else
			return NCSCC_RC_FAILURE;

		ncsparm_enc_param(&rsp_pa, &param_val);
	}

	mib->req.info.setrow_req.i_usrbuf = ncsparm_enc_done(&rsp_pa);

	ncsmib_sync_request(mib, reqfunc, DTSV_MOTV, &mem_aid);

	return dtsv_chk_mib_rsp(err, mib, mib->i_usr_key);

}

/*****************************************************************************
  PROCEDURE NAME: dtsv_set_mib_int
  DESCRIPTION   : Creates the Set request for Integer type object. Also sets
                  the row status to active if objects are not scalar.
  ARGUMENTS     :
                  mib      : Pointer to NCSMIB_ARG structure
                  pid      : Param id
                  val      : Param value                  
                  reqfnc   : Request function
                  err      : Error string
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_set_mib_int(NCSMIB_ARG *mib, uns32 pid, uns32 r_status, uns32 val, NCSMIB_REQ_FNC reqfnc, char *err)
{
	NCSMEM_AID mem_aid;
	uns8 space[100];

	ncsmem_aid_init(&mem_aid, space, 1024);

	if (r_status != 0) {
		NCSPARM_AID rsp_pa;
		NCSMIB_PARAM_VAL param_val;
		ncsparm_enc_init(&rsp_pa);
		memset(&param_val, 0, sizeof(NCSMIB_PARAM_VAL));

		mib->i_op = NCSMIB_OP_REQ_SETROW;

		param_val.i_fmat_id = NCSMIB_FMAT_INT;

		param_val.i_param_id = pid;
		param_val.i_length = 4;
		param_val.info.i_int = val;

		ncsparm_enc_param(&rsp_pa, &param_val);
		param_val.i_param_id = r_status;
		param_val.info.i_int = NCSMIB_ROWSTATUS_CREATE_GO;
		ncsparm_enc_param(&rsp_pa, &param_val);

		mib->req.info.setrow_req.i_usrbuf = ncsparm_enc_done(&rsp_pa);

		ncsmib_sync_request(mib, reqfnc, DTSV_MOTV, &mem_aid);
	} else {

		NCSMIB_SET_REQ *set = &mib->req.info.set_req;

		mib->i_op = NCSMIB_OP_REQ_SET;
		set->i_param_val.i_param_id = pid;
		set->i_param_val.i_length = 4;
		set->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
		set->i_param_val.info.i_int = val;

		ncsmib_sync_request(mib, reqfnc, DTSV_MOTV, &mem_aid);

	}

	return dtsv_chk_mib_rsp(err, mib, mib->i_usr_key);
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_set_mib_oct
  DESCRIPTION   : Creates the Set request for Octet type object. Also sets
                  the row status to active if objects are not scalar.
  ARGUMENTS     :
                  mib      : Pointer to NCSMIB_ARG structure
                  pid      : Param id
                  val      : Param value                  
                  reqfnc   : Request function
                  err      : Error string
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_set_mib_oct(NCSMIB_ARG *mib,
		       uns32 pid, uns32 r_status, NCSCONTEXT val, uns16 len, NCSMIB_REQ_FNC reqfnc, char *err)
{
	NCSMEM_AID mem_aid;
	uns8 space[100];

	ncsmem_aid_init(&mem_aid, space, 1024);

	if (r_status != 0) {
		NCSPARM_AID rsp_pa;
		NCSMIB_PARAM_VAL param_val;
		ncsparm_enc_init(&rsp_pa);
		memset(&param_val, 0, sizeof(NCSMIB_PARAM_VAL));

		mib->i_op = NCSMIB_OP_REQ_SETROW;

		param_val.i_fmat_id = NCSMIB_FMAT_OCT;
		param_val.i_param_id = pid;
		param_val.info.i_oct = (uns8 *)val;
		param_val.i_length = len;

		ncsparm_enc_param(&rsp_pa, &param_val);
		param_val.i_fmat_id = NCSMIB_FMAT_INT;
		param_val.i_param_id = r_status;
		param_val.i_length = 4;
		param_val.info.i_int = NCSMIB_ROWSTATUS_CREATE_GO;
		ncsparm_enc_param(&rsp_pa, &param_val);

		mib->req.info.setrow_req.i_usrbuf = ncsparm_enc_done(&rsp_pa);

		ncsmib_sync_request(mib, reqfnc, DTSV_MOTV, &mem_aid);
	} else {
		NCSMIB_SET_REQ *set = &mib->req.info.set_req;

		mib->i_op = NCSMIB_OP_REQ_SET;
		set->i_param_val.i_param_id = pid;
		set->i_param_val.i_fmat_id = NCSMIB_FMAT_OCT;
		set->i_param_val.i_length = len;
		set->i_param_val.info.i_oct = (uns8 *)val;
		ncsmib_sync_request(mib, reqfnc, DTSV_MOTV, &mem_aid);
	}
	return dtsv_chk_mib_rsp(err, mib, mib->i_usr_key);
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_set_mib_int_val
  DESCRIPTION   : Creates the Set request for Integer type object.
  ARGUMENTS     :
                  mib      : Pointer to NCSMIB_ARG structure
                  pid      : Param id
                  val      : Param value                  
                  reqfnc   : Request function
                  err      : Error string
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_set_mib_int_val(NCSMIB_ARG *mib, uns32 pid, uns32 val, NCSMIB_REQ_FNC reqfnc, char *err)
{
	NCSMEM_AID mem_aid;
	uns8 space[100];
	NCSMIB_SET_REQ *set = &mib->req.info.set_req;

	ncsmem_aid_init(&mem_aid, space, 1024);

	mib->i_op = NCSMIB_OP_REQ_SET;
	set->i_param_val.i_param_id = pid;
	set->i_param_val.i_fmat_id = NCSMIB_FMAT_INT;
	set->i_param_val.info.i_int = val;

	ncsmib_sync_request(mib, reqfnc, DTSV_MOTV, &mem_aid);

	return dtsv_chk_mib_rsp(err, mib, mib->i_usr_key);
}

/****************************************************************************
  Name          :  dtsv_send_cmd_mib_req

  Description   :  This function sends the MASv CLI commands issued.

  Arguments     :  bindery  - CLI bindery data

  Return Values :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

  Notes         :
****************************************************************************/
uns32 dtsv_send_cmd_mib_req(NCSMIB_ARG *mib, NCSCLI_CEF_DATA *cef_data, char *err)
{
	NCSMEM_AID mem_aid;
	uns8 space[100];

	ncsmem_aid_init(&mem_aid, space, 1024);

	if (cef_data == NULL)
		return NCSCC_RC_FAILURE;

	mib->i_usr_key = cef_data->i_bindery->i_cli_hdl;
	mib->i_mib_key = cef_data->i_bindery->i_mab_hdl;

	ncsmib_sync_request(mib, cef_data->i_bindery->i_req_fnc, DTSV_MOTV, &mem_aid);

	return dtsv_chk_mib_rsp(err, mib, mib->i_usr_key);

}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cli_display
  DESCRIPTION   : Function for printing CLI string.
  ARGUMENTS     : 
                  cli_hdl : CLI handle.
                  str     : string to be printed.                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cli_display(uns32 cli_hdl, char *str)
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
  PROCEDURE NAME: dtsv_cli_done
  DESCRIPTION   : DTSv CLI done.
  ARGUMENTS     : 
                  cli_hdl : CLI handle.
                  str     : string to be printed.                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cli_done(uns8 *string, uns32 status, uns32 cli_hdl)
{
	NCSCLI_OP_INFO req;
	uns32 rc = NCSCC_RC_SUCCESS;

	if (string) {
		dtsv_cli_display(cli_hdl, (char *)string);
	}

	memset(&req, 0, sizeof(NCSCLI_OP_INFO));
	req.i_hdl = cli_hdl;
	req.i_req = NCSCLI_OPREQ_DONE;
	req.info.i_done.i_status = status;

	rc = ncscli_opr_request(&req);

	return rc;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cfg_mib_arg
  DESCRIPTION   : Populats/Initializes the MIB arg structure.
  ARGUMENTS     :
                  mib      : Pointer to NCSMIB_ARG structure
                  index    : Index of the table
                  index_len: Index length
                  tid      : Table ID
                  usr_key  : MIB Key pointer
                  ss_key   : Subsystem key pointer
                  rspfnc   : Response function
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cfg_mib_arg(NCSMIB_ARG *mib,
		       uns32 *index,
		       uns32 index_len, NCSMIB_TBL_ID tid, uns32 usr_hdl, uns32 ss_hdl, NCSMIB_RSP_FNC rspfnc)
{

	ncsmib_init(mib);

	mib->i_idx.i_inst_ids = index;
	mib->i_idx.i_inst_len = index_len;
	mib->i_tbl_id = tid;
	mib->i_usr_key = usr_hdl;
	mib->i_mib_key = ss_hdl;
	mib->i_rsp_fnc = rspfnc;

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_get_mib_int
  DESCRIPTION   : Creates the Get request for Integer type object.
  ARGUMENTS     :
                  mib      : Pointer to NCSMIB_ARG structure
                  pid      : Param id
                  val      : Param value                  
                  reqfnc   : Request function
                  err      : Error string
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_get_mib_int(NCSMIB_ARG *mib, uns32 pid, uns8 *val, NCSMIB_REQ_FNC reqfnc, char *err)
{
	NCSMEM_AID mem_aid;
	uns8 space[100];
	NCSMIB_GET_REQ *get = &mib->req.info.get_req;

	ncsmem_aid_init(&mem_aid, space, 1024);

	mib->i_op = NCSMIB_OP_REQ_GET;
	get->i_param_id = pid;

	if (ncsmib_sync_request(mib, reqfnc, DTSV_MOTV, &mem_aid) != NCSCC_RC_SUCCESS) {
		return NCSCC_RC_FAILURE;
	}
	if (mib->rsp.i_status == NCSCC_RC_NO_INSTANCE) {
		*val = 0;
		return NCSCC_RC_SUCCESS;
	}

	if (dtsv_chk_mib_rsp(err, mib, mib->i_usr_key) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	*val = mib->rsp.info.get_rsp.i_param_val.info.i_int;
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_get_mib_oct
  DESCRIPTION   : Creates the Get request for Octet type object.
  ARGUMENTS     :
                  mib      : Pointer to NCSMIB_ARG structure
                  pid      : Param id
                  val      : Param value                  
                  reqfnc   : Request function
                  err      : Error string
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_get_mib_oct(NCSMIB_ARG *mib, uns32 pid, NCSCONTEXT val, uns32 *len, NCSMIB_REQ_FNC reqfnc, char *err)
{
	NCSMEM_AID mem_aid;
	uns8 space[100];
	NCSMIB_GET_REQ *get = &mib->req.info.get_req;

	ncsmem_aid_init(&mem_aid, space, 1024);

	mib->i_op = NCSMIB_OP_REQ_GET;
	get->i_param_id = pid;

	ncsmib_sync_request(mib, reqfnc, DTSV_MOTV, &mem_aid);

	if (mib->rsp.i_status == NCSCC_RC_NO_INSTANCE) {
		*(uns8 *)val = 0;
		return NCSCC_RC_NO_INSTANCE;
	}

	if (dtsv_chk_mib_rsp(err, mib, mib->i_usr_key) != NCSCC_RC_SUCCESS)
		return NCSCC_RC_FAILURE;

	*len = mib->rsp.info.get_rsp.i_param_val.i_length;
	if (*len == 0)
		*(uns32 *)val = 0;
	else if (*len == 1)
		*(uns8 *)val = *(uns8 *)mib->rsp.info.get_rsp.i_param_val.info.i_oct;
	else if (*len == 2)
		*(uns16 *)val = *(uns16 *)mib->rsp.info.get_rsp.i_param_val.info.i_oct;
	else if (*len == 4)
		*(uns32 *)val = *(uns32 *)mib->rsp.info.get_rsp.i_param_val.info.i_oct;
	/*else
	   val = mib->rsp.info.get_rsp.i_param_val.info.i_oct; */

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_chk_mib_rsp
  DESCRIPTION   : Validates the response of the Set request.
  ARGUMENTS     :
                  err      : Error string
                  arg      : Pointer to NCSMIB_ARG structure
                  cli_key  : CLI key pointer                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_chk_mib_rsp(char *err, NCSMIB_ARG *arg, uns32 cli_hdl)
{
	char bigerr[DTSV_BUFFER_LEN];

	if (err == NULL)
		err = "";

	if (arg->rsp.i_status != NCSCC_RC_SUCCESS) {
		char *mib_err = "Unknown MIB Error!!";

		strcpy(bigerr, err);
		strcat(bigerr, ": ");

		switch (arg->rsp.i_status) {
		case NCSCC_RC_NO_SUCH_TBL:
			mib_err = "No such table";
			break;
		case NCSCC_RC_NO_OBJECT:
			mib_err = "No object";
			break;
		case NCSCC_RC_NO_INSTANCE:
			mib_err = "No instance";
			break;
		case NCSCC_RC_INV_VAL:
			mib_err = "Invalid value";
			break;
		case NCSCC_RC_INV_SPECIFIC_VAL:
			mib_err = "Invalid specific value";
			break;
		case NCSCC_RC_REQ_TIMOUT:
			mib_err = "Request timeout";
			break;
		case NCSCC_RC_FAILURE:
			mib_err = "Failure";
			break;
		default:
			break;
		}

		strcat(bigerr, mib_err);
		if (NCSCC_RC_INV_SPECIFIC_VAL == arg->rsp.i_status) {
			strcat(bigerr, " - Check if \"row-status\" is set to \"not-in-service\" or check logs ");
		}
		strcat(bigerr, "\n");

		dtsv_cli_display(cli_hdl, bigerr);

		return NCSCC_RC_FAILURE;
	}
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  PROCEDURE NAME: dtsv_cli_cmd_rsp
  DESCRIPTION   : Response function for CLI commands.
  ARGUMENTS     :
                  resp      : Pointer to NCSMIB_ARG structure
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
uns32 dtsv_cli_cmd_rsp(NCSMIB_ARG *resp, uns32 cli_hdl)
{
	USRBUF *buff = resp->rsp.info.cli_rsp.o_answer;
	NCS_UBAID uba;
	uns32 count;
	uns8 *buff_ptr, data_buff[DTSV_CLI_MAX_SIZE] = "";
	uns8 bit_map, str_len, cons_count;
	char cli_display[50];

	/* Check response */
	if ((resp->rsp.i_status == NCSCC_RC_FAILURE) || (buff == NULL)) {
		dtsv_cli_done("\nCommand Failed", NCSCC_RC_FAILURE, cli_hdl);
		goto error_return;
	} else {
		memset(&uba, '\0', sizeof(uba));

		ncs_dec_init_space(&uba, buff);
		buff_ptr = ncs_dec_flatten_space(&uba, data_buff, sizeof(uns8));
		if (buff_ptr == NULL) {
			dtsv_cli_done("\nDecode failed", NCSCC_RC_FAILURE, cli_hdl);
			goto error_return;
		}
		cons_count = ncs_decode_8bit(&buff_ptr);
		ncs_dec_skip_space(&uba, sizeof(uns8));

		for (count = 0; count < cons_count; count++) {
			memset(cli_display, '\0', sizeof(cli_display));
			memset(data_buff, '\0', sizeof(data_buff));

			buff_ptr = ncs_dec_flatten_space(&uba, data_buff, DTSV_RSP_CONS_SIZE);
			if (buff_ptr == NULL) {
				dtsv_cli_done("\nDecode failed", NCSCC_RC_FAILURE, cli_hdl);
				goto error_return;
			}

			bit_map = ncs_decode_8bit(&buff_ptr);
			ncs_dec_skip_space(&uba, sizeof(uns8));
			str_len = ncs_decode_8bit(&buff_ptr);
			ncs_dec_skip_space(&uba, sizeof(uns8));

			/* Decode the console device now */
			if (ncs_decode_n_octets_from_uba(&uba, (char *)&data_buff, str_len) != NCSCC_RC_SUCCESS) {
				dtsv_cli_done("\nDecode failed", NCSCC_RC_FAILURE, cli_hdl);
				goto error_return;
			}

			sprintf(cli_display, "\nDevice:%s with severity:%d", data_buff, bit_map);
			/* Print the decoded stuff */
			dtsv_cli_display(cli_hdl, cli_display);
		}
	}

	return NCSCC_RC_SUCCESS;

 error_return:
	if (resp->rsp.info.cli_rsp.o_answer != NULL) {
		m_MMGR_FREE_BUFR_LIST(resp->rsp.info.cli_rsp.o_answer);
		resp->rsp.info.cli_rsp.o_answer = NULL;
	}
	return NCSCC_RC_SUCCESS;
}
