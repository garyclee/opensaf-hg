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

 MODULE NAME:  CLISTRUC.H

..............................................................................

  DESCRIPTION:

  Header file for data structures that are used by CLI modules

******************************************************************************
*/

#ifndef CLISTRUC_H
#define CLISTRUC_H


/***************************************************************************
                     The CLI Timer prototype
****************************************************************************/
typedef void (*NCSCLI_CEF_TIMER) (void *);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                    IO MODULE DATA-STRUCTURES  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/***************************************************************************\
                     Structure for storing group names of ncs cli user(59359)
\***************************************************************************/
typedef struct ncs_cli_group {
   int8 ncs_cli_superuser[NCSCLI_GROUP_LEN_MAX];
   int8 ncs_cli_admin[NCSCLI_GROUP_LEN_MAX];
   int8 ncs_cli_viewer[NCSCLI_GROUP_LEN_MAX];
}NCS_CLI_GROUP;

/***************************************************************************\
                     Structure for storing command
\***************************************************************************/
typedef struct cli_cmd_history {
   /* Links to next and previous commands in the history */
   struct cli_cmd_history  *next;
   struct cli_cmd_history  *prev;    
   int8                    *pCmdStr; /* Command string */
} CLI_CMD_HISTORY;
/***************************************************************************\
                     Structure for history marker
\***************************************************************************/
typedef struct cli_cmd_history_marker {
   uns32           cmd_count; /* Count of commands in History */
   CLI_CMD_HISTORY *cmd_node; /* Current histroy node */
} CLI_CMD_HISTORY_MARKER;

/***************************************************************************\
     This structure is used to store all terminal related information.
\***************************************************************************/
typedef struct cli_session_info {    
   uns32   session_id;    /* used to store session information */    
   uns32   session_type;  /* indicates session type, telnet, serial.*/    
   uns32   history_count; /* count of elements in history buf */    
   int8    history_buf[CLI_HIST_LEN]; /* history of cmds associated with session */    
   int8    prompt_string[CLI_BUFFER_SIZE]; /* prompt string */
} CLI_SESSION_INFO;

/***************************************************************************\
   This structure is used to store the cmd name and help string associated
   with the command.
\***************************************************************************/
typedef struct cli_cmd_help {    
   int8    cmdstring[CLI_CMD_LEN]; /* Command syntax in NetPlane format */    
   int8    helpstr[CLI_HELPSTR_LEN]; /* Description of the command. */
} CLI_CMD_HELP;

/***************************************************************************\
An array of CLI_CMD_HELP structures. Used for displaying help information
when the user types '?' or Keyword?. When the user presses only '?' then CLI
displays all the commands present under that node. This structure is filled
and returned.
\***************************************************************************/
typedef struct cli_help_set {
   uns16           count;
   CLI_CMD_HELP    helpargs[CLI_HLP_COUNT];
} CLI_HELP_SET;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                 COMMAND TREE DATA-STRUCTURES  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/***************************************************************************
            Structure that defines the command node
***************************************************************************/
typedef struct cli_cmd_node {
   struct cli_cmd_element  *pCmdl;     /* pointer to command list present 
                                          under this node */
   struct cli_cmd_node     *pChild;    /* pointer to child subnodes */    
   struct cli_cmd_node     *pSibling;  /* pointer to sibling nodes */   
   struct cli_cmd_node     *pParent;   /* Parent pointer */      
   NCSCONTEXT              *pData;     /* Void pointer for storing CEF MODE */
   NCSCLI_ACCESS_PASSWD    accPswd;    /* password data associated with node */    
   int8                    mode[CLI_MODE_LEN]; /* Mode associated with this node */    
   int8                    name[CLI_NODE_LEN]; /* node name */
} CLI_CMD_NODE;

/***************************************************************************\
  CLI Command Structures

  The Command structures are used in CLI for storing command related info

  The CLI_TOKEN_NODE struct is part of CLI_CMD_ELEMENT struct.

  The storage of commands using these structs is as follows

    - Each CLI_CMD_ELEMENT structure represents a command elementn in a
      command. The command element means a single token, it can be a keyword
      or literal.
    - The CLI_CMD_ELEMENT is used to store commands under CLI_CMD_NODE
      struct.
    - The first token represents the command name
    - All command names are linked by sibling pointers.

  The CLI_TOKEN_NODE contains a list of pointers to CLI_CMD_ELEMENT
  The following is the organisation of Nodes and Commands.

  In Cisco format the following repesents some portion of node hierarcy.

      ConfigNode -> ( Has 2 commands "Router OSPF NUMBER", "Line NUMBER"
            -> Router Node, Line Node, Interface Node (Child nodes of Config Node)
                    -> BGP Node, OSPF Node ...(Child Nodes of Router nodes)


  The following diagram shows the representation of commands using data structs

                          Commands Under Config Node
                          +-------------------------+

                           Router Command        Line Command
   Config Node        +--+ - - - - - -+   +-----+------------+
  +----------------+  |  | router     |   |     | Line       |
  | cmdnode        +--+  | Keyword    |   |     | Keyword    |
  | childnode      +-+   | Mandatory  |   |     | Mandatory  |
  | siblingnode    | |   | .....      |   |     | ...        |
  +----------------+ |   | sibpingptr +---+     | sibpingptr |
                     |   | dataptr    |         | dataptr    |
                     |   | childptr   +---+     | childptr   |
                     |   | parentptr  |   |     | parentptr  |
                     |   +------------+   |     +------------+
                     |                    |
                     |                    |       OSPF Keyword
                     |                    +-----+------------+
                     |                          | OSPF       |
                     |                          | Keyword    |
                     |                          | Mandatory  |
                     |                          | ...        |
                     |                          | sibpingptr |
                     |                          | dataptr    |
                     |       RouterNode         | childptr   |
                     +--->+--------------+      | parentptr  |
                          | cmdnode      |      +------------+
                          | childnode    +--+
                          | siblingnode  |  |
                          +--------------+  |
                                            |   BGP Node
                                            +--->+--------------+
                                                 | cmdnode      |
                                                 | childnode    |
                                                 | siblingnode  +-+
                                                 +--------------+ |
                                                                  |
                                                                  |
                                                   OSPF Node      |
                                                 +--------------+-+
                                                 | cmdnode      |
                                                 | childnode    |
                                                 | siblingnode  |
                                                 +--------------+

\******************************************************************************/

/*****************************************************************************\
            Structure that define attributes of Range
\*****************************************************************************/
typedef struct cli_range {
   NCSCONTEXT   uLimit; /* Upper limit of range */    
   NCSCONTEXT   lLimit; /* Lower limit of range */
} CLI_RANGE;

/******************************************************************************\
   Structure defining the attributes of links associated with each of the
   command node in the command tree
\*****************************************************************************/
typedef struct cli_token_node {    
   struct cli_cmd_element  *pParent;   /* pointer to the parent node */    
   struct cli_cmd_element  *pSibling;  /* points to sibling node */    
   struct cli_cmd_element  *pDataPtr;  /* pointer to the data node */    
   struct cli_cmd_element  *pChild;    /* pointer to the child node */ 
} CLI_TOKEN_NODE;

/******************************************************************************
   Structure defining attributes of the command nodes in the command tree
******************************************************************************/
typedef struct cli_cmd_element {
   CLI_TOKEN_NODE node_ptr;        /* points to token node struct*/    
   NCSCLI_BINDERY *bindery;        /* Bindary of the subsystem */    
   NCSCLI_EXEC    cmd_exec_func;   /* Command execution function associated         with each command and is stored with the last level token in the command tree.*/    
   uns8           cmd_access_level; /* Command access level associated         with each command */
   CLI_RANGE      *range;          /* pointer to range structure if range value
                                      specified otherwise NULL */    
   int8           *tokName;        /* Contains token or Literal name*/    
   int8           *helpStr;        /* help string associated with the command */    
   int8           *nodePath;       /* "config/router/bgp" or "config/Line" */
   NCSCONTEXT     defVal;          /* stores default value otherwise NULL */    
   NCSCLI_TOKEN_TYPE tokType;      /* specifies token type */    
   uns32          tokRel;          /* Relationship of token with the previuos token */    
   uns8           prntTokLvl;      /* indicates level of token, is applicable for main
                                      levels others it is NULL */    
   uns8           tokLvl;          /* indicates level of token */    
   uns8           isMand;          /* specifies if token is mandatory */    
   uns8           modChg;          /* indicates if there is node change */        
   uns8           isCont;          /* Iscontinous flag */   
} CLI_CMD_ELEMENT;

/******************************************************************************\
The CLI_EXECUTE_PARAM struct is used to pass information about the command
string, the operation user has requested. The o_status contains value indicating
the output. The Union contains differente strings based on i_cmdtype
\******************************************************************************/
typedef struct cli_execute_param {    
   CLI_CMD_EXECUTE   i_cmdtype;    /* indicates the cmd operation 'TAB', HELP' or '\n' */
   CLI_CMD_MATCH     o_status;     /* indicates the cmd execution status */
   uns32             o_errpos;     /* indicates error position */  
   uns32             o_tokprocs;   /* Number of token processed */
   int8              *i_cmdbuf;    /* command string user has entered */      
   NCS_BOOL          i_execcmd;   /* execute(TRUE)/verify(FALSE) the command */
   NCSCLI_EXEC       cmd_exec_func;/* Call back function */    
   uns8              cmd_access_level; /*cli cmd access level */
   NCSCLI_BINDERY    *bindery;     /* Bindary of the subsystem */
   
   union {        
      int8           tabstring[CLI_CMD_LEN]; /* user has presses 'TAB' contains
                                                   remaining part of command */        
      CLI_HELP_SET   hlpstr;                    /* help struct */
   } o_hotkey;
} CLI_EXECUTE_PARAM;


/******************************************************************************
This struct is used internally to fill the arg structure, The tokentype
contains the type of token, based on which the corresponding member of union
is copied into the arg struct.
******************************************************************************/
typedef struct cli_cmd_args {    
   NCSCLI_TOKEN_TYPE   i_tokentype; /* indicates the token, String, Int or IP Address */
   union {       
      uns32   value;  /* Integer number */        
      uns32   ipaddr; /* An IPv4 address */        
      int8    string[CLI_BUFFER_SIZE]; /* String value */
   } i_token;
} CLI_CMD_ARGS;

/******************************************************************************
An error struct contaning error string and position
******************************************************************************/
typedef struct cli_cmd_error {    
   uns16   errorpos;   /* error position */    
   int8    errstring[CLI_BUFFER_SIZE]; /* Contains error string */
} CLI_CMD_ERROR;


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                       GENERAL DATA-STRUCTURES  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/******************************************************************************
Global variable used by the CLI Parser i.e. Flex & Bison module
******************************************************************************/
typedef struct cli_par_cb {
   /* Variable used by Yacc and Lex for manipulation of counter */    
   CLI_OPR_MODE            mode;
   CLI_BUFFER_TYPE         bfrType;
   CLI_TOKEN_RELATION      relnType;
   CLI_BRACE_TYPE          brcType;
   CLI_BRACE_TYPE          brcStack[CLI_PAR_BRACE_LEVEL];
   uns16                   tokLvlCntr;
   int32                   brcStackPtr;
   
   /* Optional stack and the optional stack pointer */
   int32                   optStack[CLI_PAR_STACK_LEVEL];
   int32                   optStackPtr;
   uns16                   optLvlCntr;
   uns16                   optCntr;
   uns16                   optTokCnt;
   
   /* Group stack and Group stack pointer */
   int32                   grpStack[CLI_PAR_STACK_LEVEL];
   int32                   grpStackPtr;
   uns16                   grpLvlCntr;
   uns16                   grpCntr;
   uns16                   grpTokCnt;
   
   /*Cli input buffer */
   int8                    ipbuffer[CLI_BUFFER_SIZE];
   
   /* Token Buffer */    
   uns16                   tokCnt;
   CLI_CMD_ELEMENT         *tokList[CLI_MAX_TOK_BUFF];
   
   /* Error marker */
   uns32                   chCnt;
   uns16                   errPos;
   int8                    str[15];
   
   uns8                    errFlag;
   uns8                    cont_expFlag;
   uns8                    is_tok_cont;
   uns8                    orFlag;
   uns8                    resetLvlFlag;
   uns8                    grpBrace;
   uns8                    firstOptChild;
   uns8                    firstGrpChild;
} CLI_PAR_CB;

/******************************************************************************
Global variable used by the CLI Command Tree module
******************************************************************************/
typedef struct cli_cmdtree_cb {
   /* Root node marker */
   CLI_CMD_NODE            *ctxtMrkr;
   CLI_CMD_NODE            *trvMrkr;
   CLI_CMD_NODE            *rootMrkr;
   CLI_CMD_NODE            *currNode;
   
   /* Do function*/
   NCSCLI_EXEC              execFunc;
   uns8                     cmd_access_level; /* cli cmd access level( 59359 )*/
   
   /* Bindary of the subsystem */
   NCSCLI_BINDERY           *bindery;
   
   /* Sub node marker */
   CLI_CMD_ELEMENT         *cmdMrkr;
   CLI_CMD_ELEMENT         *lvlMrkr;
   CLI_CMD_ELEMENT         *optMrkr;
   CLI_CMD_ELEMENT         *orMrkr;
   CLI_CMD_ELEMENT         *currMrkr;
   CLI_CMD_ELEMENT         *contMrkr;
   
   /* Dummy node stack */
   CLI_CMD_ELEMENT         *dummyStack[CLI_DUMMY_STACK_SIZE];
   int32                   dummyStackMrkr;    
   
   /* Variable used for traversing the command tree */
   CLI_CMD_ELEMENT         *cmdElement;
   CLI_CMD_NODE            *modeStack[CLI_CMD_STACK_SIZE];
   int32                   modeStackPtr;
   
   /*Variable for command history */
   CLI_CMD_HISTORY         *cmdHtry;
   CLI_CMD_HISTORY_MARKER  htryMrkr;
   
   /* De-Registration flag */    
   CLI_CMD_ELEMENT         **dregStack;
   int32                   dregStackMrkr;
   uns8                    dereg;
   
   /* Flag used to denote the existence of the node in the tree */
   uns8                    nodeFound;
} CLI_CMDTREE_CB;


/*****************************************************************************\
 Timer Structure, CLI thread waits on a timer. CEF/Response threads can reset
 the timer using CLI API 
\*****************************************************************************/
typedef struct cli_cef_timer {    
   tmr_t             tid;     /* CEF specific Timer ID */    
   NCSCLI_CEF_TIMER  tmrCB;   /* Timer Call back function */
} CLI_CEF_TIMER;
/*****************************************************************************\
 Timer Structure, CLI thread waits on a timer to quit when delay threshold is reached.  
\*****************************************************************************/
/* added to fix the bug 58609 */
typedef struct cli_idle_timer {    
   tmr_t             timer_id;     /*  CLI Idle Timer ID */    
   uns32             period;
   uns32             grace_period;
   uns8              warning_msg;
   void              *cb;
} CLI_IDLE_TIMER;


/*****************************************************************************\
               Global variable used by the CLI
\*****************************************************************************/
typedef struct cli_cb {
#if (NCSCLI_LOG == 1)
   char                logfile[32];
   uns32               logmask;
#endif
   
   /* Key that uniquely identifies the control block */
   uns32                cli_hdl;
   NCS_LOCK             ctreeLock;
   NCSCLI_GETCHAR       readFunc;     /* Cli input function ie getchar */
   
   /* Notify function pointer */
   uns32                cli_notify_hdl;
   NCSCLI_NOTIFY        cli_notify;        
   NCS_LOCK             mainLock;      /* CLI Control block lock */    
   CLI_PAR_CB           par_cb;        /* Parser control block */    
   CLI_CMDTREE_CB       ctree_cb;      /* Command Tree control block */    
   NCSCLI_SUBSYS_CB     subsys_cb;     /* Subsystem control block */    
   CLI_CEF_TIMER        cefTmr;        /* CEF/Response functions timer struct*/    
   NCSCONTEXT           cefSem;        /* Semaphore for CEF */    
   NCSCONTEXT           consoleId;     /* CLI Console ID */        
   uns32                cefStatus;     /* CEF return status */
   uns16                semUsgCnt;
   uns8                 loginFlag;     /* Login flag */    
   uns8                 cefTmrFlag;    /* CEF Timer flag */    
   CLI_IDLE_TIMER       idleTmr;       /* Added to fix the bug 58609 */    
   uns32                cli_last_active_time;
   uns32                cli_idle_time;
   uns8                 user_access_level; /*cli user access level ( 59359 )*/
   NCS_CLI_GROUP        cli_user_group;    /*cli group names ( 59359 )*/
   int8                 *outOfBoundaryToken; /* Added to fix the bug 59854 */ 
} CLI_CB;

/* location to read the CEFs to be loaded into CLI Engine */ 
#define m_NCSCLI_CEFS_CONFIG_FILE OSAF_SYSCONFDIR  "cli_cefslib_conf"

#endif
