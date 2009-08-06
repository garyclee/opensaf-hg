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
#include "mab.h"
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

  uns32   l             line # in file
  char*   f             file name where macro invoked
  code    code          Error code value.. Usually FAILURE

  RETURNS:

  code    - just echo'ed back 

*****************************************************************************/

#if (DTA_DEBUG == 1)

uns32 dta_dbg_sink(uns32 l, char* f, uns32 code, char * str)
  {
  printf ("IN DTA_DBG_SINK: line %d, file %s\n",l,f);

  if (NULL != str)
      printf ("Reason : %s \n", str);

  return code;
  }

uns32 dta_dbg_sink_svc(uns32 l, char* f, uns32 code, char * str, SS_SVC_ID svc_id)
  {
  printf ("IN DTA_DBG_SINK: SVC_ID %d, line %d, file %s\n",svc_id,l,f);

  if (NULL != str)
      printf ("Reason : %s \n", str);

  return code;
  }

#endif

