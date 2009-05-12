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
#include "tet_ntf.h"
#include "test.h"

void saNtfInitialize_01(void)
{
    rc = saNtfInitialize(&ntfHandle, &ntfCallbacks, &ntfVersion);
    safassert(saNtfFinalize(ntfHandle), SA_AIS_OK);
    test_validate(rc, SA_AIS_OK);
}

void saNtfInitialize_02(void)
{
    rc = saNtfInitialize(NULL, &ntfCallbacks, &ntfVersion);
    test_validate(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saNtfInitialize_03(void)
{
    rc = saNtfInitialize(&ntfHandle, NULL, &ntfVersion);
    safassert(saNtfFinalize(ntfHandle), SA_AIS_OK);
    test_validate(rc, SA_AIS_OK);
}

void saNtfInitialize_04(void)
{
    rc = saNtfInitialize(&ntfHandle, NULL, NULL);
    test_validate(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saNtfInitialize_05(void)
{
    rc = saNtfInitialize(0, &ntfCallbacks, &ntfVersion);
    test_validate(rc, SA_AIS_ERR_INVALID_PARAM);
}

void saNtfInitialize_06(void)
{
    SaVersionT version = {0, 0, 0};
    rc = saNtfInitialize(&ntfHandle, &ntfCallbacks, &version);
    test_validate(rc, SA_AIS_ERR_VERSION);
}

void saNtfInitialize_07(void)
{
    SaVersionT version = {'B', 1, 1};

    rc = saNtfInitialize(&ntfHandle, &ntfCallbacks, &version);
    test_validate(rc, SA_AIS_ERR_VERSION);
}

void saNtfInitialize_08(void)
{
    SaVersionT version = {'A', 2, 1};

    rc = saNtfInitialize(&ntfHandle, &ntfCallbacks, &version);
    test_validate(rc, SA_AIS_OK);
    safassert(saNtfFinalize(ntfHandle), SA_AIS_OK);
}

void saNtfInitialize_09(void)
{
    SaVersionT version = {'A', 3, 0};

    rc = saNtfInitialize(&ntfHandle, &ntfCallbacks, &version);
    test_validate(rc, SA_AIS_ERR_VERSION);
}

__attribute__ ((constructor)) static void saNtfInitialize_constructor(void)
{
    test_suite_add(1, "Life Cykel API");
    test_case_add(1, saNtfInitialize_01, "saNtfInitialize SA_AIS_OK");
    test_case_add(1, saNtfInitialize_02, "saNtfInitialize with NULL pointer to handle");
    test_case_add(1, saNtfInitialize_03, "saNtfInitialize with NULL pointer to callbacks");
    test_case_add(1, saNtfInitialize_04, "saNtfInitialize with NULL callbacks AND version");
    test_case_add(1, saNtfInitialize_05, "saNtfInitialize with uninitialized handle");
    test_case_add(1, saNtfInitialize_06, "saNtfInitialize with uninitialized version");
    test_case_add(1, saNtfInitialize_07, "saNtfInitialize with too high release level");
    test_case_add(1, saNtfInitialize_08, "saNtfInitialize with minor version set to 1");
    test_case_add(1, saNtfInitialize_09, "saNtfInitialize with major version set to 3");
}

