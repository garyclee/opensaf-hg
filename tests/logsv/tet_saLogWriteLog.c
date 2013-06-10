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
 * Author(s): Ericsson AB
 *
 */

#include "logtest.h"

void saLogWriteLog_01(void)
{
    safassert(saLogInitialize(&logHandle, &logCallbacks, &logVersion), SA_AIS_OK);
    safassert(saLogStreamOpen_2(logHandle, &systemStreamName, NULL, 0,
                           SA_TIME_ONE_SECOND, &logStreamHandle), SA_AIS_OK);
    rc = saLogWriteLog(logStreamHandle, SA_TIME_ONE_SECOND, &genLogRecord);
    safassert(saLogFinalize(logHandle), SA_AIS_OK);
    test_validate(rc, SA_AIS_ERR_NOT_SUPPORTED);
}

