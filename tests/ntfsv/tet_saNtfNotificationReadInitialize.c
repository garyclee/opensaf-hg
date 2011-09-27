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

/* Parameter struct instances */
static saNotificationAllocationParamsT myNotificationAllocationParams;
static saNotificationFilterAllocationParamsT
		myNotificationFilterAllocationParams;
static saNotificationParamsT myNotificationParams;

void saNtfNotificationReadInitialize_01(SaNtfSearchModeT sMode, SaAisErrorT expectedRC) {
	SaNtfHandleT ntfHandle;
	SaNtfSearchCriteriaT searchCriteria;
	SaNtfAlarmNotificationFilterT myAlarmFilter;
	SaNtfNotificationTypeFilterHandlesT myNotificationFilterHandles = {0,0,0,0,0};
	SaNtfReadHandleT readHandle;

	searchCriteria.searchMode = sMode;

	fillInDefaultValues(&myNotificationAllocationParams,
			&myNotificationFilterAllocationParams, &myNotificationParams);

	safassert(saNtfInitialize(&ntfHandle, &ntfCallbacks, &ntfVersion), SA_AIS_OK);

	safassert(saNtfAlarmNotificationFilterAllocate(
					ntfHandle, /* handle to Notification Service instance */
					&myAlarmFilter, /* put filter here */
					/* number of event types */
					myNotificationFilterAllocationParams.numEventTypes,
					/* number of notification objects */
					myNotificationFilterAllocationParams.numNotificationObjects,
					/* number of notifying objects */
					myNotificationFilterAllocationParams.numNotifyingObjects,
					/* number of notification class ids */
					myNotificationFilterAllocationParams.numNotificationClassIds,
					/* number of probable causes */
					myNotificationFilterAllocationParams.numProbableCauses,
					/* number of perceived severities */
					myNotificationFilterAllocationParams.numPerceivedSeverities,
					/* number of trend indications */
					myNotificationFilterAllocationParams.numTrends), SA_AIS_OK);

	myNotificationFilterHandles.alarmFilterHandle
			= myAlarmFilter.notificationFilterHandle;
	myAlarmFilter.perceivedSeverities[0] = SA_NTF_SEVERITY_WARNING;
	myAlarmFilter.perceivedSeverities[1] = SA_NTF_SEVERITY_CLEARED;

	rc = saNtfNotificationReadInitialize(searchCriteria,
			&myNotificationFilterHandles, &readHandle);

	safassert(saNtfFinalize(ntfHandle), SA_AIS_OK);
	free(myNotificationParams.additionalText);
	test_validate(rc, expectedRC);
}

void saNtfNotificationReadInitialize_01_1(void) {
	saNtfNotificationReadInitialize_01(SA_NTF_SEARCH_AT_OR_AFTER_TIME, SA_AIS_OK);
}
void saNtfNotificationReadInitialize_01_2(void) {
	saNtfNotificationReadInitialize_01(SA_NTF_SEARCH_AT_TIME, SA_AIS_OK);
}

void saNtfNotificationReadInitialize_01_3(void) {
	saNtfNotificationReadInitialize_01(SA_NTF_SEARCH_AT_OR_AFTER_TIME, SA_AIS_OK);
}

void saNtfNotificationReadInitialize_01_4(void) {
	saNtfNotificationReadInitialize_01(SA_NTF_SEARCH_BEFORE_TIME, SA_AIS_OK);
}

void saNtfNotificationReadInitialize_01_5(void) {
	saNtfNotificationReadInitialize_01(SA_NTF_SEARCH_AFTER_TIME, SA_AIS_OK);
}

void saNtfNotificationReadInitialize_01_6(void) {
	saNtfNotificationReadInitialize_01(SA_NTF_SEARCH_NOTIFICATION_ID, SA_AIS_OK);
}

void saNtfNotificationReadInitialize_01_7(void) {
	saNtfNotificationReadInitialize_01(SA_NTF_SEARCH_ONLY_FILTER, SA_AIS_OK);
}

void saNtfNotificationReadInitialize_01_8(void) {
	saNtfNotificationReadInitialize_01(SA_NTF_SEARCH_ONLY_FILTER + 10, SA_AIS_ERR_INVALID_PARAM);
}

void saNtfNotificationReadInitialize_01_9(void) {
	saNtfNotificationReadInitialize_01(-1, SA_AIS_ERR_INVALID_PARAM);
}

/**
 * Test of NULL pointer exception.
 */
void saNtfNotificationReadInitialize_02(void) {
	SaNtfHandleT ntfHandle;
	SaNtfSearchCriteriaT searchCriteria;
	SaNtfNotificationTypeFilterHandlesT *myNotificationFilterHandles = NULL;
	SaNtfReadHandleT readHandle;

	searchCriteria.searchMode = SA_NTF_SEARCH_AT_OR_AFTER_TIME;
	safassert(saNtfInitialize(&ntfHandle, &ntfCallbacks, &ntfVersion), SA_AIS_OK);

	rc = saNtfNotificationReadInitialize(searchCriteria,
			myNotificationFilterHandles, &readHandle);

	safassert(saNtfFinalize(ntfHandle), SA_AIS_OK);
	test_validate(rc, SA_AIS_ERR_INVALID_PARAM);
}

/**
 * Test of freed handler.
 */
void saNtfNotificationReadInitialize_03(void) {
	SaNtfHandleT ntfHandle;
	SaNtfSearchCriteriaT searchCriteria;
	SaNtfAlarmNotificationFilterT myAlarmFilter;
	SaNtfNotificationTypeFilterHandlesT myNotificationFilterHandles = {0,0,0,0,0};
	SaNtfReadHandleT readHandle;

	searchCriteria.searchMode = SA_NTF_SEARCH_AT_OR_AFTER_TIME;

	fillInDefaultValues(&myNotificationAllocationParams,
			&myNotificationFilterAllocationParams, &myNotificationParams);

	safassert(saNtfInitialize(&ntfHandle, &ntfCallbacks, &ntfVersion), SA_AIS_OK);

	safassert(saNtfAlarmNotificationFilterAllocate(
					ntfHandle, /* handle to Notification Service instance */
					&myAlarmFilter, /* put filter here */
					/* number of event types */
					myNotificationFilterAllocationParams.numEventTypes,
					/* number of notification objects */
					myNotificationFilterAllocationParams.numNotificationObjects,
					/* number of notifying objects */
					myNotificationFilterAllocationParams.numNotifyingObjects,
					/* number of notification class ids */
					myNotificationFilterAllocationParams.numNotificationClassIds,
					/* number of probable causes */
					myNotificationFilterAllocationParams.numProbableCauses,
					/* number of perceived severities */
					myNotificationFilterAllocationParams.numPerceivedSeverities,
					/* number of trend indications */
					myNotificationFilterAllocationParams.numTrends), SA_AIS_OK);

	myNotificationFilterHandles.alarmFilterHandle
			= myAlarmFilter.notificationFilterHandle;

	safassert(saNtfNotificationFilterFree(myAlarmFilter.notificationFilterHandle), SA_AIS_OK);

	rc = saNtfNotificationReadInitialize(searchCriteria,
			&myNotificationFilterHandles, &readHandle);

	safassert(saNtfFinalize(ntfHandle), SA_AIS_OK);
	free(myNotificationParams.additionalText);
	test_validate(rc, SA_AIS_ERR_BAD_HANDLE);
}

/* TODO Not implemented yet */
#if 0
void saNtfNotificationReadInitialize_04(void) {
	SaNtfHandleT ntfHandle;
	SaNtfSearchCriteriaT searchCriteria;
	SaNtfAlarmNotificationFilterT myAlarmFilter;
	SaNtfObjectCreateDeleteNotificationFilterT myObjCrDeFilter;
	SaNtfNotificationTypeFilterHandlesT myNotificationFilterHandles = {0,0,0,0,0};
	SaNtfReadHandleT readHandle;

	searchCriteria.searchMode = SA_NTF_SEARCH_AT_OR_AFTER_TIME;

	fillInDefaultValues(&myNotificationAllocationParams,
			&myNotificationFilterAllocationParams, &myNotificationParams);

	safassert(saNtfInitialize(&ntfHandle, &ntfCallbacks, &ntfVersion),SA_AIS_OK);

	safassert(saNtfAlarmNotificationFilterAllocate(
					ntfHandle, /* handle to Notification Service instance */
					&myAlarmFilter, /* put filter here */
					/* number of event types */
					myNotificationFilterAllocationParams.numEventTypes,
					/* number of notification objects */
					myNotificationFilterAllocationParams.numNotificationObjects,
					/* number of notifying objects */
					myNotificationFilterAllocationParams.numNotifyingObjects,
					/* number of notification class ids */
					myNotificationFilterAllocationParams.numNotificationClassIds,
					/* number of probable causes */
					myNotificationFilterAllocationParams.numProbableCauses,
					/* number of perceived severities */
					myNotificationFilterAllocationParams.numPerceivedSeverities,
					/* number of trend indications */
					myNotificationFilterAllocationParams.numTrends), SA_AIS_OK);

	safassert(saNtfObjectCreateDeleteNotificationFilterAllocate(
					ntfHandle, /* handle to Notification Service instance */
					&myObjCrDeFilter, /* put filter here */
					/* number of event types */
					myNotificationFilterAllocationParams.numEventTypes,
					/* number of notification objects */
					myNotificationFilterAllocationParams.numNotificationObjects,
					/* number of notifying objects */
					myNotificationFilterAllocationParams.numNotifyingObjects,
					/* number of notification class ids */
					myNotificationFilterAllocationParams.numNotificationClassIds,
					/* number of source indicators */
					0), SA_AIS_OK);

	myNotificationFilterHandles.alarmFilterHandle
			= myAlarmFilter.notificationFilterHandle;

	myNotificationFilterHandles.alarmFilterHandle
			= myObjCrDeFilter.notificationFilterHandle;

	rc = saNtfNotificationReadInitialize(searchCriteria,
			&myNotificationFilterHandles, &readHandle);

	safassert(saNtfFinalize(ntfHandle), SA_AIS_OK);
	free(myNotificationParams.additionalText);
	test_validate(rc, SA_AIS_OK);
}
#endif



__attribute__ ((constructor)) static void saNtfNotificationReadInitialize_constructor(
		void) {
	test_suite_add(20, "Consumer operations - Reader API 1");
	test_case_add(20, saNtfNotificationReadInitialize_01_1,
			"saNtfNotificationReadInitialize searchCriteria SA_NTF_SEARCH_BEFORE_OR_AT_TIME");
	test_case_add(20, saNtfNotificationReadInitialize_01_2,
			"saNtfNotificationReadInitialize searchCriteria SA_NTF_SEARCH_AT_TIME");
	test_case_add(20, saNtfNotificationReadInitialize_01_3,
			"saNtfNotificationReadInitialize searchCriteria SA_NTF_SEARCH_AT_OR_AFTER_TIME");
	test_case_add(20, saNtfNotificationReadInitialize_01_4,
			"saNtfNotificationReadInitialize searchCriteria SA_NTF_SEARCH_BEFORE_TIME");
	test_case_add(20, saNtfNotificationReadInitialize_01_5,
			"saNtfNotificationReadInitialize searchCriteria SA_NTF_SEARCH_AFTER_TIME");
	test_case_add(20, saNtfNotificationReadInitialize_01_6,
			"saNtfNotificationReadInitialize searchCriteria SA_NTF_SEARCH_NOTIFICATION_ID");
	test_case_add(20, saNtfNotificationReadInitialize_01_7,
			"saNtfNotificationReadInitialize searchCriteria SA_NTF_SEARCH_ONLY_FILTER");
	test_case_add(20, saNtfNotificationReadInitialize_01_8,
			"saNtfNotificationReadInitialize searchCriteria invalid SaNtfSearchModeT too big");
	test_case_add(20, saNtfNotificationReadInitialize_01_9,
			"saNtfNotificationReadInitialize searchCriteria invalid SaNtfSearchModeT -1");
	
	test_case_add(20, saNtfNotificationReadInitialize_02,
			"saNtfNotificationReadInitialize filter NULL pointer SA_AIS_ERR_INVALID_PARAM");
	test_case_add(20, saNtfNotificationReadInitialize_03,
			"saNtfNotificationReadInitialize filterHandle freed SA_AIS_ERR_BAD_HANDLE");
/*	test_case_add(20, saNtfNotificationReadInitialize_04,
			"saNtfNotificationReadInitialize multiple filters SA_AIS_OK"); */
}
