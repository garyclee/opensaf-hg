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
 *   This object handles information about NTF subscriptions.
 */

#include "NtfSubscription.hh"
#include "logtrace.h"

/**
 * This is the constructor.
 *
 * Initial variables are set and pointer to the filter object is stored.
 *
 * @param subscriptionId
 *               Client-wide unique id of the subscription.
 * @param filter Pointer to the filter object.
 */
NtfSubscription::NtfSubscription(SaNtfSubscriptionIdT subscriptionId,
                                 NtfFilter* filter)
{

    subscriptionId_ = subscriptionId;
    TRACE_1("Subscription %u created", subscriptionId_);
    filterMap[filter->type()] = filter;
    TRACE_1("Filter %d added to subscription %u, filterMap size is %u",
            filter->type(), subscriptionId_, (unsigned int)filterMap.size());
}

/**
 * This is the destructor.
 *
 * The filter object is deleted.
 */
NtfSubscription::~NtfSubscription()
{
    // delete all filters
    FilterMap::iterator posN = filterMap.begin();
    while (posN != filterMap.end())
    {
        NtfFilter* filter = posN->second;
        TRACE_1("Filter of type %d destroyed", (int)filter->type());
        delete filter;
        posN++;
    }
    TRACE_1("Subscription %u deleted", subscriptionId_);
}
/**
 * This method is called to get the id of the subscription.
 *
 * @return Id of the subscription.
 */
SaNtfSubscriptionIdT NtfSubscription::getSubscriptionId() const
{
    return(subscriptionId_);
}

/**
 * This method is called to check if the subscription matches the received notification.
 *
 * The appropriate filter that matches the type of the received
 * notification is checked.
 *
 * @param notification
 *               Pointer to the received notifiaction object.
 *
 * @return true if the subscription matches the notification
 *         false if the subscription does not match the notification
 */
bool NtfSubscription::checkSubscription(const NtfNotification* notification)
{
    return(true);
}

void NtfSubscription::printInfo()
{
    TRACE("Subscription information");
    TRACE("  subscriptionId %u", subscriptionId_);
}
