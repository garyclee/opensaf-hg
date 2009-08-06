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

#include <saNtf.h>
#include <stdlib.h>
#include "ntfsv_mem.h"
#include <logtrace.h>

void ntfsv_free_header(const SaNtfNotificationHeaderT *notificationHeader)
{
	/* Data */
	if (notificationHeader->eventType != NULL)
		free(notificationHeader->eventType);
	if (notificationHeader->eventTime != NULL)
		free(notificationHeader->eventTime);
	if (notificationHeader->notificationObject != NULL)
		free(notificationHeader->notificationObject);
	if (notificationHeader->notifyingObject != NULL)
		free(notificationHeader->notifyingObject);
	if (notificationHeader->notificationClassId != NULL)
		free(notificationHeader->notificationClassId);
	if (notificationHeader->notificationId != NULL)
		free(notificationHeader->notificationId);

	/* Array Data */
	if (notificationHeader->correlatedNotifications != NULL) {
		free(notificationHeader->correlatedNotifications);
	}
	if (notificationHeader->additionalText != NULL) {
		free(notificationHeader->additionalText);
	}
	if (notificationHeader->additionalInfo != NULL) {
		free(notificationHeader->additionalInfo);
	}
}

void ntfsv_free_alarm(SaNtfAlarmNotificationT *alarm)
{
	TRACE_ENTER();
	ntfsv_free_header(&alarm->notificationHeader);
	if (alarm->trend != NULL)
		free(alarm->trend);
	if (alarm->thresholdInformation != NULL)
		free(alarm->thresholdInformation);
	if (alarm->probableCause != NULL)
		free(alarm->probableCause);
	if (alarm->perceivedSeverity != NULL)
		free(alarm->perceivedSeverity);
	if (alarm->specificProblems != NULL) {
		free(alarm->specificProblems);
	}
	if (alarm->monitoredAttributes != NULL) {
		free(alarm->monitoredAttributes);
	}
	if (alarm->proposedRepairActions != NULL) {
		free(alarm->proposedRepairActions);
	}
	TRACE_LEAVE();
}

void ntfsv_free_state_change(SaNtfStateChangeNotificationT *stateChange)
{
	ntfsv_free_header(&stateChange->notificationHeader);
	if (stateChange->sourceIndicator != NULL)
		free(stateChange->sourceIndicator);
	if (stateChange->changedStates != NULL) {
		free(stateChange->changedStates);
	}
}

void ntfsv_free_attribute_change(SaNtfAttributeChangeNotificationT *attrChange)
{
	ntfsv_free_header(&attrChange->notificationHeader);
	if (attrChange->sourceIndicator != NULL)
		free(attrChange->sourceIndicator);
	if (attrChange->changedAttributes != NULL) {
		free(attrChange->changedAttributes);
	}
}

void ntfsv_free_obj_create_del(SaNtfObjectCreateDeleteNotificationT *objCrDel)
{
	ntfsv_free_header(&objCrDel->notificationHeader);

	if (objCrDel->sourceIndicator != NULL)
		free(objCrDel->sourceIndicator);
	if (objCrDel->objectAttributes != NULL) {
		free(objCrDel->objectAttributes);
	}
}

void ntfsv_free_security_alarm(SaNtfSecurityAlarmNotificationT *secAlarm)
{
	ntfsv_free_header(&secAlarm->notificationHeader);
	if (secAlarm->probableCause != NULL)
		free(secAlarm->probableCause);
	if (secAlarm->severity != NULL)
		free(secAlarm->severity);
	if (secAlarm->securityAlarmDetector != NULL)
		free(secAlarm->securityAlarmDetector);
	if (secAlarm->serviceProvider != NULL)
		free(secAlarm->serviceProvider);
	if (secAlarm->serviceUser != NULL)
		free(secAlarm->serviceUser);
}

SaAisErrorT ntfsv_alloc_ntf_header(SaNtfNotificationHeaderT *notificationHeader,
				   SaUint16T numCorrelatedNotifications,
				   SaUint16T lengthAdditionalText, SaUint16T numAdditionalInfo)
{
	TRACE_ENTER();
	SaAisErrorT rc = SA_AIS_OK;
	if (notificationHeader == NULL) {
		TRACE("NULL pointer in *notificationHeader!");
		return SA_AIS_ERR_INVALID_PARAM;
	}
	notificationHeader->numCorrelatedNotifications = numCorrelatedNotifications;
	notificationHeader->lengthAdditionalText = lengthAdditionalText;
	notificationHeader->numAdditionalInfo = numAdditionalInfo;

	/* freed by ntfsv_free_header() */
	notificationHeader->notificationObject = NULL;
	notificationHeader->notifyingObject = NULL;
	notificationHeader->eventTime = NULL;
	notificationHeader->eventType = NULL;
	notificationHeader->correlatedNotifications = NULL;
	notificationHeader->notificationClassId = NULL;
	notificationHeader->additionalInfo = NULL;
	notificationHeader->additionalText = NULL;
	notificationHeader->notificationId = NULL;

	/* Event type */
	notificationHeader->eventType = malloc(sizeof(SaNtfEventTypeT));
	if (notificationHeader->eventType == NULL) {
		TRACE("Out of memory in eventType field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}

	/* Notification object */
	notificationHeader->notificationObject = malloc(sizeof(SaNameT));
	if (notificationHeader->notificationObject == NULL) {
		TRACE("Out of memory in notificationObject field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}

	/* Notifying object */
	notificationHeader->notifyingObject = malloc(sizeof(SaNameT));
	if (notificationHeader->notifyingObject == NULL) {
		TRACE("Out of memory in notifyingObject field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}

	/* Notification class ID */
	notificationHeader->notificationClassId = malloc(sizeof(SaNtfClassIdT));
	if (notificationHeader->notificationClassId == NULL) {
		TRACE("Out of memory in notificationClassId field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}

	/* Event time */
	notificationHeader->eventTime = malloc(sizeof(SaTimeT));
	if (notificationHeader->eventTime == NULL) {
		TRACE("Out of memory in eventTime field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}

	/* Notification ID */
	notificationHeader->notificationId = malloc(sizeof(SaNtfIdentifierT));
	if (notificationHeader->notificationId == NULL) {
		TRACE("Out of memory in notificationId field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}
	/* Correlated notifications */
	if (numCorrelatedNotifications != 0) {
		notificationHeader->correlatedNotifications = (SaNtfIdentifierT *)
		    malloc(numCorrelatedNotifications * sizeof(SaNtfIdentifierT));
		if (notificationHeader->correlatedNotifications == NULL) {
			TRACE("Out of memory in correlatedNotifications field");
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}

	/* Additional info */
	if (numAdditionalInfo != 0) {
		notificationHeader->additionalInfo = (SaNtfAdditionalInfoT *)
		    malloc(numAdditionalInfo * sizeof(SaNtfAdditionalInfoT));
		if (notificationHeader->additionalInfo == NULL) {
			TRACE("Out of memory in additionalInfo field");
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}

	/* Additional text */
	if (lengthAdditionalText != 0) {
		notificationHeader->additionalText = (SaStringT)malloc(lengthAdditionalText * sizeof(char));
		if (notificationHeader->additionalText == NULL) {
			TRACE("Out of memory in additionalText field");
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}

 done:
	if (rc != SA_AIS_OK) {
		ntfsv_free_header(notificationHeader);
	}
	TRACE_LEAVE();
	return rc;
}

SaAisErrorT ntfsv_alloc_ntf_alarm(SaNtfAlarmNotificationT *alarmNotification,
				  SaUint16T numSpecificProblems,
				  SaUint16T numMonitoredAttributes, SaUint16T numProposedRepairActions)
{
	TRACE_ENTER();
	SaAisErrorT rc = SA_AIS_OK;
	/* Perceived severity */
	alarmNotification->numSpecificProblems = numSpecificProblems;
	alarmNotification->numMonitoredAttributes = numMonitoredAttributes;
	alarmNotification->numProposedRepairActions = numProposedRepairActions;

	/* freed in */
	alarmNotification->probableCause = NULL;
	alarmNotification->specificProblems = NULL;
	alarmNotification->perceivedSeverity = NULL;
	alarmNotification->trend = NULL;
	alarmNotification->thresholdInformation = NULL;
	alarmNotification->monitoredAttributes = NULL;
	alarmNotification->proposedRepairActions = NULL;

	alarmNotification->perceivedSeverity = malloc(sizeof(SaNtfSeverityT));
	if (alarmNotification->perceivedSeverity == NULL) {
		TRACE("Out of memory in perceivedSeverity field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}
	/* Trend */
	alarmNotification->trend = malloc(sizeof(SaNtfSeverityTrendT));
	if (alarmNotification->trend == NULL) {
		TRACE("Out of memory in trend field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}
	/* ThresholdInformation */
	alarmNotification->thresholdInformation = malloc(sizeof(SaNtfThresholdInformationT));
	if (alarmNotification->thresholdInformation == NULL) {
		TRACE("Out of memory in thresholdInformation field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}
	/* Probable cause */
	alarmNotification->probableCause = malloc(sizeof(SaNtfProbableCauseT));
	if (alarmNotification->probableCause == NULL) {
		TRACE("Out of memory in probableCause field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}
	/* Specific problems */
	if (numSpecificProblems != 0) {
		alarmNotification->specificProblems = (SaNtfSpecificProblemT *)
		    malloc(numSpecificProblems * sizeof(SaNtfSpecificProblemT));
		if (alarmNotification->specificProblems == NULL) {
			TRACE("Out of memory in specificProblems field");
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}
	/* Monitored attributes */
	if (numMonitoredAttributes != 0) {
		alarmNotification->monitoredAttributes = (SaNtfAttributeT *)
		    malloc(numMonitoredAttributes * sizeof(SaNtfAttributeT));
		if (alarmNotification->monitoredAttributes == NULL) {
			TRACE("Out of memory in monitoredAttributes field");
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}
	/* Proposed repair actions */
	if (numProposedRepairActions != 0) {
		alarmNotification->proposedRepairActions = (SaNtfProposedRepairActionT *)
		    malloc(numProposedRepairActions * sizeof(SaNtfProposedRepairActionT));
		if (alarmNotification->proposedRepairActions == NULL) {
			TRACE("Out of memory in proposedRepairActions field");
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}
 done:
	if (rc != SA_AIS_OK) {
		ntfsv_free_alarm(alarmNotification);
	}
	TRACE_LEAVE();
	return rc;
}

SaAisErrorT ntfsv_alloc_ntf_obj_create_del(SaNtfObjectCreateDeleteNotificationT *objCrDelNotification,
					   SaUint16T numAttributes)
{
	SaAisErrorT rc = SA_AIS_OK;
	objCrDelNotification->sourceIndicator = NULL;

	objCrDelNotification->objectAttributes = NULL;
	/* Source indicator */
	objCrDelNotification->numAttributes = numAttributes;
	objCrDelNotification->sourceIndicator = malloc(sizeof(SaNtfSourceIndicatorT));
	if (objCrDelNotification->sourceIndicator == NULL) {
		TRACE_1("Out of memory in sourceIndicator field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}
	/* Object attributes */
	if (numAttributes != 0) {
		objCrDelNotification->objectAttributes = (SaNtfAttributeT *)
		    malloc(numAttributes * sizeof(SaNtfAttributeT));
		if (objCrDelNotification->objectAttributes == NULL) {
			TRACE_1("Out of memory in objectAttributes field");
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}

 done:
	if (rc != SA_AIS_OK) {
		ntfsv_free_obj_create_del(objCrDelNotification);
	}
	return rc;
}

SaAisErrorT ntfsv_alloc_ntf_attr_change(SaNtfAttributeChangeNotificationT *attrChangeNotification,
					SaUint16T numAttributes)
{
	SaAisErrorT rc = SA_AIS_OK;
	attrChangeNotification->numAttributes = numAttributes;

	attrChangeNotification->sourceIndicator = NULL;
	attrChangeNotification->changedAttributes = NULL;
	/* Source indicator */
	attrChangeNotification->sourceIndicator = malloc(sizeof(SaNtfSourceIndicatorT));
	if (attrChangeNotification->sourceIndicator == NULL) {
		TRACE_1("Out of memory in sourceIndicator field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}
	/* Changed attributes */
	if (numAttributes != 0) {
		attrChangeNotification->changedAttributes = (SaNtfAttributeChangeT *)
		    malloc(numAttributes * sizeof(SaNtfAttributeChangeT));
		if (attrChangeNotification->changedAttributes == NULL) {
			TRACE_1("Out of memory in changedAttributes field");
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}
 done:
	if (rc != SA_AIS_OK) {
		ntfsv_free_attribute_change(attrChangeNotification);
	}
	return rc;
}

SaAisErrorT ntfsv_alloc_ntf_state_change(SaNtfStateChangeNotificationT *stateChangeNotification,
					 SaUint16T numStateChanges)
{
	SaAisErrorT rc = SA_AIS_OK;

	stateChangeNotification->numStateChanges = numStateChanges;
	stateChangeNotification->changedStates = NULL;
	stateChangeNotification->sourceIndicator = NULL;
	/* Source indicator */
	stateChangeNotification->sourceIndicator = malloc(sizeof(SaNtfSourceIndicatorT));
	if (stateChangeNotification->sourceIndicator == NULL) {
		TRACE_1("Out of memory in sourceIndicator field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}
	/* Changed states */
	if (numStateChanges != 0) {
		stateChangeNotification->changedStates = (SaNtfStateChangeT *)
		    malloc(numStateChanges * sizeof(SaNtfStateChangeT));
		if (stateChangeNotification->changedStates == NULL) {
			TRACE_1("Out of memory in changedStates field");
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}
 done:
	if (rc != SA_AIS_OK) {
		ntfsv_free_state_change(stateChangeNotification);
	}
	return rc;
}

SaAisErrorT ntfsv_alloc_ntf_security_alarm(SaNtfSecurityAlarmNotificationT *securityAlarm)
{
	SaAisErrorT rc = SA_AIS_OK;

	securityAlarm->probableCause = NULL;
	securityAlarm->severity = NULL;
	securityAlarm->securityAlarmDetector = NULL;
	securityAlarm->serviceUser = NULL;
	securityAlarm->serviceProvider = NULL;
	/* Probable cause */
	securityAlarm->probableCause = calloc(1, sizeof(SaNtfProbableCauseT));
	if (securityAlarm->probableCause == NULL) {
		TRACE_1("Out of memory in probableCause field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}
	/* Severity */
	securityAlarm->severity = calloc(1, sizeof(SaNtfSeverityT));
	if (securityAlarm->severity == NULL) {
		TRACE_1("Out of memory in severity field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}
	/* Security alarm detector */
	securityAlarm->securityAlarmDetector = calloc(1, sizeof(SaNtfSecurityAlarmDetectorT));
	if (securityAlarm->securityAlarmDetector == NULL) {
		TRACE_1("Out of memory in securityAlarmDetector field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}
	/* Service user */
	securityAlarm->serviceUser = calloc(1, sizeof(SaNtfServiceUserT));
	if (securityAlarm->serviceUser == NULL) {
		TRACE_1("Out of memory in serviceUser field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}
	/* Service provider */
	securityAlarm->serviceProvider = calloc(1, sizeof(SaNtfServiceUserT));
	if (securityAlarm->serviceProvider == NULL) {
		TRACE_1("Out of memory in serviceProvider field");
		rc = SA_AIS_ERR_NO_MEMORY;
		goto done;
	}
 done:
	if (rc != SA_AIS_OK) {
		ntfsv_free_security_alarm(securityAlarm);
	}
	return SA_AIS_OK;
}

void ntfsv_copy_ntf_header(SaNtfNotificationHeaderT *dest, const SaNtfNotificationHeaderT *src)
{
	TRACE_ENTER();
	*(dest->eventType) = *(src->eventType);

	/* event time to be set automatically to current
	   time by saNtfNotificationSend */
	*(dest->eventTime) = *(src->eventTime);

	/* Set Notification Object */
	*(dest->notificationObject) = *(src->notificationObject);

	/* Set Notifying Object */
	*(dest->notifyingObject) = *(src->notifyingObject);

	/* set Notification Class Identifier */
	dest->notificationClassId->vendorId = src->notificationClassId->vendorId;

	/* sub id of this notification class within "name space" of vendor ID */
	dest->notificationClassId->majorId = src->notificationClassId->majorId;
	dest->notificationClassId->minorId = src->notificationClassId->minorId;

	*(dest->notificationId) = *src->notificationId;

	dest->lengthAdditionalText = src->lengthAdditionalText;
	/* Set additional text and additional info */
	(void)strncpy(dest->additionalText, src->additionalText, src->lengthAdditionalText);
	TRACE_LEAVE();
}

/**
 * Copy a SaNtfAlarmNotificationT* from src to dest. 
 *  Note dest needs to be allocated to the same size as src.
 *  
 *  @param dest 
 *  @param src
 *  
 */
void ntfsv_copy_ntf_alarm(SaNtfAlarmNotificationT *dest, const SaNtfAlarmNotificationT *src)
{
	TRACE_ENTER();
	ntfsv_copy_ntf_header(&dest->notificationHeader, &src->notificationHeader);
	/* Set perceived severity */
	*(dest->perceivedSeverity) = *(src->perceivedSeverity);
	/* Set probable cause */
	*(dest->probableCause) = *(src->probableCause);
	TRACE_LEAVE();
}

/**
 * Copy a SaNtfxxNotificationT* from src to dest. 
 *  Note dest needs to be allocated to the same size as src.
 *  
 *  @param dest 
 *  @param src
 *  
 */
void ntfsv_copy_ntf_security_alarm(SaNtfSecurityAlarmNotificationT *dest, const SaNtfSecurityAlarmNotificationT *src)
{
	ntfsv_copy_ntf_header(&dest->notificationHeader, &src->notificationHeader);
	dest->probableCause = src->probableCause;

	dest->severity = src->severity;
	dest->securityAlarmDetector = src->securityAlarmDetector;
	dest->serviceUser = src->serviceUser;
	dest->serviceProvider = src->serviceProvider;
}

/**
 * Copy a SaNtfxxNotificationT* from src to dest. 
 *  Note dest needs to be allocated to the same size as src.
 *  
 *  @param dest 
 *  @param src
 *  
 */
void ntfsv_copy_ntf_obj_cr_del(SaNtfObjectCreateDeleteNotificationT *dest,
			       const SaNtfObjectCreateDeleteNotificationT *src)
{
	ntfsv_copy_ntf_header(&dest->notificationHeader, &src->notificationHeader);
	dest->numAttributes = src->numAttributes;
	dest->sourceIndicator = src->sourceIndicator;

	if (0 < dest->numAttributes) {
		memcpy(dest->objectAttributes, src->objectAttributes, dest->numAttributes * sizeof(SaNtfAttributeT));
	}
}

/**
 * Copy a SaNtfxxNotificationT* from src to dest. 
 *  Note dest needs to be allocated to the same size as src.
 *  
 *  @param dest 
 *  @param src
 *  
 */
void ntfsv_copy_ntf_attr_change(SaNtfAttributeChangeNotificationT *dest, const SaNtfAttributeChangeNotificationT *src)
{
	ntfsv_copy_ntf_header(&dest->notificationHeader, &src->notificationHeader);
	dest->numAttributes = src->numAttributes;

	dest->sourceIndicator = src->sourceIndicator;
	if (0 < dest->numAttributes) {
		memcpy(dest->changedAttributes,
		       src->changedAttributes, dest->numAttributes * sizeof(SaNtfAttributeChangeT));
	}
}

/**
 * Copy a SaNtfxxNotificationT* from src to dest. 
 *  Note dest needs to be allocated to the same size as src.
 *  
 *  @param dest 
 *  @param src
 *  
 */
void ntfsv_copy_ntf_state_change(SaNtfStateChangeNotificationT *dest, const SaNtfStateChangeNotificationT *src)
{
	ntfsv_copy_ntf_header(&dest->notificationHeader, &src->notificationHeader);
	dest->numStateChanges = src->numStateChanges;

	dest->sourceIndicator = src->sourceIndicator;

	if (0 < dest->numStateChanges) {
		memcpy(dest->changedStates, src->changedStates, dest->numStateChanges * sizeof(SaNtfStateChangeT));
	}
}

void ntfsv_dealloc_notification(ntfsv_send_not_req_t *param)
{
	TRACE_ENTER2("ntfsv_send_not_req_t ptr = %p " "notificationType = %d", param, (int)param->notificationType);
	switch (param->notificationType) {
	case SA_NTF_TYPE_ALARM:
		ntfsv_free_alarm(&param->notification.alarm);
		break;
	case SA_NTF_TYPE_OBJECT_CREATE_DELETE:
		ntfsv_free_obj_create_del(&param->notification.objectCreateDelete);
		break;
	case SA_NTF_TYPE_ATTRIBUTE_CHANGE:
		ntfsv_free_attribute_change(&param->notification.attributeChange);
		break;
	case SA_NTF_TYPE_STATE_CHANGE:
		ntfsv_free_state_change(&param->notification.stateChange);
		break;
	case SA_NTF_TYPE_SECURITY_ALARM:
		ntfsv_free_security_alarm(&param->notification.securityAlarm);
		break;
	default:
		TRACE("notificationType not valid");
	}
	TRACE_LEAVE();
}

void ntfsv_alloc_and_copy_not(ntfsv_send_not_req_t *dest, const ntfsv_send_not_req_t *src)
{
	TRACE_ENTER2("ntfsv_send_not_req_t ptr = %p" "notificationType = %x", src, (int)src->notificationType);
	switch (src->notificationType) {
	case SA_NTF_TYPE_ALARM:
		ntfsv_alloc_ntf_alarm(&dest->notification.alarm,
				      src->notification.alarm.numSpecificProblems,
				      src->notification.alarm.numMonitoredAttributes,
				      src->notification.alarm.numProposedRepairActions);
		ntfsv_alloc_ntf_header(&dest->notification.alarm.notificationHeader,
				       src->notification.alarm.notificationHeader.numCorrelatedNotifications,
				       src->notification.alarm.notificationHeader.lengthAdditionalText,
				       src->notification.alarm.notificationHeader.numAdditionalInfo);
		ntfsv_copy_ntf_alarm(&dest->notification.alarm, &src->notification.alarm);
		break;
	case SA_NTF_TYPE_OBJECT_CREATE_DELETE:
		ntfsv_alloc_ntf_obj_create_del(&dest->notification.objectCreateDelete,
					       src->notification.objectCreateDelete.numAttributes);
		ntfsv_alloc_ntf_header(&dest->notification.objectCreateDelete.notificationHeader,
				       src->notification.objectCreateDelete.
				       notificationHeader.numCorrelatedNotifications,
				       src->notification.objectCreateDelete.notificationHeader.lengthAdditionalText,
				       src->notification.objectCreateDelete.notificationHeader.numAdditionalInfo);
		ntfsv_copy_ntf_obj_cr_del(&dest->notification.objectCreateDelete,
					  &src->notification.objectCreateDelete);
		break;
	case SA_NTF_TYPE_ATTRIBUTE_CHANGE:
		ntfsv_alloc_ntf_attr_change(&dest->notification.attributeChange,
					    src->notification.attributeChange.numAttributes);
		ntfsv_alloc_ntf_header(&dest->notification.attributeChange.notificationHeader,
				       src->notification.attributeChange.notificationHeader.numCorrelatedNotifications,
				       src->notification.attributeChange.notificationHeader.lengthAdditionalText,
				       src->notification.attributeChange.notificationHeader.numAdditionalInfo);
		ntfsv_copy_ntf_attr_change(&dest->notification.attributeChange, &src->notification.attributeChange);
		break;
	case SA_NTF_TYPE_STATE_CHANGE:
		ntfsv_alloc_ntf_state_change(&dest->notification.stateChange,
					     src->notification.stateChange.numStateChanges);
		ntfsv_alloc_ntf_header(&dest->notification.stateChange.notificationHeader,
				       src->notification.stateChange.notificationHeader.numCorrelatedNotifications,
				       src->notification.stateChange.notificationHeader.lengthAdditionalText,
				       src->notification.stateChange.notificationHeader.numAdditionalInfo);
		ntfsv_copy_ntf_state_change(&dest->notification.stateChange, &src->notification.stateChange);
		break;
	case SA_NTF_TYPE_SECURITY_ALARM:
		ntfsv_alloc_ntf_security_alarm(&dest->notification.securityAlarm);
		ntfsv_alloc_ntf_header(&dest->notification.securityAlarm.notificationHeader,
				       src->notification.securityAlarm.notificationHeader.numCorrelatedNotifications,
				       src->notification.securityAlarm.notificationHeader.lengthAdditionalText,
				       src->notification.securityAlarm.notificationHeader.numAdditionalInfo);
		ntfsv_copy_ntf_security_alarm(&dest->notification.securityAlarm, &src->notification.securityAlarm);
		break;
	default:
		TRACE("notificationType not valid");
	}
	TRACE_LEAVE();

}

void ntfsv_get_ntf_header(ntfsv_send_not_req_t *notif, SaNtfNotificationHeaderT **ntfHeader)
{
	TRACE_ENTER();
	switch (notif->notificationType) {
	case SA_NTF_TYPE_ALARM:
		*ntfHeader = &notif->notification.alarm.notificationHeader;
		break;
	case SA_NTF_TYPE_OBJECT_CREATE_DELETE:
		*ntfHeader = &notif->notification.objectCreateDelete.notificationHeader;
		break;
	case SA_NTF_TYPE_ATTRIBUTE_CHANGE:
		*ntfHeader = &notif->notification.attributeChange.notificationHeader;
		break;
	case SA_NTF_TYPE_STATE_CHANGE:
		*ntfHeader = &notif->notification.stateChange.notificationHeader;
		break;
	case SA_NTF_TYPE_SECURITY_ALARM:
		*ntfHeader = &notif->notification.securityAlarm.notificationHeader;
		break;
	default:
		TRACE("notificationType not valid");
		assert(0);
	}
	TRACE_LEAVE();
}
