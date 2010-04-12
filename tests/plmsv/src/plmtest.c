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

#include <sys/time.h>
#include <unistd.h>

#include "plmtest.h"
/* Highest supported version*/
#define PLM_HIGHEST_SUPPORTED_VERSION {'A', 0x01, 0x01}


const SaVersionT refVersion = PLM_HIGHEST_SUPPORTED_VERSION;
SaVersionT plmVersion = PLM_HIGHEST_SUPPORTED_VERSION;
SaAisErrorT rc;
SaPlmHandleT plmHandle;
SaPlmCallbacksT plmCallbacks = {NULL};
SaSelectionObjectT selectionObject;

int main(int argc, char **argv) 
{
    int suite = ALL_SUITES, tcase = ALL_TESTS;

    srandom(getpid());

    if (argc > 1)
    {
        suite = atoi(argv[1]);
    }

    if (argc > 2)
    {
        tcase = atoi(argv[2]);
    }

    if (suite == 0)
    {
        test_list();
        return 0;
    }

    return test_run(suite, tcase);
} 
