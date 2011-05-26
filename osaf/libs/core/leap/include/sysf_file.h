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

  H&J Counting Semaphore Facility.

******************************************************************************
*/

/** Module Inclusion Control...
 **/
#ifndef SYSF_FILE_H
#define SYSF_FILE_H

/****************************************************************************
 * m_NCS_FILE_OPS
 *
 * This macro is invoked in order to perform file operations
 *
 * ARGUMENTS:
 *
 * A pointer to NCS_OS_FILE structure and the request type
 *
 * RETURNS:
 *
 * NCSCC_RC_SUCCESS  if counting semaphore created and initialized successfully.
 * <error return>   otherwise (such as NCSCC_RC_FAILURE).
 */
#define m_NCS_FILE_OP(file, file_request)    ncs_file_op(file, file_request)

uns32 ncs_file_op(NCS_OS_FILE *file, NCS_OS_FILE_REQUEST file_request);

#endif   /* SYSF_FILE_H */
