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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <utest.h>
#include <util.h>
#include <string.h>
#include <saPlm.h>
#include <saAis.h>
#include <saNtf.h>

/* Common globals */
extern SaVersionT PlmVersion;
extern SaAisErrorT rc;
extern SaSelectionObjectT selectionObject;

/* Management globals */
extern  SaPlmHandleT plmHandle;
extern SaPlmCallbacksT plmCallbacks;
extern SaPlmEntityGroupHandleT entityGroupHandle;

extern SaNameT f120_slot_1_dn,f120_slot_16_dn,amc_slot_1_dn,amc_slot_16_dn,f120_slot_1_eedn,f120_nonexistent,f120_slot_16_eedn,amc_slot_1_eedn,amc_slot_16_eedn;
extern entityNamesNumber;



extern void TrackCallbackT (
	    SaPlmEntityGroupHandleT entityGroupHandle,
	    SaUint64T trackCookie,
	    SaInvocationT invocation,
	    SaPlmTrackCauseT cause,
	    const SaNameT *rootCauseEntity,
	    SaNtfIdentifierT rootCorrelationId,
	    const SaPlmReadinessTrackedEntitiesT *trackedEntities,
	    SaPlmChangeStepT step,
	    SaAisErrorT error);
