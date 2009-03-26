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
#include "tet_ntf.h"
#include "tet_ntf_common.h"
#include "test.h"
#include <sys/time.h>
#include <poll.h>
#include <unistd.h>

static SaNtfAlarmNotificationT myAlarmNotification;

void saNtfNotificationSend_01(void) {
	/*  struct pollfd fds[1];*/
	/*  int ret;             */
	saNotificationAllocationParamsT myNotificationAllocationParams;
	saNotificationFilterAllocationParamsT myNotificationFilterAllocationParams;
	saNotificationParamsT myNotificationParams;

	fillInDefaultValues(&myNotificationAllocationParams,
			&myNotificationFilterAllocationParams, &myNotificationParams);

	safassert(saNtfInitialize(&ntfHandle, &ntfSendCallbacks, &ntfVersion), SA_AIS_OK);
	safassert(saNtfSelectionObjectGet(ntfHandle, &selectionObject), SA_AIS_OK);

	AlarmNotificationParams myAlarmParams;

	myAlarmParams.numCorrelatedNotifications
			= myNotificationAllocationParams.numCorrelatedNotifications;
	myAlarmParams.lengthAdditionalText
			= myNotificationAllocationParams.lengthAdditionalText;
	myAlarmParams.numAdditionalInfo
			= myNotificationAllocationParams.numAdditionalInfo;
	myAlarmParams.numSpecificProblems
			= myNotificationAllocationParams.numSpecificProblems;
	myAlarmParams.numMonitoredAttributes
			= myNotificationAllocationParams.numMonitoredAttributes;
	myAlarmParams.numProposedRepairActions
			= myNotificationAllocationParams.numProposedRepairActions;
	myAlarmParams.variableDataSize
			= myNotificationAllocationParams.variableDataSize;

	safassert(saNtfAlarmNotificationAllocate(
					ntfHandle,
					&myAlarmNotification,
					myAlarmParams.numCorrelatedNotifications,
					myAlarmParams.lengthAdditionalText,
					myAlarmParams.numAdditionalInfo,
					myAlarmParams.numSpecificProblems,
					myAlarmParams.numMonitoredAttributes,
					myAlarmParams.numProposedRepairActions,
					myAlarmParams.variableDataSize), SA_AIS_OK);

	myNotificationParams.eventType = myNotificationParams.alarmEventType;
	fill_header_part(&myAlarmNotification.notificationHeader,
			(saNotificationParamsT *) &myNotificationParams,
			myAlarmParams.lengthAdditionalText);

	/* determine perceived severity */
	*(myAlarmNotification.perceivedSeverity)
			= myNotificationParams.perceivedSeverity;

	/* set probable cause*/
	*(myAlarmNotification.probableCause) = myNotificationParams.probableCause;

	rc = saNtfNotificationSend(myAlarmNotification.notificationHandle);

	/*  fds[0].fd = (int) selectionObject;                             */
	/*  fds[0].events = POLLIN;                                        */
	/*  ret = poll(fds, 1, 10000);                                     */
	/*  assert(ret > 0);                                               */
	/*  safassert(saNtfDispatch(ntfHandle, SA_DISPATCH_ONE), SA_AIS_OK);*/

	/* only for testing callback encode decode TODO: remove */
	safassert(saNtfNotificationFree(myAlarmNotification.notificationHandle), SA_AIS_OK);
	safassert(saNtfFinalize(ntfHandle), SA_AIS_OK);
	test_validate(rc, SA_AIS_OK);
}

void saNtfNotificationSend_02(void) {
	SaNtfObjectCreateDeleteNotificationT myNotification;

	saNotificationAllocationParamsT myNotificationAllocationParams;
	saNotificationFilterAllocationParamsT myNotificationFilterAllocationParams;
	saNotificationParamsT myNotificationParams;

	fillInDefaultValues(&myNotificationAllocationParams,
			&myNotificationFilterAllocationParams, &myNotificationParams);

	safassert(saNtfInitialize(&ntfHandle, &ntfSendCallbacks, &ntfVersion), SA_AIS_OK);
	safassert(saNtfObjectCreateDeleteNotificationAllocate(
					ntfHandle, /* handle to Notification Service instance */
					&myNotification,
					/* number of correlated notifications */
					myNotificationAllocationParams.numCorrelatedNotifications,
					/* length of additional text */
					myNotificationAllocationParams.lengthAdditionalText,
					/* number of additional info items*/
					myNotificationAllocationParams.numAdditionalInfo,
					/* number of state changes */
					myNotificationAllocationParams.numObjectAttributes,
					/* use default allocation size */
					myNotificationAllocationParams.variableDataSize), SA_AIS_OK);

	/* Event type */
	*(myNotification.notificationHeader.eventType) = SA_NTF_OBJECT_CREATION;

	/* event time to be set automatically to current
	 time by saNtfNotificationSend */
	*(myNotification.notificationHeader.eventTime)
			= myNotificationParams.eventTime;

	/* Set Notification Object */
	myNotification.notificationHeader.notificationObject->length
			= myNotificationParams.notificationObject.length;
	(void) memcpy(myNotification.notificationHeader.notificationObject->value,
			myNotificationParams.notificationObject.value,
			myNotificationParams.notificationObject.length);

	/* Set Notifying Object */
	myNotification.notificationHeader.notifyingObject->length
			= myNotificationParams.notifyingObject.length;
	(void) memcpy(myNotification.notificationHeader.notifyingObject->value,
			myNotificationParams.notifyingObject.value,
			myNotificationParams.notifyingObject.length);

	/* set Notification Class Identifier */
	/* vendor id 33333 is not an existing SNMP enterprise number.
	 Just an example */
	myNotification.notificationHeader.notificationClassId->vendorId
			= myNotificationParams.notificationClassId.vendorId;

	/* sub id of this notification class within "name space" of vendor ID */
	myNotification.notificationHeader.notificationClassId->majorId
			= myNotificationParams.notificationClassId.majorId;
	myNotification.notificationHeader.notificationClassId->minorId
			= myNotificationParams.notificationClassId.minorId;

	/* set additional text and additional info */
	(void) strncpy(myNotification.notificationHeader.additionalText,
			myNotificationParams.additionalText,
			myNotificationAllocationParams.lengthAdditionalText);

	/* Set source indicator */
	*myNotification.sourceIndicator
			= myNotificationParams.objectCreateDeleteSourceIndicator;

	/* Set objectAttibutes */
	myNotification.objectAttributes[0].attributeId
			= myNotificationParams.objectAttributes[0].attributeId;
	myNotification.objectAttributes[0].attributeType
			= myNotificationParams.objectAttributes[0].attributeType;
	myNotification.objectAttributes[0].attributeValue.int32Val
			= myNotificationParams.objectAttributes[0].attributeValue.int32Val;

	myNotificationParams.eventType
			= myNotificationParams.objectCreateDeleteEventType;
	fill_header_part(&myNotification.notificationHeader,
			(saNotificationParamsT *) &myNotificationParams,
			myNotificationAllocationParams.lengthAdditionalText);

	rc = saNtfNotificationSend(myNotification.notificationHandle);
	safassert(saNtfNotificationFree(myNotification.notificationHandle), SA_AIS_OK);
	safassert(saNtfFinalize(ntfHandle), SA_AIS_OK);
	test_validate(rc, SA_AIS_OK);
}


void saNtfNotificationSend_03(void) {
	SaNtfAttributeChangeNotificationT myNotification;
	saNotificationAllocationParamsT myNotificationAllocationParams;
	saNotificationFilterAllocationParamsT myNotificationFilterAllocationParams;
	saNotificationParamsT myNotificationParams;

	fillInDefaultValues(&myNotificationAllocationParams,
			&myNotificationFilterAllocationParams, &myNotificationParams);

	safassert(saNtfInitialize(&ntfHandle, &ntfSendCallbacks, &ntfVersion), SA_AIS_OK);

	safassert(saNtfAttributeChangeNotificationAllocate(
					ntfHandle, /* handle to Notification Service instance */
					&myNotification,
					/* number of correlated notifications */
					myNotificationAllocationParams.numCorrelatedNotifications,
					/* length of additional text */
					myNotificationAllocationParams.lengthAdditionalText,
					/* number of additional info items*/
					myNotificationAllocationParams.numAdditionalInfo,
					/* number of state changes */
					myNotificationAllocationParams.numAttributes,
					/* use default allocation size */
					myNotificationAllocationParams.variableDataSize), SA_AIS_OK);

	/* Event type */
	*(myNotification.notificationHeader.eventType) = SA_NTF_ATTRIBUTE_CHANGED;

	/* event time to be set automatically to current
	 time by saNtfNotificationSend */
	*(myNotification.notificationHeader.eventTime)
			= myNotificationParams.eventTime;

	/* set Notification Object */
	myNotification.notificationHeader.notificationObject->length
			= myNotificationParams.notificationObject.length;

	(void) memcpy(myNotification.notificationHeader.notificationObject->value,
			myNotificationParams.notificationObject.value,
			myNotificationParams.notificationObject.length);

	/* Set Notifying Object */
	myNotification.notificationHeader.notifyingObject->length
			= myNotificationParams.notifyingObject.length;

	(void) memcpy(myNotification.notificationHeader.notifyingObject->value,
			myNotificationParams.notifyingObject.value,
			myNotificationParams.notifyingObject.length);

	/* set Notification Class Identifier */
	/* vendor id 33333 is not an existing SNMP enterprise number.
	 Just an example */
	myNotification.notificationHeader.notificationClassId->vendorId
			= myNotificationParams.notificationClassId.vendorId;

	/* sub id of this notification class within "name space" of vendor ID */
	myNotification.notificationHeader.notificationClassId->majorId
			= myNotificationParams.notificationClassId.majorId;
	myNotification.notificationHeader.notificationClassId->minorId
			= myNotificationParams.notificationClassId.minorId;

	/* set additional text and additional info */
	(void) strncpy(myNotification.notificationHeader.additionalText,
			myNotificationParams.additionalText,
			myNotificationAllocationParams.lengthAdditionalText);

	/* Set source indicator */
	*myNotification.sourceIndicator
			= myNotificationParams.attributeChangeSourceIndicator;

	/* Set objectAttributes */
	myNotification.changedAttributes[0].attributeId
			= myNotificationParams.changedAttributes[0].attributeId;
	myNotification.changedAttributes[0].attributeType
				= myNotificationParams.changedAttributes[0].attributeType;
	myNotification.changedAttributes[0].newAttributeValue.int64Val
				= myNotificationParams.changedAttributes[0].newAttributeValue.int64Val;
	myNotification.changedAttributes[0].oldAttributePresent
				= myNotificationParams.changedAttributes[0].oldAttributePresent;
	myNotification.changedAttributes[0].oldAttributeValue.int64Val
				= myNotificationParams.changedAttributes[0].oldAttributeValue.int64Val;


	myNotificationParams.eventType
			= myNotificationParams.attributeChangeEventType;
	fill_header_part(&myNotification.notificationHeader,
			(saNotificationParamsT *) &myNotificationParams,
			myNotificationAllocationParams.lengthAdditionalText);

	/*  the magic */
	rc = saNtfNotificationSend(myNotification.notificationHandle);
	safassert(saNtfNotificationFree(myNotification.notificationHandle), SA_AIS_OK);
	safassert(saNtfFinalize(ntfHandle), SA_AIS_OK);
	test_validate(rc, SA_AIS_OK);

}

void saNtfNotificationSend_04(void) {
	SaNtfStateChangeNotificationT  myNotification;
	saNotificationAllocationParamsT myNotificationAllocationParams;
	saNotificationFilterAllocationParamsT myNotificationFilterAllocationParams;
	saNotificationParamsT myNotificationParams;

	fillInDefaultValues(&myNotificationAllocationParams,
			&myNotificationFilterAllocationParams, &myNotificationParams);

	safassert(saNtfInitialize(&ntfHandle, &ntfSendCallbacks, &ntfVersion), SA_AIS_OK);

	safassert(saNtfStateChangeNotificationAllocate(
					ntfHandle, /* handle to Notification Service instance */
					&myNotification,
					/* number of correlated notifications */
					myNotificationAllocationParams.numCorrelatedNotifications,
					/* length of additional text */
					myNotificationAllocationParams.lengthAdditionalText,
					/* number of additional info items*/
					myNotificationAllocationParams.numAdditionalInfo,
					/* number of state changes */
					myNotificationAllocationParams.numAttributes,
					/* use default allocation size */
					myNotificationAllocationParams.variableDataSize), SA_AIS_OK);

	/* Event type */
	*(myNotification.notificationHeader.eventType) = SA_NTF_OBJECT_STATE_CHANGE;

	/* event time to be set automatically to current
	 time by saNtfNotificationSend */
	*(myNotification.notificationHeader.eventTime)
			= myNotificationParams.eventTime;

	/* set Notification Class Identifier */
	/* vendor id 33333 is not an existing SNMP enterprise number.
	 Just an example */
	myNotification.notificationHeader.notificationClassId->vendorId
			= myNotificationParams.notificationClassId.vendorId;

	/* sub id of this notification class within "name space" of vendor ID */
	myNotification.notificationHeader.notificationClassId->majorId
			= myNotificationParams.notificationClassId.majorId;
	myNotification.notificationHeader.notificationClassId->minorId
			= myNotificationParams.notificationClassId.minorId;

	/* set additional text and additional info */
	(void) strncpy(myNotification.notificationHeader.additionalText,
			myNotificationParams.additionalText,
			myNotificationAllocationParams.lengthAdditionalText);

	/* Set source indicator */
	*myNotification.sourceIndicator
			= myNotificationParams.stateChangeSourceIndicator;

	/* Set objectAttributes */
	myNotification.changedStates[0].stateId
			= myNotificationParams.changedStates[0].stateId;
	myNotification.changedStates[0].oldStatePresent
				= myNotificationParams.changedStates[0].oldStatePresent;
	myNotification.changedStates[0].oldState
				= myNotificationParams.changedStates[0].oldState;
	myNotification.changedStates[0].newState
				= myNotificationParams.changedStates[0].newState;


	myNotificationParams.eventType
			= myNotificationParams.stateChangeEventType;
	fill_header_part(&myNotification.notificationHeader,
			(saNotificationParamsT *) &myNotificationParams,
			myNotificationAllocationParams.lengthAdditionalText);

	/*  the magic */
	rc = saNtfNotificationSend(myNotification.notificationHandle);
	safassert(saNtfNotificationFree(myNotification.notificationHandle), SA_AIS_OK);
	safassert(saNtfFinalize(ntfHandle), SA_AIS_OK);
	test_validate(rc, SA_AIS_OK);

}

void saNtfNotificationSend_05(void) {
	SaNtfSecurityAlarmNotificationT  myNotification;
	saNotificationAllocationParamsT myNotificationAllocationParams;
	saNotificationFilterAllocationParamsT myNotificationFilterAllocationParams;
	saNotificationParamsT myNotificationParams;

	fillInDefaultValues(&myNotificationAllocationParams,
			&myNotificationFilterAllocationParams, &myNotificationParams);

	safassert(saNtfInitialize(&ntfHandle, &ntfSendCallbacks, &ntfVersion), SA_AIS_OK);

	safassert(saNtfSecurityAlarmNotificationAllocate(
					ntfHandle, /* handle to Notification Service instance */
					&myNotification,
					/* number of correlated notifications */
					myNotificationAllocationParams.numCorrelatedNotifications,
					/* length of additional text */
					myNotificationAllocationParams.lengthAdditionalText,
					/* number of additional info items*/
					myNotificationAllocationParams.numAdditionalInfo,
					/* use default allocation size */
					myNotificationAllocationParams.variableDataSize), SA_AIS_OK);

	fill_header_part(&myNotification.notificationHeader,
			(saNotificationParamsT *) &myNotificationParams,
			myNotificationAllocationParams.lengthAdditionalText);

	/* Event type */
	*(myNotification.notificationHeader.eventType) = myNotificationParams.securityAlarmEventType;

	/* event time to be set automatically to current
	 time by saNtfNotificationSend */
	*(myNotification.notificationHeader.eventTime)
			= myNotificationParams.eventTime;

	/* set Notification Class Identifier */
	/* vendor id 33333 is not an existing SNMP enterprise number.
	 Just an example */
	myNotification.notificationHeader.notificationClassId->vendorId
			= myNotificationParams.notificationClassId.vendorId;

	/* sub id of this notification class within "name space" of vendor ID */
	myNotification.notificationHeader.notificationClassId->majorId
			= myNotificationParams.notificationClassId.majorId;
	myNotification.notificationHeader.notificationClassId->minorId
			= myNotificationParams.notificationClassId.minorId;

	/* Set severity  */
	*(myNotification.severity) = SA_NTF_SEVERITY_MAJOR;

	/* set additional text and additional info */
	(void) strncpy(myNotification.notificationHeader.additionalText,
			myNotificationParams.additionalText,
			myNotificationAllocationParams.lengthAdditionalText);


	/* Set probable cause */
	*(myNotification.probableCause) = myNotificationParams.probableCause;

	/* Set service user */
	myNotification.serviceUser->valueType = myNotificationParams.serviceUser.valueType;
	myNotification.serviceUser->value.int32Val = myNotificationParams.serviceUser.value.int32Val;

	/* Set service provider */
	myNotification.serviceProvider->valueType = myNotificationParams.serviceProvider.valueType;
	myNotification.serviceProvider->value.int32Val = myNotificationParams.serviceProvider.value.int32Val;

	/* Set alarm detector */
	myNotification.securityAlarmDetector->valueType = myNotificationParams.securityAlarmDetector.valueType;
	myNotification.securityAlarmDetector->value.int32Val = myNotificationParams.securityAlarmDetector.value.int32Val;

	/* set additional text and additional info */
	(void) strncpy(myNotification.notificationHeader.additionalText,
			myNotificationParams.additionalText,
			myNotificationAllocationParams.lengthAdditionalText);


	/*  the magic */
	rc = saNtfNotificationSend(myNotification.notificationHandle);
	safassert(saNtfNotificationFree(myNotification.notificationHandle), SA_AIS_OK);
	safassert(saNtfFinalize(ntfHandle), SA_AIS_OK);
	test_validate(rc, SA_AIS_OK);

}

void saNtfNotificationSend_06(void) {
	// TODO MiscellaneousNotification
	test_validate(SA_AIS_ERR_NOT_SUPPORTED, SA_AIS_OK);
}

__attribute__ ((constructor)) static void saNtfNotificationSend_constructor(
		void) {
	test_suite_add(8, "Producer API 3 send");
	test_case_add(8, saNtfNotificationSend_01, "saNtfNotificationSend Alarm");
	test_case_add(8, saNtfNotificationSend_02,
			"saNtfNotificationSend ObjectCreateDeleteNotification");
	test_case_add(8, saNtfNotificationSend_03,
			"saNtfNotificationSend AttributeChangeNotification");
	test_case_add(8, saNtfNotificationSend_04,
			"saNtfNotificationSend StateChangeNotification");
	test_case_add(8, saNtfNotificationSend_05,
			"saNtfNotificationSend SecurityAlarm");
	test_case_add(8, saNtfNotificationSend_06,
			"saNtfNotificationSend Miscellaneous");
}

