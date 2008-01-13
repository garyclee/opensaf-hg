/*

  Header file of SA Forum AIS EVT APIs (SAI-AIS-B.01.00.04)
  compiled on 15SEP2004 by sayandeb.saha@motorola.com.
*/

#ifndef _SA_MSG_H
#define _SA_MSG_H

#include "saAis.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef SaUint64T SaMsgHandleT;
typedef SaUint64T SaMsgQueueHandleT;
typedef SaUint64T SaMsgSenderIdT;

#define SA_MSG_MESSAGE_DELIVERED_ACK 0x1
typedef SaUint32T SaMsgAckFlagsT;

#define SA_MSG_QUEUE_PERSISTENT  0x1
typedef SaUint32T SaMsgQueueCreationFlagsT;

#define SA_MSG_MESSAGE_HIGHEST_PRIORITY 0
#define SA_MSG_MESSAGE_LOWEST_PRIORITY 3

typedef struct {
    SaMsgQueueCreationFlagsT creationFlags;
    SaSizeT size[SA_MSG_MESSAGE_LOWEST_PRIORITY+1];
    SaTimeT retentionTime;
} SaMsgQueueCreationAttributesT;

#define SA_MSG_QUEUE_CREATE 0x1
#define SA_MSG_QUEUE_RECEIVE_CALLBACK 0x2
#define SA_MSG_QUEUE_EMPTY 0x4
typedef SaUint32T SaMsgQueueOpenFlagsT;

typedef struct {
    SaUint32T queueSize;
    SaSizeT queueUsed;
    SaUint32T numberOfMessages;
} SaMsgQueueUsageT;

typedef struct {
    SaMsgQueueCreationFlagsT creationFlags;
    SaTimeT retentionTime;
    SaTimeT closeTime;
    SaMsgQueueUsageT saMsgQueueUsage[SA_MSG_MESSAGE_LOWEST_PRIORITY+1];
} SaMsgQueueStatusT;

typedef enum {
   SA_MSG_QUEUE_GROUP_ROUND_ROBIN = 1,
   SA_MSG_QUEUE_GROUP_LOCAL_ROUND_ROBIN = 2,
   SA_MSG_QUEUE_GROUP_LOCAL_BEST_QUEUE = 3,
   SA_MSG_QUEUE_GROUP_BROADCAST = 4
} SaMsgQueueGroupPolicyT;

typedef enum {
    SA_MSG_QUEUE_GROUP_NO_CHANGE = 1,
    SA_MSG_QUEUE_GROUP_ADDED = 2,
    SA_MSG_QUEUE_GROUP_REMOVED = 3,
    SA_MSG_QUEUE_GROUP_STATE_CHANGED = 4
} SaMsgQueueGroupChangesT;

typedef struct {
    SaNameT queueName;
} SaMsgQueueGroupMemberT;

typedef struct {
   SaMsgQueueGroupMemberT member;
   SaMsgQueueGroupChangesT change;
} SaMsgQueueGroupNotificationT;

typedef struct {
   SaUint32T numberOfItems;
   SaMsgQueueGroupNotificationT *notification;
   SaMsgQueueGroupPolicyT queueGroupPolicy;
} SaMsgQueueGroupNotificationBufferT;

typedef struct {
   SaUint32T type;
   SaUint32T version;
   SaSizeT size;
   SaNameT *senderName;
   void *data;
   SaUint8T priority;
} SaMsgMessageT;

typedef void
(*SaMsgQueueOpenCallbackT)(SaInvocationT invocation,
                           SaMsgQueueHandleT queueHandle,
                           SaAisErrorT error);

typedef void
(*SaMsgQueueGroupTrackCallbackT) (
                           const SaNameT *queueGroupName,
                           const SaMsgQueueGroupNotificationBufferT *notificationBuffer,
                           SaUint32T numberOfMembers,
                           SaAisErrorT error
);

typedef void 
(*SaMsgMessageDeliveredCallbackT)(
                          SaInvocationT invocation,
                          SaAisErrorT error
);

typedef void 
(*SaMsgMessageReceivedCallbackT)(
                          SaMsgQueueHandleT queueHandle
);

typedef struct {
    SaMsgQueueOpenCallbackT saMsgQueueOpenCallback;
    SaMsgQueueGroupTrackCallbackT saMsgQueueGroupTrackCallback;
    SaMsgMessageDeliveredCallbackT saMsgMessageDeliveredCallback;
    SaMsgMessageReceivedCallbackT saMsgMessageReceivedCallback; 
} SaMsgCallbacksT;

    extern SaAisErrorT 
saMsgInitialize(SaMsgHandleT *msgHandle, 
                const SaMsgCallbacksT *msgCallbacks,
                SaVersionT *version);
    extern SaAisErrorT 
saMsgSelectionObjectGet(SaMsgHandleT msgHandle,
                        SaSelectionObjectT *selectionObject);
    extern SaAisErrorT 
saMsgDispatch(SaMsgHandleT msgHandle, SaDispatchFlagsT dispatchFlags);
    extern SaAisErrorT 
saMsgFinalize(SaMsgHandleT msgHandle);
    extern SaAisErrorT 
saMsgQueueOpen(SaMsgHandleT msgHandle,
               const SaNameT *queueName,
               const SaMsgQueueCreationAttributesT *creationAttributes,
               SaMsgQueueOpenFlagsT openFlags,
               SaTimeT timeout,
               SaMsgQueueHandleT *queueHandle);
    extern SaAisErrorT 
saMsgQueueOpenAsync(SaMsgHandleT msgHandle,
                    SaInvocationT invocation,
                    const SaNameT *queueName,
                    const SaMsgQueueCreationAttributesT *creationAttributes,
                    SaMsgQueueOpenFlagsT openFlags);
    extern SaAisErrorT 
saMsgQueueClose(SaMsgQueueHandleT queueHandle);
    extern SaAisErrorT 
saMsgQueueStatusGet(SaMsgHandleT msgHandle, 
                    const SaNameT *queueName, 
                    SaMsgQueueStatusT *queueStatus);
    extern SaAisErrorT 
saMsgQueueUnlink(SaMsgHandleT msgHandle, 
                 const SaNameT *queueName);
    extern SaAisErrorT
saMsgQueueGroupCreate(SaMsgHandleT msgHandle, 
                      const SaNameT *queueGroupName,
                      SaMsgQueueGroupPolicyT queueGroupPolicy);
    extern SaAisErrorT 
saMsgQueueGroupDelete(SaMsgHandleT msgHandle, 
                      const SaNameT *queueGroupName);
    extern SaAisErrorT 
saMsgQueueGroupInsert(SaMsgHandleT msgHandle, 
                      const SaNameT *queueGroupName,
                      const SaNameT *queueName);
    extern SaAisErrorT 
saMsgQueueGroupRemove(SaMsgHandleT msgHandle, 
                      const SaNameT *queueGroupName, 
                      const SaNameT *queueName);
    extern SaAisErrorT 
saMsgQueueGroupTrack(SaMsgHandleT msgHandle,
                     const SaNameT *queueGroupName,
                     SaUint8T trackFlags,
                     SaMsgQueueGroupNotificationBufferT *notificationBuffer);
    extern SaAisErrorT 
saMsgQueueGroupTrackStop(SaMsgHandleT msgHandle,
                         const SaNameT *queueGroupName);
    extern SaAisErrorT 
saMsgMessageSend(SaMsgHandleT msgHandle,
                 const SaNameT *destination,
                 const SaMsgMessageT *message,
                 SaTimeT timeout);
    extern SaAisErrorT 
saMsgMessageSendAsync(SaMsgHandleT msgHandle,
                      SaInvocationT invocation,
                      const SaNameT *destination,
                      const SaMsgMessageT *message,
                      SaMsgAckFlagsT ackFlags);
    extern SaAisErrorT 
saMsgMessageGet(SaMsgQueueHandleT queueHandle,
                SaMsgMessageT *message,
                SaTimeT *sendTime,
                SaMsgSenderIdT *senderId,
                SaTimeT timeout);
    extern SaAisErrorT 
saMsgMessageCancel(SaMsgQueueHandleT queueHandle);
    extern SaAisErrorT 
saMsgMessageSendReceive(
                SaMsgHandleT msgHandle,
                const SaNameT *destination,
                const SaMsgMessageT *sendMessage,
                SaMsgMessageT *receiveMessage,
                SaTimeT *replySendTime,
                SaTimeT timeout);
    extern SaAisErrorT 
saMsgMessageReply(
                SaMsgHandleT msgHandle,
                const SaMsgMessageT *replyMessage,
                const SaMsgSenderIdT *senderId,
                SaTimeT timeout);
    extern SaAisErrorT 
saMsgMessageReplyAsync(
                SaMsgHandleT msgHandle,
                SaInvocationT invocation,
                const SaMsgMessageT *replyMessage,
                const SaMsgSenderIdT *senderId,
                SaMsgAckFlagsT ackFlags);

#ifdef  __cplusplus
}

#endif

#endif  /* _SA_MSG_H */



