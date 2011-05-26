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


*/

/** H&J Includes...
 **/

#include "dts.h"

/*****************************************************************************

  PROCEDURE NAME:    dts_dbg_sink

  DESCRIPTION:

   DTS is entirely instrumented to fall into this debug sink function
   if it hits any if-conditional that fails, but should not fail. This
   is the default implementation of that SINK macro.
 
   The customer is invited to redefine the macro (in rms_tgt.h) or 
   re-populate the function here.  This sample function is invoked by
   the macro m_DTS_DBG_SINK.
   
  ARGUMENTS:

  uint32_t   l             line # in file
  char*   f             file name where macro invoked
  code    code          Error code value.. Usually FAILURE

  RETURNS:

  code    - just echo'ed back 

*****************************************************************************/

#if ((DTS_DEBUG == 1) || (DTS_LOG == 1))

uint32_t dts_dbg_sink(uint32_t l, char *f, uint32_t code, char *str)
{
#if (DTS_DEBUG == 1)
	TRACE("IN DTS_DBG_SINK: line %d, file %s\n", l, f);
#endif

	if (NULL != str) {
#if (DTS_DEBUG == 1)
		TRACE("Reason : %s \n", str);
#endif

#if (DTS_LOG == 1)
		m_LOG_DTS_DBGSTRL(DTS_IGNORE, str, f, l);
#endif
	}
	fflush(stdout);
	return code;
}

uint32_t dts_dbg_sink_svc(uint32_t l, char *f, uint32_t code, char *str, uint32_t svc)
{
#if (DTS_DEBUG == 1)
	TRACE("IN DTS_DBG_SINK: SVC = %d, line %d, file %s\n", svc, l, f);
#endif

	if (NULL != str) {
#if (DTS_DEBUG == 1)
		TRACE("Reason : %s \n", str);
#endif

#if (DTS_LOG == 1)
		m_LOG_DTS_DBGSTRL_SVC(DTS_IGNORE, str, f, l, svc);
#endif

	}
	fflush(stdout);
	return code;
}

uint32_t dts_dbg_sink_svc_name(uint32_t l, char *f, uint32_t code, char *str, char *svc)
{
#if (DTS_DEBUG == 1)
	TRACE("IN DTS_DBG_SINK: SVC = %s, line %d, file %s\n", svc, l, f);
#endif

	if (NULL != str) {
#if (DTS_DEBUG == 1)
		TRACE("Reason : %s \n", str);
#endif

#if (DTS_LOG == 1)
		m_LOG_DTS_DBGSTRL_SVC_NAME(DTS_IGNORE, str, f, l, svc);
#endif

	}
	fflush(stdout);
	return code;
}
#endif

/*****************************************************************************

  PROCEDURE NAME:    sysf_<dts-component>-validate

  DESCRIPTION:
  
  return the correct DTS subcomponent handle, which was handed over to the 
  DTS client at sub-component CREATE time in the form of the o_<dts>_hdl 
  return value.

  NOTES: This is sample code. The customer can choose any strategy that suits
  thier design.

*****************************************************************************/

/* For DTS validation */

void *sysf_dts_validate(uint32_t k)
{
	return (void *)ncshm_take_hdl(NCS_SERVICE_ID_DTSV, k);
}
