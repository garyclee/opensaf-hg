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

  This module contains routines related H&J File operations.
..............................................................................

  FUNCTIONS INCLUDED in this module:

  ncs_file_op....perform the requested file operations

 ******************************************************************************
 */

#include <ncsgl_defs.h>
#include "ncs_osprm.h"
#include "sysf_file.h"

uint32_t ncs_file_op(NCS_OS_FILE *file, NCS_OS_FILE_REQUEST file_request)
{
	return m_NCS_OS_FILE(file, file_request);
}
