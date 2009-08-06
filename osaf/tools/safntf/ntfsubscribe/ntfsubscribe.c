
/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2009 The OpenSAF Foundation
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
 *   NTF subscriber command line program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <time.h>
#include <fcntl.h>
#include <ctype.h>
#include <poll.h>

#include <saAis.h>
#include <saNtf.h>
#include <ntfclient.h>

static SaNtfHandleT ntfHandle;
static SaSelectionObjectT selObj;

/* Name of current testproxy (argv[0]) */
static char *progname;

/* Release code, major version, minor version */
static SaVersionT version = { 'A', 0x01, 0x01 };

/* help functions for printouts */
static void exitIfFalse(int expression)
{				/* instead of assert */
	if (!expression) {
		exit(EXIT_FAILURE);
	}
}

static const char *sa_probable_cause_list[] = {
	"SA_NTF_ADAPTER_ERROR",
	"SA_NTF_APPLICATION_SUBSYSTEM_FAILURE",
	"SA_NTF_BANDWIDTH_REDUCED",
	"SA_NTF_CALL_ESTABLISHMENT_ERROR",
	"SA_NTF_COMMUNICATIONS_PROTOCOL_ERROR",
	"SA_NTF_COMMUNICATIONS_SUBSYSTEM_FAILURE",
	"SA_NTF_CONFIGURATION_OR_CUSTOMIZATION_ERROR",
	"SA_NTF_CONGESTION",
	"SA_NTF_CORRUPT_DATA",
	"SA_NTF_CPU_CYCLES_LIMIT_EXCEEDED",
	"SA_NTF_DATASET_OR_MODEM_ERROR",
	"SA_NTF_DEGRADED_SIGNAL",
	"SA_NTF_D_T_E",
	"SA_NTF_ENCLOSURE_DOOR_OPEN",
	"SA_NTF_EQUIPMENT_MALFUNCTION",
	"SA_NTF_EXCESSIVE_VIBRATION",
	"SA_NTF_FILE_ERROR",
	"SA_NTF_FIRE_DETECTED",
	"SA_NTF_FLOOD_DETECTED",
	"SA_NTF_FRAMING_ERROR",
	"SA_NTF_HEATING_OR_VENTILATION_OR_COOLING_SYSTEM_PROBLEM",
	"SA_NTF_HUMIDITY_UNACCEPTABLE",
	"SA_NTF_INPUT_OUTPUT_DEVICE_ERROR",
	"SA_NTF_INPUT_DEVICE_ERROR",
	"SA_NTF_L_A_N_ERROR",
	"SA_NTF_LEAK_DETECTED",
	"SA_NTF_LOCAL_NODE_TRANSMISSION_ERROR",
	"SA_NTF_LOSS_OF_FRAME",
	"SA_NTF_LOSS_OF_SIGNAL",
	"SA_NTF_MATERIAL_SUPPLY_EXHAUSTED",
	"SA_NTF_MULTIPLEXER_PROBLEM",
	"SA_NTF_OUT_OF_MEMORY",
	"SA_NTF_OUTPUT_DEVICE_ERROR",
	"SA_NTF_PERFORMANCE_DEGRADED",
	"SA_NTF_POWER_PROBLEM",
	"SA_NTF_PRESSURE_UNACCEPTABLE",
	"SA_NTF_PROCESSOR_PROBLEM",
	"SA_NTF_PUMP_FAILURE",
	"SA_NTF_QUEUE_SIZE_EXCEEDED",
	"SA_NTF_RECEIVE_FAILURE",
	"SA_NTF_RECEIVER_FAILURE",
	"SA_NTF_REMOTE_NODE_TRANSMISSION_ERROR",
	"SA_NTF_RESOURCE_AT_OR_NEARING_CAPACITY",
	"SA_NTF_RESPONSE_TIME_EXCESSIVE",
	"SA_NTF_RETRANSMISSION_RATE_EXCESSIVE",
	"SA_NTF_SOFTWARE_ERROR",
	"SA_NTF_SOFTWARE_PROGRAM_ABNORMALLY_TERMINATED",
	"SA_NTF_SOFTWARE_PROGRAM_ERROR",
	"SA_NTF_STORAGE_CAPACITY_PROBLEM",
	"SA_NTF_TEMPERATURE_UNACCEPTABLE",
	"SA_NTF_THRESHOLD_CROSSED",
	"SA_NTF_TIMING_PROBLEM",
	"SA_NTF_TOXIC_LEAK_DETECTED",
	"SA_NTF_TRANSMIT_FAILURE",
	"SA_NTF_TRANSMITTER_FAILURE",
	"SA_NTF_UNDERLYING_RESOURCE_UNAVAILABLE",
	"SA_NTF_VERSION_MISMATCH",
	"SA_NTF_AUTHENTICATION_FAILURE",
	"SA_NTF_BREACH_OF_CONFIDENTIALITY",
	"SA_NTF_CABLE_TAMPER",
	"SA_NTF_DELAYED_INFORMATION",
	"SA_NTF_DENIAL_OF_SERVICE",
	"SA_NTF_DUPLICATE_INFORMATION",
	"SA_NTF_INFORMATION_MISSING",
	"SA_NTF_INFORMATION_MODIFICATION_DETECTED",
	"SA_NTF_INFORMATION_OUT_OF_SEQUENCE",
	"SA_NTF_INTRUSION_DETECTION",
	"SA_NTF_KEY_EXPIRED",
	"SA_NTF_NON_REPUDIATION_FAILURE",
	"SA_NTF_OUT_OF_HOURS_ACTIVITY",
	"SA_NTF_OUT_OF_SERVICE",
	"SA_NTF_PROCEDURAL_ERROR",
	"SA_NTF_UNAUTHORIZED_ACCESS_ATTEMPT",
	"SA_NTF_UNEXPECTED_INFORMATION",
	"SA_NTF_UNSPECIFIED_REASON",
};

static const char *sa_severity_list[] = {
	"SA_NTF_SEVERITY_CLEARED",
	"SA_NTF_SEVERITY_INDETERMINATE",
	"SA_NTF_SEVERITY_WARNING",
	"SA_NTF_SEVERITY_MINOR",
	"SA_NTF_SEVERITY_MAJOR",
	"SA_NTF_SEVERITY_CRITICAL",
};

static const char *sa_alarm_event_type_list[] = {
	"SA_NTF_ALARM_NOTIFICATIONS_START",
	"SA_NTF_ALARM_COMMUNICATION",
	"SA_NTF_ALARM_QOS",
	"SA_NTF_ALARM_PROCESSING",
	"SA_NTF_ALARM_EQUIPMENT",
	"SA_NTF_ALARM_ENVIRONMENT",
};

static const char *sa_state_change_event_type_list[] = {
	"SA_NTF_STATE_CHANGE_NOTIFICATIONS_START",
	"SA_NTF_OBJECT_STATE_CHANGE",
};

static const char *sa_object_create_delete_event_type_list[] = {
	"SA_NTF_OBJECT_NOTIFICATIONS_START",
	"SA_NTF_OBJECT_CREATION",
	"SA_NTF_OBJECT_DELETION",
};

static const char *sa_attribute_change_event_type_list[] = {
	"SA_NTF_ATTRIBUTE_NOTIFICATIONS_START",
	"SA_NTF_ATTRIBUTE_ADDED",
	"SA_NTF_ATTRIBUTE_REMOVED",
	"SA_NTF_ATTRIBUTE_CHANGED",
	"SA_NTF_ATTRIBUTE_RESET",
};

static const char *sa_security_alarm_event_type_list[] = {
	"SA_NTF_SECURITY_ALARM_NOTIFICATIONS_START",
	"SA_NTF_INTEGRITY_VIOLATION",
	"SA_NTF_OPERATION_VIOLATION",
	"SA_NTF_PHYSICAL_VIOLATION",
	"SA_NTF_SECURITY_SERVICE_VIOLATION",
	"SA_NTF_TIME_VIOLATION",
};

static const char *sa_source_indicator_list[] = {
	"SA_NTF_OBJECT_OPERATION",
	"SA_NTF_UNKNOWN_OPERATION",
	"SA_NTF_MANAGEMENT_OPERATION",
};

static char *error_output(SaAisErrorT result)
{
	static char error_result[256];

	sprintf(error_result, "error: %u", result);
	return (error_result);
}

static void print_severity(SaNtfSeverityT input)
{
	exitIfFalse(input >= SA_NTF_SEVERITY_CLEARED);
	exitIfFalse(input <= SA_NTF_SEVERITY_CRITICAL);

	printf("%s\n", (char *)sa_severity_list[input]);
}

static void print_probable_cause(SaNtfProbableCauseT input)
{
	exitIfFalse(input >= SA_NTF_ADAPTER_ERROR);
	exitIfFalse(input <= SA_NTF_UNSPECIFIED_REASON);

	printf("%s\n", (char *)sa_probable_cause_list[input]);
}

static void print_event_type(SaNtfEventTypeT input, SaNtfNotificationTypeT notificationType)
{
	int listIndex;

	switch (notificationType) {
	case SA_NTF_TYPE_STATE_CHANGE:
		if (input >= (int)SA_NTF_STATE_CHANGE_NOTIFICATIONS_START) {
			listIndex = (int)input - (int)SA_NTF_TYPE_STATE_CHANGE;

			exitIfFalse(input >= SA_NTF_STATE_CHANGE_NOTIFICATIONS_START);
			exitIfFalse(input <= SA_NTF_OBJECT_STATE_CHANGE);

			printf("%s\n", (char *)sa_state_change_event_type_list[listIndex]);

		}
		break;

	case SA_NTF_TYPE_ALARM:
		if (input >= (int)SA_NTF_ALARM_NOTIFICATIONS_START) {
			listIndex = (int)input - (int)SA_NTF_TYPE_ALARM;

			exitIfFalse(input >= SA_NTF_ALARM_NOTIFICATIONS_START);
			exitIfFalse(input <= SA_NTF_ALARM_ENVIRONMENT);

			printf("%s\n", (char *)sa_alarm_event_type_list[listIndex]);
		}
		break;

	case SA_NTF_TYPE_OBJECT_CREATE_DELETE:
		if (input >= (int)SA_NTF_OBJECT_NOTIFICATIONS_START) {
			listIndex = (int)input - (int)SA_NTF_TYPE_OBJECT_CREATE_DELETE;

			exitIfFalse(input >= SA_NTF_OBJECT_NOTIFICATIONS_START);
			exitIfFalse(input <= SA_NTF_OBJECT_DELETION);

			printf("%s\n", (char *)sa_object_create_delete_event_type_list[listIndex]);
		}
		break;

	case SA_NTF_TYPE_ATTRIBUTE_CHANGE:
		if (input >= (int)SA_NTF_ATTRIBUTE_NOTIFICATIONS_START) {
			listIndex = (int)input - (int)SA_NTF_TYPE_ATTRIBUTE_CHANGE;

			exitIfFalse(input >= SA_NTF_ATTRIBUTE_NOTIFICATIONS_START);
			exitIfFalse(input <= SA_NTF_ATTRIBUTE_RESET);

			printf("%s\n", (char *)sa_attribute_change_event_type_list[listIndex]);
		}
		break;

	case SA_NTF_TYPE_SECURITY_ALARM:
		if (input >= (int)SA_NTF_SECURITY_ALARM_NOTIFICATIONS_START) {
			listIndex = (int)input - (int)SA_NTF_TYPE_SECURITY_ALARM;

			exitIfFalse(input >= SA_NTF_SECURITY_ALARM_NOTIFICATIONS_START);
			exitIfFalse(input <= SA_NTF_TIME_VIOLATION);

			printf("%s\n", (char *)sa_security_alarm_event_type_list[listIndex]);
		}
		break;

	default:
		printf("Unknown Notification Type!!");
		exit(1);
		break;
	}
}

static void print_change_states(SaNtfStateChangeT *input)
{
	printf("- State ID: %d -\n", input->stateId);

	if (input->oldStatePresent == SA_TRUE) {
		printf("Old State Present: Yes\n");
		printf("Old State: %d\n", input->oldState);
	} else {
		printf("Old State Present: No\n");
	}
	printf("New State: %d\n", input->newState);
}

static void print_object_attributes(SaNtfAttributeT *input)
{
	printf("- Attribute ID: %d -\n", (int)input->attributeId);
	printf("Attribute Type: %d\n", (int)input->attributeType);
	printf("Attribute Value: %d\n", (int)input->attributeValue.int32Val);
}

static void print_changed_attributes(SaNtfAttributeChangeT *input)
{
	printf("- Attribute ID: %d -\n", input->attributeId);

	printf("Attribute Type: %d\n", input->attributeType);
	if (input->oldAttributePresent == SA_TRUE) {
		printf("Old Attribute Present: Yes\n");
		printf("Old Attribute: %d\n", input->oldAttributeValue.int32Val);
	} else {
		printf("Old Attribute Present: No\n");
	}
	printf("New Attribute Value: %d\n", input->newAttributeValue.int32Val);
}

static void print_security_alarm_types(SaNtfSecurityAlarmNotificationT *input)
{
	printf("Security Alarm Detector Type: %d\n", input->securityAlarmDetector->valueType);
	printf("Security Alarm Detector Value: %d\n", input->securityAlarmDetector->value.int32Val);

	printf("Service User Type: %d\n", input->serviceUser->valueType);
	printf("Service User Value: %d\n", input->serviceUser->value.int32Val);

	printf("Service Provider Type: %d\n", input->serviceProvider->valueType);
	printf("Service Provider Value: %d\n", input->serviceProvider->value.int32Val);
}

static void print_source_indicator(SaNtfSourceIndicatorT input)
{
	exitIfFalse(input >= SA_NTF_OBJECT_OPERATION);
	exitIfFalse(input <= SA_NTF_MANAGEMENT_OPERATION);

	printf("%s\n", (char *)sa_source_indicator_list[input]);
}

/* end help functions for printouts */

static void print_header(SaNtfNotificationHeaderT *notificationHeader,
			 SaNtfSubscriptionIdT subscriptionId, SaNtfNotificationTypeT notificationType)
{
	SaTimeT totalTime;
	SaTimeT ntfTime = (SaTimeT)0;
	char time_str[24];
	char tmpObj[SA_MAX_NAME_LENGTH + 1];

	/* Notification ID */
	printf("notificationID = %d\n", (int)*(notificationHeader->notificationId));

	printf("subscriptionId = %u\n", (unsigned int)subscriptionId);

	/* Event type */
	printf("eventType = ");
	print_event_type(*notificationHeader->eventType, notificationType);

	/* Notification Object */
	printf("notificationObject.length = %u\n", notificationHeader->notificationObject->length);
	strncpy(tmpObj,
		(char *)notificationHeader->notificationObject->value, notificationHeader->notificationObject->length);
	tmpObj[notificationHeader->notificationObject->length] = '\0';
	printf("notificationObject value: \"%s\"\n", tmpObj);

	/* Notifying Object */
	strncpy(tmpObj,
		(char *)notificationHeader->notifyingObject->value, notificationHeader->notifyingObject->length);
	tmpObj[notificationHeader->notifyingObject->length] = '\0';
	printf("notifyingObject.length = %u\n", notificationHeader->notifyingObject->length);

	printf("notifyingObject value: \"%s\"\n", tmpObj);

	/* Notification Class ID */
	printf("VendorID = %d\nmajorID = %d\nminorID = %d\n",
	       notificationHeader->notificationClassId->vendorId,
	       notificationHeader->notificationClassId->majorId, notificationHeader->notificationClassId->minorId);

	/* Event Time */
	ntfTime = *notificationHeader->eventTime;

	totalTime = (ntfTime / (SaTimeT)SA_TIME_ONE_SECOND);
	(void)strftime(time_str, sizeof(time_str), "%d-%m-%Y %T", localtime((const time_t *)&totalTime));

	printf("eventTime = %lld = %s\n", (SaTimeT)ntfTime, time_str);

	/* Additional text */
	printf("additionalText = \"%s\"\n", notificationHeader->additionalText);

}

static void saNtfNotificationCallback(SaNtfSubscriptionIdT subscriptionId, const SaNtfNotificationsT *notification)
{
	SaInt32T i;
	SaNtfNotificationHandleT notificationHandle;
	switch (notification->notificationType) {
	case SA_NTF_TYPE_ALARM:
		printf("===  notificationType: alarm notification ===\n");
		notificationHandle = notification->notification.alarmNotification.notificationHandle;

		print_header((SaNtfNotificationHeaderT *)&notification->notification.alarmNotification.
			     notificationHeader, subscriptionId, notification->notificationType);

		/* Probable Cause */
		printf("probableCause = ");
		print_probable_cause(*(notification->notification.alarmNotification.probableCause));

		printf("perceivedSeverity = ");
		print_severity(*(notification->notification.alarmNotification.perceivedSeverity));

		break;

	case SA_NTF_TYPE_STATE_CHANGE:
		printf("===  notificationType:  state change notification ===\n");
		notificationHandle = notification->notification.stateChangeNotification.notificationHandle;

		print_header((SaNtfNotificationHeaderT *)&notification->notification.stateChangeNotification.
			     notificationHeader, subscriptionId, notification->notificationType);

		printf("sourceIndicator = ");
		print_source_indicator(*(notification->notification.stateChangeNotification.sourceIndicator));

		printf("Num of StateChanges: %d\n", notification->notification.stateChangeNotification.numStateChanges);

		/* Changed states */
		for (i = 0; i < notification->notification.stateChangeNotification.numStateChanges; i++) {

			print_change_states(&notification->notification.stateChangeNotification.changedStates[i]);
		}
		break;

	case SA_NTF_TYPE_OBJECT_CREATE_DELETE:
		printf("===  notificationType: object create/delete notification ===\n");
		notificationHandle = notification->notification.objectCreateDeleteNotification.notificationHandle;

		print_header((SaNtfNotificationHeaderT *)&notification->notification.objectCreateDeleteNotification.
			     notificationHeader, subscriptionId, notification->notificationType);

		printf("sourceIndicator = ");
		print_source_indicator(*(notification->notification.objectCreateDeleteNotification.sourceIndicator));
		printf("numAttributes: %d\n", notification->notification.objectCreateDeleteNotification.numAttributes);
		/* Object Attributes */
		for (i = 0; i < notification->notification.objectCreateDeleteNotification.numAttributes; i++) {

			print_object_attributes(&notification->notification.
						objectCreateDeleteNotification.objectAttributes[i]);
		}
		break;

	case SA_NTF_TYPE_ATTRIBUTE_CHANGE:
		printf("===  notificationType: attribute change notification ===\n");
		notificationHandle = notification->notification.attributeChangeNotification.notificationHandle;
		print_header((SaNtfNotificationHeaderT *)&notification->notification.attributeChangeNotification.
			     notificationHeader, subscriptionId, notification->notificationType);

		printf("sourceIndicator = ");
		print_source_indicator(*(notification->notification.attributeChangeNotification.sourceIndicator));
		printf("numAttributes: %d\n", notification->notification.attributeChangeNotification.numAttributes);
		/* Changed Attributes */
		for (i = 0; i < notification->notification.attributeChangeNotification.numAttributes; i++) {

			print_changed_attributes(&notification->notification.
						 attributeChangeNotification.changedAttributes[i]);
		}
		break;

	case SA_NTF_TYPE_SECURITY_ALARM:
		printf("===  notificationType:  security alarm notification ===\n");
		notificationHandle = notification->notification.securityAlarmNotification.notificationHandle;
		print_header((SaNtfNotificationHeaderT *)&notification->notification.securityAlarmNotification.
			     notificationHeader, subscriptionId, notification->notificationType);

		printf("probableCause = ");
		print_probable_cause(*(notification->notification.securityAlarmNotification.probableCause));

		printf("Severity = ");
		print_severity(*(notification->notification.securityAlarmNotification.severity));

		print_security_alarm_types((SaNtfSecurityAlarmNotificationT *)&notification->notification.
					   securityAlarmNotification);

		break;

	default:
		printf("unknown notification type %d", (int)notification->notificationType);
		break;
	}

	switch (notification->notificationType) {
	case SA_NTF_TYPE_ALARM:
		saNtfNotificationFree(notification->notification.alarmNotification.notificationHandle);
		break;
	case SA_NTF_TYPE_SECURITY_ALARM:
		saNtfNotificationFree(notification->notification.securityAlarmNotification.notificationHandle);
		break;
	case SA_NTF_TYPE_STATE_CHANGE:
		saNtfNotificationFree(notification->notification.stateChangeNotification.notificationHandle);
		break;
	case SA_NTF_TYPE_OBJECT_CREATE_DELETE:
		saNtfNotificationFree(notification->notification.objectCreateDeleteNotification.notificationHandle);
		break;
	case SA_NTF_TYPE_ATTRIBUTE_CHANGE:
		saNtfNotificationFree(notification->notification.attributeChangeNotification.notificationHandle);
		break;

	default:
		printf("wrong type");
		assert(0);
	}
	printf("\n");
}

static void saNtfNotificationDiscardedCallback(SaNtfSubscriptionIdT subscriptionId,
					       SaNtfNotificationTypeT notificationType,
					       SaUint32T numberDiscarded,
					       const SaNtfIdentifierT *discardedNotificationIdentifiers)
{
	unsigned int i = 0;

	printf("Discarded callback function  notificationType: %d\n\
                  subscriptionId  : %u \n\
                  numberDiscarded : %u\n", (int)notificationType, (unsigned int)subscriptionId, (unsigned int)numberDiscarded);
	for (i = 0; i < numberDiscarded; i++)
		printf("[%u]", (unsigned int)discardedNotificationIdentifiers[i]);

	printf("\n");
}

static SaNtfCallbacksT ntfCallbacks = {
	saNtfNotificationCallback,
	saNtfNotificationDiscardedCallback
};

static SaAisErrorT waitForNotifications(SaNtfHandleT myHandle, int selectionObject, int timeout_ms)
{
	SaAisErrorT error;
	int rv;
	struct pollfd fds[1];

	fds[0].fd = (int)selectionObject;
	fds[0].events = POLLIN;

	for (;;) {
 poll_retry:
		rv = poll(fds, 1, timeout_ms);

		if (rv == EINTR)
			goto poll_retry;

		if (rv == -1) {
			fprintf(stderr, "poll FAILED: %s\n", strerror(errno));
			return SA_AIS_ERR_BAD_OPERATION;
		}

		if (rv == 0) {
			printf("poll timeout\n");
			return SA_AIS_OK;
		}

		do {
			error = saNtfDispatch(myHandle, SA_DISPATCH_ALL);
			if (SA_AIS_ERR_TRY_AGAIN == error)
				sleep(1);
		} while (SA_AIS_ERR_TRY_AGAIN == error);

		if (error != SA_AIS_OK)
			fprintf(stderr, "saNtfDispatch Error %d\n", error);
	}

	return error;
}

static void usage(void)
{
	printf("\nNAME\n");
	printf("\t%s - subscribe for notfifications\n", progname);

	printf("\nSYNOPSIS\n");
	printf("\t%s subscribe [-t timeout]\n", progname);

	printf("\nDESCRIPTION\n");
	printf("\t%s is a SAF NTF client used to subscribe for all incoming notifications.\n", progname);
	printf("\nOPTIONS\n");
	printf("  -t or --timeout=TIME                      timeout (sec) waiting for notification\n");
	printf("  -h or --help                              this help\n");
	exit((int)SA_AIS_ERR_INVALID_PARAM);
}

/* Subscribe */
static SaAisErrorT subscribeForNotifications(const saNotificationFilterAllocationParamsT
					     *notificationFilterAllocationParams, SaNtfSubscriptionIdT subscriptionId)
{
	SaAisErrorT errorCode = SA_AIS_OK;
	SaNtfAlarmNotificationFilterT myAlarmFilter;
	SaNtfNotificationTypeFilterHandlesT notificationFilterHandles;

	errorCode = saNtfAlarmNotificationFilterAllocate(ntfHandle,
							 &myAlarmFilter,
							 notificationFilterAllocationParams->numEventTypes,
							 notificationFilterAllocationParams->numNotificationObjects,
							 notificationFilterAllocationParams->numNotifyingObjects,
							 notificationFilterAllocationParams->numNotificationClassIds,
							 notificationFilterAllocationParams->numProbableCauses,
							 notificationFilterAllocationParams->numPerceivedSeverities,
							 notificationFilterAllocationParams->numTrends);

	if (errorCode != SA_AIS_OK) {
		fprintf(stderr, "saNtfAlarmNotificationFilterAllocate failed - %s\n", error_output(errorCode));
		return errorCode;
	}

	/* Set perceived severities */
	myAlarmFilter.perceivedSeverities[0] = SA_NTF_SEVERITY_WARNING;
	myAlarmFilter.perceivedSeverities[1] = SA_NTF_SEVERITY_CLEARED;

	/* Initialize filter handles */
	notificationFilterHandles.alarmFilterHandle = myAlarmFilter.notificationFilterHandle;
	notificationFilterHandles.attributeChangeFilterHandle = SA_NTF_FILTER_HANDLE_NULL;
	notificationFilterHandles.objectCreateDeleteFilterHandle = SA_NTF_FILTER_HANDLE_NULL;
	notificationFilterHandles.securityAlarmFilterHandle = SA_NTF_FILTER_HANDLE_NULL;
	notificationFilterHandles.stateChangeFilterHandle = SA_NTF_FILTER_HANDLE_NULL;

	errorCode = saNtfNotificationSubscribe(&notificationFilterHandles, subscriptionId);
	if (SA_AIS_OK != errorCode) {
		fprintf(stderr, "saNtfNotificationSubscribe failed - %s\n", error_output(errorCode));
		return errorCode;
	}

	errorCode = saNtfNotificationFilterFree(notificationFilterHandles.alarmFilterHandle);
	if (SA_AIS_OK != errorCode) {
		fprintf(stderr, "saNtfNotificationFilterFree failed - %s\n", error_output(errorCode));
		return errorCode;
	}

	return errorCode;
}

int main(int argc, char *argv[])
{
	int c;
	SaAisErrorT error;
	int timeout = -1;	/* block indefintively in poll */
	saNotificationFilterAllocationParamsT notificationFilterAllocationParams;
	SaNtfSubscriptionIdT subscriptionId = 1;
	struct option long_options[] = {
		{"timeout", required_argument, 0, 't'},
		{0, 0, 0, 0}
	};

	progname = argv[0];

	notificationFilterAllocationParams.numEventTypes = 0;
	notificationFilterAllocationParams.numNotificationObjects = 0;
	notificationFilterAllocationParams.numNotifyingObjects = 0;
	notificationFilterAllocationParams.numNotificationClassIds = 0;
	notificationFilterAllocationParams.numProbableCauses = 0;
	notificationFilterAllocationParams.numPerceivedSeverities = 2;
	notificationFilterAllocationParams.numTrends = 0;

	/* Check options */
	while (1) {
		c = getopt_long(argc, argv, "ht:", long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 't':
			timeout = atoi(optarg) * 1000;
			break;
		case 'h':
		case '?':
		default:
			usage();
			break;
		}
	}

	error = saNtfInitialize(&ntfHandle, &ntfCallbacks, &version);
	if (SA_AIS_OK != error) {
		fprintf(stderr, "saNtfInitialize failed - %s\n", error_output(error));
		exit(EXIT_FAILURE);
	}

	error = saNtfSelectionObjectGet(ntfHandle, &selObj);
	if (SA_AIS_OK != error) {
		fprintf(stderr, "saNtfSelectionObjectGet failed - %s\n", error_output(error));
		exit(EXIT_FAILURE);
	}

	error = subscribeForNotifications(&notificationFilterAllocationParams, subscriptionId);
	if (SA_AIS_OK != error) {
		fprintf(stderr, "subscribeForNotifications failed - %s\n", error_output(error));
		exit(EXIT_FAILURE);
	}

	error = waitForNotifications(ntfHandle, selObj, timeout);
	if (SA_AIS_OK != error) {
		fprintf(stderr, "subscribeForNotifications failed - %s\n", error_output(error));
		exit(EXIT_FAILURE);
	}

	error = saNtfNotificationUnsubscribe(subscriptionId);
	if (SA_AIS_OK != error) {
		fprintf(stderr, "saNtfNotificationUnsubscribe failed - %s\n", error_output(error));
		exit(EXIT_FAILURE);
	}

	error = saNtfFinalize(ntfHandle);
	if (SA_AIS_OK != error) {
		fprintf(stderr, "saNtfFinalize failed - %s\n", error_output(error));
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
