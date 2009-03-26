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
 *   This object handles information about NTF clients.
 *
 */

#include "NtfClient.hh"
#include "logtrace.h"
#include "NtfAdmin.hh"

/**
 * This is the constructor.
 *
 * Client  object is created, initial variables are set.
 *
 * @param clientId   Node-wide unique id of this client.
 * @param mds_dest   MDS communication pointer to this client.
 *                  
 * @param locatedOnThisNode
 *                   Flag that is set if the client is located on this node.
 */
NtfClient::NtfClient(unsigned int clientId,
                     MDS_DEST mds_dest):readerId_(0),mdsDest_(mds_dest)
{
    clientId_ = clientId;
    mdsDest_ = mds_dest;
    TRACE_3("NtfClient::NtfClient NtfClient %u created mdest: %llu",
            clientId_, mdsDest_);
}

/**
 * This is the destructor.
 *
 * It is called when a client is removed, i.e. a client finalized its
 * connection.
 *
 * Subscription objects belonging to this client are deleted.
 */
NtfClient::~NtfClient()
{
    // delete all subscriptions
    SubscriptionMap::iterator pos;
    for (pos = subscriptionMap.begin(); pos != subscriptionMap.end(); pos++)
    {
        NtfSubscription* subscription = pos->second;
        delete subscription;
    }
    // delete all readers
    ReaderMapT::iterator rpos;
    for (rpos = readerMap.begin(); rpos != readerMap.end(); rpos++)
    {
        unsigned int readerId = 0;
        NtfReader* reader = rpos->second;
        if (reader != NULL)
        {
            readerId =  reader->getId();
            TRACE_3("~Client delete reader Id %u ", readerId);
            delete reader;
        }
    }
    TRACE_3("NtfClient::~NtfClient NtfClient %u destroyed, mdest %llu",
            clientId_, mdsDest_);
}

/**
 * This method is called to get the id of this client.
 *
 * @return Node-wide unique id of this client.
 */
unsigned int NtfClient::getClientId() const
{
    return clientId_;
}

MDS_DEST NtfClient::getMdsDest() const
{
    return mdsDest_;
}

/**
 * This method is called when the client made a new subscription.
 *
 * The pointer to the subscription object is stored if it did not
 * exist. If the client is located on this node, a confirmation
 * for the subscription is sent.
 *
 * @param subscription
 *               Pointer to the subscription object.
 */
void NtfClient::subscriptionAdded(NtfSubscription* subscription,
                                  MDS_SYNC_SND_CTXT *mdsCtxt)
{

    // check if subscription already exists
    SubscriptionMap::iterator pos;
    pos = subscriptionMap.find(subscription->getSubscriptionId());
    if (pos != subscriptionMap.end())
    {
        // subscription found
        TRACE_3("NtfClient::subscriptionAdded subscription %u already exists"
                ", client %u",
                subscription->getSubscriptionId(), clientId_);
        delete subscription;
    }
    else
    {
        // store new subscription in subscriptionMap
        subscriptionMap[subscription->getSubscriptionId()] = subscription;
        TRACE_3("NtfClient::subscriptionAdded subscription %u added,"
                " client %u, subscriptionMap size is %d",
                subscription->getSubscriptionId(),
                clientId_,
                subscriptionMap.size());

        if (activeController())
        {
            sendSubscriptionUpdate(clientId_,
                                   subscription->getSubscriptionId());
            confirmNtfSubscribe(subscription->getSubscriptionId(),
                                mdsCtxt);
        }
    }
}

/**
 * This method is called when the client received a notification.
 *
 * If the notification is send from this client, a confirmation 
 * for the notification is sent. 
 *
 * The client scans through its subscriptions and if it finds a
 * matching one, it stores the id of the matching subscription in
 * the notification object.
 *
 * @param clientId Node-wide unique id of the client who sent the notification.
 * @param notification
 *                 Pointer to the notification object.
 */
void NtfClient::notificationReceived(unsigned int clientId,
                                     NtfNotification* notification,
                                     MDS_SYNC_SND_CTXT *mdsCtxt)
{
    TRACE_ENTER();
    // send acknowledgement
    if (clientId_ == clientId)
    {
        // this is the client who sent the notification
        if (activeController())
        {

            confirmNtfNotification(notification->getNotificationId(),
                                   mdsCtxt, mdsDest_);
            if (notification->loggedOk())
            {
                sendLoggedConfirmUpdate(notification->getNotificationId());
            }
            else
            {
                notification->loggFromCallback_= true;
            }                
        }
    }

    // scan through all subscriptions
    SubscriptionMap::iterator pos;

    for (pos = subscriptionMap.begin(); pos != subscriptionMap.end(); pos++)
    {
        NtfSubscription* subscription = pos->second;

        if (subscription->checkSubscription(notification))
        {
            // notification matches the subscription
            TRACE_2("NtfClient::notificationReceived notification %llu matches"
                    " subscription %d, client %u",
                    notification->getNotificationId(),
                    subscription->getSubscriptionId(),
                    clientId_);
            // first store subscription data in notifiaction object for
            //  tracking purposes
            notification->storeMatchingSubscription(clientId_,
                                                    subscription->getSubscriptionId());
            // if active, send out the notification
            if (activeController())
            {
                // store the matching subscriptionId in the notification
                notification->getNotInfo()->subscriptionId
                = subscription->getSubscriptionId();
                sendNotification(notification);
            }
        }
        else
        {
            TRACE_2("NtfClient::notificationReceived notification %llu does not"
                    " match subscription %u, client %u",
                    notification->getNotificationId(),
                    subscription->getSubscriptionId(),
                    clientId_);
        }
    }
    TRACE_LEAVE();
}

/**
 * This method is called if the client made an unsubscribe.
 *
 * The subscription object is deleted. If the client is located on
 * this node, a confirmation is sent.
 *
 * @param subscriptionId
 *               Client-wide unique id of the subscription that was removed.
 */
void NtfClient::subscriptionRemoved(SaNtfSubscriptionIdT subscriptionId,
                                    MDS_SYNC_SND_CTXT *mdsCtxt)
{

    // find subscription
    SubscriptionMap::iterator pos;
    pos = subscriptionMap.find(subscriptionId);
    if (pos != subscriptionMap.end())
    {
        // subscription found
        NtfSubscription* subscription = pos->second;
        delete subscription;
        // remove subscription from subscription map
        subscriptionMap.erase(pos);
    }
    else
    {
        LOG_ER( "NtfClient::subscriptionRemoved subscription"
                " %u not found", subscriptionId);
    }
    if (activeController())
    {
        // client is located on this node
        sendUnsubscribeUpdate(clientId_,
                              subscriptionId);
        confirmNtfUnsubscribe(subscriptionId, mdsCtxt);
    }
}

/**
 * This method is called when information about this client is
 * requested by another node.
 *
 * The client scans through its subscriptions and sends them out one by one.
 */
void NtfClient::syncRequest(NCS_UBAID *uba)
{
    // scan through all subscriptions
    sendNoOfSubscriptions(subscriptionMap.size(), uba);
    SubscriptionMap::iterator pos;
    for (pos = subscriptionMap.begin(); pos != subscriptionMap.end(); pos++)
    {
        NtfSubscription* subscription = pos->second;
        TRACE_3("NtfClient::syncRequest sending info about subscription %u for "
                "client %u", subscription->getSubscriptionId(), clientId_);
        int retval = sendNewSubscription(clientId_,
                                         subscription->getSubscriptionId(),
                                         uba);
        if (retval != 1)
        {
            LOG_ER(
                  "NtfClient::syncRequest sendNewSubscription was not sent"
                  ", error code is %d", retval);
        }
    }
}

/**
 * This method is called when a notification should be sent to the client.
 *
 * If there are no notifications in the discarded notification list, then
 * we try to send it out. If it does not succeed, we put the newly
 * received notification in the list.
 *
 * If there are notifications in the discarded notification list, then
 * we try to send them out first. If it succeeds, we try to send the newly
 * received notifiaction. If it does not succeed, we put the newly
 * received notification to the end of the list.
 *
 * @param notification
 *               Pointer to the notification object.
 */
void NtfClient::sendNotification(NtfNotification* notification)
{
    TRACE_ENTER();
    // check if there are discarded notifications
    if (discardedNotificationIdList.empty())
    {
        // send notification
        TRACE_3("NtfClient::sendNotification send_notification_lib called,"
                " client %u, notification %llu",
                clientId_, notification->getNotificationId());
        if (send_notification_lib(
                                 notification->getNotInfo(),
                                 clientId_,
                                 &mdsDest_) != NCSCC_RC_SUCCESS)
        {
            // send failed, put notification id in discard list
            discardedNotificationIdList.push_back(
                                                 notification->getNotificationId());
            TRACE_3("NtfClient::sendNotification send_notification_lib"
                    " failed, new discardedNotificationIdList size is %d",
                    discardedNotificationIdList.size());
        }
    }
    else
    {
     // there are already discarded notifications in the queue, send them first
        struct DiscardedInfo discardedInfo;
        discardedInfo.subscriptionId =
        notification->getNotInfo()->subscriptionId;
        discardedInfo.notificationType =
        (SaNtfNotificationTypeT)notification->getNotificationType();
        discardedInfo.numberDiscarded = discardedNotificationIdList.size();
        DiscardedNotificationIdList::iterator pos;
        int i=0;
        pos = discardedNotificationIdList.begin();
        while ((i < MAX_DISCARDED_NOTIFICATIONS) &&
               (pos != discardedNotificationIdList.end()))
        {
            discardedInfo.discardedNotificationIdentifiers[i] = *pos;
            i++;
            pos++;
        }
        // first try to send discarded notifications
        TRACE_3("NtfClient::sendNotification send_discard_notification_lib"
                " called");
        if (send_discard_notification_lib( &discardedInfo) == 0)
        {
            // sending discarded notifications was successful, empty list
            discardedNotificationIdList.clear();
            TRACE_3("NtfClient::sendNotification "
                    "send_discard_notification_lib succeeded, "
                    "new discardedNotificationIdList size is %d",
                    discardedNotificationIdList.size());
            // try to send the new notification
            TRACE_3("NtfClient::sendNotification send_notification_lib"
                    " called, client %u, notification %llu",
                    clientId_, notification->getNotificationId());
            if (send_notification_lib(notification->getNotInfo(),
                                      clientId_,
                                      &mdsDest_) != NCSCC_RC_SUCCESS)
            {
                // sending new notification failed,
                //  put notification in discard list
                if (discardedNotificationIdList.size() <
                    MAX_DISCARDED_NOTIFICATIONS)
                {
                    discardedNotificationIdList.push_back(
                                                         notification->getNotificationId());
                    TRACE_2("NtfClient::sendNotification "
                            "send_notification_lib failed, "
                            "new discardedNotificationIdList size is %d",
                            discardedNotificationIdList.size());
                }
                else
                {
                    TRACE_2("NtfClient::sendNotification "
                            "send_notification_lib failed, "
                            "discardedNotificationIdList full, "
                            "size is %d", discardedNotificationIdList.size());
                }
            }
        }
        else
        {
            // sending discarded notification failed,
            // add new notification to the existing list,
            //  we will try to send it later
            if (discardedNotificationIdList.size() <
                MAX_DISCARDED_NOTIFICATIONS)
            {
                discardedNotificationIdList.
                push_back(notification->getNotificationId());
                TRACE_2("NtfClient::sendNotification "
                        "send_discard_notification_lib failed, "
                        "new discardedNotificationIdList size is %d",
                        discardedNotificationIdList.size());
            }
            else
            {
                TRACE_2("NtfClient::sendNotification "
                        "send_discard_notification_lib failed, "
                        "discardedNotificationIdList full, size is %d",
                        discardedNotificationIdList.size());
            }
        }
    }
    TRACE_LEAVE();
}

void NtfClient::sendNotConfirmedNotification(NtfNotification* notification)
{
    TRACE_ENTER();
    // if active, send out the notification
    if (activeController())
    {
        sendNotification(notification);
    }
    TRACE_LEAVE();
}

/**
 * This method is called when a confirmation for the subscription
 * should be sent to a client.
 *
 * @param subscriptionId Client-wide unique id of the subscription that should
 *                       be confirmed.
 */
void NtfClient::confirmNtfSubscribe(SaNtfSubscriptionIdT subscriptionId,
                                    MDS_SYNC_SND_CTXT *mds_ctxt)
{

    TRACE_2("NtfClient::confirmNtfSubscribe subscribe_res_lib called, "
            "client %u, subscription %u", clientId_, subscriptionId);
    subscribe_res_lib( SA_AIS_OK, subscriptionId, mdsDest_, mds_ctxt);
}

/**
 * This method is called when a confirmation for the unsubscription
 * should be sent to a client.
 *
 * @param subscriptionId Client-wide unique id of the subscription that shoul be
 *                       confirmed.
 */
void NtfClient::confirmNtfUnsubscribe(SaNtfSubscriptionIdT subscriptionId,
                                      MDS_SYNC_SND_CTXT *mdsCtxt)
{

    TRACE_2("NtfClient::confirmNtfUnsubscribe unsubscribe_res_lib called,"
            " client %u, subscription %u", clientId_, subscriptionId);
    unsubscribe_res_lib(SA_AIS_OK, subscriptionId, mdsDest_, mdsCtxt);
}

/**
 * This method is called when a confirmation for the notification
 * should be sent to a client.
 *
 * @param notificationId
 *               Cluster-wide unique id of the notification that should be confirmed.
 */
void NtfClient::confirmNtfNotification(SaNtfIdentifierT notificationId,
                                       MDS_SYNC_SND_CTXT *mdsCtxt,
                                       MDS_DEST mdsDest)
{

    notfication_result_lib( SA_AIS_OK, notificationId, mdsCtxt, mdsDest);
}

void NtfClient::newReaderResponse(SaAisErrorT* error,
                                  unsigned int readerId,
                                  MDS_SYNC_SND_CTXT *mdsCtxt)
{
    new_reader_res_lib( *error, readerId, mdsDest_, mdsCtxt);
}

void NtfClient::readNextResponse(SaAisErrorT* error,
                                 NtfNotification& notification,
                                 MDS_SYNC_SND_CTXT *mdsCtxt)
{
    read_next_res_lib(*error, notification.sendNotInfo_, mdsDest_, mdsCtxt);
}

void NtfClient::deleteReaderResponse(SaAisErrorT* error,
                                     MDS_SYNC_SND_CTXT *mdsCtxt)
{
    delete_reader_res_lib( *error, mdsDest_, mdsCtxt);
}

void NtfClient::newReader(MDS_SYNC_SND_CTXT *mdsCtxt)
{
    SaAisErrorT error = SA_AIS_OK;
    readerId_++;
    NtfReader* reader = new NtfReader(NtfAdmin::theNtfAdmin->logger, readerId_);
    readerMap[readerId_] = reader;
    newReaderResponse(&error,readerId_, mdsCtxt);
}
void NtfClient::readNext(unsigned int readerId,
                         SaNtfSearchDirectionT searchDirection,
                         MDS_SYNC_SND_CTXT *mdsCtxt)
{
    TRACE_ENTER();
    TRACE_6("readerId %u", readerId);
    // check if reader already exists
    SaAisErrorT error = SA_AIS_ERR_NOT_EXIST;
    ReaderMapT::iterator pos;
    pos = readerMap.find(readerId);
    if (pos != readerMap.end())
    {
        // reader found
        TRACE_3("NtfClient::readNext readerId %u FOUND!",
                readerId);
        NtfReader* reader = pos->second;
        NtfNotification notif(reader->next(searchDirection, &error));
        readNextResponse(&error, notif, mdsCtxt);
        TRACE_LEAVE();
        return;
    }
    else
    {
        NtfNotification notif;
        // reader not found
        TRACE_3("NtfClient::readNext readerId %u not found",
                readerId);
        error = SA_AIS_ERR_BAD_HANDLE;
        readNextResponse(&error, notif, mdsCtxt);
        TRACE_LEAVE();
    }
}
void NtfClient::deleteReader(unsigned int readerId, MDS_SYNC_SND_CTXT *mdsCtxt)
{
    SaAisErrorT error = SA_AIS_ERR_NOT_EXIST;
    ReaderMapT::iterator pos;
    pos = readerMap.find(readerId);
    if (pos != readerMap.end())
    {
        // reader found
        TRACE_3("NtfClient::deleteReader readerId %u ",
                readerId);
        NtfReader* reader = pos->second;
        error = SA_AIS_OK;
        reader = NULL;
        delete reader;
    }
    else
    {
        // reader not found
        TRACE_3("NtfClient::readNext readerId %u not found",
                readerId);
    }
    deleteReaderResponse(&error, mdsCtxt);
}

void NtfClient::printInfo()
{
    TRACE("Client information");
    TRACE("  clientId:              %u", clientId_);
    TRACE("  mdsDest                %llu", mdsDest_);
    SubscriptionMap::iterator pos;
    for (pos = subscriptionMap.begin(); pos != subscriptionMap.end(); pos++)
    {
        NtfSubscription* subscription = pos->second;
        subscription->printInfo();
    }
    TRACE("  readerId counter:   %u", readerId_);
    ReaderMapT::iterator rpos;
    for (rpos = readerMap.begin(); rpos != readerMap.end(); rpos++)
    {
        unsigned int readerId = 0;
        NtfReader* reader = rpos->second;
        if (reader != NULL)
        {
            readerId =  reader->getId();
            TRACE("   Reader Id %u ", readerId);
        }
    }
}

