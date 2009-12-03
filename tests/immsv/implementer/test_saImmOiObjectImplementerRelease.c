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

static const SaNameT objectName = {
    .value = "opensafImm=opensafImm,safApp=safImmService",
    .length = sizeof("opensafImm=opensafImm,safApp=safImmService"),
};

void saImmOiObjectImplementerRelease_01(void)
{
    const SaImmOiImplementerNameT implementerName = (SaImmOiImplementerNameT) __FUNCTION__;

    safassert(saImmOiInitialize_2(&immOiHandle, &immOiCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOiImplementerSet(immOiHandle, implementerName), SA_AIS_OK);
    safassert(saImmOiObjectImplementerSet(immOiHandle, &objectName, SA_IMM_ONE), SA_AIS_OK);
    test_validate(saImmOiObjectImplementerRelease(immOiHandle, &objectName, SA_IMM_ONE), SA_AIS_OK);
    safassert(saImmOiFinalize(immOiHandle), SA_AIS_OK);
}

void saImmOiObjectImplementerRelease_02(void)
{
    const SaImmOiImplementerNameT implementerName = (SaImmOiImplementerNameT) __FUNCTION__;

    safassert(saImmOiInitialize_2(&immOiHandle, &immOiCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOiImplementerSet(immOiHandle, implementerName), SA_AIS_OK);
    safassert(saImmOiObjectImplementerSet(immOiHandle, &objectName, SA_IMM_ONE), SA_AIS_OK);
    test_validate(saImmOiObjectImplementerRelease(-1, &objectName, SA_IMM_ONE), SA_AIS_ERR_BAD_HANDLE);
    safassert(saImmOiObjectImplementerRelease(immOiHandle, &objectName, SA_IMM_ONE), SA_AIS_OK);
    safassert(saImmOiFinalize(immOiHandle), SA_AIS_OK);
}

void saImmOiObjectImplementerRelease_03(void)
{
    const SaImmOiImplementerNameT implementerName = (SaImmOiImplementerNameT) __FUNCTION__;
    SaNameT nonExistentObjectName = {
        .value = "XXX",
        .length = sizeof("XXX"),
    };

    safassert(saImmOiInitialize_2(&immOiHandle, &immOiCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOiImplementerSet(immOiHandle, implementerName), SA_AIS_OK);
    safassert(saImmOiObjectImplementerSet(immOiHandle, &objectName, SA_IMM_ONE), SA_AIS_OK);
    test_validate(saImmOiObjectImplementerRelease(immOiHandle, &nonExistentObjectName, SA_IMM_ONE), SA_AIS_ERR_NOT_EXIST);
    safassert(saImmOiObjectImplementerRelease(immOiHandle, &objectName, SA_IMM_ONE), SA_AIS_OK);
    safassert(saImmOiFinalize(immOiHandle), SA_AIS_OK);
}

void saImmOiObjectImplementerRelease_04(void)
{
    const SaImmOiImplementerNameT implementerName = (SaImmOiImplementerNameT) __FUNCTION__;

    safassert(saImmOiInitialize_2(&immOiHandle, &immOiCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOiImplementerSet(immOiHandle, implementerName), SA_AIS_OK);
    safassert(saImmOiObjectImplementerSet(immOiHandle, &objectName, SA_IMM_ONE), SA_AIS_OK);
    test_validate(saImmOiObjectImplementerRelease(immOiHandle, &objectName, -1), SA_AIS_ERR_INVALID_PARAM);
    safassert(saImmOiObjectImplementerRelease(immOiHandle, &objectName, SA_IMM_ONE), SA_AIS_OK);
    safassert(saImmOiFinalize(immOiHandle), SA_AIS_OK);
}

