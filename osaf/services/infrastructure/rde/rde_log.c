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

  MODULE NAME: rde_log.c

..............................................................................

  DESCRIPTION:

  FUNCTIONS INCLUDED in this module:

  *******************************************************************************
  */

#include "rde.h"
#include "rde_log.h"
#include "rde_log_str.h"
#include "syslog.h"

/************************************************************************
 ************************************************************************

    I n d e x   M a n i p u l a t i o n   M a c r o s

  One of the key types in Flexlog is the Index value, which leads to a
  canned printable string. This value is as follows:

  uns16  index
  =======================
  Most segnificant byte  : Table ID otherwise known as an NCSFL_SET.
  Least segnificant byte : String ID offset in said table.

  ************************************************************************
  ************************************************************************/

#define m_NCSFL_MAKE_IDX(i,j)   (uns16)((i<<8)|j)

/*****************************************************************************

  PROCEDURE NAME:       rde_get_log_level_str

  DESCRIPTION:          Get the logging level string
                     
  ARGUMENTS:

  RETURNS:

  NOTES:

  
*****************************************************************************/

const char *rde_get_log_level_str(uns32 log_level)
{
	char *rde_log_level_str[] = {
		"EMERGENCY",
		"ALERT",
		"CRITICAL",
		"ERROR",
		"WARNING",
		"NOTICE",
		"INFO",
		"DEBUG"
	};

	return rde_log_level_str[log_level];
}

/*****************************************************************************

  PROCEDURE NAME:    rdeSysLog

  DESCRIPTION:       Logs to syslog 
           
*****************************************************************************/

void rdeSysLog(uns32 severity, char *logMessage)
{
	uns32 syslogPriority;

	/* filter based on priority
	 */

	/* Convert Flexlog severity into Syslog priority
	 */

	switch (severity) {
	case NCSFL_SEV_EMERGENCY:
		syslogPriority = LOG_EMERG;
		break;
	case NCSFL_SEV_ALERT:
		syslogPriority = LOG_ALERT;
		break;
	case NCSFL_SEV_CRITICAL:
		syslogPriority = LOG_CRIT;
		break;
	case NCSFL_SEV_ERROR:
		syslogPriority = LOG_ERR;
		break;
	case NCSFL_SEV_WARNING:
		syslogPriority = LOG_WARNING;
		break;
	case NCSFL_SEV_NOTICE:
		syslogPriority = LOG_NOTICE;
		break;
	case NCSFL_SEV_INFO:
		syslogPriority = LOG_INFO;
		break;
	case NCSFL_SEV_DEBUG:
		syslogPriority = LOG_DEBUG;
		break;
	default:
		syslogPriority = LOG_NOTICE;
		break;
	}

	syslog(syslogPriority, "%s", logMessage);

}

/*****************************************************************************

  PROCEDURE NAME:    rde_log_headline

  DESCRIPTION:       Headline logging info
           Defaults severity to NOTICE

*****************************************************************************/

void rde_log_headline(uns32 line, const char *file, const char *func, uns8 hdln_id)
{

	ncs_logmsg( /* service ID   */ NCS_SERVICE_ID_RDE,
		   /* format ID    */ RDE_FMT_HEADLINE,
		   /* str tbl ID   */ RDE_FC_HEADLINE,
		   /* category     */ NCSFL_LC_HEADLINE,
		   /* severity     */ NCSFL_SEV_NOTICE,
		   /* format type  */ NCSFL_TYPE_TI,
		   /* va_arg (...) */ hdln_id);

}

/*****************************************************************************

  PROCEDURE NAME:    rde_log_condition

  DESCRIPTION:       Log conditions

*****************************************************************************/

void rde_log_condition(uns32 line, const char *file, const char *func, uns8 sev, uns8 hdln_id)
{

	ncs_logmsg( /* service ID   */ NCS_SERVICE_ID_RDE,
		   /* format ID    */ RDE_FMT_HEADLINE,
		   /* str tbl ID   */ RDE_FC_CONDITION,
		   /* category     */ NCSFL_LC_HEADLINE,
		   /* severity     */ sev,
		   /* format type  */ NCSFL_TYPE_TI,
		   /* va_arg (...) */ hdln_id);

}

/*****************************************************************************

  PROCEDURE NAME:    rde_log_condition_int

  DESCRIPTION:       Log condition with int value

*****************************************************************************/

void rde_log_condition_int(uns32 line, const char *file, const char *func, uns8 sev, uns8 hdln_id, uns32 val)
{

	ncs_logmsg( /* service ID   */ NCS_SERVICE_ID_RDE,
		   /* format ID    */ RDE_FMT_HEADLINE_NUM,
		   /* str tbl ID   */ RDE_FC_CONDITION,
		   /* category     */ NCSFL_LC_HEADLINE,
		   /* severity     */ sev,
		   /* format type  */ NCSFL_TYPE_TIL,
		   /* va_arg (...) */ hdln_id,
		   val);

}

/*****************************************************************************

  PROCEDURE NAME:    rde_log_condition_float

  DESCRIPTION:       Headline logging info

*****************************************************************************/

void rde_log_condition_float(uns32 line, const char *file, const char *func, uns8 sev, uns8 hdln_id, DOUBLE val)
{
	ncs_logmsg( /* service ID   */ NCS_SERVICE_ID_RDE,
		   /* format ID    */ RDE_FMT_HEADLINE_TIME,
		   /* str tbl ID   */ RDE_FC_CONDITION,
		   /* category     */ NCSFL_LC_HEADLINE,
		   /* severity     */ sev,
		   /* format type  */ NCSFL_TYPE_TIF,
		   /* va_arg (...) */ hdln_id,
		   val);

}

/*****************************************************************************

  PROCEDURE NAME:    rde_log_condition_string

  DESCRIPTION:       Condition logging info

  TBD: This is for expediency only, may need to to be redone when
       integrating with FlexLog server ...

*****************************************************************************/

void rde_log_condition_string(uns32 line, const char *file, const char *func, uns8 sev, uns8 hdln_id, const char *str)
{
	ncs_logmsg( /* service ID   */ NCS_SERVICE_ID_RDE,
		   /* fomat ID     */ RDE_FMT_HEADLINE_STR,
		   /* str tbl ID   */ RDE_FC_CONDITION,
		   /* category     */ NCSFL_LC_HEADLINE,
		   /* severity     */ sev,
		   /* format type  */ NCSFL_TYPE_TIC,
		   /* va_arg (...) */ hdln_id,
		   str);
}

/*****************************************************************************

  PROCEDURE NAME:    rde_log_mem_fail

  DESCRIPTION:       Memory alloc failure

*****************************************************************************/

void rde_log_mem_fail(uns32 line, const char *file, const char *func, uns8 id)
{
	ncs_logmsg( /* service ID   */ NCS_SERVICE_ID_RDE,
		   /* fomat ID     */ RDE_FMT_MEM_FAIL,
		   /* str tbl ID   */ RDE_FC_MEM_FAIL,
		   /* category     */ NCSFL_LC_EVENT,
		   /* severity     */ NCSFL_SEV_ERROR,
		   /* format type  */ NCSFL_TYPE_TI,
		   /* va_arg (...) */ id);

	return;
}
