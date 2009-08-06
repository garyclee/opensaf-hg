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
 *
 *   This file contains the interface between the Admin (c++ framework) and the
 *   comunication layer (c-part) mds, mbcsv and amf.
 *
 */
#ifndef NTF_COM_H
#define NTF_COM_H

/* ========================================================================
 *   INCLUDE FILES
 * ========================================================================
 */
#include "saAis.h"
#include "saNtf.h"
#include "ntfsv_msg.h"
#include "ntfs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 *   DATA DECLARATIONS
 * ========================================================================
 */

/* ========================================================================
 *   DEFINITIONS
 * ========================================================================
 */

/* From communication layer --> Admin */
	void initAdmin(void);
	void startSendSync(void);
	void clientAdded(unsigned int clientId, MDS_DEST mdsDest, MDS_SYNC_SND_CTXT *mdsCtxt);
	void subscriptionAdded(unsigned int clientId, SaNtfSubscriptionIdT subscriptionId, MDS_SYNC_SND_CTXT *mdsCtxt);

	void notificationReceived(unsigned int clientId,
				  SaNtfNotificationTypeT notificationType,
				  ntfsv_send_not_req_t *sendNotInfo, MDS_SYNC_SND_CTXT *mdsCtxt);
	void notificationReceivedUpdate(unsigned int clientId,
					SaNtfNotificationTypeT notificationType, ntfsv_send_not_req_t *sendNotInfo);
	void notificationReceivedColdSync(unsigned int clientId,
					  SaNtfNotificationTypeT notificationType, ntfsv_send_not_req_t *sendNotInfo);
	void notificationSentConfirmed(unsigned int clientId,
				       SaNtfSubscriptionIdT subscriptionId, SaNtfIdentifierT notificationId);
	void notificationLoggedConfirmed(SaNtfIdentifierT notificationId);
	void clientRemoved(unsigned int clientId);
	void clientRemoveMDS(MDS_DEST mds_dest);
	void subscriptionRemoved(unsigned int clientId,
				 SaNtfSubscriptionIdT subscriptionId, MDS_SYNC_SND_CTXT *mdsCtxt);
	void syncRequest(NCS_UBAID *uba);
	int syncFinished();
	void newReader(unsigned int clientId, SaNtfSearchCriteriaT searchCriteria, MDS_SYNC_SND_CTXT *mdsCtxt);
	void readNext(unsigned int clientId,
		      unsigned int readerId, SaNtfSearchDirectionT searchDirection, MDS_SYNC_SND_CTXT *mdsCtxt);
	void deleteReader(unsigned int clientId, unsigned int readerId, MDS_SYNC_SND_CTXT *mdsCtxt);

	void printAdminInfo(void);
	void syncGlobals(const struct NtfGlobals *ntfGlobals);
	void storeMatchingSubscription(SaNtfIdentifierT notificationId,
				       unsigned int clientId, SaNtfSubscriptionIdT subscriptionId);

/* Calls from Admin --> communication layer */

/**
 *  Find out if this controller is active
 *
 *   @return 0 if not active
 */
	int activeController(void);

	void client_added_res_lib(SaAisErrorT error,
				  unsigned int clientId, MDS_DEST mdsDest, MDS_SYNC_SND_CTXT *mdsCtxt);

	void client_removed_res_lib(SaAisErrorT error,
				    unsigned int clientId, MDS_DEST mdsDest, MDS_SYNC_SND_CTXT *mdsCtxt);

	void subscribe_res_lib(SaAisErrorT error,
			       SaNtfSubscriptionIdT subId, MDS_DEST mdsDest, MDS_SYNC_SND_CTXT *mdsCtxt);

	void unsubscribe_res_lib(SaAisErrorT error,
				 SaNtfSubscriptionIdT subId, MDS_DEST mdsDest, MDS_SYNC_SND_CTXT *mdsCtxt);

	void notfication_result_lib(SaAisErrorT error,
				    SaNtfIdentifierT notificationId, MDS_SYNC_SND_CTXT *mdsCtxt, MDS_DEST frDest);

	void new_reader_res_lib(SaAisErrorT error, unsigned int readerId, MDS_DEST mdsDest, MDS_SYNC_SND_CTXT *mdsCtxt);

	void read_next_res_lib(SaAisErrorT error,
			       ntfsv_send_not_req_t *notification, MDS_DEST mdsDest, MDS_SYNC_SND_CTXT *mdsCtxt);

	void delete_reader_res_lib(SaAisErrorT error, MDS_DEST mdsDest, MDS_SYNC_SND_CTXT *mdsCtxt);

	int send_notification_lib(ntfsv_send_not_req_t *dispatchInfo, uns32 client_id, MDS_DEST *mds_dest);

	void sendLoggedConfirm(SaNtfIdentifierT notificationId);

	int send_discard_notification_lib(struct DiscardedInfo *discardedInfo);

	void setStartSyncTimer();
	void setSyncWaitTimer();

/* messages sent over MBCSv to cold sync stby NTF server */
	int sendSyncGlobals(const struct NtfGlobals *ntfGlobals, NCS_UBAID *uba);
	int sendNewNotification(unsigned int connId, ntfsv_send_not_req_t *notificationInfo, NCS_UBAID *uba);
	int sendNewClient(unsigned int clientId, MDS_DEST mdsDest, NCS_UBAID *uba);
	int sendNewSubscription(unsigned int clientId, unsigned int subscriptionId, NCS_UBAID *uba);

	void sendMapNoOfSubscriptionToNotification(unsigned int noOfSubcriptions, NCS_UBAID *uba);
	void sendMapSubscriptionToNotification(unsigned int clientId, unsigned int subscriptionId, NCS_UBAID *uba);

	int syncLoggedConfirm(unsigned int logged, NCS_UBAID *uba);
	void sendSubscriptionUpdate(unsigned int clientId, unsigned int subscriptionId);
	void sendUnsubscribeUpdate(unsigned int clientId, unsigned int subscriptionId);
	void sendNotificationUpdate(unsigned int clientId, ntfsv_send_not_req_t *notification);
	void sendLoggedConfirmUpdate(SaNtfIdentifierT notificationId);
	void sendNotConfirmUpdate(unsigned int clientId,
				  SaNtfSubscriptionIdT subscriptionId, SaNtfIdentifierT notificationId);
	int sendNoOfNotifications(uns32 num_rec, NCS_UBAID *uba);
	int sendNoOfSubscriptions(uns32 num_rec, NCS_UBAID *uba);
	int sendNoOfClients(uns32 num_rec, NCS_UBAID *uba);

/* Calls from c --> c++ layer */
	void logEvent();
	void checkNotificationList();

#ifdef __cplusplus
}
#endif

#endif
