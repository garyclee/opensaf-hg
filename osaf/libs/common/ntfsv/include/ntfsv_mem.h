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

#ifndef NTFSV_MEM_H
#define NTFSV_MEM_H

#include <saNtf.h>
#include <ntfsv_msg.h>

#ifdef  __cplusplus
extern "C" {
#endif
	SaAisErrorT ntfsv_alloc_ntf_header(SaNtfNotificationHeaderT *notificationHeader,
					   SaUint16T numCorrelatedNotifications,
					   SaUint16T lengthAdditionalText, SaUint16T numAdditionalInfo);
	SaAisErrorT ntfsv_alloc_ntf_alarm(SaNtfAlarmNotificationT *alarmNotification,
					  SaUint16T numSpecificProblems,
					  SaUint16T numMonitoredAttributes, SaUint16T numProposedRepairActions);
	SaAisErrorT ntfsv_alloc_ntf_obj_create_del(SaNtfObjectCreateDeleteNotificationT *objCrDelNotification,
						   SaUint16T numAttributes);
	SaAisErrorT ntfsv_alloc_ntf_attr_change(SaNtfAttributeChangeNotificationT *attrChangeNotification,
						SaUint16T numAttributes);
	SaAisErrorT ntfsv_alloc_ntf_state_change(SaNtfStateChangeNotificationT *stateChangeNotification,
						 SaUint16T numStateChanges);
	SaAisErrorT ntfsv_alloc_ntf_security_alarm(SaNtfSecurityAlarmNotificationT *securityAlarm);

	void ntfsv_free_header(const SaNtfNotificationHeaderT *notificationHeader);
	void ntfsv_free_alarm(SaNtfAlarmNotificationT *alarm);
	void ntfsv_free_state_change(SaNtfStateChangeNotificationT *stateChange);
	void ntfsv_free_attribute_change(SaNtfAttributeChangeNotificationT *attrChange);
	void ntfsv_free_obj_create_del(SaNtfObjectCreateDeleteNotificationT *objCrDel);
	void ntfsv_free_security_alarm(SaNtfSecurityAlarmNotificationT *secAlarm);
	void ntfsv_dealloc_notification(ntfsv_send_not_req_t *param);

	void ntfsv_copy_ntf_header(SaNtfNotificationHeaderT *dest, const SaNtfNotificationHeaderT *src);
	void ntfsv_copy_ntf_alarm(SaNtfAlarmNotificationT *dest, const SaNtfAlarmNotificationT *src);
	void ntfsv_copy_ntf_obj_cr_del(SaNtfObjectCreateDeleteNotificationT *dest,
				       const SaNtfObjectCreateDeleteNotificationT *src);
	void ntfsv_copy_ntf_security_alarm(SaNtfSecurityAlarmNotificationT *dest,
					   const SaNtfSecurityAlarmNotificationT *src);
	void ntfsv_copy_ntf_attr_change(SaNtfAttributeChangeNotificationT *dest,
					const SaNtfAttributeChangeNotificationT *src);
	void ntfsv_copy_ntf_state_change(SaNtfStateChangeNotificationT *dest, const SaNtfStateChangeNotificationT *src);

	void ntfsv_alloc_and_copy_not(ntfsv_send_not_req_t *dest, const ntfsv_send_not_req_t *src);
	void ntfsv_get_ntf_header(ntfsv_send_not_req_t *notif, SaNtfNotificationHeaderT **ntfHeader);
#ifdef  __cplusplus
}
#endif

#endif   /* NTFSV_MEM_H */
