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

  MODULE : CLILOG.H

  $Header: /ncs/software/leap/base/products/cli/inc/clilog.h 7     3/28/01 12:01p Claytom $

..............................................................................

  DESCRIPTION:

  This module contains logging/tracing defines for CLI.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef CLILOG_H
#define CLILOG_H

#include "ncs_log.h"
#include "dts_papi.h"
#include "dta_papi.h"

#define NCSCLI_LC_HEADLINES          0x00000001
#define NCSCLI_LC_CMT                0x00000002

/* DTSv versining support */
#define CLI_LOG_VERSION   1

/* Forward declarations */
struct CLI_CB;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                          Manifest Constants
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
typedef enum ncscli_comments
{
    NCSCLI_REL_CHILDOF,
    NCSCLI_REL_SIBLINGOF,
    NCSCLI_REL_OPTCHILDOF,
    NCSCLI_REL_OPTSIBLINGOF,
    NCSCLI_PAR_KEYWORD,
    NCSCLI_PAR_STRING,
    NCSCLI_PAR_NUMBER,
    NCSCLI_PAR_IPv4,
    NCSCLI_PAR_IPv6,
    NCSCLI_PAR_MASKv4,
    NCSCLI_PAR_CIDRv6,
    NCSCLI_PAR_GRPBRACE,
    NCSCLI_PAR_OPTBRACE,
    NCSCLI_PAR_HELPSTR,
    NCSCLI_PAR_DEFVAL,
    NCSCLI_PAR_MODECHG,
    NCSCLI_PAR_RANGE,
    NCSCLI_PAR_INVALID_IPADDR,
    NCSCLI_PAR_UNTERMINATED_OPTBRACE,
    NCSCLI_PAR_UNTERMINATED_GRPBRACE,
    NCSCLI_PAR_INVALID_HELPSTR,
    NCSCLI_PAR_INVALID_DEFVAL,
    NCSCLI_PAR_INVALID_MODECHG,
    NCSCLI_PAR_INVALID_RANGE,
    NCSCLI_PAR_MANDATORY_TOKEN,
    NCSCLI_PAR_OPTIONAL_TOKEN,
    NCSCLI_PAR_INVALID_CONTINOUS_EXP,

    NCSCLI_SCRIPT_FILE_CLOSE,
    NCSCLI_SCRIPT_FILE_OPEN,
    NCSCLI_SCRIPT_FILE_READ

} NCSCLI_COMMENTS;

/******************************************************************************
 Logging offset indexes for Headline logging
 ******************************************************************************/

typedef enum ncscli_hdln_flex
{
    NCSCLI_HDLN_CLI_CB_ALREADY_EXISTS,
    NCSCLI_HDLN_CLI_CREATE_SUCCESS,
    NCSCLI_HDLN_CLI_MAIN_LOCKED,
    NCSCLI_HDLN_CLI_MAIN_UNLOCKED,
    NCSCLI_HDLN_CLI_CTREE_LOCKED,
    NCSCLI_HDLN_CLI_CTREE_UNLOCKED,
    NCSCLI_HDLN_CLI_RANGE_NOT_INT,
    NCSCLI_HDLN_CLI_INVALID_PASSWD,
    NCSCLI_HDLN_CLI_INVALID_NODE,
    NCSCLI_HDLN_CLI_NO_MATCH,
    NCSCLI_HDLN_CLI_PARTIAL_MATCH,
    NCSCLI_HDLN_CLI_AMBIG_MATCH,
    NCSCLI_HDLN_CLI_SUCCESSFULL_MATCH,
    NCSCLI_HDLN_CLI_INVALID_PTR,
    NCSCLI_HDLN_CLI_MALLOC_FAILED,
    NCSCLI_HDLN_CLI_NEWCMD,
    NCSCLI_HDLN_CLI_TASK_CREATE_FAILED,
    NCSCLI_HDLN_CLI_DISPLAY_FAILED,
    NCSCLI_HDLN_CLI_CEF_FAILED,
    NCSCLI_HDLN_CLI_INVALID_BINDERY,
    NCSCLI_HDLN_CLI_SET_LOGFILE_SUCCESS,
    NCSCLI_HDLN_CLI_SET_LOGMASK_SUCCESS,
    NCSCLI_HDLN_CLI_REG_CMDS_SUCCESS,
    NCSCLI_HDLN_CLI_REG_CMDS_FAILURE,
    NCSCLI_HDLN_CLI_DEREG_CMDS_SUCCESS, 
    NCSCLI_HDLN_CLI_DEREG_CMDS_FAILURE,
    NCSCLI_HDLN_CLI_CONSOLE_READY,
    NCSCLI_HDLN_CLI_SUBSYSTEM_REGISTER,
    NCSCLI_HDLN_CLI_CEF_TMR_ELAPSED,
    NCSCLI_HDLN_CLI_MORE_TIME,
    NCSCLI_HDLN_CLI_DISPLAY_WRG_CMD, 
    NCSCLI_HDLN_CLI_CONF_FILE_OPEN_FAILED,
    NCSCLI_HDLN_CLI_USER, 
    NCSCLI_HDLN_CLI_DLIB_LOAD_FAILED,
    NCSCLI_HDLN_CLI_DLIB_SYM_FAILED,
    NCSCLI_HDLN_CLI_CEFS_REG_DEREG_FAILED,
    NCSCLI_HDLN_CLI_CEFS_REG_DEREG_SUCCESS,
    NCSCLI_HDLN_CLI_CONF_FILE_CORRUPTED,
    NCSCLI_HDLN_CLI_CONF_FILE_READ,
    NCSCLI_HDLN_CLI_USER_INFO_ACCESS_ERROR,
    NCSCLI_HDLN_CLI_MAX,
}NCSCLI_HDLN_FLEX;



/******************************************************************************
 Logging offset indexes for canned constant strings for the ASCII SPEC
 ******************************************************************************/

typedef enum cli_flex_sets
{
    NCSCLI_FC_CMT,
    NCSCLI_FC_HDLN,
    NCSCLI_FC_MAX
} CLI_FLEX_SETS;


typedef enum cli_log_ids
{
    NCSCLI_LID_HEADLINE,
    NCSCLI_LID_CMT,
    NCSCLI_LID_SCRIPT, 
    CLI_LID_C, 
    CLI_LID_CC, 
    CLI_LID_CCC, 
    CLI_LID_CI, 
    CLI_LID_I, 
    CLI_LID_MAX
} CLI_LOG_IDS;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                          NCSCLI Logging Control
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
extern const char *ncscli_evt_str[];    /* Defined in /src/ncscliutil.c */

#if (NCSCLI_LOG == 1)
EXTERN_C void  log_cli_headline(uns8);
EXTERN_C void  log_cli_comments(char *, uns8, char *);
EXTERN_C uns32 cli_log_ascii_reg(void);
EXTERN_C uns32 cli_log_ascii_dereg();

#define m_LOG_NCSCLI_HEADLINE(id)                   log_cli_headline(id)
#define m_LOG_NCSCLI_COMMENTS(c1, id, c2)           log_cli_comments(c1, id, c2)

#define m_LOG_NCSCLI_STR(c, s, id, str) \
        ncs_logmsg(NCS_SERVICE_ID_CLI, CLI_LID_C, \
                   NCSCLI_FC_HDLN, c, s, \
                   "TIC", id, str)

#define m_LOG_NCSCLI_STR_STR(c, s, id, str1, str2) \
        ncs_logmsg(NCS_SERVICE_ID_CLI, CLI_LID_CC, \
                   NCSCLI_FC_HDLN, c, s, \
                   "TICC", id, str1, str2)

#define m_LOG_NCSCLI_STR_STR_STR(c, s, id, s1, s2, s3) \
        ncs_logmsg(NCS_SERVICE_ID_CLI, CLI_LID_CCC, \
                   NCSCLI_FC_HDLN, c, s, \
                   "TICCC", id, s1, s2, s3)

#define m_LOG_NCSCLI_STR_I(c, s, id, str, i) \
        ncs_logmsg(NCS_SERVICE_ID_CLI, CLI_LID_CI, \
                   NCSCLI_FC_HDLN, c, s, \
                   "TICL", id, str, i)
        
#define m_LOG_NCSCLI_I(c, s, id, i) \
        ncs_logmsg(NCS_SERVICE_ID_CLI, CLI_LID_I, \
                   NCSCLI_FC_HDLN, c, s, \
                   "TIL", id, i)
#else
#define m_LOG_NCSCLI_HEADLINE(id)
#define m_LOG_NCSCLI_COMMENTS(c1, id, c2)
#define m_LOG_NCSCLI_STR(c, s, id, str) 
#define m_LOG_NCSCLI_STR_STR(c, s, id, str1, str2) 
#define m_LOG_NCSCLI_STR_STR_STR(c, s, id, s1, s2, s3)
#define m_LOG_NCSCLI_STR_I(c, s, id, str, i)
#define m_LOG_NCSCLI_I(c, s, id, i)
#endif /* NCSCLI_LOG == 1 */ 

#endif

