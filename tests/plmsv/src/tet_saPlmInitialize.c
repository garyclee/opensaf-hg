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

#include "plmtest.h"


void saPlmInitialize_01(void) 
{
        rc = saPlmInitialize(&plmHandle, &plmCallbacks, &plmVersion);
        safassert(saPlmFinalize(plmHandle), SA_AIS_OK);
        test_validate(rc, SA_AIS_OK);
}

__attribute__ ((constructor)) static void saPlmInitialize_constructor(void)
{
        test_suite_add(1, "Life Cykel API");
        test_case_add(1, saPlmInitialize_01, "saClmInitialize with A.01.01 SA_AIS_OK");
}

