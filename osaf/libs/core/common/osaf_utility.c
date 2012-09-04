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

void osaf_abort(long i_cause)
{
	syslog(LOG_ERR, "osaf_abort(%ld) called from %p with errno=%d",
		i_cause, __builtin_return_address(0), errno);
	abort();
}
