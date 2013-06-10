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

void saLogLimitGet_01(void)
{
    SaLimitValueT limitValue;

    safassert(saLogInitialize(&logHandle, &logCallbacks, &logVersion), SA_AIS_OK);
    rc = saLogLimitGet(logHandle, SA_LOG_MAX_NUM_CLUSTER_APP_LOG_STREAMS_ID, &limitValue);
    safassert(saLogFinalize(logHandle), SA_AIS_OK);
    test_validate(rc, SA_AIS_ERR_NOT_SUPPORTED);
}

__attribute__ ((constructor)) static void saLibraryLifeCycle_constructor(void)
{
    test_suite_add(3, "Limit Fetch API");
    test_case_add(3, saLogLimitGet_01, "saLogLimitGet(), Not supported");
}
