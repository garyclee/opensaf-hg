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

#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include "NtfFilter.hh"
#include "NtfNotification.hh"
#include "saNtf.h"

class NtfSubscription{

public:

    NtfSubscription(SaNtfSubscriptionIdT subscriptionId, NtfFilter* filter);
    virtual ~NtfSubscription();
    bool checkSubscription(const NtfNotification* notification);
    void confirmNtfSend();
    SaNtfSubscriptionIdT getSubscriptionId() const;
    void printInfo();
    typedef std::map<SaNtfNotificationTypeT,NtfFilter*> FilterMap;
    FilterMap filterMap;

private:

    SaNtfSubscriptionIdT subscriptionId_;
};

#endif // SUBSCRIPTION_H
