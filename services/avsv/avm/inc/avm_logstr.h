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



..............................................................................

  DESCRIPTION:

  This module contains the extern declaration for registering
  Availability Managers canned strings with Dtsv server.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVM_LOGSTR_H
#define AVM_LOGSTR_H
#if (NCS_DTS == 1)


EXTERN_C uns32 avm_reg_strings(void);
EXTERN_C uns32 avm_unreg_strings(void);
EXTERN_C uns32 avm_log_str_lib_req(NCS_LIB_REQ_INFO *req_info);

#endif /* (NCS_DTS == 1) */

#endif /* AVM_LOGSTR_H */
