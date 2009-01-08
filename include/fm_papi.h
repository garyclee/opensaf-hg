/*****************************************************************************
 *                                                                           *
 * NOTICE TO PROGRAMMERS:  RESTRICTED USE.                                   *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED TO YOU AND YOUR COMPANY UNDER A LICENSE         *
 * AGREEMENT FROM EMERSON, INC.                                              *
 *                                                                           *
 * THE TERMS OF THE LICENSE AGREEMENT RESTRICT THE USE OF THIS SOFTWARE TO   *
 * SPECIFIC PROJECTS ("LICENSEE APPLICATIONS") IDENTIFIED IN THE AGREEMENT.  *
 *                                                                           *
 * IF YOU ARE UNSURE WHETHER YOU ARE AUTHORIZED TO USE THIS SOFTWARE ON YOUR *
 * PROJECT,  PLEASE CHECK WITH YOUR MANAGEMENT.                              *
 *                                                                           *
 *****************************************************************************
 
 
..............................................................................

DESCRIPTION:

******************************************************************************
*/

 
 
 
#ifndef FM_PAPI_H
#define FM_PAPI_H

/*
** Includes
*/
#include "saAis.h"
#include "SaHpi.h"


#define FMA_UNS32_HDL_MAX 0xffffffff

/*
** Structure declarations
*/
typedef SaUint64T fmHandleT;

typedef enum 
{
    fmHeartbeatLost,
    fmHeartbeatRestore
} fmHeartbeatIndType; 

/*
** Callback functions declarations
*/
typedef void (*fmNodeResetIndCallbackT )(SaInvocationT invocation, SaHpiEntityPathT entityReset);

typedef void (*fmSysManSwitchReqCallbackT )(SaInvocationT invocation);


typedef struct {
    fmNodeResetIndCallbackT    fmNodeResetIndCallback;
    fmSysManSwitchReqCallbackT fmSysManSwitchReqCallback;
} fmCallbacksT;

/*
** API Declarations
*/
SaAisErrorT fmInitialize(fmHandleT *fmHandle, const fmCallbacksT *fmCallbacks, SaVersionT *version);

SaAisErrorT fmSelectionObjectGet(fmHandleT fmHandle, SaSelectionObjectT *selectionObject);

SaAisErrorT fmDispatch(fmHandleT fmHandle, SaDispatchFlagsT dispatchFlags);

SaAisErrorT fmFinalize(fmHandleT fmHandle);

SaAisErrorT fmCanSwitchoverProceed(fmHandleT fmHandle, SaBoolT *CanSwitchoverProceed);

SaAisErrorT fmNodeResetInd(fmHandleT fmHandle, SaHpiEntityPathT entityReset);

SaAisErrorT fmNodeHeartbeatInd(fmHandleT fmHandle,fmHeartbeatIndType fmHbType, SaHpiEntityPathT entityReset);

SaAisErrorT fmResponse(fmHandleT fmHandle, SaInvocationT invocation, SaAisErrorT error);

EXTERN_C uns32 fma_lib_req (NCS_LIB_REQ_INFO *);
#endif
