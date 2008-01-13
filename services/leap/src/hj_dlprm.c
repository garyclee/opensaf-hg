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


  MODULE NAME: NCS_DLPRM.C



..............................................................................

  DESCRIPTION:


  NOTES:


-----------------------------------------------------------------------
******************************************************************************
*/

#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"
#include "ncs_dlprm.h"

#include "ncs_dl.h"
#include "ncssysf_def.h"

#if (SYSF_IP_LOG == 1)
#ifdef __NCSINC_OSE__
#define m_SYSF_IP_LOG_ERROR(str,num)   dbgprintf("%s::0x%x\n",(str),(unsigned int)(num))
#define m_SYSF_IP_LOG_INFO(str,num)    dbgprintf("%s::0x%x\n",(str),(unsigned int)(num))
#else
#define m_SYSF_IP_LOG_ERROR(str,num)   m_NCS_CONS_PRINTF("%s::0x%x\n",(str),(unsigned int)(num))
#define m_SYSF_IP_LOG_INFO(str,num)    m_NCS_CONS_PRINTF("%s::0x%x\n",(str),(unsigned int)(num))
#endif
#else
#define m_SYSF_IP_LOG_ERROR(str,num)
#define m_SYSF_IP_LOG_INFO(str,num)
#endif



#if (NCS_USE_BSD_STYLE_ERROR == 1)
/***************************************************************************
 *
 * NCS_IP_ERROR
 * ncs_bsd_sock_error(void)
 *
 * Description:
 *   This routine converts a BSD error to an NCS error
 *
 * Synopsis:
 *
 * Call Arguments:
 *
 * Returns:
 *   Returns NCSCC_RC_SUCCESS if successful, otherwise NCSCC_RC_FAILURE.
 *
 ****************************************************************************/
NCSSOCK_ERROR_TYPES
ncs_bsd_sock_error(void)
{

   m_SYSF_IP_LOG_INFO("ncs_bsd_sock_error::errno", errno);

   switch (errno)
   {
#ifdef EPROTO
   case EPROTO:
      return NCSSOCK_ERROR_TYPES_IP_DOWN;
#endif
   case ENOMEM:
   case ENOBUFS:
      return NCSSOCK_ERROR_TYPES_NO_MEM;

   case EOPNOTSUPP:
      return NCSSOCK_ERROR_TYPES_CONN_UNSUPPORTED;

#ifdef ECONNREFUSED
   case ECONNREFUSED:
#endif
   case ECONNRESET:
   case ECONNABORTED:
   case ENOTCONN:
      return NCSSOCK_ERROR_TYPES_CONN_DOWN;

   case ENOTSOCK:
#ifdef EBADF
   case EBADF:
#endif
      return NCSSOCK_ERROR_TYPES_CONN_UNKNOWN;

#ifdef EINPROGRESS
   case EINPROGRESS:
#endif
   case EWOULDBLOCK:
      return NCSSOCK_ERROR_TYPES_NO_ERROR;

   default:
      return NCSSOCK_ERROR_TYPES_UNKNOWN;

   }

}
#endif
