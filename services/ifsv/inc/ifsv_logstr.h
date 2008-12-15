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

  MODULE NAME: IFSV_LOGSTR.H

$Header: 
..............................................................................

  DESCRIPTION: Defines Log register/deregister functions prototype.


******************************************************************************/

#ifndef IFSV_LOGSTR_H
#define IFSV_LOGSTR_H
EXTERN_C void ifsv_flx_log_ascii_set_reg (void);

EXTERN_C void ifsv_flx_log_ascii_set_dereg (void);

EXTERN_C uns32 ifsv_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);
#endif
