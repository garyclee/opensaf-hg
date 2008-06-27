/*******************************************************************************
**
** FILE:
**   saNtf.h
**
** DESCRIPTION: 
**   This file provides the C language binding for the Service 
**   Availability(TM) Forum Notification Service (NTF). 
**   It contains all of the prototypes and type definitions required 
**   for user API functions
**   
** SPECIFICATION VERSION:
**   SAI-AIS-NTF-A.01.01
**
** DATE: 
**   Fri  Nov   18  2005  
**
** LEGAL:
**   OWNERSHIP OF SPECIFICATION AND COPYRIGHTS. 
**   The Specification and all worldwide copyrights therein are
**   the exclusive property of Licensor.  You may not remove, obscure, or
**   alter any copyright or other proprietary rights notices that are in or
**   on the copy of the Specification you download.  You must reproduce all
**   such notices on all copies of the Specification you make.  Licensor
**   may make changes to the Specification, or to items referenced therein,
**   at any time without notice.  Licensor is not obligated to support or
**   update the Specification. 
**   
**   Copyright(c) 2005, Service Availability(TM) Forum. All rights
**   reserved. 
**
*******************************************************************************/

#ifndef _SA_NTF_H
#define _SA_NTF_H

#include <saAis.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*  3.12.1.1        SaNtfHandleT  */
typedef SaUint64T SaNtfHandleT;

/*  3.12.1.2        SaNtfNotificationHandleT  */
typedef SaUint64T SaNtfNotificationHandleT;

/*  3.12.1.3        SaNtfNotificationFilterHandleT  */
typedef SaUint64T SaNtfNotificationFilterHandleT;

/*  3.12.1.4        SaNtfReadHandleT */
typedef SaUint64T SaNtfReadHandleT;

/*  3.12.3  SaNtfNotificationTypeT  */
typedef enum
{
    SA_NTF_TYPE_OBJECT_CREATE_DELETE = 0x1000,
    SA_NTF_TYPE_ATTRIBUTE_CHANGE = 0x2000,
    SA_NTF_TYPE_STATE_CHANGE = 0x3000,
    SA_NTF_TYPE_ALARM = 0x4000,
    SA_NTF_TYPE_SECURITY_ALARM = 0x5000
} SaNtfNotificationTypeT;

/*  3.12.4  SaNtfEventTypeT  */
#define SA_NTF_NOTIFICATIONS_TYPE_MASK 0xF000

/*  Event types enum, these are only generic
 *  types as defined by the X.73x standards  
 */
typedef enum
{
    SA_NTF_OBJECT_NOTIFICATIONS_START = SA_NTF_TYPE_OBJECT_CREATE_DELETE,
    SA_NTF_OBJECT_CREATION,
    SA_NTF_OBJECT_DELETION,

    SA_NTF_ATTRIBUTE_NOTIFICATIONS_START = SA_NTF_TYPE_ATTRIBUTE_CHANGE,
    SA_NTF_ATTRIBUTE_ADDED,
    SA_NTF_ATTRIBUTE_REMOVED,
    SA_NTF_ATTRIBUTE_CHANGED,
    SA_NTF_ATTRIBUTE_RESET,

    SA_NTF_STATE_CHANGE_NOTIFICATIONS_START = SA_NTF_TYPE_STATE_CHANGE,
    SA_NTF_OBJECT_STATE_CHANGE,

    SA_NTF_ALARM_NOTIFICATIONS_START = SA_NTF_TYPE_ALARM,
    SA_NTF_ALARM_COMMUNICATION,
    SA_NTF_ALARM_QOS,
    SA_NTF_ALARM_PROCESSING,
    SA_NTF_ALARM_EQUIPMENT,
    SA_NTF_ALARM_ENVIRONMENT,

    SA_NTF_SECURITY_ALARM_NOTIFICATIONS_START = SA_NTF_TYPE_SECURITY_ALARM,
    SA_NTF_INTEGRITY_VIOLATION,
    SA_NTF_OPERATION_VIOLATION,
    SA_NTF_PHYSICAL_VIOLATION,
    SA_NTF_SECURITY_SERVICE_VIOLATION,
    SA_NTF_TIME_VIOLATION
} SaNtfEventTypeT;

/*  3.12.5  Notification Object  */
/*  Use SaNameT.  */

/*  3.12.6  Notifying Object  */
/*  Use SaNameT.  */

/*  3.12.7  SaNtfClassIdT  */
typedef struct 
{ 
    SaUint32T vendorId; 
    SaUint16T majorId;
    SaUint16T minorId;
} SaNtfClassIdT; 

#define SA_NTF_VENDOR_ID_SAF 18568

/* 3.12.8 SaServicesT */
/* defined in saAis.h */

/*  3.12.9  SaNtfElementIdT  */
typedef SaUint16T SaNtfElementIdT;

/*  3.12.10  SaNtfIdentifierT  */
/*typedef SaUint64T SaNtfIdentifierT;*/

#define SA_NTF_IDENTIFIER_UNUSED   ((SaNtfIdentifierT) 0)


/*  3.12.11 Event Time  */
/*  Use SaTimeT.  */

/*  3.12.12 SaNtfValueTypeT  */
typedef enum {
    SA_NTF_VALUE_UINT8,	    /* A byte long - unsigned int */
    SA_NTF_VALUE_INT8,	    /* A byte long - signed int */
    SA_NTF_VALUE_UINT16,    /* 2 bytes long - unsigned int */
    SA_NTF_VALUE_INT16,	    /* 2 bytes long - signed int */
    SA_NTF_VALUE_UINT32,    /* 4 bytes long - unsigned int */
    SA_NTF_VALUE_INT32,	    /* 4 bytes long - signed int */
    SA_NTF_VALUE_FLOAT,	    /* 4 bytes long - float */
    SA_NTF_VALUE_UINT64,    /* 8 bytes long - unsigned int */
    SA_NTF_VALUE_INT64,	    /* 8 bytes long - signed int */
    SA_NTF_VALUE_DOUBLE,    /* 8 bytes long - double */
    SA_NTF_VALUE_LDAP_NAME, /* SaNameT type */
    SA_NTF_VALUE_STRING,    /* '\0' terminated char array (UTF-8
			       encoded) */
    SA_NTF_VALUE_IPADDRESS, /* IPv4 or IPv6 address as '\0' terminated
			       char array */
    SA_NTF_VALUE_BINARY,    /* Binary data stored in bytes - number of
			       bytes stored separately */
    SA_NTF_VALUE_ARRAY	    /* Array of some data type - size of elements
			       and number of elements stored separately */
} SaNtfValueTypeT;

/*  3.12.13 SaNtfValueT  */
typedef union
{
    /* The first few are fixed size data types	*/
    SaUint8T	uint8Val;	/* SA_NTF_VALUE_UINT8 */
    SaInt8T	int8Val;	/* SA_NTF_VALUE_INT8 */
    SaUint16T	uint16Val;	/* SA_NTF_VALUE_UINT16 */
    SaInt16T	int16Val;	/* SA_NTF_VALUE_INT16 */
    SaUint32T	uint32Val;	/* SA_NTF_VALUE_UINT32 */
    SaInt32T	int32Val;	/* SA_NTF_VALUE_INT32 */
    SaFloatT	floatVal;	/* SA_NTF_VALUE_FLOAT */
    SaUint64T	uint64Val;	/* SA_NTF_VALUE_UINT64 */
    SaInt64T	int64Val;	/* SA_NTF_VALUE_INT64 */
    SaDoubleT	doubleVal;	/* SA_NTF_VALUE_DOUBLE */

    /* This struct can represent variable length fields like        
    * LDAP names, strings, IP addresses and binary data. 
    * It may only be used in conjunction with the data type values 
    * SA_NTF_VALUE_LDAP_NAME, SA_NTF_VALUE_STRING, 
    * SA_NTF_VALUE_IPADDRESS and SA_NTF_VALUE_BINARY.
    * This field shall not be directly accessed. 
    * To initialise this structure and to set a pointer to the real data   
    * use saNtfPtrValAllocate(). The function saNtfPtrValGet() shall be used        
    * for retrieval of the real data.
    */
    struct
    {
        SaUint16T dataOffset;
        SaUint16T dataSize;

    } ptrVal;

    /* This struct represents sets of data of identical type        
    * like notification identifiers, attributes  etc. 
    * It may only be used in conjunction with the data type value 
    * SA_NTF_VALUE_ARRAY. Functions        
    * SaNtfArrayValAllocate() or SaNtfArrayValGet() shall be used to       
    * get a pointer for accessing the real data. Direct access is not allowed.
    */
    struct
    {
        SaUint16T arrayOffset;
        SaUint16T numElements;
        SaUint16T elementSize;
    } arrayVal;

} SaNtfValueT;

/*  3.12.14 Additional Text  */
/*  Use SaStringT. A string consists of UTF-8 encoded characters and is
    terminated by the '\0' character. */

/*  3.12.15 SaNtfAdditionalInfoT  */
typedef struct
{
    SaNtfElementIdT infoId;
    /* API user is expected to define this field    */

    SaNtfValueTypeT infoType;

    SaNtfValueT infoValue;

} SaNtfAdditionalInfoT;

/*  3.12.28     SaNtfNotificationHeaderT  */
typedef struct 
{
    SaNtfEventTypeT *eventType;
    /* This will point to the event type in *
    * the internal notification structure   */

    SaNameT *notificationObject;
    /* This will point to the notification object   *
    * in the internal notification structure        */

    SaNameT *notifyingObject;
    /* This will point to the notifying object      *
    * in the internal notification structure     */

    SaNtfClassIdT *notificationClassId;
    /* This will point to the notification class identifier */

    SaTimeT *eventTime;
    /* Points to eventTime  */

    SaUint16T numCorrelatedNotifications;
    /* Number of correlated notifications in the notification       */

    SaUint16T lengthAdditionalText;
    /* Length of additional text    */

    SaUint16T numAdditionalInfo;
    /* Number of additional info fields     */

    SaNtfIdentifierT *notificationId;
    /* Points to the notification ID in             *
    * the internal notification structure   */

    SaNtfIdentifierT *correlatedNotifications;
    /* Points to the correlated             *
    * notification identifiers array        */

    SaStringT additionalText;
    /* Points to the additional text in     *
     * the internal notification structure (\0 terminated, UTF-8 encoded) */

    SaNtfAdditionalInfoT *additionalInfo;
    /* Points to the additional info array in       *
    * the internal notification structure   */

} SaNtfNotificationHeaderT;

/*  3.12.22     SaNtfSourceIndicatorT  */
typedef enum
{
    SA_NTF_OBJECT_OPERATION = 0,
    SA_NTF_UNKNOWN_OPERATION = 1,
    SA_NTF_MANAGEMENT_OPERATION = 2
} SaNtfSourceIndicatorT;

/*  3.12.25     SaNtfAttributeChangeT  */
typedef struct
{
    SaNtfElementIdT attributeId;
    /* API user is expected to define this field    */

    SaNtfValueTypeT attributeType;

    SaBoolT oldAttributePresent;
    SaNtfValueT oldAttributeValue;

    SaNtfValueT newAttributeValue;

} SaNtfAttributeChangeT;

/*  3.12.30 SaNtfAttributeChangeNotificationT  */
typedef struct
{
    SaNtfNotificationHandleT notificationHandle;
    /* A handle to the internal notification structure      */

    SaNtfNotificationHeaderT notificationHeader;
    /* Pointers to the notification header  */

    SaUint16T numAttributes;
    /* Number of changed attributes in the notification     */

    SaNtfSourceIndicatorT *sourceIndicator;
    /* Points to the source indicator       *
    * field in the internal notification structure  */

    SaNtfAttributeChangeT *changedAttributes;
    /* Points to changed attributes *
    * array in the internal notification structure  */

} SaNtfAttributeChangeNotificationT;

/*  3.12.24     SaNtfAttributeT  */
typedef struct
{
    SaNtfElementIdT attributeId;
    /* API user is expected to define this field    */

    SaNtfValueTypeT attributeType;

    SaNtfValueT attributeValue;

} SaNtfAttributeT;

/*  3.12.29 SaNtfObjectCreateDeleteNotificationT  */
typedef struct
{
    SaNtfNotificationHandleT notificationHandle;
    /* A handle to the internal notification structure      */

    SaNtfNotificationHeaderT notificationHeader;
    /* Notification header  */

    SaUint16T numAttributes;
    /* Number of object attributes in the notification      */

    SaNtfSourceIndicatorT *sourceIndicator;
    /* Points to the source indicator       *
    * field in the internal notification structure  */

    SaNtfAttributeT *objectAttributes;
    /* Pointer to attributes array in the internal notification structure   */

} SaNtfObjectCreateDeleteNotificationT;

/*  3.12.23     SaNtfStateChangeT  */
typedef struct
{
    SaNtfElementIdT stateId;
    SaBoolT oldStatePresent;
    SaUint16T oldState;
    SaUint16T newState;
} SaNtfStateChangeT;

/*  3.12.31 SaNtfStateChangeNotificationT  */
typedef struct
{
    SaNtfNotificationHandleT notificationHandle;
    /* A handle to the internal notification structure      */

    SaNtfNotificationHeaderT notificationHeader;
    /* Pointers to the notification header  */

    SaUint16T numStateChanges;
    /* Number of state changes in the notification  */

    SaNtfSourceIndicatorT *sourceIndicator;
    /* Points to the source indicator       *
    * field in the internal notification structure  */

    SaNtfStateChangeT *changedStates;
    /* Points to changed states array in the internal notification structure        */

} SaNtfStateChangeNotificationT;

/*  3.12.16     SaNtfProbableCauseT  */
typedef enum
{
    SA_NTF_ADAPTER_ERROR,
    SA_NTF_APPLICATION_SUBSYSTEM_FAILURE,
    SA_NTF_BANDWIDTH_REDUCED,
    SA_NTF_CALL_ESTABLISHMENT_ERROR,
    SA_NTF_COMMUNICATIONS_PROTOCOL_ERROR,
    SA_NTF_COMMUNICATIONS_SUBSYSTEM_FAILURE,
    SA_NTF_CONFIGURATION_OR_CUSTOMIZATION_ERROR,
    SA_NTF_CONGESTION,
    SA_NTF_CORRUPT_DATA,
    SA_NTF_CPU_CYCLES_LIMIT_EXCEEDED,
    SA_NTF_DATASET_OR_MODEM_ERROR,
    SA_NTF_DEGRADED_SIGNAL,
    SA_NTF_D_T_E,
    SA_NTF_ENCLOSURE_DOOR_OPEN,
    SA_NTF_EQUIPMENT_MALFUNCTION,
    SA_NTF_EXCESSIVE_VIBRATION,
    SA_NTF_FILE_ERROR,
    SA_NTF_FIRE_DETECTED,
    SA_NTF_FLOOD_DETECTED,
    SA_NTF_FRAMING_ERROR,
    SA_NTF_HEATING_OR_VENTILATION_OR_COOLING_SYSTEM_PROBLEM,
    SA_NTF_HUMIDITY_UNACCEPTABLE,
    SA_NTF_INPUT_OUTPUT_DEVICE_ERROR,
    SA_NTF_INPUT_DEVICE_ERROR,
    SA_NTF_L_A_N_ERROR,
    SA_NTF_LEAK_DETECTED,
    SA_NTF_LOCAL_NODE_TRANSMISSION_ERROR,
    SA_NTF_LOSS_OF_FRAME,
    SA_NTF_LOSS_OF_SIGNAL,
    SA_NTF_MATERIAL_SUPPLY_EXHAUSTED,
    SA_NTF_MULTIPLEXER_PROBLEM,
    SA_NTF_OUT_OF_MEMORY,
    SA_NTF_OUTPUT_DEVICE_ERROR,
    SA_NTF_PERFORMANCE_DEGRADED,
    SA_NTF_POWER_PROBLEM,
    SA_NTF_PRESSURE_UNACCEPTABLE,
    SA_NTF_PROCESSOR_PROBLEM,
    SA_NTF_PUMP_FAILURE,
    SA_NTF_QUEUE_SIZE_EXCEEDED,
    SA_NTF_RECEIVE_FAILURE,
    SA_NTF_RECEIVER_FAILURE,
    SA_NTF_REMOTE_NODE_TRANSMISSION_ERROR,
    SA_NTF_RESOURCE_AT_OR_NEARING_CAPACITY,
    SA_NTF_RESPONSE_TIME_EXCESSIVE,
    SA_NTF_RETRANSMISSION_RATE_EXCESSIVE,
    SA_NTF_SOFTWARE_ERROR,
    SA_NTF_SOFTWARE_PROGRAM_ABNORMALLY_TERMINATED,
    SA_NTF_SOFTWARE_PROGRAM_ERROR,
    SA_NTF_STORAGE_CAPACITY_PROBLEM,
    SA_NTF_TEMPERATURE_UNACCEPTABLE,
    SA_NTF_THRESHOLD_CROSSED,
    SA_NTF_TIMING_PROBLEM,
    SA_NTF_TOXIC_LEAK_DETECTED,
    SA_NTF_TRANSMIT_FAILURE,
    SA_NTF_TRANSMITTER_FAILURE,
    SA_NTF_UNDERLYING_RESOURCE_UNAVAILABLE,
    SA_NTF_VERSION_MISMATCH,
    SA_NTF_AUTHENTICATION_FAILURE,
    SA_NTF_BREACH_OF_CONFIDENTIALITY,
    SA_NTF_CABLE_TAMPER,
    SA_NTF_DELAYED_INFORMATION,
    SA_NTF_DENIAL_OF_SERVICE,
    SA_NTF_DUPLICATE_INFORMATION,
    SA_NTF_INFORMATION_MISSING,
    SA_NTF_INFORMATION_MODIFICATION_DETECTED,
    SA_NTF_INFORMATION_OUT_OF_SEQUENCE,
    SA_NTF_INTRUSION_DETECTION,
    SA_NTF_KEY_EXPIRED,
    SA_NTF_NON_REPUDIATION_FAILURE,
    SA_NTF_OUT_OF_HOURS_ACTIVITY,
    SA_NTF_OUT_OF_SERVICE,
    SA_NTF_PROCEDURAL_ERROR,
    SA_NTF_UNAUTHORIZED_ACCESS_ATTEMPT,
    SA_NTF_UNEXPECTED_INFORMATION,
    SA_NTF_UNSPECIFIED_REASON

} SaNtfProbableCauseT;

/*  3.12.17 SaNtfSpecificProblemT  */
typedef struct
{
    SaNtfElementIdT problemId;
    /* API user is expected to define this field    */

    SaNtfClassIdT problemClassId;
    /* optional field to identify problemId values from other NCIs,
    needed for correlation between clear and non-clear alarms */

    SaNtfValueTypeT problemType;

    SaNtfValueT problemValue;

} SaNtfSpecificProblemT;

/*  3.12.18     SaNtfSeverityT  */
typedef enum
{
    SA_NTF_SEVERITY_CLEARED, /* alarm notification, only */
    SA_NTF_SEVERITY_INDETERMINATE,
    SA_NTF_SEVERITY_WARNING,
    SA_NTF_SEVERITY_MINOR,
    SA_NTF_SEVERITY_MAJOR,
    SA_NTF_SEVERITY_CRITICAL
} SaNtfSeverityT;

/*  3.12.19     SaNtfSeverityTrendT  */
typedef enum
{
    SA_NTF_TREND_MORE_SEVERE,
    SA_NTF_TREND_NO_CHANGE,
    SA_NTF_TREND_LESS_SEVERE
} SaNtfSeverityTrendT;

/*  3.12.20 SaNtfThresholdInformationT  */
typedef struct
{
    SaNtfElementIdT thresholdId;
    /* API user is expected to define this field    */

    SaNtfValueTypeT thresholdValueType;

    SaNtfValueT thresholdValue;

    SaNtfValueT thresholdHysteresis;
    /* This has to be of the same type as threshold */

    SaNtfValueT observedValue;

    SaTimeT armTime;

} SaNtfThresholdInformationT;

/*  3.12.21     SaNtfProposedRepairActionT  */
typedef struct
{
    SaNtfElementIdT actionId;
    /* API user is expected to define this field    */

    SaNtfValueTypeT actionValueType;

    SaNtfValueT actionValue;

} SaNtfProposedRepairActionT;

/*  3.12.32     SaNtfAlarmNotificationT  */
typedef struct 
{
    SaNtfNotificationHandleT notificationHandle;
    /* A handle to the internal notification structure  */

    SaNtfNotificationHeaderT notificationHeader;
    /* Pointers to the notification header  */

    SaUint16T numSpecificProblems;
    /* Number of specific problems  */

    SaUint16T numMonitoredAttributes;
    /* Number of monitored attributes       */

    SaUint16T numProposedRepairActions;
    /* Number of proposed repair actions    */

    SaNtfProbableCauseT *probableCause;
    /* Points to the probable cause field   */

    SaNtfSpecificProblemT *specificProblems;
    /* Points to the array of specific problems     */

    SaNtfSeverityT *perceivedSeverity;
    /* Points to perceived severity */

    SaNtfSeverityTrendT *trend;
    /* Points to trend of severity  */

    SaNtfThresholdInformationT *thresholdInformation;
    /* Points to the threshold information field    */

    SaNtfAttributeT *monitoredAttributes;
    /* Monitored attributes array   */

    SaNtfProposedRepairActionT *proposedRepairActions;
    /* Proposed repair actions array */

} SaNtfAlarmNotificationT;

/*  3.12.27     SaNtfSecurityAlarmDetectorT  */
typedef struct
{
    SaNtfValueTypeT valueType;

    SaNtfValueT value;

} SaNtfSecurityAlarmDetectorT;

/*  3.12.26     SaNtfServiceUserT  */
typedef struct
{
    SaNtfValueTypeT valueType;

    SaNtfValueT value;

} SaNtfServiceUserT;

/*  3.12.33 SaNtfSecurityAlarmNotificationT  */
typedef struct
{
    SaNtfNotificationHandleT notificationHandle;
    /* A handle to the internal notification structure  */

    SaNtfNotificationHeaderT notificationHeader;
    /* Pointers to the notification header  */

    SaNtfProbableCauseT *probableCause;
    /* Points to the probable cause field   */

    SaNtfSeverityT *severity;
    /* Points to severity field     */

    SaNtfSecurityAlarmDetectorT *securityAlarmDetector;
    /* Pointer to the alarm detector field  */

    SaNtfServiceUserT*serviceUser;
    /* Pointer to the service user field    */

    SaNtfServiceUserT *serviceProvider;
    /* Pointer to the service user field    */

} SaNtfSecurityAlarmNotificationT;


/*  3.12.34 Default variable notification data size  */

/* Default for API user when not sure how much memory is in sum needed to accommodate the variable size data */
#define SA_NTF_ALLOC_SYSTEM_LIMIT    (-1)

/*  3.12.35     SaNtfSubscriptionIdT  */
typedef SaUint32T SaNtfSubscriptionIdT; 

/*  3.12.36     SaNtfNotificationFilterHeaderT  */
typedef struct 
{
    SaUint16T numEventTypes;
    /* number of event types */

    SaNtfEventTypeT *eventTypes;
    /* the array of event types */

    SaUint16T numNotificationObjects;
    /* number of notification objects */

    SaNameT *notificationObjects;
    /* the array of notification objects */

    SaUint16T numNotifyingObjects;
    /* number of notifying objects */

    SaNameT *notifyingObjects;
    /* the array of notifying objects */

    SaUint16T numNotificationClassIds;
    /* number of notification class ids */

    SaNtfClassIdT *notificationClassIds;
    /* the array of notification class ids */

} SaNtfNotificationFilterHeaderT;

/*  3.12.37 SaNtfObjectCreateDeleteNotificationFilterT  */
typedef struct
{
    SaNtfNotificationFilterHandleT notificationFilterHandle;
    /* a handle to the internal notification filter structure */

    SaNtfNotificationFilterHeaderT notificationFilterHeader;
    /* the notification filter header */

    SaUint16T numSourceIndicators;
    /* number of source indicators */

    SaNtfSourceIndicatorT *sourceIndicators;
    /* the array of source indicators */

} SaNtfObjectCreateDeleteNotificationFilterT;

/*  3.12.38 SaNtfAttributeChangeNotificationFilterT  */
typedef struct
{
    SaNtfNotificationFilterHandleT notificationFilterHandle;
    /* a handle to the internal notification filter structure */

    SaNtfNotificationFilterHeaderT notificationFilterHeader;
    /* the notification filter header */

    SaUint16T numSourceIndicators;
    /* number of source indicators */

    SaNtfSourceIndicatorT *sourceIndicators;
    /* the array of source indicators */

} SaNtfAttributeChangeNotificationFilterT;

/*  3.12.39 SaNtfStateChangeNotificationFilterT  */
typedef struct
{
    SaNtfNotificationFilterHandleT notificationFilterHandle;
    /* a handle to the internal notification filter structure */

    SaNtfNotificationFilterHeaderT notificationFilterHeader;
    /* the notification filter header */

    SaUint16T numSourceIndicators;
    /* number of source indicators */

    SaNtfSourceIndicatorT *sourceIndicators;
    /* the array of source indicators */

    SaUint16T numStateChanges;
    /* number of state changes */

    SaNtfStateChangeT *changedStates;
    /* the array of changed states */

} SaNtfStateChangeNotificationFilterT;

/*  3.12.40 SaNtfAlarmNotificationFilterT  */
typedef struct 
{
    SaNtfNotificationFilterHandleT notificationFilterHandle;
    /* a handle to the internal notification filter structure */

    SaNtfNotificationFilterHeaderT notificationFilterHeader;
    /* the notification filter header */

    SaUint16T numProbableCauses;
    /* number of probable causes */

    SaUint16T numPerceivedSeverities;
    /* number of perceived severities */

    SaUint16T numTrends;
    /* number of severity trends */

    SaNtfProbableCauseT *probableCauses;
    /* the array of probable causes */

    SaNtfSeverityT *perceivedSeverities;
    /* the array of perceived severities */

    SaNtfSeverityTrendT *trends;
    /* the array of severity trends */

} SaNtfAlarmNotificationFilterT;

/*  3.12.41 SaNtfSecurityAlarmNotificationFilterT  */
typedef struct
{
    SaNtfNotificationFilterHandleT notificationFilterHandle;
    /* a handle to the internal notification filter structure */

    SaNtfNotificationFilterHeaderT notificationFilterHeader;
    /* the notification filter header */

    SaUint16T numProbableCauses;
    /* number of probable causes */

    SaUint16T numSeverities;
    /* number of severities */

    SaUint16T numSecurityAlarmDetectors;
    /* number of security alarm detectors */

    SaUint16T numServiceUsers;
    /* number of service users */

    SaUint16T numServiceProviders;
    /* number of service providers */

    SaNtfProbableCauseT *probableCauses;
    /* the array of probable causes */

    SaNtfSeverityT *severities;
    /* the array of severities */

    SaNtfSecurityAlarmDetectorT *securityAlarmDetectors;
    /* the array of security alarm detectors */

    SaNtfServiceUserT *serviceUsers;
    /* the array of service users */

    SaNtfServiceUserT *serviceProviders;
    /* the array of service providers */

} SaNtfSecurityAlarmNotificationFilterT;

/*  3.12.42 SaNtfSearchModeT  */
typedef enum
{
    SA_NTF_SEARCH_BEFORE_OR_AT_TIME = 1,
    SA_NTF_SEARCH_AT_TIME = 2,
    SA_NTF_SEARCH_AT_OR_AFTER_TIME = 3,
    SA_NTF_SEARCH_BEFORE_TIME = 4,
    SA_NTF_SEARCH_AFTER_TIME = 5,
    SA_NTF_SEARCH_NOTIFICATION_ID = 6,
    SA_NTF_SEARCH_ONLY_FILTER = 7
} SaNtfSearchModeT;

/*  3.12.43     SaNtfSearchCriteriaT  */
typedef struct
{
    SaNtfSearchModeT searchMode;
    /* indicates the search mode */

    SaTimeT eventTime;
    /* event time (relevant only if searchMode is one of SA_NTF_SEARCH_*_TIME) */

    SaNtfIdentifierT notificationId;
    /* notification ID (relevant only if searchMode is SA_NTF_SEARCH_NOTIFICATION_ID) */
} SaNtfSearchCriteriaT;

/*  3.12.44     SaNtfSearchDirectionT  */
typedef enum
{
    SA_NTF_SEARCH_OLDER = 1,
    SA_NTF_SEARCH_YOUNGER = 2
} SaNtfSearchDirectionT;

/*  3.12.45     SaNtfNotificationTypeFilterHandlesT  */
typedef struct
{
    SaNtfNotificationFilterHandleT objectCreateDeleteFilterHandle;
    SaNtfNotificationFilterHandleT attributeChangeFilterHandle;
    SaNtfNotificationFilterHandleT stateChangeFilterHandle;
    SaNtfNotificationFilterHandleT alarmFilterHandle;
    SaNtfNotificationFilterHandleT securityAlarmFilterHandle;
} SaNtfNotificationTypeFilterHandlesT;

#define SA_NTF_FILTER_HANDLE_NULL       ((SaNtfNotificationFilterHandleT) NULL)

#define SA_NTF_TYPE_FILTER_HANDLE_NULL       ((SaNtfNotificationTypeFilterHandlesT) NULL)

/*  3.12.46     SaNtfNotificationsT  */
typedef struct
{
    SaNtfNotificationTypeT notificationType;
    union
    {
        SaNtfObjectCreateDeleteNotificationT objectCreateDeleteNotification;
        SaNtfAttributeChangeNotificationT    attributeChangeNotification;
        SaNtfStateChangeNotificationT        stateChangeNotification;
        SaNtfAlarmNotificationT              alarmNotification;
        SaNtfSecurityAlarmNotificationT      securityAlarmNotification;
    } notification;
} SaNtfNotificationsT;

/* 3.15.3.3 SaNtfNotificationCallbackT */
typedef void (*SaNtfNotificationCallbackT)(
    SaNtfSubscriptionIdT subscriptionId,
    const SaNtfNotificationsT *notification);

/* 3.15.3.4 SaNtfNotificationDiscardedCallbackT */
typedef void (*SaNtfNotificationDiscardedCallbackT)(
    SaNtfSubscriptionIdT subscriptionId,
    SaNtfNotificationTypeT notificationType,
    SaUint32T numberDiscarded,
    const SaNtfIdentifierT *discardedNotificationIdentifiers);

/*  3.12.2.1        SaNtfCallbacksT  */
typedef struct {
    SaNtfNotificationCallbackT saNtfNotificationCallback;
    SaNtfNotificationDiscardedCallbackT saNtfNotificationDiscardedCallback;
} SaNtfCallbacksT;

/*  3.13    Library Life Cycle  */

/*  3.13.1  saNtfInitialize()  */
SaAisErrorT
saNtfInitialize(
                SaNtfHandleT *ntfHandle,
                const SaNtfCallbacksT *ntfCallbacks,
                SaVersionT *version);

/*  3.13.2  saNtfSelectionObjectGet()  */
SaAisErrorT
saNtfSelectionObjectGet(
                        SaNtfHandleT ntfHandle,
                        SaSelectionObjectT *selectionObject);

/*  3.13.3  saNtfDispatch()  */
SaAisErrorT 
saNtfDispatch(
              SaNtfHandleT ntfHandle,
              SaDispatchFlagsT dispatchFlags);

/*  3.13.4  saNtfFinalize()  */
SaAisErrorT saNtfFinalize(
                          SaNtfHandleT ntfHandle);

/*  3.14        Producer Operations  */
/*  3.14.1      saNtfObjectCreateDeleteNotificationAllocate()  */
SaAisErrorT 
saNtfObjectCreateDeleteNotificationAllocate( 
    SaNtfHandleT ntfHandle,
    SaNtfObjectCreateDeleteNotificationT *notification,
    SaUint16T numCorrelatedNotifications,
    SaUint16T lengthAdditionalText,
    SaUint16T numAdditionalInfo,
    SaUint16T numAttributes,
    SaInt16T variableDataSize);

/*  3.14.2      saNtfAttributeChangeNotificationAllocate()  */
SaAisErrorT 
saNtfAttributeChangeNotificationAllocate( 
    SaNtfHandleT ntfHandle,
    SaNtfAttributeChangeNotificationT *notification,
    SaUint16T numCorrelatedNotifications,
    SaUint16T lengthAdditionalText,
    SaUint16T numAdditionalInfo,
    SaUint16T numAttributes,
    SaInt16T variableDataSize);

/*  3.14.3      saNtfStateChangeNotificationAllocate()  */
SaAisErrorT 
saNtfStateChangeNotificationAllocate( 
    SaNtfHandleT ntfHandle,
    SaNtfStateChangeNotificationT *notification,
    SaUint16T numCorrelatedNotifications,
    SaUint16T lengthAdditionalText,
    SaUint16T numAdditionalInfo,
    SaUint16T numStateChanges,
    SaInt16T variableDataSize);

/*  3.14.4      saNtfAlarmNotificationAllocate()  */
SaAisErrorT saNtfAlarmNotificationAllocate( 
    SaNtfHandleT ntfHandle,
    SaNtfAlarmNotificationT *notification,
    SaUint16T numCorrelatedNotifications,
    SaUint16T lengthAdditionalText,
    SaUint16T numAdditionalInfo,
    SaUint16T numSpecificProblems,
    SaUint16T numMonitoredAttributes,
    SaUint16T numProposedRepairActions,
    SaInt16T variableDataSize);

/*  3.14.5	saNtfSecurityAlarmNotificationAllocate()  */
SaAisErrorT saNtfSecurityAlarmNotificationAllocate( 
    SaNtfHandleT ntfHandle,
    SaNtfSecurityAlarmNotificationT *notification,
    SaUint16T numCorrelatedNotifications,
    SaUint16T lengthAdditionalText,
    SaUint16T numAdditionalInfo,
    SaInt16T variableDataSize);	

/*  3.14.6	saNtfPtrValAllocate()  */
SaAisErrorT saNtfPtrValAllocate(
    SaNtfNotificationHandleT notificationHandle,
    SaUint16T dataSize,
    void **dataPtr,
    SaNtfValueT *value);

/*  3.14.7	saNtfArrayValAllocate()  */
SaAisErrorT saNtfArrayValAllocate(
    SaNtfNotificationHandleT notificationHandle,
    SaUint16T numElements,
    SaUint16T elementSize,
    void **arrayPtr,
    SaNtfValueT *value);

/*  3.14.8	saNtfNotificationSend()  */
SaAisErrorT saNtfNotificationSend(
    SaNtfNotificationHandleT notificationHandle);

/*  3.14.9	saNtfNotificationFree()  */
SaAisErrorT saNtfNotificationFree(
    SaNtfNotificationHandleT notificationHandle);

/*  3.15	Consumer Operations  */
/*  3.15.2	Common Operations  */
/*  3.15.2.1	saNtfLocalizedMessageGet()  */
SaAisErrorT saNtfLocalizedMessageGet(
    SaNtfNotificationHandleT notificationHandle,
    SaStringT *message);

/*  3.15.2.2 saNtfLocalizedMessageFree() */
SaAisErrorT saNtfLocalizedMessageFree(
    SaStringT message);

/*  3.15.2.3	saNtfPtrValGet()  */
SaAisErrorT saNtfPtrValGet(
    SaNtfNotificationHandleT notificationHandle,
    SaNtfValueT *value,
    void **dataPtr,
    SaUint16T *dataSize);

/*  3.15.2.4	saNtfArrayValGet()  */
SaAisErrorT saNtfArrayValGet(
    SaNtfNotificationHandleT notificationHandle,
    SaNtfValueT *value,
    void **arrayPtr,
    SaUint16T *numElements,
    SaUint16T *elementSize);

/* 3.15.2.5  saNtfObjectCreateDeleteNotificationFilterAllocate() */
SaAisErrorT saNtfObjectCreateDeleteNotificationFilterAllocate( 
    SaNtfHandleT ntfHandle,
    SaNtfObjectCreateDeleteNotificationFilterT *notificationFilter,
    SaUint16T numEventTypes,
    SaUint16T numNotificationObjects,
    SaUint16T numNotifyingObjects,
    SaUint16T numNotificationClassIds,
    SaUint16T numSourceIndicators);

/*  3.15.2.6	saNtfAttributeChangeNotificationFilterAllocate()  */
SaAisErrorT saNtfAttributeChangeNotificationFilterAllocate( 
    SaNtfHandleT ntfHandle,
    SaNtfAttributeChangeNotificationFilterT *notificationFilter,
    SaUint16T numEventTypes,
    SaUint16T numNotificationObjects,
    SaUint16T numNotifyingObjects,
    SaUint16T numNotificationClassIds,
    SaUint32T numSourceIndicators);

/*  3.15.2.7	saNtfStateChangeNotificationFilterAllocate()  */
SaAisErrorT saNtfStateChangeNotificationFilterAllocate( 
    SaNtfHandleT ntfHandle,
    SaNtfStateChangeNotificationFilterT *notificationFilter,
    SaUint16T numEventTypes,
    SaUint16T numNotificationObjects,
    SaUint16T numNotifyingObjects,
    SaUint16T numNotificationClassIds,
    SaUint32T numSourceIndicators,
    SaUint32T numChangedStates);

/*  3.15.2.8	saNtfAlarmNotificationFilterAllocate()  */
SaAisErrorT saNtfAlarmNotificationFilterAllocate( 
    SaNtfHandleT ntfHandle,
    SaNtfAlarmNotificationFilterT *notificationFilter,
    SaUint16T numEventTypes,
    SaUint16T numNotificationObjects,
    SaUint16T numNotifyingObjects,
    SaUint16T numNotificationClassIds,
    SaUint32T numProbableCauses,
    SaUint32T numPerceivedSeverities,
    SaUint32T numTrends);

/*  3.15.2.9	saNtfSecurityAlarmNotificationFilterAllocate()  */
SaAisErrorT saNtfSecurityAlarmNotificationFilterAllocate( 
    SaNtfHandleT ntfHandle,
    SaNtfSecurityAlarmNotificationFilterT *notificationFilter,
    SaUint16T numEventTypes,
    SaUint16T numNotificationObjects,
    SaUint16T numNotifyingObjects,
    SaUint16T numNotificationClassIds,
    SaUint32T numProbableCauses,
    SaUint32T numSeverities,
    SaUint32T numSecurityAlarmDetectors,
    SaUint32T numServiceUsers,
    SaUint32T numServiceProviders);

/*  3.15.2.10	saNtfNotificationFilterFree()  */
SaAisErrorT saNtfNotificationFilterFree(
    SaNtfNotificationFilterHandleT notificationFilterHandle);

/*  3.15.3	Subscriber Operations   */
/*  3.15.3.1	saNtfNotificationSubscribe()  */
SaAisErrorT saNtfNotificationSubscribe(
    const SaNtfNotificationTypeFilterHandlesT *notificationFilterHandles,
    SaNtfSubscriptionIdT subscriptionId);

/*  3.15.3.2	saNtfNotificationUnsubscribe()  */
SaAisErrorT saNtfNotificationUnsubscribe(
    SaNtfSubscriptionIdT subscriptionId);

/*  3.15.4	 Reader Operations  */
/*  3.15.4.1	saNtfNotificationReadInitialize()  */
SaAisErrorT saNtfNotificationReadInitialize(
	SaNtfSearchCriteriaT searchCriteria,
	const SaNtfNotificationTypeFilterHandlesT *notificationFilterHandles,
	SaNtfReadHandleT *readHandle);

/*  3.15.4.2	saNtfNotificationReadNext()  */
SaAisErrorT saNtfNotificationReadNext(
	SaNtfReadHandleT readHandle,
	SaNtfSearchDirectionT searchDirection,
	SaNtfNotificationsT *notification);

/*  3.15.4.2	saNtfNotificationReadFinalize()  */
SaAisErrorT saNtfNotificationReadFinalize(
    	SaNtfReadHandleT readhandle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SA_NTF_H */

