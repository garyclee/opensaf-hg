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

  $Header: 

  ..............................................................................

  FUNCTIONS INCLUDED in this module:

  mbcsv_dbg_sink     - Function for printing debug messages.
  mbcsv_dbg_sink_svc - Function for printing debug messages with service ID.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "mbcsv.h"

/*****************************************************************************

  PROCEDURE:    mbcsv_dbg_sink/mbcsv_dbg_sink_svc

  DESCRIPTION:

   MBCSV is entirely instrumented to fall into this debug sink function
   if it hits any if-conditional that fails, but should not fail. This
   is the default implementation of that SINK macro.

  ARGUMENTS:

  uns32   l             line # in file
  char*   f             file name where macro invoked
  code    code          Error code value.. Usually FAILURE
  char*   str           Error string which tell the reason of this debug sink.

  RETURNS:

  code    - just echo'ed back 

*****************************************************************************/
uns32 mbcsv_dbg_sink(uns32 l, char* f, long code, char *str)
{
#if 0
   m_NCS_CONS_PRINTF ("IN MBCSV_DBG_SINK: line %d, file %s\n",l,f);
   
   if (NULL != str)
   {
      m_NCS_CONS_PRINTF ("Reason : %s \n", str);
   }
#endif

   m_LOG_MBCSV_DBG_SNK(str, f, l);

   return code;
}

uns32 mbcsv_dbg_sink_svc(uns32 l, char* f, uns32 code, char *str, uns32 svc_id)
{
#if 0
   m_NCS_CONS_PRINTF ("IN MBCSV_DBG_SINK: line %d, file %s\n",l,f);
   
   if (NULL != str)
   {
      m_NCS_CONS_PRINTF ("Reason : %s, svc = %d \n", str, svc_id);
   }
#endif

   m_LOG_MBCSV_DBG_SNK_SVC(str, svc_id, f, l);

   return code;
}
