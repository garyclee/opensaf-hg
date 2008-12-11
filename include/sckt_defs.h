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

  MODULE NAME:  SCKT_DEFS.H

  DESCRIPTION:


******************************************************************************
*/
#ifndef SCKT_DEFS_H
#define SCKT_DEFS_H

#include "ncsgl_defs.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define SOCKET   int

/*****************************************************************************
 **                                                                         **
 **                                                                         **
 **             Socket macro definitions                                    **
 **                                                                         **
 ** Override default definition in ncs_ipprm.h                               **
 **                                                                         **
 **                                                                         **
 ****************************************************************************/

/* install SIGPIPE signal handler on socket init */
uns32 ncs_os_install_sigpipe_handler(void);

#define m_NCS_TS_SOCK_CREATE            ncs_os_install_sigpipe_handler()

#define m_NCS_TS_SOCK_SETSOCKOPT(sock,lvl,opt,optval,optlen)  \
                setsockopt((int)sock,(int)lvl,(int)opt,(const void *)optval,(socklen_t)optlen)

#define m_NCS_TS_SOCK_GETSOCKOPT(sock,lvl,opt,optval,optlen)  \
                getsockopt((int)sock,(int)lvl,(int)opt, (void *)optval,(socklen_t *)optlen)

#define m_NCS_TS_SOCK_GETSOCKNAME(sock, sa, sa_len)  \
                getsockname((int)sock, (struct sockaddr *)sa, (socklen_t *)sa_len)

#ifdef  __cplusplus
}
#endif

#endif /* SCKT_DEFS_H */
