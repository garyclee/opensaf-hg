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

#include "immtest.h"

static SaImmClassNameT className = (SaImmClassNameT) "OpensafImm";
/* 
static const SaNameT objectName = {
    .value = "safAmfNode=Node01,safAmfCluster=1",
    .length = sizeof("safAmfNode=Node01,safAmfCluster=1"),
};
*/

void saImmOiClassImplementerRelease_01(void)
{
    const SaImmOiImplementerNameT implementerName = (SaImmOiImplementerNameT) __FUNCTION__;

    safassert(saImmOiInitialize_2(&immOiHandle, &immOiCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOiImplementerSet(immOiHandle, implementerName), SA_AIS_OK);
    safassert(saImmOiClassImplementerSet(immOiHandle, className), SA_AIS_OK);
    rc = saImmOiClassImplementerRelease(immOiHandle, className);
    test_validate(rc, SA_AIS_OK);
    safassert(saImmOiFinalize(immOiHandle), SA_AIS_OK);
}

void saImmOiClassImplementerRelease_02(void)
{
    const SaImmOiImplementerNameT implementerName = (SaImmOiImplementerNameT) __FUNCTION__;

    safassert(saImmOiInitialize_2(&immOiHandle, &immOiCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOiImplementerSet(immOiHandle, implementerName), SA_AIS_OK);
    safassert(saImmOiClassImplementerSet(immOiHandle, className), SA_AIS_OK);
    rc = saImmOiClassImplementerRelease(-1, className);
    safassert(saImmOiClassImplementerRelease(immOiHandle, className), SA_AIS_OK);
    safassert(saImmOiFinalize(immOiHandle), SA_AIS_OK);
    test_validate(rc, SA_AIS_ERR_BAD_HANDLE);
}

void saImmOiClassImplementerRelease_03(void)
{
    const SaImmOiImplementerNameT implementerName = (SaImmOiImplementerNameT) __FUNCTION__;
    SaImmClassNameT nonExistingClassName = (SaImmClassNameT) "nonExistingClassName";

    safassert(saImmOiInitialize_2(&immOiHandle, &immOiCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOiImplementerSet(immOiHandle, implementerName), SA_AIS_OK);
    safassert(saImmOiClassImplementerSet(immOiHandle, className), SA_AIS_OK);
    rc = saImmOiClassImplementerRelease(immOiHandle, nonExistingClassName);
    safassert(saImmOiClassImplementerRelease(immOiHandle, className), SA_AIS_OK);
    safassert(saImmOiFinalize(immOiHandle), SA_AIS_OK);
    test_validate(rc, SA_AIS_ERR_NOT_EXIST);
}

