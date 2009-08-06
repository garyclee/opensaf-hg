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

#ifndef FM_PAPI_H
#define FM_PAPI_H

#include "saAis.h"
#include <SaHpi.h>

#define FMA_UNS32_HDL_MAX 0xffffffff

/*
** Structure declarations
*/
typedef SaUint64T fmHandleT;

typedef enum {
	fmHeartbeatLost,
	fmHeartbeatRestore
} fmHeartbeatIndType;

/*************************************************/
/****** Callback functions declarations **********/
/*************************************************/
typedef void (*fmNodeResetIndCallbackT) (SaInvocationT invocation, SaHpiEntityPathT entityReset);

typedef void (*fmSysManSwitchReqCallbackT) (SaInvocationT invocation);

typedef struct {
	fmNodeResetIndCallbackT fmNodeResetIndCallback;
	fmSysManSwitchReqCallbackT fmSysManSwitchReqCallback;
} fmCallbacksT;

/*************************************************/
/********* FMA API declarations ******************/
/*************************************************/

EXTERN_C SaAisErrorT fmInitialize(fmHandleT *fmHandle, const fmCallbacksT *fmCallbacks, SaVersionT *version);

EXTERN_C SaAisErrorT fmSelectionObjectGet(fmHandleT fmHandle, SaSelectionObjectT *selectionObject);

EXTERN_C SaAisErrorT fmDispatch(fmHandleT fmHandle, SaDispatchFlagsT dispatchFlags);

EXTERN_C SaAisErrorT fmFinalize(fmHandleT fmHandle);

EXTERN_C SaAisErrorT fmCanSwitchoverProceed(fmHandleT fmHandle, SaBoolT *CanSwitchoverProceed);

EXTERN_C SaAisErrorT fmNodeResetInd(fmHandleT fmHandle, SaHpiEntityPathT entityReset);

EXTERN_C SaAisErrorT fmNodeHeartbeatInd(fmHandleT fmHandle, fmHeartbeatIndType fmHbType, SaHpiEntityPathT entityReset);

EXTERN_C SaAisErrorT fmResponse(fmHandleT fmHandle, SaInvocationT invocation, SaAisErrorT error);

#endif
