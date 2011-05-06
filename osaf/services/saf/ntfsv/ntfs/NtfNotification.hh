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
/**
 *   This object handles information about NTF notifications.
 */

#ifndef NTFNOTIFICATION_H
#define NTFNOTIFICATION_H

#include <list>
#include <deque>
#include <iterator>
#include <map>
#include <queue>
#include "ntfs_com.h"
#include "ntfsv_msg.h"
#include <tr1/memory>

#define NACK_THRESHOLD 4

typedef struct
{
    unsigned int clientId;
    SaNtfSubscriptionIdT subscriptionId;
} UniqueSubscriptionId;

class NtfNotification{

public:

    NtfNotification(SaNtfIdentifierT notificationId,
                    SaNtfNotificationTypeT notificationType,
                    ntfsv_send_not_req_t* sendNotInfo);

    NtfNotification(const NtfNotification&);
    NtfNotification();
    virtual ~NtfNotification();
    SaNtfNotificationTypeT getNotificationType() const;
    SaNtfIdentifierT getNotificationId() const;
    void storeMatchingSubscription(unsigned int clientId, SaNtfSubscriptionIdT subscriptionId);
    void notificationSentConfirmed(unsigned int clientId, SaNtfSubscriptionIdT subscriptionId);
    void notificationLoggedConfirmed();
    bool loggedOk() const;
    bool isSubscriptionListEmpty() const;
    void removeSubscription(unsigned int clientId);
    void removeSubscription(unsigned int clientId,
                            SaNtfSubscriptionIdT subscriptionId);
    ntfsv_send_not_req_t* getNotInfo();
    void syncRequest(NCS_UBAID *uba);
    void resetSubscriptionIdList();
    SaAisErrorT getNextSubscription(UniqueSubscriptionId& subId);
    void printInfo();
	 SaNtfNotificationHeaderT* header();
    ntfsv_send_not_req_t *sendNotInfo_;
    bool loggFromCallback_;

private:
    void setData(SaNtfIdentifierT notificationId,
                 SaNtfNotificationTypeT notificationType,
                 const ntfsv_send_not_req_t* sendNotInfo);
    bool logged;
    SaNtfIdentifierT notificationId_;
    SaNtfNotificationTypeT notificationType_;
    typedef std::list<UniqueSubscriptionId> SubscriptionList;
    SubscriptionList subscriptionList;
    SubscriptionList::iterator idListPos;
};

typedef std::tr1::shared_ptr<NtfNotification> NtfSmartPtr;

#endif // NTFNOTIFICATION_H

