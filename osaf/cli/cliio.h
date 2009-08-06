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

 MODULE NAME:  CLIIO.H

..............................................................................

  DESCRIPTION:

  Header file for CLI input and output and its utility functions.

******************************************************************************
*/

#ifndef CLIIO_H
#define CLIIO_H

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                           Macro's
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#define m_CLI_RESET_CB(pCli)\
    pCli->par_cb.orFlag = FALSE;\
    pCli->par_cb.resetLvlFlag = FALSE;\
    pCli->par_cb.grpBrace = FALSE;\
    pCli->par_cb.firstOptChild = FALSE;\
    pCli->par_cb.firstGrpChild = FALSE;\
    pCli->par_cb.tokLvlCntr = 0;\
    pCli->par_cb.brcStackPtr = -1;\
    pCli->par_cb.optStackPtr = -1;\
    pCli->par_cb.optLvlCntr = 0;\
    pCli->par_cb.optCntr = 0;\
    pCli->par_cb.optTokCnt = 0;\
    pCli->par_cb.grpStackPtr = -1;\
    pCli->par_cb.grpLvlCntr = 0;\
    pCli->par_cb.grpCntr = 0;\
    pCli->par_cb.grpTokCnt = 0;\
    pCli->par_cb.tokCnt = 0;\
    pCli->par_cb.errFlag = FALSE;\
    pCli->ctree_cb.ctxtMrkr = 0;\
    pCli->ctree_cb.trvMrkr = 0;\
    pCli->ctree_cb.rootMrkr = 0;\
    pCli->ctree_cb.dummyStackMrkr = -1;\
    pCli->ctree_cb.nodeFound = FALSE;\
    pCli->ctree_cb.execFunc = 0;\
    pCli->ctree_cb.cmd_access_level = 0;\
    pCli->ctree_cb.modeStackPtr = -1;\
    pCli->ctree_cb.cmdHtry = 0; \
    pCli->ctree_cb.dereg = FALSE;\
    pCli->ctree_cb.dregStackMrkr = -1;\
    pCli->user_access_level = 0;\
    pCli->cli_idle_time = m_CLI_DEFAULT_IDLE_TIME_IN_SEC * 100;\
    pCli->loginFlag = FALSE;

#define m_CLI_SET_PROMPT(prompt_str, display)\
{\
    uns32 len;\
    putchar(CLI_CONS_EOL); \
    for(len=0; len<strlen(prompt_str); len++)\
    {\
        putchar(prompt_str[len]); \
    }\
    if(FALSE == display)\
        putchar(CLI_CONS_BEFORE_LOGIN_PMT);\
    else\
        putchar(CLI_CONS_AFTER_LOGIN_PMT);\
}

#define m_CLI_SET_CURRENT_CONTEXT(pCli, context_flag)\
{\
    if(TRUE == context_flag)\
        pCli->ctree_cb.trvMrkr = \
            pCli->ctree_cb.rootMrkr;\
    else if(FALSE == context_flag && \
            0 != pCli->ctree_cb.trvMrkr)\
    {\
        --pCli->ctree_cb.modeStackPtr;\
        if(-1 >= pCli->ctree_cb.modeStackPtr)\
        {\
            pCli->ctree_cb.trvMrkr = 0;\
            pCli->ctree_cb.modeStackPtr = -1;\
        }\
        else\
            pCli->ctree_cb.trvMrkr = \
                pCli->ctree_cb.modeStack[pCli->ctree_cb.modeStackPtr];\
   }\
}

#define m_CLI_CONS_PUTLINE(string)\
{\
    uns32   index = 0;\
    putchar(CLI_CONS_EOL);\
    putchar(CLI_CONS_EOL);\
    while('\0' != string[index])\
    {\
        putchar(string[index]);\
        index++;\
    }\
}

#define m_CLI_GET_CURRENT_CONTEXT(pCli, current_context)\
{\
    current_context = pCli->ctree_cb.trvMrkr;\
}

#define m_CLI_DISPLAY_ERROR_MARKER(pos)\
{\
    int8   index;\
    putchar(CLI_CONS_EOL);\
    for(index=0; index<pos; index++)\
    {\
        putchar(32);\
    }\
    putchar(CLI_CONS_ERR_MARK);\
}

typedef enum {			/* Request types to be made to the CLI LM Routine */
	CLI_INST_REQ_CREATE,
	CLI_INST_REQ_DESTROY,
	CLI_INST_REQ_BEGIN,
} CLI_INST_REQ;

/***************************************************************************\
                  LM  C R E A T E   a CLI instance
\***************************************************************************/
typedef struct ncscli_lm_create {
	uns32 o_hdl;
	uns32 i_notify_hdl;	/* Which CLI instance is this? */
	NCSCLI_NOTIFY i_notify;	/* CLI hit an issue internally */
	NCSCLI_GETCHAR i_read_func;	/* character Input source */
	NCSCLI_PUTSTR i_write_func;	/* formatted string output destination */
	uns8 i_hmpool_id;	/* Handle Manager Pool Id */
} CLI_INST_CREATE;

/***************************************************************************\
                  NCSCLI_BEGIN  R E Q U E S T  S T R U C T U R E
\***************************************************************************/
typedef struct ncscli_lm_begin {
	NCS_VRID vr_id;		/* its virtual router ID */
} CLI_INST_BEGIN;

/***************************************************************************\
                I N S T   R E Q U E S T  S T R U C T U R E
\***************************************************************************/
typedef struct cli_inst_info {
	uns32 i_hdl;
	CLI_INST_REQ i_req;

	union {
		CLI_INST_CREATE io_create;
		CLI_INST_BEGIN i_begin;
	} info;
} CLI_INST_INFO;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                     ANSI Function Prototypes
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
EXTERN_C uns32 cli_inst_request(struct cli_inst_info *);
EXTERN_C uns32 cli_create(CLI_INST_CREATE *, CLI_CB **);
EXTERN_C void cli_destroy(CLI_CB **);
EXTERN_C void cli_init(CLI_CB *);
EXTERN_C uns32 cli_display(CLI_CB *, int8 *);
EXTERN_C uns32 cli_done(CLI_CB *, NCSCLI_OP_DONE *);
EXTERN_C uns32 cli_ctree_lock(CLI_CB *);
EXTERN_C uns32 cli_ctree_unlock(CLI_CB *);
EXTERN_C uns32 cli_stricmp(int8 *, int8 *);
EXTERN_C uns32 cli_move_char_backward(uns32);
EXTERN_C uns32 cli_move_char_forward(int8 *, uns32);
EXTERN_C uns32 cli_delete_char(int8 *, uns32);
EXTERN_C uns32 cli_delete_word(int8 *, uns32);
EXTERN_C uns32 cli_delete_beginning_of_line(int8 *, uns32);
EXTERN_C uns32 cli_move_beginning_of_line(uns32);
EXTERN_C uns32 cli_move_end_of_line(int8 *, uns32);
EXTERN_C void cli_read_input(CLI_CB *, NCS_VRID);
EXTERN_C void cli_set_cmd_into_history(CLI_CB *, int8 *);
EXTERN_C void cli_get_cmd_from_history(CLI_CB *, int8 *, uns32);
EXTERN_C void cli_current_mode_exit(CLI_CB *, CLI_SESSION_INFO *, NCS_BOOL *);
EXTERN_C void cli_cef_exp_tmr(void *);
EXTERN_C uns32 cli_cb_main_lock(CLI_CB *);
EXTERN_C uns32 cli_cb_main_unlock(CLI_CB *);
EXTERN_C uns32 cli_register_cmds(CLI_CB *, NCSCLI_OP_REGISTER *);
EXTERN_C uns32 cli_deregister_cmds(CLI_CB *, NCSCLI_OP_DEREGISTER *);
EXTERN_C void cli_clean_history(CLI_CMD_HISTORY *);
EXTERN_C uns32 cli_clean_mode(CLI_CB *, int8 *);
EXTERN_C uns32 cli_clean_cmds(CLI_CB *, NCSCLI_DEREG_CMD_LIST *);
EXTERN_C uns32 cli_lib_shut_except_task_release(void);
EXTERN_C uns32 cli_timer_start(CLI_CB *);
EXTERN_C uns8 ncscli_user_access_level_find(CLI_CB *);
EXTERN_C int32 ncscli_user_access_level_authenticate(CLI_CB *pCli);

#if (NCSCLI_FILE == 1)
EXTERN_C void cli_exec_cmd_from_file(CLI_CB *, CLI_EXECUTE_PARAM *, CLI_SESSION_INFO *);
#endif   /* NCSCLI_FILE == 1 */

#endif   /* CLIIO_H */
