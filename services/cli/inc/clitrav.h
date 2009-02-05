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

 MODULE NAME:  CLITRAV.H

..............................................................................

  DESCRIPTION:

  Header file for Command Tree Traversing and its utility functions.

******************************************************************************
*/

#ifndef CLITRAV_H
#define CLITRAV_H

#define m_CLI_GET_LAST_TOKEN(cmd_token)\
{\
        for(;;)\
        {\
        cmd_token = cmd_token >> 1;\
        if (cmd_token & 1)\
            count++;\
        else\
            break;\
        }\
}

#define m_CHECK_IF_NUMBER(argtype, argvalue, argbuf)\
    if (NCSCLI_NUMBER == argtype)\
        sysf_sprintf(argbuf, "%d", argvalue.intval);\
    else\
        sysf_sprintf(argbuf, "%s", (int8 *)argvalue.strval);


#define m_TRAVERSE_DATANODE(cmd_Element, dummy_stack, stack_level)\
{\
    dummy_stack[stack_level] = cmd_Element;\
    stack_level++;\
    cmd_Element = cmd_Element->node_ptr.pDataPtr;\
}


#define m_TRAVERSE_PREV_OPTNODE(cmd_Element, dummy_stack, stack_level)\
{\
    while(stack_level)\
    {\
        /* Go to prev opt/grp node */\
        stack_level--;\
        cmd_Element = dummy_stack[stack_level];\
        /* If opt node has child then traverse childnode */\
        if (cmd_Element->node_ptr.pChild)\
            break;\
    }\
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        COMMAND TREE TRAVERSING FUNCTIONS

This section defines the functions that are used by the command tree for 
traversing and tree and searching the token for validation and execution of 
action associated with the command.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

EXTERN_C uns32 cli_check_macaddr(int8 *);
EXTERN_C uns32 cli_check_community(int8 *);
EXTERN_C uns32 cli_check_ipv6addr (int8 *);
EXTERN_C uns32 cli_check_ipv6prefix (int8 *);
EXTERN_C uns32 cli_check_number (int8 *);
EXTERN_C uns32 cli_check_ipv4addr (int8 *);
EXTERN_C uns32 cli_check_ipv4prefix (int8 *);
EXTERN_C uns32 cli_matchtoken(int8 *, int8 *, CLI_CMD_ERROR *);
EXTERN_C uns32 cli_check_syntax(CLI_CB *, NCSCLI_ARG_SET *, 
                                CLI_EXECUTE_PARAM *, CLI_CMD_ERROR *);
EXTERN_C uns32 cli_execute_command(CLI_CB *, CLI_EXECUTE_PARAM *, CLI_SESSION_INFO *);
EXTERN_C void cli_get_helpstr(CLI_CB *, NCSCLI_ARG_SET *, CLI_EXECUTE_PARAM *, 
                              CLI_SESSION_INFO *);
EXTERN_C void cli_complete_cmd(CLI_CB *i_cb, NCSCLI_ARG_SET *, CLI_EXECUTE_PARAM *, 
                               CLI_SESSION_INFO *);
EXTERN_C uns32 cli_ipv4_to_int(int8 *);
EXTERN_C uns32 cli_get_tokentype(int8 *);
/*EXTERN_C uns32 cli_set_arg(int8 *, NCSCLI_ARG_SET *);*/
EXTERN_C uns32 cli_set_arg(int8 *, NCSCLI_ARG_SET *, CLI_CB *); /* Fix for the bug 59854 ( CLI_CB* argument is added extra) */
/*EXTERN_C uns32 cli_getcommand_tokens(int8 *, NCSCLI_ARG_SET *);*/
EXTERN_C uns32 cli_getcommand_tokens(int8 *, NCSCLI_ARG_SET *,CLI_CB *); /* Fix for the bug 59854(CLI_CB* argument is added extra) */
EXTERN_C uns32 cli_check_token(CLI_CB *, NCSCLI_ARG_VAL *, CLI_CMD_ERROR *);
EXTERN_C CLI_CMD_NODE *cli_node_change(CLI_CB *);
EXTERN_C uns32 cli_check_range(CLI_CB *, NCSCLI_ARG_VAL *, CLI_CMD_ERROR *);
EXTERN_C void cli_add_default_values(CLI_CMD_ELEMENT * i_cmdElement, 
                                     NCSCLI_ARG_SET *, uns32);
EXTERN_C int32 cli_find_errpos(int8 *, int8 *, int32);
EXTERN_C void cli_modify_cmdargs(NCSCLI_ARG_SET *);
EXTERN_C uns32 cli_get_help_desc(CLI_CB *, NCSCLI_ARG_SET *, CLI_EXECUTE_PARAM *, 
                                 CLI_CMD_ERROR *);
EXTERN_C  int32 cli_to_power(int32, int32);
#endif
