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

 MODULE NAME:  CLIAPI.C

  ..............................................................................
  DESCRIPTION:  API's used to init and create PWE of CLI

*******************************************************************************/

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                    Common Include Files.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#include "cli.h"
#include<errno.h>

#if (NCS_CLI == 1)

static uns32 cli_lib_init(void);
static uns32 cli_lib_shut(void);
static void cli_timer_cb(void *arg);
static uns32 cli_dflt_reg(uns32);
static uns32 cli_cef_default(NCSCLI_ARG_SET *, NCSCLI_CEF_DATA *);
static void cli_log_dereg(void);
static uns32 cli_log_reg(void);

static NCSCONTEXT gl_CliTaskId;
uns32 gl_cli_hdl = 0;
NCS_BOOL gl_cli_valid = FALSE;
CLI_INST_INFO g_lm;

#define NCS_CLI_STACKSIZE      NCS_STACKSIZE_HUGE
#define NCS_CLI_PRIORITY       NCS_TASK_PRIORITY_7
#define NCS_CLI_TASKNAME       "CLI"

/* load the CEFs from configuration file */
static uns32 cli_apps_cefs_load(uns8 *file_name, uns32 what_to_do);

/****************************************************************************\
  Name          : cli_lib_req
 
  Description   : This is the NCS SE API which is used to init/destroy or 
                  Create/destroy PWE's. This will be called by SBOM.
 
  Arguments     : req  - This is the pointer to the input information which
                         SBOM gives.  
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 
  Notes         : None.
\*****************************************************************************/
uns32 cli_lib_req(NCS_LIB_REQ_INFO *req)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	switch (req->i_op) {
	case NCS_LIB_REQ_CREATE:
		rc = cli_lib_init();
		break;

	case NCS_LIB_REQ_DESTROY:
		rc = cli_lib_shut();
		break;

	default:
		break;
	}

	return rc;
}

/****************************************************************************\
  Name          : cli_lib_init
 
  Description   : This is the function which initalize the CLI libarary.
                  This function creates an IPC mail Box and spawns CLI
                  thread.                  
 
  Arguments     : none
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
\*****************************************************************************/
static uns32 cli_lib_init(void)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	CLI_CB *pCli;
#if (NCS_PC_CLI == 1)
	m_NCS_SEM_CREATE(&gl_pcode_sem_id);
#endif

	/* Register logging */
	rc = cli_log_reg();
	if (NCSCC_RC_SUCCESS != rc) {
		printf("\nERROR cli_lib_init(): Logging registration failed\n");
		return rc;
	}

	/* Populate CLI LM INFO */
	g_lm.i_req = CLI_INST_REQ_CREATE;
	g_lm.i_hdl = gl_cli_hdl;
	g_lm.info.io_create.i_hmpool_id = 3;
	g_lm.info.io_create.i_notify = sysf_cli_notify;
	g_lm.info.io_create.i_read_func = sysf_getchar;

	/* Create CLI */
	rc = cli_inst_request(&g_lm);
	if (NCSCC_RC_SUCCESS != rc) {
		cli_log_dereg();
		printf("\nERROR cli_lib_init(): CLI_INST_REQ_CREATE failed\n");
		return rc;
	}

	gl_cli_hdl = g_lm.info.io_create.o_hdl;

	/* Register defualt commands with CLI */
	rc = cli_dflt_reg(gl_cli_hdl);
	if (NCSCC_RC_SUCCESS != rc) {
		cli_log_dereg();

		g_lm.i_hdl = gl_cli_hdl;
		g_lm.i_req = CLI_INST_REQ_DESTROY;
		cli_inst_request(&g_lm);

		printf("\nERROR cli_lib_init(): NCSCLI_OPREQ_REGISTER failed\n");
		return rc;
	}

	/* do the SPA job */
	/* try to load the application's CEFs into CLI Engine */
	/* CLI will not care if there is a problem in loading the Application CEFs */
	rc = cli_apps_cefs_load(m_NCSCLI_CEFS_CONFIG_FILE, 1 /* register */ );
	if (NCSCC_RC_SUCCESS != rc) {
		printf("\ncli_lib_init(): cli_apps_cefs_load() failed\n");
		printf("cli_lib_init(): Quiting the CLI process\n");
		cli_lib_shut_except_task_release();
		exit(1);
	}
	/* Needed for cli user authentication (59359 ) */
	pCli = (CLI_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CLI, gl_cli_hdl);
	/* Find user access level(59359) */
	pCli->user_access_level = ncscli_user_access_level_find(pCli);
	/* for 59359 */
	ncshm_give_hdl(pCli->cli_hdl);

	/* Create thread for each CLI instance */
	g_lm.info.i_begin.vr_id = 1;
	g_lm.i_hdl = gl_cli_hdl;
	g_lm.i_req = CLI_INST_REQ_BEGIN;
	if (m_NCS_TASK_CREATE((NCS_OS_CB)cli_inst_request,
			      &g_lm,
			      NCS_CLI_TASKNAME,
			      NCS_CLI_PRIORITY, NCS_CLI_STACKSIZE, &gl_CliTaskId) != NCSCC_RC_SUCCESS) {
		cli_log_dereg();

		g_lm.i_hdl = gl_cli_hdl;
		g_lm.i_req = CLI_INST_REQ_DESTROY;
		cli_inst_request(&g_lm);

		printf("\nERROR cli_lib_init(): m_NCS_TASK_CREATE failed\n");
		return rc;
	}

	/* Start the thread */
	rc = m_NCS_TASK_START(gl_CliTaskId);
	if (NCSCC_RC_SUCCESS != rc) {
		printf("\nERROR cli_lib_init(): m_NCS_TASK_START failed\n");

		cli_log_dereg();

		g_lm.i_hdl = gl_cli_hdl;
		g_lm.i_req = CLI_INST_REQ_DESTROY;
		cli_inst_request(&g_lm);

		m_NCS_TASK_RELEASE(gl_CliTaskId);
		return rc;
	}

	gl_cli_valid = TRUE;
	return rc;
}

/****************************************************************************
  Name          : cli_lib_shut
 
  Description   : This is the function which destroy the CLI library.
                  This function releases the Task and the IPC mail Box.
                  This function destroies handle manager, CB and clean up 
                  all the component specific databases.
 
  Arguments     : none
  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE. 
\*****************************************************************************/
static uns32 cli_lib_shut(void)
{
	CLI_INST_INFO lm;
	uns32 rc = NCSCC_RC_SUCCESS;

	lm.i_hdl = gl_cli_hdl;
	lm.i_req = CLI_INST_REQ_DESTROY;

	rc = cli_inst_request(&lm);
	if (NCSCC_RC_SUCCESS != rc) {
		printf("\nERROR cli_lib_shut(): CLI_INST_REQ_DESTROY failed\n");
	}

	rc = m_NCS_TASK_RELEASE(gl_CliTaskId);
	if (NCSCC_RC_SUCCESS != rc) {
		printf("\nERROR cli_lib_shut(): m_NCS_TASK_RELEASE failed\n");
	}

	/* Deregister CLI from logging service */
	cli_log_dereg();

#if(NCS_PC_CLI == 1)
	m_NCS_SEM_RELEASE(gl_pcode_sem_id);
	gl_pcode_sem_id = 0;
#endif

	gl_cli_hdl = 0;
	gl_cli_valid = FALSE;
	return rc;
}

/* cli_lib_shut_except_task_release method is developed inorder to fix the bug  */
/****************************************************************************
  Name          : cli_lib_shut_except_task_release
  Description   : This is the function which destroy the CLI library.
                  This function does not releases the Task and the IPC mail Box.
                  This function destroies handle manager, CB and clean up 
                  all the component specific databases.
  Arguments     : none
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE. 

\****************************************************************************/
uns32 cli_lib_shut_except_task_release(void)
{
	CLI_INST_INFO lm;
	uns32 rc = NCSCC_RC_SUCCESS;
	lm.i_hdl = gl_cli_hdl;
	lm.i_req = CLI_INST_REQ_DESTROY;
	rc = cli_inst_request(&lm);
	if (NCSCC_RC_SUCCESS != rc) {
		printf("\nERROR cli_lib_shut(): CLI_INST_REQ_DESTROY failed\n");
	}
	/* Deregister CLI from logging service */
	cli_log_dereg();
#if(NCS_PC_CLI == 1)
	m_NCS_SEM_RELEASE(gl_pcode_sem_id);
	gl_pcode_sem_id = 0;
#endif
	gl_cli_hdl = 0;
	gl_cli_valid = FALSE;
	return rc;

}

/****************************************************************************
 Name          : ncscli_user_access_level_find
 Description   :This is the function which returns the access level of the user 
 Arguments     : 
 Return Values : Access level of the user. -1 if user does not belongs to valid cli groups.

****************************************************************************/
uns8 ncscli_user_access_level_find(CLI_CB *pCli)
{
	struct group *gp;
	gid_t *list = NULL;
	int numofgroups;
	int i;
	errno = 0;
	numofgroups = getgroups(0, list);
	if (numofgroups == ERROR) {
		printf("\nError %d  occured while fetching the list of all the groups the cli user belongs to \n",
		       errno);
		m_LOG_NCSCLI_STR(NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, NCSCLI_HDLN_CLI_USER_INFO_ACCESS_ERROR,
				 "in getgroups");
		perror("getgroups: ");
		return NCSCLI_USER_FIND_ERROR;
	}
	list = (gid_t *)malloc(numofgroups * sizeof(gid_t));
	errno = 0;
	numofgroups = getgroups(numofgroups, list);
	if (numofgroups == ERROR) {
		printf("\nError %d  occured while fetching the list of all the groups the cli user belongs to \n",
		       errno);
		m_LOG_NCSCLI_STR(NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, NCSCLI_HDLN_CLI_USER_INFO_ACCESS_ERROR,
				 "in getgroups");
		perror("getgroups: ");
		return NCSCLI_USER_FIND_ERROR;
	}
	errno = 0;
	for (i = 0; i < numofgroups; i++) {
 retry1:	gp = getgrgid(list[i]);
		if (gp) {
			if ((strcmp(gp->gr_name, pCli->cli_user_group.ncs_cli_superuser) == 0) ||
			    (strcmp(gp->gr_name, "root") == 0)) {
				return (NCSCLI_USER_MASK >> (NCSCLI_ACCESS_END - NCSCLI_SUPERUSER_ACCESS - 1));
			}
		} else if (gp == NULL) {
			printf("\nError %d occured while fetching the group name of the cli user\n", errno);
			perror("getgrgid: ");
			m_LOG_NCSCLI_STR(NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, NCSCLI_HDLN_CLI_USER_INFO_ACCESS_ERROR,
					 "in getgrgid");
			if (errno == EINTR) {
				errno = 0;
				goto retry1;
			}
			return NCSCLI_USER_FIND_ERROR;
		}
	}
	for (i = 0; i < numofgroups; i++) {
 retry2:	gp = getgrgid(list[i]);
		if (gp) {
			if (strcmp(gp->gr_name, pCli->cli_user_group.ncs_cli_admin) == 0) {
				return (NCSCLI_USER_MASK >> (NCSCLI_ACCESS_END - NCSCLI_ADMIN_ACCESS - 1));
			}
		} else if (gp == NULL) {
			printf("\nError %d occured while fetching the group name of the cli user\n", errno);
			perror("getgrgid: ");
			m_LOG_NCSCLI_STR(NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, NCSCLI_HDLN_CLI_USER_INFO_ACCESS_ERROR,
					 "in getgrgid");
			if (errno == EINTR) {
				errno = 0;
				goto retry2;
			}
			return NCSCLI_USER_FIND_ERROR;
		}
	}
	for (i = 0; i < numofgroups; i++) {
 retry3:	gp = getgrgid(list[i]);
		if (gp) {
			if (strcmp(gp->gr_name, pCli->cli_user_group.ncs_cli_viewer) == 0) {
				return (NCSCLI_USER_MASK >> (NCSCLI_ACCESS_END - NCSCLI_VIEWER_ACCESS - 1));
			}
		} else if (gp == NULL) {
			printf("\nError %d occured while fetching the group name of the cli user\n", errno);
			perror("getgrgid: ");
			m_LOG_NCSCLI_STR(NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, NCSCLI_HDLN_CLI_USER_INFO_ACCESS_ERROR,
					 "in getgrgid");
			if (errno == EINTR) {
				errno = 0;
				goto retry3;
			}
			return NCSCLI_USER_FIND_ERROR;
		}
	}
	printf("\n\tncs cli user is not an authenticated cli user...exiting CLI process.\n");
	m_LOG_NCSCLI_STR(NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, NCSCLI_HDLN_CLI_USER, "non cli user");
	return NCSCLI_USER_FIND_ERROR;
}

/****************************************************************************
 Name          : ncscli_user_access_level_authenticate
 Description   :This is the function which checks whether a cli user is allowed to execute a command 
 Arguments     : CLI_CB*
 Return Values : Returns 1 if the user is allowed to execute a command.Else  returns 0.

****************************************************************************/
int32 ncscli_user_access_level_authenticate(CLI_CB *pCli)
{
	if (pCli->user_access_level & pCli->ctree_cb.cmdElement->cmd_access_level)
		return 1;
	else
		return 0;
}

/****************************************************************************
  Name          : cli_timer_start
  Description   : This is the function which starts the timer for tracking the                    CLI Idle period 
  Arguments     : CLI_CB* 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE. 
\****************************************************************************/
/* Added to fix the bug 58609 */
uns32 cli_timer_start(CLI_CB *cliCb)
{
	CLI_IDLE_TIMER *timer_data = &(cliCb->idleTmr);
	timer_data->period = cliCb->cli_idle_time;
	timer_data->grace_period = 60 * 100;	/*one minute */
	timer_data->warning_msg = 0;
	printf("\ncli idle time is %u seconds\n", cliCb->cli_idle_time / 100);
	timer_data->cb = (void *)cliCb;
	if (timer_data->timer_id == TMR_T_NULL) {
		/* create the timer */
		m_NCS_TMR_CREATE(timer_data->timer_id, timer_data->period, cli_timer_cb, (void *)(timer_data));
	}
	/* start the timer */
	m_NCS_TMR_START(timer_data->timer_id,
			(timer_data->period - timer_data->grace_period), cli_timer_cb, (void *)(timer_data));
	if (timer_data->timer_id == TMR_T_NULL)
		return NCSCC_RC_FAILURE;
	return NCSCC_RC_SUCCESS;

}

/****************************************************************************
  Name          : cli_timer_cb

  Description   : This is the call back function for CLI Idle time period 

  Arguments     : void *arg

  Return Values :void 
\*****************************************************************************/
/* Added to fix the bug 58609 */
static void cli_timer_cb(void *arg)
{
	CLI_IDLE_TIMER *timer_data = (CLI_IDLE_TIMER *)arg;
	CLI_CB *cliCb = (CLI_CB *)timer_data->cb;
	uns32 lapsed_time;
	uns32 current_time;
	/* stop the timer */
	m_NCS_TMR_STOP(timer_data->timer_id);
	m_GET_TIME_STAMP(current_time);
	lapsed_time = (current_time - (cliCb->cli_last_active_time)) * 100;	/* in senti seconds */
	if (lapsed_time >= (cliCb->cli_idle_time - timer_data->grace_period)) {
		if (!timer_data->warning_msg) {
			timer_data->warning_msg = 1;
			printf("\ncli will quit if it is idle for 1 more minute \n\r");
			m_NCS_TMR_START(timer_data->timer_id,
					timer_data->grace_period, cli_timer_cb, (void *)(timer_data));

		} else {
			if (timer_data->timer_id)
				m_NCS_TMR_DESTROY(timer_data->timer_id);

			cli_lib_shut_except_task_release();
			ncs_stty_reset();
			exit(0);
		}
	} else {
		timer_data->warning_msg = 0;
		timer_data->period = ((cliCb->cli_idle_time - timer_data->grace_period) - lapsed_time);
		m_NCS_TMR_START(timer_data->timer_id, timer_data->period, cli_timer_cb, (void *)(timer_data));
	}
}

/****************************************************************************
  Name          : cli_dflt_reg
 
  Description   : This function registers default command to log into CLI 

  Arguments     : cli_hdl
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
\*****************************************************************************/
static uns32 cli_dflt_reg(uns32 hdl)
{
	NCSCLI_BINDERY bindery;
	NCSCLI_CMD_LIST data;
	NCSCLI_ACCESS_PASSWD pwd;
	NCSCLI_OP_INFO info;

	/* Fill the bindery info */
	bindery.i_cli_hdl = hdl;
	bindery.i_mab_hdl = 0;
	bindery.i_req_fnc = 0;

	/*Register the default commands with CLI */
	memset(&data, 0, sizeof(data));
	memset(&pwd, 0, sizeof(pwd));

	/*Fill the root i_node */
	data.i_node = CLI_ROOT_NODE;
	data.i_command_mode = CLI_ROOT_NODE;
	data.i_access_req = TRUE;
	/* Supressing Password for enable cmd */
	/*pwd.i_encrptflag = FALSE;
	   strcpy(pwd.passwd.i_passwd, "ncs");    
	   data.i_access_passwd = &pwd; */	/*commented for 59359 */
	data.i_cmd_count = 3;	/* set the command count */

	/* List of commands associated with this i_node */
	/*data.i_command_list[0].i_cmdstr = "enable!Turn on privileged commands! @root/exec@ NCSCLI_PASSWORD!Password!"; */	/*commented  for 59359 */
	data.i_command_list[0].i_cmdstr = "enable!Turn on privileged commands! @root/exec@ ";
	data.i_command_list[1].i_cmdstr = "help!Description of the interactive help system!";
	data.i_command_list[2].i_cmdstr = "clishut!Shut down the CLI!";
	data.i_command_list[0].i_cmd_exec_func = cli_cef_default;
	data.i_command_list[1].i_cmd_exec_func = cli_cef_default;
	data.i_command_list[0].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;	/*For 59359 */
	data.i_command_list[1].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
	data.i_command_list[2].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;

	info.i_hdl = hdl;
	info.i_req = NCSCLI_OPREQ_REGISTER;
	info.info.i_register.i_bindery = &bindery;
	info.info.i_register.i_cmdlist = &data;
	if (NCSCC_RC_SUCCESS != ncscli_opr_request(&info))
		return NCSCC_RC_FAILURE;

	/*Register the default commands with CLI */
	memset(&data, 0, sizeof(data));
	data.i_node = "root/exec";
	data.i_command_mode = "exec";
	data.i_access_req = FALSE;
	data.i_access_passwd = 0;
	data.i_cmd_count = 4;
	data.i_command_list[0].i_cmdstr =
	    "configure!Enter configuration mode! @root/exec/config@ NCSCLI_STRING!terminal!";
	data.i_command_list[1].i_cmdstr = "help!Description of the interactive help system!";
	data.i_command_list[2].i_cmdstr = "exit!Go to the previous mode!";
	data.i_command_list[3].i_cmdstr = "clishut!Shut down the CLI!";
	data.i_command_list[0].i_cmd_exec_func = cli_cef_default;
	data.i_command_list[1].i_cmd_exec_func = cli_cef_default;
	data.i_command_list[0].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;	/*For 59359 */
	data.i_command_list[1].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
	data.i_command_list[2].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
	data.i_command_list[3].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;

	info.info.i_register.i_cmdlist = &data;
	if (NCSCC_RC_SUCCESS != ncscli_opr_request(&info))
		return NCSCC_RC_FAILURE;

	/*Register the default commands with CLI */
	memset(&data, 0, sizeof(data));
	data.i_node = "root/exec/config";
	data.i_command_mode = "config";
	data.i_access_req = FALSE;
	data.i_access_passwd = 0;
	data.i_cmd_count = 3;
	data.i_command_list[0].i_cmdstr = "exit!Go to the previous mode!";
	data.i_command_list[1].i_cmdstr = "help!Description of the interactive help system!";
	data.i_command_list[2].i_cmdstr = "clishut!Shut down the CLI!";
	data.i_command_list[0].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;	/*For 59359 */
	data.i_command_list[1].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;
	data.i_command_list[2].i_cmd_access_level = NCSCLI_VIEWER_ACCESS;

	info.info.i_register.i_cmdlist = &data;
	if (NCSCC_RC_SUCCESS != ncscli_opr_request(&info))
		return NCSCC_RC_FAILURE;

	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : cli_cef_default
 
  Description   : This function is default CEF for the defualt commands 
 
  Arguments     : arg_list - arg list
                  cef_data - data
  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
\*****************************************************************************/
static uns32 cli_cef_default(NCSCLI_ARG_SET *arg, NCSCLI_CEF_DATA *data)
{
	NCSCLI_OP_INFO info;

	info.i_hdl = data->i_bindery->i_cli_hdl;
	info.i_req = NCSCLI_OPREQ_DONE;
	info.info.i_done.i_status = NCSCC_RC_SUCCESS;

	ncscli_opr_request(&info);
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
   PROCEDURE NAME  : cli_create
   DESCRIPTION     : This function is used to create CLI control block.
   ARGUMENTS       : io_info  -  Create request info pointer   
                     o_cli    -  CLI pointer
   RETURNS         : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
   NOTES           :  
*****************************************************************************/
uns32 cli_create(CLI_INST_CREATE *io_info, CLI_CB **o_cli)
{
	void *console_id = 0;
	uns32 hdl = 0;
	CLI_CB *pCli = 0;

	/* Create the CLI control block */
	*o_cli = 0;
	pCli = m_MMGR_ALLOC_CLI_CB;
	if (!pCli)
		return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
	else {
		/* Create the handle manager for CB */
		hdl = ncshm_create_hdl(io_info->i_hmpool_id, NCS_SERVICE_ID_CLI, (NCSCONTEXT)pCli);
		if (!hdl) {
			m_MMGR_FREE_CLI_CB(pCli);
			return NCSCC_RC_FAILURE;	/* Failed to create handle */
		}
		io_info->o_hdl = hdl;
	}

	/* Initializes CB */
	memset(pCli, 0, sizeof(CLI_CB));
	m_CLI_RESET_CB(pCli);

	pCli->readFunc = io_info->i_read_func;

	/* Set the notify function */
	pCli->cli_notify_hdl = io_info->i_notify_hdl;
	pCli->cli_notify = io_info->i_notify;

	/* Assign the CLI key for this instance of CLI */
	pCli->cli_hdl = hdl;

	/* Set the console associated iwt this control block of CLI */
	m_GET_CONSOLE(i_key, console_id);
	pCli->consoleId = console_id;

	/* Set the Timer callback handler */
	pCli->cefTmr.tmrCB = cli_cef_exp_tmr;

	/*Initialize CLI */
	cli_init(pCli);

	m_NCS_LOCK_INIT(&pCli->mainLock);
	m_NCS_LOCK_INIT(&pCli->ctreeLock);

	*o_cli = pCli;		/* CLI sucessfully created */
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
   PROCEDURE NAME :  cli_destroy
   DESCRIPTION    :  This function is used to destroy CLI control block.
   ARGUMENTS      :  io_cb.
   RETURNS        :  none
   NOTES          :          
*****************************************************************************/
void cli_destroy(CLI_CB **io_cli)
{
	CLI_CB *pCli = *io_cli;

	cli_cb_main_lock(pCli);

	/* Clean the command tree */
	cli_clean_ctree(pCli);

	/* Clean History */
	cli_clean_history(pCli->ctree_cb.cmdHtry);

	/* Clean the left over data */
	if (pCli->subsys_cb.i_cef_data)
		m_MMGR_FREE_CLI_DEFAULT_VAL(pCli->subsys_cb.i_cef_data);
	if (pCli->subsys_cb.i_cef_mode)
		m_MMGR_FREE_CLI_DEFAULT_VAL(pCli->subsys_cb.i_cef_mode);

	cli_cb_main_unlock(pCli);
	m_MMGR_FREE_CLI_CB(pCli);
	*io_cli = 0;		/* CLI Destroyed */
}

/*****************************************************************************
   PROCEDURE NAME  : cli_register_cmds
   DESCRIPTION     : This function is invoke from LM API when any subsystem wants 
                     to register its CLI commands in NCL format
   ARGUMENTS       : pCli - Control block of CLI
                     info - Register info
   RETURNS         : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
   NOTES           :            
*****************************************************************************/
uns32 cli_register_cmds(CLI_CB *pCli, NCSCLI_OP_REGISTER *info)
{
	uns32 count = 0;
	NCSCLI_BINDERY *pBindery = 0;
	NCSCLI_ACCESS_LEVEL cmd_access_level;

	/* Reset all marker */
	cli_cb_main_lock(pCli);

	cli_reset_all_marker(pCli);
	if (0 != strlen(info->i_cmdlist->i_node))
		cli_parse_node(pCli, info->i_cmdlist, TRUE);
	else {
		cli_cb_main_unlock(pCli);
		return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
	}

	/* Set the mode */
	strncpy(pCli->ctree_cb.ctxtMrkr->mode, info->i_cmdlist->i_command_mode,
		sizeof(pCli->ctree_cb.ctxtMrkr->mode) - 1);

	if (!info->i_bindery || (0 == (pBindery = m_MMGR_ALLOC_NCSCLI_BINDERY))) {
		cli_cb_main_unlock(pCli);
		return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
	}
	memcpy(pBindery, info->i_bindery, sizeof(NCSCLI_BINDERY));

	/* Parse each command string */
	for (count = 0; count < info->i_cmdlist->i_cmd_count; count++) {
		pCli->ctree_cb.bindery = pBindery;
		cmd_access_level = info->i_cmdlist->i_command_list[count].i_cmd_access_level;
		pCli->ctree_cb.execFunc = info->i_cmdlist->i_command_list[count].i_cmd_exec_func;
		if (cmd_access_level == NCSCLI_VIEWER_ACCESS || cmd_access_level == NCSCLI_ADMIN_ACCESS
		    || cmd_access_level == NCSCLI_SUPERUSER_ACCESS)
			pCli->ctree_cb.cmd_access_level = (1 << cmd_access_level);
		else {
			printf
			    ("\nThe command %s is not registered with proper access level...bydefault registering with NCSCLI_VIEWER_ACCESS level\n",
			     info->i_cmdlist->i_command_list[count].i_cmdstr);
			pCli->ctree_cb.cmd_access_level = (1 << NCSCLI_VIEWER_ACCESS);
		}
		cli_set_cmd_to_parse(pCli, info->i_cmdlist->i_command_list[count].i_cmdstr, CLI_CMD_BUFFER);
	}

	cli_cb_main_unlock(pCli);
	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
   PROCEDURE NAME  : cli_deregister_cmds
   DESCRIPTION     : This function is invoke from LM API when any subsystem wants 
                     to de-register its CLI commands in NCL format
   ARGUMENTS       : pCli - Control block of CLI
                     info - Register info
   RETURNS         : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
   NOTES           :
*****************************************************************************/
uns32 cli_deregister_cmds(CLI_CB *pCli, NCSCLI_OP_DEREGISTER *info)
{
	uns32 rc = NCSCC_RC_SUCCESS;

	/* Check what is to be deregistered, complete mode and the commands
	 * under that mode or just the commands only not the mode 
	 */
	cli_cb_main_lock(pCli);
	if (info->i_cmdlist == NULL) {
		cli_cb_main_unlock(pCli);
		return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
	}
	if (!info->i_cmdlist->i_cmd_count) {
		/* OK, I am suppose to delete the mode and all the commands that 
		 * registered, under that mode
		 */
		rc = cli_clean_mode(pCli, info->i_cmdlist->i_node);
	} else {
		/* OK, I am suppose to delete the commnads that are asked for
		 * deletion and not the mode 
		 */
		rc = cli_clean_cmds(pCli, info->i_cmdlist);
		if (NCSCC_RC_SUCCESS != rc)
			return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);

		/* Chek if all the commnads under that mode are deleted then delete the 
		 * mode as well
		 */
		if (!pCli->ctree_cb.ctxtMrkr->pCmdl)
			rc = cli_clean_mode(pCli, info->i_cmdlist->i_node);
	}

	cli_cb_main_unlock(pCli);
	if (NCSCC_RC_SUCCESS != rc)
		return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
	return rc;
}

/*****************************************************************************\
   PROCEDURE NAME  : cli_log_reg

   DESCRIPTION     : This is the function which registers the CLI logging with
                     the Loging service.

   ARGUMENTS       : None
                     
   RETURNS         : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
   NOTES           :
\*****************************************************************************/
static uns32 cli_log_reg(void)
{
#if (NCSCLI_LOG != 0)
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(reg));
	reg.i_op = NCS_DTSV_OP_BIND;
	reg.info.bind_svc.svc_id = NCS_SERVICE_ID_CLI;
	/* fill version no. */
	reg.info.bind_svc.version = CLI_LOG_VERSION;
	/* fill svc_name */
	strcpy(reg.info.bind_svc.svc_name, "CLI");

	/* Bind with Loging service */
	return ncs_dtsv_su_req(&reg);
#else
	return NCSCC_RC_SUCCESS;
#endif
}

/*****************************************************************************\
   PROCEDURE NAME  : cli_log_dereg

   DESCRIPTION     : This is the function which deregisters the CLI logging 
                     from the Loging service.

   ARGUMENTS       : None
                     
   RETURNS         : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
   NOTES           :
\*****************************************************************************/
static void cli_log_dereg(void)
{
#if (NCSCLI_LOG != 0)
	NCS_DTSV_RQ reg;

	memset(&reg, 0, sizeof(reg));
	reg.i_op = NCS_DTSV_OP_UNBIND;
	reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_CLI;

	/* Unbinf from Loging service */
	ncs_dtsv_su_req(&reg);
#endif
	return;
}

/****************************************************************************\
*  Name:          cli_apps_cefs_load                                         * 
*                                                                            *
*  Description:   To load/unload the Application specific CEFs into CLI      *  
*                                                                            *
*  Arguments:     uns8* - Name of the Configuration File                     *
*                 uns32 - what_to_do                                         *
*                          1 - REGISTER the CEFs                             *
*                          0 - UNREGISTER the CEFs                           *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
*                 NCSCC_RC_FAILURE   -  failure                              *
\****************************************************************************/
static uns32 cli_apps_cefs_load(uns8 *file_name, uns32 what_to_do)
{
	/* get the instruments ready */
	CLI_CB *pCli = 0;
	int8 *cli_idle_time;
	uns32 cliidletime;
	FILE *fp = NULL;
	/* int8   lib_name[255] = {0};
	   int8   func_name[255] = {0}; */
	int8 arg1[255] = { 0 };
	int8 arg2[255] = { 0 };
	int32 nargs = 0;
	uns32 status = NCSCC_RC_SUCCESS;
	uns32 (*reg_unreg_routine) (NCS_LIB_REQ_INFO *) = NULL;
	int8 *dl_error = NULL;
	NCS_OS_DLIB_HDL *lib_hdl = NULL;
	NCS_LIB_REQ_INFO libreq;
	uns8 cefs_loaded = FALSE;

	/* open the file */
	/*fp = fopen(file_name, "r"); */
	fp = sysf_fopen(file_name, "r");
	if (fp == NULL) {
		/* inform that there is no such file, and return */
		printf("\ncli_apps_cefs_load(): fopen() failed for the file: %s\n", file_name);
		printf("cli_apps_cefs_load(): or You may be trying to bring up CLI on standby node\n");
		/* log the critical error, file_name */
		m_LOG_NCSCLI_STR(NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_CRITICAL,
				 NCSCLI_HDLN_CLI_CONF_FILE_OPEN_FAILED, file_name);
		return NCSCC_RC_FAILURE;
	}

	/* fill in the required parameters to be passed to the application */
	memset(&libreq, 0, sizeof(NCS_LIB_REQ_INFO));
	libreq.i_op = NCS_LIB_REQ_CREATE;

	/* log the filename, from where CLI is reading the config spec */
	m_LOG_NCSCLI_STR(NCSFL_LC_DATA, NCSFL_SEV_NOTICE, NCSCLI_HDLN_CLI_CONF_FILE_READ, file_name);

	pCli = (CLI_CB *)ncshm_take_hdl(NCS_SERVICE_ID_CLI, gl_cli_hdl);
	if (pCli == NULL)
		return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);

	/* continue till the you reach the end of the file */
	while (((nargs = fscanf(fp, "%s %s", arg1, arg2)) == 2) && (nargs != EOF)) {
		/* Load ncs cli user group names(59359) */
		if (strcmp(arg1, "NCS_CLI_SUPERUSER_GROUP") == 0) {
			if (strlen(arg2) <= NCSCLI_GROUP_LEN_MAX)
				strcpy(pCli->cli_user_group.ncs_cli_superuser, arg2);
			else
				printf
				    ("\nLength of the SUPERUSER GROUP given in configuration file is greater than %d . So default superuser group is considered\n",
				     NCSCLI_GROUP_LEN_MAX);
			continue;
		}
		if (strcmp(arg1, "NCS_CLI_ADMIN_GROUP") == 0) {
			if (strlen(arg2) <= NCSCLI_GROUP_LEN_MAX)
				strcpy(pCli->cli_user_group.ncs_cli_admin, arg2);
			else
				printf
				    ("\nLength of the ADMIN GROUP given in configuration file is greater than %d . So default admin group is considered\n",
				     NCSCLI_GROUP_LEN_MAX);
			continue;
		}
		if (strcmp(arg1, "NCS_CLI_VIEWER_GROUP") == 0) {
			if (strlen(arg2) <= NCSCLI_GROUP_LEN_MAX)
				strcpy(pCli->cli_user_group.ncs_cli_viewer, arg2);
			else
				printf
				    ("\nLength of the VIEWER GROUP given in configuration file is greater than %d . So default viewer group is considered\n",
				     NCSCLI_GROUP_LEN_MAX);
			continue;
		}
		/* The below 'if' Added to read configurable parameter CLI_IDLE_TIME */
		if (strcmp(arg1, "CLI_IDLE_TIME") == 0) {
			cli_idle_time = arg2;
			if ((cli_idle_time != NULL) && (cliidletime = atoi(cli_idle_time))) {
				cliidletime = cliidletime * 60 * 100;	/*converting into centi seconds */
				if (cliidletime >= (m_CLI_MIN_IDLE_TIME_IN_SEC * 100)
				    && cliidletime <= (m_CLI_MAX_IDLE_TIME_IN_SEC * 100))
					pCli->cli_idle_time = cliidletime;
				else {
					pCli->cli_idle_time = m_CLI_DEFAULT_IDLE_TIME_IN_SEC * 100;
					printf
					    ("\ncli idle time is out of valid range,. so taking default value of %u seconds\n",
					     pCli->cli_idle_time / 100);
				}

			} else {
				pCli->cli_idle_time = m_CLI_DEFAULT_IDLE_TIME_IN_SEC * 100;
				printf
				    ("\ncli idle time is out of valid range,. so taking default value of %u seconds\n",
				     pCli->cli_idle_time / 100);
			}
			continue;
		}
		/* Load the library if REGISTRATION is to be peformed */
		if (what_to_do == 1) {
			lib_hdl = m_NCS_OS_DLIB_LOAD(arg1, m_NCS_OS_DLIB_ATTR);
			if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) {
				/* log the error returned from dlopen() */
				m_LOG_NCSCLI_STR_STR(NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_CRITICAL,
						     NCSCLI_HDLN_CLI_DLIB_LOAD_FAILED, arg1, dl_error);
				printf("\nWarning: %s\n", dl_error);
				reg_unreg_routine = NULL;
				lib_hdl = NULL;
				memset(arg2, 0, 255);
				memset(arg1, 0, 255);
				continue;
			}
		}

		/* load the symbol into CLI Engine process space */
		reg_unreg_routine = m_NCS_OS_DLIB_SYMBOL(lib_hdl, arg2);
		if ((dl_error = m_NCS_OS_DLIB_ERROR()) != NULL) {
			/* log the error returned from dlopen() */
			m_LOG_NCSCLI_STR_STR_STR(NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_CRITICAL,
						 NCSCLI_HDLN_CLI_DLIB_SYM_FAILED, arg1, arg2, dl_error);
			printf("\nWarning: %s\n", dl_error);
			reg_unreg_routine = NULL;
			lib_hdl = NULL;
			memset(arg2, 0, 255);
			memset(arg1, 0, 255);
			continue;
		}

		/* do the INIT/DEINIT now... */
		if (reg_unreg_routine != NULL) {
			/* do the registration */
			status = (*reg_unreg_routine) (&libreq);
			if (status != NCSCC_RC_SUCCESS) {
				/* lgo the error */
				m_LOG_NCSCLI_STR_I(NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_ERROR,
						   NCSCLI_HDLN_CLI_CEFS_REG_DEREG_FAILED, arg2, status);
			} else {
				/* log the success, and the function name */
				m_LOG_NCSCLI_STR_I(NCSFL_LC_DATA, NCSFL_SEV_NOTICE,
						   NCSCLI_HDLN_CLI_CEFS_REG_DEREG_SUCCESS, arg2, 0);
				cefs_loaded = TRUE;
			}
		}

		reg_unreg_routine = NULL;
		lib_hdl = NULL;
		memset(arg2, 0, 255);
		memset(arg1, 0, 255);

	}			/* for all the libraries */
	if (nargs != EOF) {
		printf("\ncli_apps_cefs_load(): cli conf file %s is corrupted.\n", file_name);
		/* log the error */
		m_LOG_NCSCLI_STR_I(NCSFL_LC_DATA, NCSFL_SEV_ERROR,
				   NCSCLI_HDLN_CLI_CONF_FILE_CORRUPTED, file_name, nargs);

		/* set the return code */
		status = NCSCC_RC_FAILURE;
	}

	/* close the file */
	fclose(fp);
	if (cefs_loaded == FALSE) {
		status = NCSCC_RC_FAILURE;
		printf("\ncli_apps_cefs_load(): No cefs loaded with CLI\n");
	}
	ncshm_give_hdl(pCli->cli_hdl);
	return status;
}
#else
extern int dummy;

#endif   /* (if NCS_CLI == 1) */
