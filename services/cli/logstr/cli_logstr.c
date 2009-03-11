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
  FILE NAME: CLI_LOGSTR.C

  DESCRIPTION: This file contains all the log string related to CLI.

******************************************************************************/

#include "cli.h"

#if(NCSCLI_LOG == 1)

uns32 cli_log_ascii_dereg(void);
uns32 cli_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);

/******************************************************************************\
                  Logging stuff for Command Tree Tokens
\******************************************************************************/
const NCSFL_STR ncscli_var_comments[] = {
    {NCSCLI_REL_CHILDOF, "CHILD-OF"                                   },
    {NCSCLI_REL_SIBLINGOF, "SIBLING-OF"                               },
    {NCSCLI_REL_OPTCHILDOF, "OPTIONAL-CHILD-OF"                       },
    {NCSCLI_REL_OPTSIBLINGOF, "OPTIONAL-SIBLING-OF"                   },
    {NCSCLI_PAR_KEYWORD, "(NCSCLI_KEYWORD)"                           },
    {NCSCLI_PAR_STRING, "(NCSCLI_STRING)"                             },
    {NCSCLI_PAR_NUMBER, "(NCSCLI_NUMBER)"                             },
    {NCSCLI_PAR_IPv4, "(NCSCLI_IPv4)"                                 },
    {NCSCLI_PAR_IPv6, "(NCSCLI_IPv6)"                                 },
    {NCSCLI_PAR_MASKv4, "(NCSCLI_MASKv4)"                             },
    {NCSCLI_PAR_CIDRv6, "(NCSCLI_CIDRv6)"                             },
    {NCSCLI_PAR_GRPBRACE, "(NCSCLI_OPTIONAL)"                         },
    {NCSCLI_PAR_OPTBRACE, "(NCSCLI_GROUP)"                            },
    {NCSCLI_PAR_HELPSTR, "(HELP STRING)"                              },
    {NCSCLI_PAR_DEFVAL, "(DEFAULT VALUE)"                             },
    {NCSCLI_PAR_MODECHG, "(MODE CHANGE)"                              },
    {NCSCLI_PAR_RANGE, "(RANGE)"                                      },
    {NCSCLI_PAR_INVALID_IPADDR, "INVALID-IPADDRESS"                   },
    {NCSCLI_PAR_UNTERMINATED_OPTBRACE, "UNTERMINATED-OPTIONAL-BRACE"  },
    {NCSCLI_PAR_UNTERMINATED_GRPBRACE, "UNTERMINATED-GROUP-BRACE"     },
    {NCSCLI_PAR_INVALID_HELPSTR, "INVALID-HELP-STRING"                },
    {NCSCLI_PAR_INVALID_DEFVAL, "DEFAULT-VALUE"                       },
    {NCSCLI_PAR_INVALID_MODECHG, "INAVLID-MODE-CHANGE"                },
    {NCSCLI_PAR_INVALID_RANGE, "INVALID-RANGE"                        },
    {NCSCLI_PAR_MANDATORY_TOKEN, "MANDATORY-TOKEN"                    },
    {NCSCLI_PAR_OPTIONAL_TOKEN, "OPTIONAL-TOKEN"                      },
    {NCSCLI_PAR_INVALID_CONTINOUS_EXP, "INVALID CONTINOUS EXPRESSION" },
    {NCSCLI_SCRIPT_FILE_CLOSE, "ERROR : FILE-CLOSE"                   },
    {NCSCLI_SCRIPT_FILE_OPEN,  "ERROR : FILE-OPEN"                    },
    {NCSCLI_SCRIPT_FILE_READ,  "ERROR : FILE-READ"                    },
    {0, 0}
};

/*****************************************************************************\
                        Logging stuff for Headlines
\*****************************************************************************/
const NCSFL_STR cli_hdln_set[] = {
    {NCSCLI_HDLN_CLI_CB_ALREADY_EXISTS, "CLI Control Block already exists"  },
    {NCSCLI_HDLN_CLI_CREATE_SUCCESS, "CLI Control Block created successfuly"},
    {NCSCLI_HDLN_CLI_MAIN_LOCKED, "CLI Control Block locked"                },
    {NCSCLI_HDLN_CLI_MAIN_UNLOCKED, "CLI Control Block unlocked"            },
    {NCSCLI_HDLN_CLI_CTREE_LOCKED, "CLI locked for registration"            },
    {NCSCLI_HDLN_CLI_CTREE_UNLOCKED, "CLI unlocked after registration"      },
    {NCSCLI_HDLN_CLI_RANGE_NOT_INT, "CLI Range Values Not Integer"          },
    {NCSCLI_HDLN_CLI_INVALID_PASSWD, "Invalid Password"                     },
    {NCSCLI_HDLN_CLI_INVALID_NODE, "Node does not exists"                   },
    {NCSCLI_HDLN_CLI_NO_MATCH, "Command Syntax not matched"                 },
    {NCSCLI_HDLN_CLI_PARTIAL_MATCH, "Partial Command Matched"               },
    {NCSCLI_HDLN_CLI_AMBIG_MATCH, "Ambiguous Command Entered"               },
    {NCSCLI_HDLN_CLI_SUCCESSFULL_MATCH, "Command Matched"                   },
    {NCSCLI_HDLN_CLI_INVALID_PTR, "Invalid Pointer"                         },
    {NCSCLI_HDLN_CLI_MALLOC_FAILED, "Malloc Failed"                         },
    {NCSCLI_HDLN_CLI_NEWCMD, "/*****************Command*****************/"  },
    {NCSCLI_HDLN_CLI_TASK_CREATE_FAILED, "Task Create Failed"               },
    {NCSCLI_HDLN_CLI_DISPLAY_FAILED, "Display of information failed"        },
    {NCSCLI_HDLN_CLI_CEF_FAILED, "Command Execution Failed"                 },
    {NCSCLI_HDLN_CLI_INVALID_BINDERY, "Invalid Bindery"                     },
    {NCSCLI_HDLN_CLI_SET_LOGFILE_SUCCESS, "Log file successfully created"   },
    {NCSCLI_HDLN_CLI_SET_LOGMASK_SUCCESS, "Log mask successfully set"       },
    {NCSCLI_HDLN_CLI_REG_CMDS_SUCCESS, "Commands Registration success"      },
    {NCSCLI_HDLN_CLI_REG_CMDS_FAILURE, "Commands Registration failed"       },
    {NCSCLI_HDLN_CLI_DEREG_CMDS_SUCCESS, "Commands Deregistration success"  },
    {NCSCLI_HDLN_CLI_DEREG_CMDS_FAILURE, "Commands Deregistration failed"   },
    {NCSCLI_HDLN_CLI_CONSOLE_READY, "CLI Console ready for use"             },
    {NCSCLI_HDLN_CLI_SUBSYSTEM_REGISTER, "Subsystem registered successfully"},
    {NCSCLI_HDLN_CLI_CEF_TMR_ELAPSED, "CEF Timer Elapsed"                   },
    {NCSCLI_HDLN_CLI_MORE_TIME, "CEF Requested More Time"                   },
    {NCSCLI_HDLN_CLI_DISPLAY_WRG_CMD, "Response Function printing for wrong command"                   },
    {NCSCLI_HDLN_CLI_CONF_FILE_OPEN_FAILED, "CLI cli_apps_cefs_load(): fopen() failed (file name)"     }, 
    {NCSCLI_HDLN_CLI_USER, "NCS CLI user is an authenticated "   },
    {NCSCLI_HDLN_CLI_DLIB_LOAD_FAILED, "CLI cli_apps_cefs_load(): m_NCS_OS_DLIB_LOAD() failed (lib name, error)" }, 
    {NCSCLI_HDLN_CLI_DLIB_SYM_FAILED, "CLI cli_apps_cefs_load(): m_NCS_OS_DLIB_SYMBOL() failed (lib name, func name, error)" }, 
    {NCSCLI_HDLN_CLI_CEFS_REG_DEREG_FAILED, "CLI cli_apps_cefs_load(): CEF reg failed (func name, error)" }, 
    {NCSCLI_HDLN_CLI_CEFS_REG_DEREG_SUCCESS, "CLI cli_apps_cefs_load(): CEF reg Success" }, 
    {NCSCLI_HDLN_CLI_CONF_FILE_CORRUPTED, "CLI cli_apps_cefs_load(): CLI CEFs configuration file corrupted (filename, read_args):" }, 
    {NCSCLI_HDLN_CLI_CONF_FILE_READ,"CLI cli_apps_cefs_load(): CLI is reading the CEFs from this file:"}, 
    {NCSCLI_HDLN_CLI_USER_INFO_ACCESS_ERROR,"error while fetching the cli user& group info"}, 
    
    { 0,0 }
};


/*****************************************************************************\
 Build up the canned constant strings for the ASCII SPEC
\******************************************************************************/
NCSFL_SET cli_str_set[] = {
    { NCSCLI_FC_CMT, 0, (NCSFL_STR*)ncscli_var_comments    },
    { NCSCLI_FC_HDLN, 0, (NCSFL_STR*)cli_hdln_set         },
    { 0,0,0 }
};

NCSFL_FMAT cli_fmat_set[] = {
    { NCSCLI_LID_HEADLINE, NCSFL_TYPE_TI,
        "CLI (STATUS DUMP) %s : %s\n"  },
    { NCSCLI_LID_CMT, NCSFL_TYPE_TCIC,
        "CLI (CLI CMD TREE DUMP) %s : [TOKEN] %s %s %s\n" },
    { NCSCLI_LID_SCRIPT, NCSFL_TYPE_TCIC,
        "CLI (SCRIPT DUMP) %s : [FILE] %s %s %s\n" },
    { CLI_LID_C, "TIC",
        "CLI %s : %s data: %s \n" },
    { CLI_LID_CC, "TICC",
        "CLI %s : %s str1: %s str2: %s \n" },
    { CLI_LID_CCC, "TICCC",
        "CLI %s : %s str1: %s str2: %s str3: %s\n" },
    { CLI_LID_CI, "TICL",
        "CLI %s : %s str: %s integer: %ld\n" },
    { CLI_LID_I, "TIL",
        "CLI %s : %s integer: %l\n" },
    { 0, 0, 0 }
};

NCSFL_ASCII_SPEC cli_ascii_spec = {   
   NCS_SERVICE_ID_CLI,            /* CLI subsystem */
   CLI_LOG_VERSION,               /* CLI (IPRP) log version ID */
   "CLI",                         /* CLI opening Banner in log */    
   (NCSFL_SET*)cli_str_set,       /* CLI const strings referenced by index */
   (NCSFL_FMAT*)cli_fmat_set,     /* CLI string format info */
   0,                             /* Place holder for str_set count */
   0                              /* Place holder for fmat_set count */
};

/****************************************************************************
 * Name          : cli_log_str_lib_req
 *
 * Description   : Library entry point for cli log string library.
 *
 * Arguments     : req_info  - This is the pointer to the input information 
 *                             which SBOM gives.  
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32
cli_log_str_lib_req(NCS_LIB_REQ_INFO *req_info)
{
   uns32 res = NCSCC_RC_FAILURE;

   switch (req_info->i_op)
   {
      case NCS_LIB_REQ_CREATE:
         res = cli_log_ascii_reg();
         break;

      case NCS_LIB_REQ_DESTROY:
         res = cli_log_ascii_dereg();
         break;

      default:
         break;
   }
   return (res);
}

/****************************************************************************\
  PROCEDURE NAME : cli_log_ascii_reg
 
  DESCRIPTION    : This is the function which registers the CLI's logging
                   ascii set with the DTSv Log server.                  
 
  ARGUMENTS      : None
 
  RETURNS        : SUCCESS - All went well
                   FAILURE - internal processing didn't like something.
\*****************************************************************************/
uns32 cli_log_ascii_reg(void)
{
   NCS_DTSV_REG_CANNED_STR arg;
   uns32                   rc = NCSCC_RC_SUCCESS;

   memset(&arg, 0, sizeof(arg));

   arg.i_op = NCS_DTSV_OP_ASCII_SPEC_REGISTER;
   arg.info.reg_ascii_spec.spec = &cli_ascii_spec;

   /* Regiser CLI with DTSv */
   rc = ncs_dtsv_ascii_spec_api(&arg);
   return rc;
}

/****************************************************************************\
  PROCEDURE NAME : cli_log_ascii_dereg
 
  DESCRIPTION    : This is the function which deregisters the CLI's logging
                   ascii set from the DTSv Log server.                  
 
  ARGUMENTS      : None
 
  RETURNS        : SUCCESS - All went well
                   FAILURE - internal processing didn't like something.
\*****************************************************************************/
uns32 cli_log_ascii_dereg()
{
   NCS_DTSV_REG_CANNED_STR arg;   
   uns32                   rc = NCSCC_RC_SUCCESS;

   memset(&arg, 0, sizeof(arg));      
   arg.i_op = NCS_DTSV_OP_ASCII_SPEC_DEREGISTER;
   arg.info.dereg_ascii_spec.svc_id = NCS_SERVICE_ID_CLI;
   arg.info.dereg_ascii_spec.version = CLI_LOG_VERSION;

   /* Deregister CLI from DTSv */
   rc = ncs_dtsv_ascii_spec_api(&arg);
   return rc;
}

#else
extern int dummy;
#endif  /* NCSCLI_LOG == 1*/

