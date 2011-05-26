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

  This module is the include file exposed to the managment enity
  that manages the Availability Director by initiating it.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_DL_API_H
#define AVD_DL_API_H

/* DL SE API for init and destroy of the AVD module */
uns32 avd_lib_req(NCS_LIB_REQ_INFO *req_info);

#endif   /* AVD_DL_API_H */
