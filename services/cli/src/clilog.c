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

  MODULE: CLILOG.C



@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  DESCRIPTION:

  This module contains the logging/tracing functions for CLI

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "cli.h"

#if(NCS_CLI == 1)
#if(NCSCLI_LOG == 1)

/*****************************************************************************

  PROCEDURE NAME:    log_cli_headline

  DESCRIPTION:       Headline logging info

*****************************************************************************/
void log_cli_headline(uns8 hdln_id)
{
   uns8  sev = 0;

   switch(hdln_id) {
   case NCSCLI_HDLN_CLI_CB_ALREADY_EXISTS:
      sev  = NCSFL_SEV_WARNING;
      break;
      
   case NCSCLI_HDLN_CLI_RANGE_NOT_INT:
   case NCSCLI_HDLN_CLI_INVALID_PASSWD:
   case NCSCLI_HDLN_CLI_INVALID_NODE:
   case NCSCLI_HDLN_CLI_INVALID_PTR:
   case NCSCLI_HDLN_CLI_MALLOC_FAILED:
   case NCSCLI_HDLN_CLI_DISPLAY_WRG_CMD:
      sev  = NCSFL_SEV_ERROR;
      break;
      
   default:
      sev  = NCSFL_SEV_INFO;
      break;
   }

   /* Log headlines */
   ncs_logmsg(NCS_SERVICE_ID_CLI, NCSCLI_LID_HEADLINE, NCSCLI_FC_HDLN, 
              NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI, hdln_id);    
}


/*****************************************************************************

  PROCEDURE NAME:    log_cli_comments

  DESCRIPTION:       Timer logging info

*****************************************************************************/
void log_cli_comments(char *c1, uns8 id, char *c2)
{
   uns8  lid = 0, sev = 0;
   
   switch(id) {
   case NCSCLI_PAR_INVALID_IPADDR:
   case NCSCLI_PAR_UNTERMINATED_OPTBRACE:
   case NCSCLI_PAR_UNTERMINATED_GRPBRACE:
   case NCSCLI_PAR_INVALID_HELPSTR:
   case NCSCLI_PAR_INVALID_DEFVAL:
   case NCSCLI_PAR_INVALID_MODECHG:
   case NCSCLI_PAR_INVALID_RANGE:
      lid = NCSCLI_LID_CMT;
      sev = NCSFL_SEV_ERROR;
      break;

   case NCSCLI_SCRIPT_FILE_CLOSE:
   case NCSCLI_SCRIPT_FILE_OPEN:
   case NCSCLI_SCRIPT_FILE_READ:
      lid = NCSCLI_LID_SCRIPT;
      sev = NCSFL_SEV_ERROR;
      break;
      
   default:
      lid = NCSCLI_LID_CMT;
      sev = NCSFL_SEV_INFO;
      break;
   }
   
   /* Log comments */
   ncs_logmsg(NCS_SERVICE_ID_CLI, lid, NCSCLI_FC_CMT, 
      NCSFL_LC_API, sev, NCSFL_TYPE_TCIC, c1, id, c2);    
}

#endif  /* NCSCLI_LOG == 1*/

#else
extern int dummy;
#endif /* (if NCS_CLI == 1) */
