/*      -*- OpenSAF  -*-
 *
 * Copyright Ericsson AB 2017 - All Rights Reserved.
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
 */

#ifndef BASE_FILE_DESCRIPTOR_H_
#define BASE_FILE_DESCRIPTOR_H_

namespace base {

// Set the a file descriptor's non-blocking flag. Returns true if successful,
// and false otherwise.
bool MakeFdNonblocking(int fd);

}  // namespace base

#endif  // BASE_FILE_DESCRIPTOR_H_
