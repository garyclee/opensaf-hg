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

 MODULE NAME:  CLICONST.H

..............................................................................

  DESCRIPTION:

  Header file for define constant that are used by CLI modules

******************************************************************************
*/

#ifndef CLICONST_H
#define CLICONST_H

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                GLOBAL CONSTANTS

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/** Bit Masks for LOGGING
 **/

#define CLI_REV 100 /* CLI Release 1.00 */
#define CLI_REV_STR "1.00"


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                    IO MODULE CONSTANTS

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#define CONTROL(X)                  ((X) - '@')
#define CLI_CR                       "<cr>"
#define CLI_CONS_BEEP               '\a'

#define CLI_CMD_HISTORY_SIZE        10
#define CLI_CMD_STACK_SIZE          10
#define CLI_CONS_INVALID_CMD        0
#define CLI_CONS_INCOMPLETE_CMD     1
#define CLI_CONS_AMBIGUOUS_CMD      2
#define CLI_CONS_PASSWORD           3
#define CLI_CONS_INVALID_PASSWORD   4
#define CLI_CONS_UNRECOGNISED_CMD   5
#define CLI_CONS_FILE_OPEN_ERROR    6
#define CLI_CONS_FILE_READ_ERROR    7
#define CLI_CONS_LOGIN_CMD          8
#define CLI_CONS_NO_MODE            9  


#define CLI_HIS_FWD_MVMT            0
#define CLI_HIS_BWD_MVMT            1

#define CLI_CONS_ARROW_KEY          224
#define CLI_CONS_UP_ARROW           72 
#define CLI_CONS_DOWN_ARROW         80    
#define CLI_CONS_LEFT_ARROW         75   
#define CLI_CONS_RIGHT_ARROW        77
#define CLI_CONS_BLANK_SPACE        0x0020
#define CLI_CONS_PWD_MARK           '*'    
#define CLI_CONS_BACWARD_SLASH      '\b'
#define CLI_CONS_TAB                '\t'
#define CLI_CONS_RETURN             '\r'
#define CLI_CONS_EOL                '\n'
#define CLI_CONS_ERR_MARK           '^'
#define CLI_CONS_HELP               '?'
#define CLI_CONS_END                'C'
#define CLI_CONS_EXIT_MODE          'Z'
#define CLI_CONS_HISTORY_FWD        'N'
#define CLI_CONS_HISTORY_BWD        'P'
#define CLI_CONS_END_OF_LINE        'E'
#define CLI_CONS_START_OF_LINE      'A'
#define CLI_CONS_DELETE_WORD        'W'
#define CLI_CONS_CLEAR_ALL          'X'
#define CLI_CONS_DELETE_CHAR        'D'
#define CLI_CONS_REDISPLAY_CMD      'R'
#define CLI_CONS_DELETE_REST        'K'
#define CLI_CONS_BEFORE_LOGIN_PMT   '>'
#define CLI_CONS_AFTER_LOGIN_PMT    '#'
#define CLI_EXIT                    "exit"
#define CLI_HELP_DESC               "help"
#define CLI_QUIT                    "quit"
#define CLI_SHUT                    "clishut"   
#define CLI_DEFAULT_HELP_STR        "No command registed under this mode"

#define NCSCLI_OPTIONAL        NCSCLI_TOK_MAX    
#define NCSCLI_GROUP           NCSCLI_TOK_MAX+1
#define NCSCLI_BRACE_END       NCSCLI_TOK_MAX+2
#define NCSCLI_CONTINOUS_EXP   NCSCLI_TOK_MAX+3    
#define NCSCLI_KEYWORD_ARG     NCSCLI_TOK_MAX+4
#define NCSCLI_UNS32_HLIMIT    0xffffffff /* added to to fix the bug 59854 */
/*Added for cli user authentication requirement( 59359) */
#define NCSCLI_SUPERUSER "ncs_cli_superuser"
#define NCSCLI_ADMIN "ncs_cli_admin"
#define NCSCLI_VIEWER "ncs_cli_viewer"
#define NCSCLI_GROUP_LEN_MAX 255
#define NCSCLI_USER_MASK 7
#define ERROR -1
#define NCSCLI_USER_FIND_ERROR 0
/* Enums that define the match conditions */
typedef enum {
    CLI_SUCCESSFULL_MATCH = 0,
    CLI_NO_MATCH,
    CLI_PARTIAL_MATCH,
    CLI_AMBIGUOUS_MATCH,
    CLI_PASSWD_REQD,
    CLI_INVALID_PASSWD,
    CLI_PASSWORD_MATCHED,
    CLI_ERR_FILEOPEN,
    CLI_ERR_FILEREAD,
    CLI_NO_MODE 
} CLI_CMD_MATCH;

/* Enums that define the command execution type */
typedef enum {
   CLI_EXECUTE = 0,
   CLI_TAB,
   CLI_HELP,
   CLI_HELP_PARTIAL_MATCH,  /* Used to differentiate Help commands */
   CLI_HELP_NEXT_LEVEL,     /* Used to differentiate Help commands */
   CLI_SERIAL_SESSION,
   CLI_TELNET_SESSION,
   CLI_HTTP_SESSION
} CLI_CMD_EXECUTE;

/* Enums that define the authentication modes */
typedef enum {
   CLI_USER_MODE = 0,
   CLI_ADMIN_MODE,
} CLI_AUTH_MODES;

/* enums for cursor movement */
typedef enum {
   CLI_FORWARD = 0,
   CLI_BACKWARD
} CLI_CURSOR_MODE;


/* enums that define error conditions */
typedef enum {
    CLI_IPv4_ERROR = 100,
    CLI_IPv6_ERROR
} CLI_ERR_TYPE;

/* enums that indicate the path traversal direction */
typedef enum {
    CLI_DATA_PATH = 0,
    CLI_SIBLING_PATH,
    CLI_CHILD_PATH,
    CLI_EMPTY_PATH
} CLI_TRAVERSE_TYPE;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                      INIT MODULE CONSTANTS
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#define CLI_ROOT_NODE               "root"
#define CLI_TOK_SEPARATOR           "/"
#define CLI_BUFFER_SIZE             1024
#define CLI_HELPSTR_LEN             128
#define CLI_NODEPATH_LEN            128
#define CLI_ERR_STR_LEN             32
#define CLI_FILE_READ               128  
#define CLI_HIST_LEN                10
#define CLI_ARG_LEN                 64
#define CLI_CMD_LEN                 64 


/* Enum that define ipnput type for CLI */
typedef enum {
    CLI_CMD_BUFFER = 0,
    CLI_FILE_BUFFER
} CLI_BUFFER_TYPE;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                      COMMAND TREE CONSTANTS
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#define CLI_DUMMY_STACK_SIZE        10
#define CLI_START_CMD               1
#define CLI_CMD_ADDED_PARTIALL      0
#define CLI_CMD_ADDED_COMPLETE      1


/* Enum that define token relationship with the previous token */
typedef enum {
    CLI_CHILD_TOKEN = 0,
    CLI_OPT_CHILD_TOKEN,
    CLI_GRP_CHILD_TOKEN,
    CLI_SIBLING_TOKEN,
    CLI_OPT_SIBLING_TOKEN,
    CLI_GRP_SIBLING_TOKEN,
    CLI_NO_RELATION
} CLI_TOKEN_RELATION;

/* Enum that define diffrent attribute for the token */
typedef enum {
    CLI_HELP_STR = 0,
    CLI_DEFALUT_VALUE,
    CLI_RANGE_VALUE,
    CLI_MODE_CHANGE,
    CLI_CONTINOUS_RANGE,
    CLI_DO_FUNC
} CLI_TOKEN_ATTRIB;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                      (NCL) PARSER CONSTANTS
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#define CLI_PARSER_RANGE            256     /* Start of PARSER constants */
#define CLI_TOK_KEYWORD             CLI_PARSER_RANGE + 1
#define CLI_TOK_PARAM               CLI_PARSER_RANGE + 2
#define CLI_TOK_NUMBER              CLI_PARSER_RANGE + 3
#define CLI_TOK_PASSWORD            CLI_PARSER_RANGE + 4
#define CLI_TOK_CIDRv4              CLI_PARSER_RANGE + 5
#define CLI_TOK_IPADDRv4            CLI_PARSER_RANGE + 6
#define CLI_TOK_IPADDRv6            CLI_PARSER_RANGE + 7
#define CLI_TOK_MASKv4              CLI_PARSER_RANGE + 8
#define CLI_TOK_CIDRv6              CLI_PARSER_RANGE + 9
#define CLI_TOK_RANGE               CLI_PARSER_RANGE + 10
#define CLI_TOK_DEFAULT_VAL         CLI_PARSER_RANGE + 11
#define CLI_TOK_HELP_STR            CLI_PARSER_RANGE + 12
#define CLI_TOK_MODE_CHANGE         CLI_PARSER_RANGE + 13
#define CLI_TOK_LCURLYBRACE         CLI_PARSER_RANGE + 14
#define CLI_TOK_RCURLYBRACE         CLI_PARSER_RANGE + 15
#define CLI_TOK_LBRACE              CLI_PARSER_RANGE + 16
#define CLI_TOK_RBRACE              CLI_PARSER_RANGE + 17
#define CLI_TOK_LESS_THAN           CLI_PARSER_RANGE + 18
#define CLI_TOK_GRTR_THAN           CLI_PARSER_RANGE + 19
#define CLI_TOK_OR                  CLI_PARSER_RANGE + 20
#define CLI_TOK_CONTINOUS_EXP       CLI_PARSER_RANGE + 21
#define CLI_TOK_COMMUNITY           CLI_PARSER_RANGE + 22
#define CLI_TOK_WILDCARD            CLI_PARSER_RANGE + 23
#define CLI_TOK_MACADDR             CLI_PARSER_RANGE + 24

#define CLI_TOK_EOC                 "\n"
#define CLI_HELP_IDENTIFIER         "!"
#define CLI_DEFAULT_IDENTIFIER      "%"
#define CLI_MODE_IDENTIFIER         "@"

#define CLI_HLP_COUNT               32
#define CLI_MODE_LEN                24
#define CLI_NODE_LEN                24
#define CLI_MAX_TOK_BUFF            64
#define CLI_PAR_BRACE_LEVEL         20
#define CLI_PAR_STACK_LEVEL         20
#define CLI_OPT_NODE                "Optional Node"
#define CLI_GRP_NODE                "Group Node"
#define CLI_KEYWORD                 "Keyword"
#define CLI_STRING                  "Word"
#define CLI_NUMBER                  "Number"
#define CLI_IPADDRESSv4             "<A.B.C.D>"
#define CLI_IPADDRESSv6             "<X:X:X:X::X>"
#define CLI_CIDRv4                  "<A.B.C.D>/<0-32>"
#define CLI_IPMASKv4                "<A.B.C.D>"
#define CLI_CIDRv6                  "X:X:X:X::X/<0-128>"
#define CLI_PASSWORD                "Password"
#define CLI_COMMUNITY               "<Asn:nn Or A.B.C.D:nn>"  
#define CLI_WILDCARD                "Wildcard expression"  
#define CLI_CONTINOUS_EXP           "Continous Expression"
#define CLI_MACADDR                 "<A:B:C:D:E:F>"
/* added to fix the bug 58609 */
#define m_CLI_DEFAULT_IDLE_TIME_IN_SEC     300
#define m_CLI_MAX_IDLE_TIME_IN_SEC     1800
#define m_CLI_MIN_IDLE_TIME_IN_SEC     120
/* Enum that define CLI mode type */
typedef enum {
    CLI_COMMAND_MODE = 0,
    CLI_CONSOLE_MODE,
} CLI_OPR_MODE;

/* Enum that define brace type */
typedef enum {
    CLI_NONE_BRACE = 0,
    CLI_OPT_BRACE,
    CLI_GRP_BRACE
} CLI_BRACE_TYPE;
#endif
