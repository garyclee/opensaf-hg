/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2012 The OpenSAF Foundation
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
 * Author(s): Ericsson AB
 *
 */

#include "osaf_utility.h"
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>

void osaf_abort(const char* __restrict i_file, uint32_t i_line,
	uint32_t i_info1, uint32_t i_info2, uint32_t i_info3, uint32_t i_info4)
{
	syslog(LOG_ERR, "osaf_abort() called from %s:%u with info words "
		"0x%x 0x%x 0x%x 0x%x. errno=%d", i_file, (unsigned) i_line,
		(unsigned) i_info1, (unsigned) i_info2, (unsigned) i_info3,
		(unsigned) i_info4, errno);
	abort();
}
