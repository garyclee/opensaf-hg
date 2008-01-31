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

  DESCRIPTION:

  This module contains target system specific declarations related to
  resource locking using abstract semaphores.

..............................................................................
*/

/*
 * Module Inclusion Control...
 */
#ifndef SYSF_LCK_H
#define SYSF_LCK_H

#include "ncssysf_lck.h"
#include "ncs_svd.h"



EXTERN_C unsigned int ncs_lock_create_mngr(void);
EXTERN_C unsigned int ncs_lock_destroy_mngr(void);

#endif
