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
#include "dta.h"

/*****************************************************************************

  PROCEDURE NAME:    dta_dbg_sink

  DESCRIPTION:

   DTA is entirely instrumented to fall into this debug sink function
   if it hits any if-conditional that fails, but should not fail. This
   is the default implementation of that SINK macro.
 
   The customer is invited to redefine the macro (in rms_tgt.h) or 
   re-populate the function here.  This sample function is invoked by
   the macro m_DTA_DBG_SINK.
   
  ARGUMENTS:

  uint32_t   l             line # in file
  char*   f             file name where macro invoked
  code    code          Error code value.. Usually FAILURE

  RETURNS:

  code    - just echo'ed back 

*****************************************************************************/

#if (DTA_DEBUG == 1)

uint32_t dta_dbg_sink(uint32_t l, char *f, uint32_t code, char *str)
{
	TRACE("IN DTA_DBG_SINK: line %d, file %s\n", l, f);

	if (NULL != str)
		TRACE("Reason : %s \n", str);

	return code;
}

uint32_t dta_dbg_sink_svc(uint32_t l, char *f, uint32_t code, char *str, SS_SVC_ID svc_id)
{
	TRACE("IN DTA_DBG_SINK: SVC_ID %d, line %d, file %s\n", svc_id, l, f);

	if (NULL != str)
		TRACE("Reason : %s \n", str);

	return code;
}

#endif
