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

  MODULE NAME: NCS_DUMMY.H

..............................................................................

  DESCRIPTION

  This file has the following functions.

  ncs_dummy_var_arg_func      A empty function to nullify the debug prints.
******************************************************************************
*/

#ifndef NCS_DUMMY_H
#define NCS_DUMMY_H

EXTERN_C LEAPDLL_API int ncs_dummy_var_arg_func(const char *format, ...);

#endif
