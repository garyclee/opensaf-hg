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

#ifndef plmtest_h
#define plmtest_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <saPlm.h>
#include <assert.h>
#include <utest.h>
#include <util.h>

extern SaVersionT plmVersion;
extern SaAisErrorT rc;
extern SaPlmHandleT plmHandle;
extern SaPlmCallbacksT plmCallbacks;
extern SaSelectionObjectT selectionObject;

#endif
