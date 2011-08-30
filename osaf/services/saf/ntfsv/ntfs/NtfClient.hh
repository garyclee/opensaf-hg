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

#ifndef CLIENT_H
#define CLIENT_H

#include "NtfSubscription.hh"
#include "NtfNotification.hh"
#include "NtfReader.hh"

class NtfClient{

public:

    NtfClient(unsigned int clientId,
              MDS_DEST mds_dest);
    virtual ~NtfClient();
    void subscriptionAdded(NtfSubscription* subscription,
                           MDS_SYNC_SND_CTXT *mdsCtxt);
    void notificationReceived(unsigned int clientId,
                              NtfNotification* notification,
                              MDS_SYNC_SND_CTXT *mdsCtxt);
    void confirmNtfSend();
    unsigned int getClientId() const;
    MDS_DEST getMdsDest() const;
    void subscriptionRemoved(SaNtfSubscriptionIdT subscriptionId,
                             MDS_SYNC_SND_CTXT *mdsCtxt);
    void syncRequest(NCS_UBAID *uba);
    void sendNotConfirmedNotification(NtfNotification* notification, SaNtfSubscriptionIdT subscriptionId);

    void confirmNtfNotification(SaNtfIdentifierT notificationId,
                                MDS_SYNC_SND_CTXT *mdsCtxt,
                                MDS_DEST mdsDest);
    void confirmNotificationLogged(SaNtfIdentifierT notificationId);
    void confirmNtfSubscribe(SaNtfSubscriptionIdT subscriptionId, MDS_SYNC_SND_CTXT *mds_ctxt);
    void confirmNtfUnsubscribe(SaNtfSubscriptionIdT subscriptionId,
                               MDS_SYNC_SND_CTXT *mdsCtxt);

	 void newReader(SaNtfSearchCriteriaT searchCriteria,
		 ntfsv_filter_ptrs_t *f_rec,
		 MDS_SYNC_SND_CTXT *mdsCtxt);

    void readNext(unsigned int readerId,
                  SaNtfSearchDirectionT searchDirection,
                  MDS_SYNC_SND_CTXT *mdsCtxt);
    void deleteReader(unsigned int readerId, MDS_SYNC_SND_CTXT *mdsCtxt);

	 void discardedAdd(SaNtfSubscriptionIdT subscriptionId, SaNtfIdentifierT notificationId);
	 void discardedClear(SaNtfSubscriptionIdT subscriptionId);
    void printInfo();

private:
    void newReaderResponse(SaAisErrorT* error,
                           unsigned int readerId,
                           MDS_SYNC_SND_CTXT *mdsCtxt);
    void readNextResponse(SaAisErrorT* error,
                          NtfNotification& notification,
                          MDS_SYNC_SND_CTXT *mdsCtxt);
    void deleteReaderResponse(SaAisErrorT* error,
                              MDS_SYNC_SND_CTXT *mdsCtxt);
	 void addDiscardedNotification(NtfNotification* notification);
	 
    unsigned int clientId_;
    unsigned int readerId_;

    MDS_DEST mdsDest_;
    typedef std::map<SaNtfSubscriptionIdT,NtfSubscription*> SubscriptionMap;
    SubscriptionMap subscriptionMap;

    typedef std::map<unsigned int, NtfReader*> ReaderMapT;
    ReaderMapT readerMap;

};

#endif // CLIENT_H
