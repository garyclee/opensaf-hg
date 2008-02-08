/*

  Header file of SA Forum AIS CLM APIs (SAI-AIS-B.01.00.03)
  compiled on 26AUG2004 by sayandeb.saha@motorola.com.
*/

#ifndef _SA_CLM_H
#define _SA_CLM_H

#include "saAis.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef SaUint64T SaClmHandleT;
typedef SaUint32T SaClmNodeIdT;

#define SA_CLM_LOCAL_NODE_ID 0XFFFFFFFF

#define SA_CLM_MAX_ADDRESS_LENGTH 64

typedef enum {
         SA_CLM_AF_INET = 1,
         SA_CLM_AF_INET6 = 2
} SaClmNodeAddressFamilyT;

typedef struct {
         SaClmNodeAddressFamilyT family;
         SaUint16T length;
         SaUint8T value[SA_CLM_MAX_ADDRESS_LENGTH];
} SaClmNodeAddressT;

typedef struct {
         SaClmNodeIdT nodeId;
         SaClmNodeAddressT nodeAddress; 
         SaNameT nodeName;
         SaBoolT member; 
         SaTimeT bootTimestamp;
         SaUint64T initialViewNumber;
} SaClmClusterNodeT;

typedef enum {
         SA_CLM_NODE_NO_CHANGE = 1,
         SA_CLM_NODE_JOINED = 2,
         SA_CLM_NODE_LEFT = 3,
         SA_CLM_NODE_RECONFIGURED = 4
} SaClmClusterChangesT;

typedef struct {
         SaClmClusterNodeT clusterNode;
         SaClmClusterChangesT clusterChange;
} SaClmClusterNotificationT;

typedef struct {
         SaUint64T viewNumber;
         SaUint32T numberOfItems;
         SaClmClusterNotificationT *notification;
} SaClmClusterNotificationBufferT;

typedef void (*SaClmClusterTrackCallbackT) (
         const SaClmClusterNotificationBufferT *notificationBuffer,
         SaUint32T numberOfMembers,
         SaAisErrorT error);

typedef void (*SaClmClusterNodeGetCallbackT) (
         SaInvocationT invocation,
         const SaClmClusterNodeT *clusterNode,
         SaAisErrorT error);

typedef struct {
         SaClmClusterNodeGetCallbackT saClmClusterNodeGetCallback;
         SaClmClusterTrackCallbackT saClmClusterTrackCallback;
} SaClmCallbacksT;

    extern SaAisErrorT 
saClmInitialize(SaClmHandleT *clmHandle, const SaClmCallbacksT *clmCallbacks,
                SaVersionT *version);
    extern SaAisErrorT 
saClmSelectionObjectGet(SaClmHandleT clmHandle, 
                        SaSelectionObjectT *selectionObject);
    extern SaAisErrorT
saClmDispatch(SaClmHandleT clmHandle, 
              SaDispatchFlagsT dispatchFlags);
    extern SaAisErrorT 
saClmFinalize(SaClmHandleT clmHandle);
    extern SaAisErrorT 
saClmClusterTrack(SaClmHandleT clmHandle,
                  SaUint8T trackFlags,
                  SaClmClusterNotificationBufferT *notificationBuffer
);
    extern SaAisErrorT 
saClmClusterTrackStop(SaClmHandleT clmHandle);

    extern SaAisErrorT 
saClmClusterNodeGet(SaClmHandleT clmHandle,
                    SaClmNodeIdT nodeId, 
                    SaTimeT timeout,
                    SaClmClusterNodeT *clusterNode);
    extern SaAisErrorT
saClmClusterNodeGetAsync(SaClmHandleT clmHandle,
                         SaInvocationT invocation,
                         SaClmNodeIdT nodeId);

#ifdef  __cplusplus
}
#endif

#endif  /* _SA_CLM_H */



