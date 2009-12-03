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

static const SaImmClassNameT configClassName = "OpensafImm";

static const SaNameT objectName = {
    .value = "opensafImm=opensafImm,safApp=safImmService",
    .length = sizeof("opensafImm=opensafImm,safApp=safImmService"),
};

void saImmOiObjectImplementerSet_01(void)
{
    const SaImmOiImplementerNameT implementerName = (SaImmOiImplementerNameT) __FUNCTION__;

    safassert(saImmOiInitialize_2(&immOiHandle, &immOiCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOiImplementerSet(immOiHandle, implementerName), SA_AIS_OK);
    rc = saImmOiObjectImplementerSet(immOiHandle, &objectName, SA_IMM_ONE);
    safassert(saImmOiObjectImplementerRelease(immOiHandle, &objectName, SA_IMM_ONE), SA_AIS_OK);
    safassert(saImmOiFinalize(immOiHandle), SA_AIS_OK);
    test_validate(rc, SA_AIS_OK);
}

void saImmOiObjectImplementerSet_02(void)
{
    const SaImmOiImplementerNameT implementerName = (SaImmOiImplementerNameT) __FUNCTION__;

    safassert(saImmOiInitialize_2(&immOiHandle, &immOiCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOiImplementerSet(immOiHandle, implementerName), SA_AIS_OK);
    rc = saImmOiObjectImplementerSet(-1, &objectName, SA_IMM_ONE);
    test_validate(rc, SA_AIS_ERR_BAD_HANDLE);
    safassert(saImmOiFinalize(immOiHandle), SA_AIS_OK);
}

void saImmOiObjectImplementerSet_03(void)
{
    safassert(saImmOiInitialize_2(&immOiHandle, &immOiCallbacks, &immVersion), SA_AIS_OK);
    rc = saImmOiObjectImplementerSet(immOiHandle, &objectName, SA_IMM_ONE);
    test_validate(rc, SA_AIS_ERR_BAD_HANDLE);
    safassert(saImmOiFinalize(immOiHandle), SA_AIS_OK);
}

void saImmOiObjectImplementerSet_04(void)
{
    const SaImmOiImplementerNameT implementerName = (SaImmOiImplementerNameT) __FUNCTION__;
    SaNameT nonExistentObjectName = {
        .value = "XXX",
        .length = sizeof("XXX"),
    };

    safassert(saImmOiInitialize_2(&immOiHandle, &immOiCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOiImplementerSet(immOiHandle, implementerName), SA_AIS_OK);
    rc = saImmOiObjectImplementerSet(immOiHandle, &nonExistentObjectName, SA_IMM_ONE);
    test_validate(rc, SA_AIS_ERR_NOT_EXIST);
    safassert(saImmOiFinalize(immOiHandle), SA_AIS_OK);
}

void saImmOiObjectImplementerSet_05(void)
{
    SaImmOiHandleT newhandle;
    const SaImmOiImplementerNameT implementerName = (SaImmOiImplementerNameT) __FUNCTION__;
    SaImmOiImplementerNameT implementerName2 = (SaImmOiImplementerNameT) __FILE__;

    safassert(saImmOiInitialize_2(&immOiHandle, &immOiCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOiImplementerSet(immOiHandle, implementerName), SA_AIS_OK);
    safassert(saImmOiObjectImplementerSet(immOiHandle, &objectName, SA_IMM_ONE), SA_AIS_OK);

    safassert(saImmOiInitialize_2(&newhandle, &immOiCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOiImplementerSet(newhandle, implementerName2), SA_AIS_OK);
    rc = saImmOiObjectImplementerSet(newhandle, &objectName, SA_IMM_ONE);

    safassert(saImmOiObjectImplementerRelease(immOiHandle, &objectName, SA_IMM_ONE), SA_AIS_OK);
    safassert(saImmOiFinalize(immOiHandle), SA_AIS_OK);
    safassert(saImmOiFinalize(newhandle), SA_AIS_OK);
    test_validate(rc, SA_AIS_ERR_EXIST);
}

void saImmOiObjectImplementerSet_06(void)
{
    SaImmOiHandleT newhandle;
    const SaImmOiImplementerNameT implementerName = (SaImmOiImplementerNameT) __FUNCTION__;
    SaImmOiImplementerNameT implementerName2 = (SaImmOiImplementerNameT) __FILE__;

    safassert(saImmOiInitialize_2(&immOiHandle, &immOiCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOiImplementerSet(immOiHandle, implementerName), SA_AIS_OK);
    safassert(saImmOiClassImplementerSet(immOiHandle, configClassName), SA_AIS_OK);

    safassert(saImmOiInitialize_2(&newhandle, &immOiCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOiImplementerSet(newhandle, implementerName2), SA_AIS_OK);
    rc = saImmOiObjectImplementerSet(newhandle, &objectName, SA_IMM_ONE);

    safassert(saImmOiClassImplementerRelease(immOiHandle, configClassName), SA_AIS_OK);
    safassert(saImmOiImplementerClear(newhandle), SA_AIS_OK);
    safassert(saImmOiImplementerClear(immOiHandle), SA_AIS_OK);
    safassert(saImmOiFinalize(immOiHandle), SA_AIS_OK);
    safassert(saImmOiFinalize(newhandle), SA_AIS_OK);
    test_validate(rc, SA_AIS_ERR_EXIST);
}

