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

#include <string.h>
#include "ntfa.h"
#include "ntfsv_mem.h"

#define NCS_SAF_MIN_ACCEPT_TIME 10
#define NTFS_WAIT_TIME 1000

/* Macro to validate the dispatch flags */
#define m_DISPATCH_FLAG_IS_VALID(flag) \
   ( (SA_DISPATCH_ONE == flag) || \
     (SA_DISPATCH_ALL == flag) || \
     (SA_DISPATCH_BLOCKING == flag) )

#define NTFSV_NANOSEC_TO_LEAPTM 10000000

/* The main controle block */
ntfa_cb_t ntfa_cb = {
    .cb_lock = PTHREAD_MUTEX_INITIALIZER,
};

/* list of subscriptions for this process */
ntfa_subscriber_list_t *subscriberNoList = NULL;

/* help functions */
static SaAisErrorT checkAlarmParameters(
    SaNtfAlarmNotificationT* alarmNotification)
{

    if (*alarmNotification->probableCause < SA_NTF_ADAPTER_ERROR ||
        *alarmNotification->probableCause > SA_NTF_UNSPECIFIED_REASON)
    {
        TRACE_1("Invalid probableCause value");
        return SA_AIS_ERR_INVALID_PARAM;
    }

    if (*alarmNotification->perceivedSeverity < SA_NTF_SEVERITY_CLEARED ||
        *alarmNotification->perceivedSeverity > SA_NTF_SEVERITY_CRITICAL)
    {
        TRACE_1("Invalid perceivedSeverity value");
        return SA_AIS_ERR_INVALID_PARAM;
    }

    if (*alarmNotification->notificationHeader.eventType < SA_NTF_ALARM_NOTIFICATIONS_START ||
        *alarmNotification->notificationHeader.eventType > SA_NTF_ALARM_ENVIRONMENT)
    {
        TRACE_1("Invalid eventType value = %d", (int)*alarmNotification->notificationHeader.eventType);
        return SA_AIS_ERR_INVALID_PARAM;
    }

    TRACE_1("Returning SA_AIS_OK!");
    return SA_AIS_OK;
}

static SaAisErrorT checkSecurityAlarmParameters(
    SaNtfSecurityAlarmNotificationT* notification)
{

    if (*notification->notificationHeader.eventType < SA_NTF_SECURITY_ALARM_NOTIFICATIONS_START ||
        *notification->notificationHeader.eventType > SA_NTF_TIME_VIOLATION)
    {
        TRACE_1("Invalid eventType value");
        return SA_AIS_ERR_INVALID_PARAM;
    }

    return SA_AIS_OK;
}


static SaAisErrorT checkStateChangeParameters(
    SaNtfStateChangeNotificationT* notification)
{

    if (*notification->notificationHeader.eventType < SA_NTF_STATE_CHANGE_NOTIFICATIONS_START ||
        *notification->notificationHeader.eventType > SA_NTF_OBJECT_STATE_CHANGE)
    {
        TRACE_1("Invalid eventType value");
        return SA_AIS_ERR_INVALID_PARAM;
    }

    return SA_AIS_OK;
}

static SaAisErrorT checkAttributeChangeParameters(
    SaNtfAttributeChangeNotificationT* notification)
{

    if (*notification->notificationHeader.eventType < SA_NTF_ATTRIBUTE_NOTIFICATIONS_START ||
        *notification->notificationHeader.eventType > SA_NTF_ATTRIBUTE_RESET)
    {
        TRACE_1("Invalid eventType value");
        return SA_AIS_ERR_INVALID_PARAM;
    }
    if (notification->numAttributes > MAX_NUMBER_OF_CHANGED_ATTRIBUTES)
    {
        TRACE_1("Bad numAttributes");
        return SA_AIS_ERR_INVALID_PARAM;
    }

    return SA_AIS_OK;
}

static SaAisErrorT checkObjectCreateDeleteParameters(
    SaNtfObjectCreateDeleteNotificationT* notification)
{

    if (*notification->notificationHeader.eventType < SA_NTF_OBJECT_NOTIFICATIONS_START ||
        *notification->notificationHeader.eventType > SA_NTF_OBJECT_DELETION)
    {
        TRACE_1("Invalid eventType value");
        return SA_AIS_ERR_INVALID_PARAM;
    }

    if (notification->numAttributes > MAX_NUMBER_OF_OBJECT_ATTRIBUTES)
    {
        TRACE_1("Bad numAttributes");
        return SA_AIS_ERR_INVALID_PARAM;
    }

    return SA_AIS_OK;
}

/**
 *
 * @param ntfNotificationInstance
 * @param reqNtfSendnot
 */
static SaAisErrorT fillSendStruct(
    SaNtfNotificationHeaderT *notificationHeader,
    ntfsv_send_not_req_t *reqNtfSendnot)
{
    SaAisErrorT rc = SA_AIS_OK;
    SaTimeT ntfTime;
    struct timeval currentTime;

    /* Fetch current system time for time stamp value */
    (void) gettimeofday(&currentTime, 0);

    ntfTime = ((unsigned)currentTime.tv_sec * 1000000000ULL) + \
        ((unsigned)currentTime.tv_usec * 1000ULL);

    /* only used in send back, server-->lib */
    reqNtfSendnot->subscriptionId = 0;

    /* Set the current system time stamp if SA_TIME_UNKNOWN */
    if (*notificationHeader->eventTime == (SaTimeT)SA_TIME_UNKNOWN)
    {
        *notificationHeader->eventTime = (SaTimeT)ntfTime;
    }

    /* nodificationId set to zero means that this is a new notification */
    /* and not an sync message send from the server. */
    *(notificationHeader->notificationId) = 0;
    return rc;
}

/* end help functions */


/***************************************************************************
 * 8.4.1
 *
 * saNtfInitialize()
 *
 * This function initializes the Ntf Service.
 *
 * Parameters
 *
 * ntfHandle - [out] A pointer to the handle for this initialization of the
 *                   Ntf Service.
 * callbacks - [in] A pointer to the callbacks structure that contains the
 *                  callback functions of the invoking process that the
 *                  Ntf Service may invoke.
 * version - [in] A pointer to the version of the Ntf Service that the
 *                invoking process is using.
 *
 ***************************************************************************/
SaAisErrorT saNtfInitialize(SaNtfHandleT *ntfHandle,
    const SaNtfCallbacksT *ntfCallbacks,
    SaVersionT *version)
{
    ntfa_client_hdl_rec_t *ntfa_hdl_rec;
    ntfsv_msg_t i_msg, *o_msg;
    SaAisErrorT rc;
    uns32 client_id, mds_rc;

    TRACE_ENTER();

    if ((ntfHandle == NULL) || (version == NULL))
    {
        TRACE("version or handle FAILED");
        rc = SA_AIS_ERR_INVALID_PARAM;
        goto done;
    }

    if ((rc = ntfa_startup()) != NCSCC_RC_SUCCESS)
    {
        TRACE("ntfa_startup FAILED");
        rc = SA_AIS_ERR_LIBRARY;
        goto done;
    }

    /* validate the version */
    if ((version->releaseCode == NTF_RELEASE_CODE) &&
        (version->majorVersion <= NTF_MAJOR_VERSION))
    {
        version->majorVersion = NTF_MAJOR_VERSION;
        version->minorVersion = NTF_MINOR_VERSION;
    }
    else
    {
        TRACE("version FAILED, required: %c.%u.%u, supported: %c.%u.%u\n",
            version->releaseCode, version->majorVersion, version->minorVersion,
            NTF_RELEASE_CODE, NTF_MAJOR_VERSION, NTF_MINOR_VERSION);
        version->releaseCode = NTF_RELEASE_CODE;
        version->majorVersion = NTF_MAJOR_VERSION;
        version->minorVersion = NTF_MINOR_VERSION;
        ntfa_shutdown();
        rc = SA_AIS_ERR_VERSION;
        goto done;
    }

    if (!ntfa_cb.ntfs_up)
    {
        ntfa_shutdown();
        TRACE("NTFS server is down");
        rc = SA_AIS_ERR_TRY_AGAIN;
        goto done;
    }

    /* Populate the message to be sent to the NTFS */
    memset(&i_msg, 0, sizeof(ntfsv_msg_t));
    i_msg.type = NTFSV_NTFA_API_MSG;
    i_msg.info.api_info.type = NTFSV_INITIALIZE_REQ;
    i_msg.info.api_info.param.init.version = *version;

    /* Send a message to NTFS to obtain a client_id/server ref id which is cluster
    * wide unique.
    */
    mds_rc = ntfa_mds_msg_sync_send(&ntfa_cb, &i_msg, &o_msg, NTFS_WAIT_TIME);
    switch (mds_rc)
    {
        case NCSCC_RC_SUCCESS:
            break;
        case NCSCC_RC_REQ_TIMOUT:
            rc = SA_AIS_ERR_TIMEOUT;
            TRACE("ntfa_mds_msg_sync_send FAILED: %u", rc);
            goto err;
        default:
            TRACE("mtfa_mds_msg_sync_send FAILED: %u", rc);
            rc = SA_AIS_ERR_TRY_AGAIN;
            goto err;
    }

    /** Make sure the NTFS return status was SA_AIS_OK
     **/
    if (SA_AIS_OK != o_msg->info.api_resp_info.rc)
    {
        rc = o_msg->info.api_resp_info.rc;
        TRACE("NTFS return FAILED");
        goto err;
    }

    /** Store the regId returned by the NTFS
     ** to pass into the next routine
     **/
    client_id = o_msg->info.api_resp_info.param.init_rsp.client_id;

    /* create the hdl record & store the callbacks */
    ntfa_hdl_rec = ntfa_hdl_rec_add(&ntfa_cb, ntfCallbacks, client_id);
    if (ntfa_hdl_rec == NULL)
    {
        rc = SA_AIS_ERR_NO_MEMORY;
        goto err;
    }

    /* pass the handle value to the appl */
    if (SA_AIS_OK == rc)
        *ntfHandle = ntfa_hdl_rec->local_hdl;
    TRACE_1("OK intitialize with client_id: %u", client_id);

err:
    /* free up the response message */
    if (o_msg)
        ntfa_msg_destroy(o_msg);

    if (rc != SA_AIS_OK)
    {
        TRACE_2("NTFA INIT FAILED\n");
        ntfa_shutdown();
    }

done:
    TRACE_LEAVE();
    return rc;
}

/***************************************************************************
 * 8.4.2
 *
 * saNtfSelectionObjectGet()
 *
 * The saNtfSelectionObjectGet() function returns the operating system handle
 * selectionObject, associated with the handle ntfHandle, allowing the
 * invoking process to ascertain when callbacks are pending. This function
 * allows a process to avoid repeated invoking saNtfDispatch() to see if
 * there is a new event, thus, needlessly consuming CPU time. In a POSIX
 * environment the system handle could be a file descriptor that is used with
 * the poll() or select() system calls to detect incoming callbacks.
 *
 *
 * Parameters
 *
 * ntfHandle - [in]
 *   A pointer to the handle, obtained through the saNtfInitialize() function,
 *   designating this particular initialization of the Ntf Service.
 *
 * selectionObject - [out]
 *   A pointer to the operating system handle that the process may use to
 *   detect pending callbacks.
 *
 ***************************************************************************/
SaAisErrorT saNtfSelectionObjectGet(SaNtfHandleT ntfHandle,
    SaSelectionObjectT *selectionObject)
{
    SaAisErrorT rc = SA_AIS_OK;
    ntfa_client_hdl_rec_t *hdl_rec;
    NCS_SEL_OBJ sel_obj;

    TRACE_ENTER();

    if (selectionObject == NULL)
    {
        TRACE("selectionObject = NULL");
        rc = SA_AIS_ERR_INVALID_PARAM;
        goto done;
    }

    /* retrieve hdl rec */
    hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, ntfHandle);
    if (hdl_rec == NULL)
    {
        TRACE("ncshm_take_hdl failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done;
    }

    /* Obtain the selection object from the IPC queue */
    sel_obj = m_NCS_IPC_GET_SEL_OBJ(&hdl_rec->mbx);

    /* everything's fine.. pass the sel fd to the appl */
    *selectionObject = (SaSelectionObjectT) m_GET_FD_FROM_SEL_OBJ(sel_obj);

    /* return hdl rec */
    ncshm_give_hdl(ntfHandle);

done:
    TRACE_LEAVE();
    return rc;
}


/***************************************************************************
 * 8.4.3
 *
 * saNtfDispatch()
 *
 * The saNtfDispatch() function invokes, in the context of the calling thread,
 * one or all of the pending callbacks for the handle ntfHandle.
 *
 *
 * Parameters
 *
 * ntfHandle - [in]
 *   A pointer to the handle, obtained through the saNtfInitialize() function,
 *   designating this particular initialization of the Ntf Service.
 *
 * dispatchFlags - [in]
 *   Flags that specify the callback execution behavior of the the
 *   saNtfDispatch() function, which have the values SA_DISPATCH_ONE,
 *   SA_DISPATCH_ALL or SA_DISPATCH_BLOCKING, as defined in Section 3.3.8.
 *
 ***************************************************************************/
SaAisErrorT saNtfDispatch(SaNtfHandleT ntfHandle,
    SaDispatchFlagsT dispatchFlags)
{
    ntfa_client_hdl_rec_t *hdl_rec;
    SaAisErrorT rc;

    TRACE_ENTER();

    if (!m_DISPATCH_FLAG_IS_VALID(dispatchFlags))
    {
        TRACE("Invalid dispatchFlags");
        rc = SA_AIS_ERR_INVALID_PARAM;
        goto done;
    }

    /* retrieve hdl rec */
    hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, ntfHandle);
    if (hdl_rec == NULL)
    {
        TRACE("ncshm_take_hdl ntfHandle ");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done;
    }

    if ((rc = ntfa_hdl_cbk_dispatch(&ntfa_cb, hdl_rec, dispatchFlags)) != SA_AIS_OK)
        TRACE("NTFA_DISPATCH_FAILURE");

    ncshm_give_hdl(ntfHandle);

done:
    TRACE_LEAVE();
    return rc;
}


/***************************************************************************
 * 8.4.4
 *
 * saNtfFinalize()
 *
 * The saNtfFinalize() function closes the association, represented by the
 * ntfHandle parameter, between the process and the Ntf Service.
 * It may free up resources.
 *
 * This function cannot be invoked before the process has invoked the
 * corresponding saNtfInitialize() function for the Ntf Service.
 * After this function is invoked, the selection object is no longer valid.
 * Moreover, the Ntf Service is unavailable for further use unless it is
 * reinitialized using the saNtfInitialize() function.
 *
 * Parameters
 *
 * ntfHandle - [in]
 *   A pointer to the handle, obtained through the saNtfInitialize() function,
 *   designating this particular initialization of the Ntf Service.
 *
 ***************************************************************************/
SaAisErrorT saNtfFinalize(SaNtfHandleT ntfHandle)
{
    ntfa_client_hdl_rec_t *hdl_rec;
    ntfsv_msg_t msg, *o_msg = NULL;
    SaAisErrorT rc;
    uns32 mds_rc;

    TRACE_ENTER();

    /* retrieve hdl rec */
    hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, ntfHandle);
    if (hdl_rec == NULL)
    {
        TRACE("ncshm_take_hdl failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done;
    }

    /* Check Whether NTFS is up or not */
    if (!ntfa_cb.ntfs_up)
    {
        TRACE("NTFS down");
        rc = SA_AIS_ERR_TRY_AGAIN;
        goto done_give_hdl;
    }

    /** populate & send the finalize message
     ** and make sure the finalize from the server
     ** end returned before deleting the local records.
     **/
    memset(&msg, 0, sizeof(ntfsv_msg_t));
    msg.type = NTFSV_NTFA_API_MSG;
    msg.info.api_info.type = NTFSV_FINALIZE_REQ;
    msg.info.api_info.param.finalize.client_id = hdl_rec->ntfs_client_id;

    mds_rc = ntfa_mds_msg_sync_send(&ntfa_cb, &msg, &o_msg, NTFS_WAIT_TIME);
    switch (mds_rc)
    {
        case NCSCC_RC_SUCCESS:
            break;
        case NCSCC_RC_REQ_TIMOUT:
            rc = SA_AIS_ERR_TIMEOUT;
            TRACE("ntfa_mds_msg_sync_send FAILED: %u", rc);
            goto done_give_hdl;
        default:
            TRACE("ntfa_mds_msg_sync_send FAILED: %u", rc);
            rc = SA_AIS_ERR_NO_RESOURCES;
            goto done_give_hdl;
    }

    if (o_msg != NULL)
    {
        rc = o_msg->info.api_resp_info.rc;
        ntfa_msg_destroy(o_msg);
    }
    else
        rc = SA_AIS_ERR_NO_RESOURCES;

    if (rc == SA_AIS_OK)
    {
        /** delete the hdl rec
         ** including all resources allocated by client if MDS send is 
         ** succesful. 
         **/
        rc = ntfa_hdl_rec_del(&ntfa_cb.client_list, hdl_rec);
        if (rc != NCSCC_RC_SUCCESS)
        {
            TRACE_1("ntfa_hdl_rec_del failed");
            rc = SA_AIS_ERR_BAD_HANDLE;
        }
    }

done_give_hdl:
    ncshm_give_hdl(ntfHandle);

    if (rc == SA_AIS_OK)
    {
        rc = ntfa_shutdown();
        if (rc != NCSCC_RC_SUCCESS)
            TRACE_1("ntfa_shutdown failed");
    }

done:
    TRACE_LEAVE();
    return rc;
}





/**
 * This API internally allocates memory for an alarm notification and
 * initializes the notification structure.
 *
 * @param ntfHandle
 * @param notification
 * @param numCorrelatedNotifications
 * @param lengthAdditionalText
 * @param numAdditionalInfo
 * @param numSpecificProblems
 * @param numMonitoredAttributes
 * @param numProposedRepairActions
 * @param variableDataSize
 *
 * @return SaAisErrorT
 */
SaAisErrorT saNtfAlarmNotificationAllocate(
    SaNtfHandleT ntfHandle,
    SaNtfAlarmNotificationT *notification,
    SaUint16T numCorrelatedNotifications,
    SaUint16T lengthAdditionalText,
    SaUint16T numAdditionalInfo,
    SaUint16T numSpecificProblems,
    SaUint16T numMonitoredAttributes,
    SaUint16T numProposedRepairActions,
    SaInt16T variableDataSize)
{
    SaAisErrorT rc;
    ntfa_notification_hdl_rec_t *notification_hdl_rec;

    ntfa_client_hdl_rec_t *hdl_rec;
    TRACE_ENTER();

    /* Input pointer check */
    if (notification == NULL)
    {
        TRACE("NULL pointer in notification struct inparameter to \
	       saNtfAlarmNotificationAllocate()");
        rc = SA_AIS_ERR_INVALID_PARAM;
        goto done;
    }

    /* retrieve hdl rec */
    hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, ntfHandle);
    if (hdl_rec == NULL)
    {
        TRACE("ncshm_take_hdl failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done;
    }


    /**                 Lock ntfa_CB                 **/
    pthread_mutex_lock(&ntfa_cb.cb_lock);

    /** Allocate an ntfa_LOG_STREAM_HDL_REC structure and insert this
     *  into the list of channel hdl record.
     **/
    notification_hdl_rec = ntfa_notification_hdl_rec_add(&hdl_rec);
    TRACE_1("notification_hdl_rec = %u", notification_hdl_rec->notification_hdl);
    if (notification_hdl_rec == NULL)
    {
        pthread_mutex_unlock(&ntfa_cb.cb_lock);
        rc = SA_AIS_ERR_NO_MEMORY;
        goto done_give_hdl;
    }
    /**                  UnLock ntfa_CB            **/
    pthread_mutex_unlock(&ntfa_cb.cb_lock);

    notification_hdl_rec->ntfNotificationType = SA_NTF_TYPE_ALARM;
    notification_hdl_rec->ntfNotification.ntfAlarmNotification.notificationHandle = (SaUint64T) notification_hdl_rec->notification_hdl;
    notification_hdl_rec->ntfNotificationVariableDataSize = variableDataSize;   
    notification_hdl_rec->parent_hdl = hdl_rec;

    /* Allocate all data fields in the header */
    rc = ntfsv_alloc_ntf_header(
        &notification_hdl_rec->ntfNotification.ntfAlarmNotification.\
        notificationHeader,
        numCorrelatedNotifications,
        lengthAdditionalText,
        numAdditionalInfo);
    if (rc != SA_AIS_OK)
    {
        goto error_put;
    }

    /* Allocate all alarm specific data fields */
    rc = ntfsv_alloc_ntf_alarm(&notification_hdl_rec->
        ntfNotification.ntfAlarmNotification,
        numSpecificProblems,
        numMonitoredAttributes,
        numProposedRepairActions);
    if (rc != SA_AIS_OK)
    {
        goto error_put;
    }

    /* Assign alarm specific data pointers for the client */
    *notification = notification_hdl_rec->ntfNotification.ntfAlarmNotification;
    goto done_give_hdl;

error_put:
    pthread_mutex_lock(&ntfa_cb.cb_lock);
    if (NCSCC_RC_SUCCESS != ntfa_notification_hdl_rec_del(&hdl_rec->notification_list, notification_hdl_rec))
    {
        TRACE("Unable to delete notifiction record");
    }
    pthread_mutex_unlock(&ntfa_cb.cb_lock);
done_give_hdl:
    ncshm_give_hdl(ntfHandle);
done:
    TRACE_LEAVE();
    return rc;
}

/**
 * Frees the memory previously allocated for a notification.
 *
 * @param notificationHandle
 *
 * @return SaAisErrorT
 */
SaAisErrorT saNtfNotificationFree(
    SaNtfNotificationHandleT notificationHandle)
{
    SaAisErrorT rc = SA_AIS_OK;
    unsigned int    client_handle;
    ntfa_notification_hdl_rec_t *notification_hdl_rec;

    ntfa_client_hdl_rec_t *client_rec;
    TRACE_ENTER();

    /**                 Lock ntfa_CB                 **/
    pthread_mutex_lock(&ntfa_cb.cb_lock);
    TRACE_1("notificationHandle = %llu", notificationHandle);
    /* retrieve notification hdl rec */
    notification_hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA,
        notificationHandle);
    if (notification_hdl_rec == NULL)
    {
        pthread_mutex_unlock(&ntfa_cb.cb_lock);
        TRACE("ncshm_take_hdl notificationHandle failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done;
    }
    client_handle =notification_hdl_rec->parent_hdl->local_hdl;
    /* retrieve client hdl rec */
    client_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA,
        client_handle);
    if (client_rec == NULL)
    {
        TRACE("ncshm_take_hdl client_handle failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done_give_hdl;
    }

    /* free the resources allocated by saNtf<ntfType>NotificationAllocate */
    ntfa_hdl_rec_destructor(notification_hdl_rec);

    /** Delete the resources related to the notificationHandle &
     *  remove reference in the client.
    **/
    if (NCSCC_RC_SUCCESS != ntfa_notification_hdl_rec_del(&client_rec->notification_list, notification_hdl_rec))
    {
        TRACE("Unable to delete notifiction record");
        rc = SA_AIS_ERR_LIBRARY;
    }
    /**                  UnLockntfa_CB            **/
    pthread_mutex_unlock(&ntfa_cb.cb_lock);

    ncshm_give_hdl(client_handle);
done_give_hdl:
    ncshm_give_hdl(notificationHandle);
done:
    TRACE_LEAVE();
    return rc;
}

/**
 * This method is used to send a notification. The notification is identified
 * by the notification handle that is returned in the notification structure
 * created with a preceding call to one of the
 * saNtf<notification type>NotificationAllocate() functions.
 *
 * @param notificationHandle
 *
 * @return SaAisErrorT
 */
SaAisErrorT saNtfNotificationSend(SaNtfNotificationHandleT notificationHandle)
{
    TRACE_ENTER();
    SaAisErrorT rc = SA_AIS_OK;

    unsigned int                client_handle;
    ntfa_client_hdl_rec_t       *client_rec;
    ntfa_notification_hdl_rec_t *notification_hdl_rec;
    ntfsv_msg_t                 msg, *o_msg = NULL;
    ntfsv_send_not_req_t        *send_param;
    uns32                       timeout= NTFS_WAIT_TIME;
    uns32                       mds_rc;
    SaNtfNotificationHeaderT    *ntfHeader;

    send_param = calloc(1, sizeof(ntfsv_send_not_req_t));
    if (send_param == NULL)
    {
        rc = SA_AIS_ERR_NO_MEMORY;
        goto done;
    }

    notification_hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA,
        notificationHandle);
    if (notification_hdl_rec == NULL)
    {
        TRACE("ncshm_take_hdl notificationHandle failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done;
    }

    client_handle = notification_hdl_rec->parent_hdl->local_hdl;
    /* retrieve client hdl rec */
    client_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA,
        client_handle);
    if (client_rec == NULL)
    {
        TRACE("ncshm_take_hdl client_handle failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done_give_hdl;
    }

    /**
     ** Populate a sync MDS message
     **/
    memset(&msg, 0, sizeof(ntfsv_msg_t));
    msg.type = NTFSV_NTFA_API_MSG;
    msg.info.api_info.type = NTFSV_SEND_NOT_REQ;
    msg.info.api_info.param.send_notification = send_param;
    send_param->client_id = client_rec->ntfs_client_id;
    send_param->notificationType = notification_hdl_rec->ntfNotificationType;


    /* Check parameters, depending on type */
    switch (notification_hdl_rec->ntfNotificationType)
    {
        case SA_NTF_TYPE_ALARM:
            TRACE_1("Checking Alarm Notification Parameters");
    /* TODO: assign send_param for all */
            send_param->notification.alarm = notification_hdl_rec->ntfNotification.ntfAlarmNotification;
            ntfHeader = &notification_hdl_rec->ntfNotification.ntfAlarmNotification.notificationHeader;
            rc = checkAlarmParameters(
                &notification_hdl_rec->ntfNotification.ntfAlarmNotification);
            break;
        case SA_NTF_TYPE_SECURITY_ALARM:
            TRACE_1("Checking Security Alarm Notification Parameters");
            send_param->notification.securityAlarm = notification_hdl_rec->ntfNotification.ntfSecurityAlarmNotification;
            ntfHeader = &notification_hdl_rec->ntfNotification.ntfSecurityAlarmNotification.notificationHeader;
            rc = checkSecurityAlarmParameters(
                &notification_hdl_rec->ntfNotification.ntfSecurityAlarmNotification);
            break;
        case SA_NTF_TYPE_STATE_CHANGE:
            TRACE_1("Checking State Change Notification Parameters");
            send_param->notification.stateChange = notification_hdl_rec->ntfNotification.ntfStateChangeNotification;
            ntfHeader = &notification_hdl_rec->ntfNotification.ntfStateChangeNotification.notificationHeader;
            rc = checkStateChangeParameters(
                &notification_hdl_rec->ntfNotification.ntfStateChangeNotification);
            break;
        case SA_NTF_TYPE_ATTRIBUTE_CHANGE:
            TRACE_1("Checking Attribute Change Notification Parameters");
            send_param->notification.attributeChange = notification_hdl_rec->ntfNotification.ntfAttributeChangeNotification;
            ntfHeader = &notification_hdl_rec->ntfNotification.ntfAttributeChangeNotification.notificationHeader;
            rc = checkAttributeChangeParameters(
                &notification_hdl_rec->ntfNotification.ntfAttributeChangeNotification);
            break;
        case SA_NTF_TYPE_OBJECT_CREATE_DELETE:
            TRACE_1("Checking Object Create/Delete Notification Parameters");
            send_param->notification.objectCreateDelete = notification_hdl_rec->ntfNotification.ntfObjectCreateDeleteNotification;
            ntfHeader = &notification_hdl_rec->ntfNotification.ntfObjectCreateDeleteNotification.notificationHeader;
            rc = checkObjectCreateDeleteParameters(
                &notification_hdl_rec->ntfNotification.ntfObjectCreateDeleteNotification);
            break;
        default:
            TRACE_1("Unkown notification type");
            rc = SA_AIS_ERR_INVALID_PARAM;
            goto done_give_hdls;
    }

    if (rc != SA_AIS_OK)
    {
        TRACE_1("Invalid parameters");
        goto done_give_hdls;
    }

    rc = fillSendStruct(ntfHeader, send_param);
    if (rc != SA_AIS_OK)
    {
        goto done_give_hdls;
    }

    /* Check whether NTFS is up or not */
    if (!ntfa_cb.ntfs_up)
    {
        TRACE("NTFS down");
        rc = SA_AIS_ERR_TRY_AGAIN;
        goto done_give_hdls;
    }

    /* Send a sync MDS message to obtain a notification id */
    mds_rc = ntfa_mds_msg_sync_send(&ntfa_cb, &msg, &o_msg, timeout);
    switch (mds_rc)
    {
        case NCSCC_RC_SUCCESS:
            break;
        case NCSCC_RC_REQ_TIMOUT:
            rc = SA_AIS_ERR_TIMEOUT;
            TRACE("ntfa_mds_msg_sync_send FAILED: %u", rc);
            break;
        default:
            TRACE("mtfa_mds_msg_sync_send FAILED: %u", rc);
            rc = SA_AIS_ERR_TRY_AGAIN;
    }
    if (mds_rc != NCSCC_RC_SUCCESS)
    {
        if (o_msg)
            ntfa_msg_destroy(o_msg);
        goto done_give_hdls;
    }

    if (SA_AIS_OK != o_msg->info.api_resp_info.rc)
    {
        rc = o_msg->info.api_resp_info.rc;
        TRACE("Bad return status!!! rc = %d", rc);
        if (o_msg)
            ntfa_msg_destroy(o_msg);
        goto done_give_hdls;
    }

    /* Return the notificationId to client from right notification */
    switch (notification_hdl_rec->ntfNotificationType)
    {
        case SA_NTF_TYPE_ALARM:
            TRACE_1("Not id back: %llu", o_msg->info.api_resp_info.param.send_not_rsp.notificationId);
            TRACE_1("Not ptr %p:", notification_hdl_rec->ntfNotification.ntfAlarmNotification.notificationHeader.notificationId);
            *(notification_hdl_rec->ntfNotification.ntfAlarmNotification.notificationHeader.notificationId) = o_msg->info.api_resp_info.param.send_not_rsp.notificationId;
            break;

        case SA_NTF_TYPE_STATE_CHANGE:
            *notification_hdl_rec->ntfNotification.ntfStateChangeNotification.\
                notificationHeader.notificationId =
                o_msg->info.api_resp_info.param.send_not_rsp.notificationId;
            break;

        case SA_NTF_TYPE_OBJECT_CREATE_DELETE:
            *notification_hdl_rec->ntfNotification.ntfObjectCreateDeleteNotification.\
                notificationHeader.notificationId =
                o_msg->info.api_resp_info.param.send_not_rsp.notificationId;
            break;

        case SA_NTF_TYPE_ATTRIBUTE_CHANGE:
            *notification_hdl_rec->ntfNotification.ntfAttributeChangeNotification.\
                notificationHeader.notificationId =
                o_msg->info.api_resp_info.param.send_not_rsp.notificationId;
            break;

        case SA_NTF_TYPE_SECURITY_ALARM:
            *notification_hdl_rec->ntfNotification.ntfSecurityAlarmNotification.\
                notificationHeader.notificationId =
                o_msg->info.api_resp_info.param.send_not_rsp.notificationId;
            break;

        default:
            TRACE_1("Unkown notification type");
            rc = SA_AIS_ERR_INVALID_PARAM;
            break;
    }
    if (o_msg)
        ntfa_msg_destroy(o_msg);

done_give_hdls:
    ncshm_give_hdl(client_handle);
done_give_hdl:
    ncshm_give_hdl(notificationHandle);
    free(send_param);
done:
    TRACE_LEAVE();
    return rc;
}

/*  3.15.3	Subscriber Operations   */
/*  3.15.3.1	saNtfNotificationSubscribe()  */
SaAisErrorT saNtfNotificationSubscribe(
    const SaNtfNotificationTypeFilterHandlesT *notificationFilterHandles,
    SaNtfSubscriptionIdT subscriptionId)
{
    SaAisErrorT rc = SA_AIS_ERR_NOT_SUPPORTED;

    ntfa_subscriber_list_t           *listPtr;
    ntfa_subscriber_list_t           *listPtrNext;
    ntfa_subscriber_list_t           *ntfSubscriberList;

    ntfa_filter_hdl_rec_t *filter_hdl_rec;
    ntfa_client_hdl_rec_t *client_hdl_rec;
    SaNtfNotificationFilterHandleT filterHndl;

    ntfsv_msg_t                 msg, *o_msg = NULL;
    ntfsv_subscribe_req_t       *send_param;
    uns32                       timeout= NTFS_WAIT_TIME;

    TRACE_ENTER();
    if (notificationFilterHandles == NULL)
    {
        TRACE_1("NULL Pointer exception of notificationFilterHandles!");
        TRACE_LEAVE();
        return SA_AIS_ERR_INVALID_PARAM;
    }
    else
    {
        if (notificationFilterHandles->alarmFilterHandle == SA_NTF_FILTER_HANDLE_NULL &&
            notificationFilterHandles->attributeChangeFilterHandle == SA_NTF_FILTER_HANDLE_NULL &&
            notificationFilterHandles->objectCreateDeleteFilterHandle == SA_NTF_FILTER_HANDLE_NULL &&
            notificationFilterHandles->securityAlarmFilterHandle == SA_NTF_FILTER_HANDLE_NULL &&
            notificationFilterHandles->stateChangeFilterHandle == SA_NTF_FILTER_HANDLE_NULL)
        {
            TRACE_1("All handles in notificationFilterHandles set to NULL!");
            TRACE_LEAVE();
            return SA_AIS_ERR_INVALID_PARAM;
        }
    }

    if (notificationFilterHandles->alarmFilterHandle != SA_NTF_FILTER_HANDLE_NULL)
    {
        filterHndl = notificationFilterHandles->alarmFilterHandle;

        TRACE_1("Getting notificationFilterHandle!");

        /* retrieve notification filter hdl rec */
        filter_hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA,
            filterHndl);
        if (filter_hdl_rec == NULL)
        {
            TRACE_1("ncshm_take_hdl failed");
            TRACE_LEAVE();
            return SA_AIS_ERR_BAD_HANDLE;
        }


        /* retrieve client hdl rec */
        client_hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, filter_hdl_rec->ntfHandle);
        if (client_hdl_rec == NULL)
        {
            TRACE_1("ncshm_take_hdl failed");
            ncshm_give_hdl(filterHndl);
            TRACE_LEAVE();
            return SA_AIS_ERR_BAD_HANDLE;
        }

        /* Check earlier subscriptions in subscriber list */
        if (NULL != subscriberNoList)
        {
            for (listPtr = subscriberNoList,
                listPtrNext = listPtr->next;
                listPtr != NULL ;
                listPtr = listPtrNext,
                listPtrNext = listPtr->next)
            {
                TRACE_1("listPtr->SubscriptionId %d",
                    listPtr->subscriberListSubscriptionId);
                if (listPtr->subscriberListSubscriptionId ==
                    subscriptionId)
                {
                    ncshm_give_hdl(filter_hdl_rec->ntfHandle);
                    ncshm_give_hdl(filterHndl);
                    TRACE_LEAVE();
                    return SA_AIS_ERR_EXIST;
                }
                if (listPtrNext == NULL)
                {
                    break;
                }
            }
        }

        /* An unique subscriptionId was passed */
        ntfSubscriberList = malloc(sizeof (ntfa_subscriber_list_t));
        if (!ntfSubscriberList)
        {
            LOG_ER("Out of memory in ntfSubscriberList");
            ncshm_give_hdl(filter_hdl_rec->ntfHandle);
            ncshm_give_hdl(filterHndl);
            TRACE_LEAVE();
            return SA_AIS_ERR_NO_MEMORY;
        }
        /* Add ntfHandle and subscriptionId into list */
        ntfSubscriberList->subscriberListNtfHandle =
            filter_hdl_rec->ntfHandle;
        ntfSubscriberList->subscriberListSubscriptionId =
            subscriptionId;

        if (NULL == subscriberNoList)
        {
            subscriberNoList = ntfSubscriberList;
            subscriberNoList->prev = NULL;
            subscriberNoList->next = NULL;
        }
        else
        {
            ntfSubscriberList->prev = NULL;
            ntfSubscriberList->next = subscriberNoList;
            subscriberNoList->prev = ntfSubscriberList;
            subscriberNoList = ntfSubscriberList;
        }
        TRACE_1("ADD: subscriberNoList->SubscriptionId %d",
            subscriberNoList->subscriberListSubscriptionId);

        /**
             ** Populate a sync MDS message
         **/    
        memset(&msg, 0, sizeof(ntfsv_msg_t));
        msg.type = NTFSV_NTFA_API_MSG;
        msg.info.api_info.type = NTFSV_SUBSCRIBE_REQ;
        send_param = &msg.info.api_info.param.subscribe;

        send_param->client_id = client_hdl_rec->ntfs_client_id;
        send_param->subscriptionId =
            ntfSubscriberList->subscriberListSubscriptionId;

        /* Check whether NTFS is up or not */
        if (ntfa_cb.ntfs_up)
        {
                /* Send a sync MDS message to obtain a log stream id */
            rc = ntfa_mds_msg_sync_send(&ntfa_cb, &msg, &o_msg, timeout);
            if (rc == NCSCC_RC_SUCCESS)
            {
                if (SA_AIS_OK == o_msg->info.api_resp_info.rc)
                {
                    TRACE_1("subscriptionId from server %u",
                        o_msg->info.api_resp_info.param.subscribe_rsp.subscriptionId);
                }
                else
                {
                    rc = o_msg->info.api_resp_info.rc;
                    TRACE("Bad return status!!! rc = %d", rc);
                }
            }
            else
            {
                rc = SA_AIS_ERR_TRY_AGAIN;
            }
        }
        else
        {
            TRACE_1("NTFS down");
            rc = SA_AIS_ERR_TRY_AGAIN;
        }

        if (rc != SA_AIS_OK)
        {
            subscriberNoList = subscriberNoList->next;
            subscriberNoList->prev = NULL;
            free(ntfSubscriberList);
        }
        if (o_msg)
            ntfa_msg_destroy(o_msg);
        ncshm_give_hdl(filter_hdl_rec->ntfHandle);
        ncshm_give_hdl(filterHndl);
    }
    else
    {
        TRACE_1("No filter except the alarm filter supported!");
        rc = SA_AIS_ERR_NOT_SUPPORTED;
    }

    TRACE_LEAVE();
    return rc;
}

/*  Producer Operations  */
/*  ===================  */
/**
 * This API internally allocates memory for an object create delete notification
 * and initializes the notification structure.
 *
 * @param ntfHandle
 * @param notification
 * @param numCorrelatedNotifications
 * @param lengthAdditionalText
 * @param numAdditionalInfo
 * @param numAttributes
 * @param variableDataSize
 *
 * @return SaAisErrorT
 */
SaAisErrorT
    saNtfObjectCreateDeleteNotificationAllocate(
    SaNtfHandleT ntfHandle,
    SaNtfObjectCreateDeleteNotificationT *notification,
    SaUint16T numCorrelatedNotifications,
    SaUint16T lengthAdditionalText,
    SaUint16T numAdditionalInfo,
    SaUint16T numAttributes,
    SaInt16T variableDataSize)
{
    SaAisErrorT rc;
    ntfa_notification_hdl_rec_t *notification_hdl_rec;
    ntfa_client_hdl_rec_t *hdl_rec;

    TRACE_ENTER();

    /* Input pointer check */
    if (notification == NULL)
    {
        TRACE("NULL pointer in notification struct inparameter");
        rc = SA_AIS_ERR_INVALID_PARAM;
        goto done;
    }

    /* retrieve hdl rec */
    hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, ntfHandle);
    if (hdl_rec == NULL)
    {
        TRACE("ncshm_take_hdl failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done;
    }


    /**                 Lock ntfa_CB                 **/
    pthread_mutex_lock(&ntfa_cb.cb_lock);

    notification_hdl_rec = ntfa_notification_hdl_rec_add(&hdl_rec);
    TRACE_1("notification_hdl_rec = %u", notification_hdl_rec->notification_hdl);
    if (notification_hdl_rec == NULL)
    {
        pthread_mutex_unlock(&ntfa_cb.cb_lock);
        rc = SA_AIS_ERR_NO_MEMORY;
        goto done_give_hdl;
    }
    /**                  UnLock ntfa_CB            **/
    pthread_mutex_unlock(&ntfa_cb.cb_lock);
    notification_hdl_rec->ntfNotification.ntfObjectCreateDeleteNotification. 
        notificationHandle = (SaUint64T) notification_hdl_rec->notification_hdl;
    notification_hdl_rec->parent_hdl = hdl_rec;
    notification_hdl_rec->ntfNotificationType =
        SA_NTF_TYPE_OBJECT_CREATE_DELETE;
    notification_hdl_rec->ntfNotificationVariableDataSize = variableDataSize;

    /* Allocate all data fields in the header */
    rc = ntfsv_alloc_ntf_header(
        &notification_hdl_rec->ntfNotification.\
        ntfObjectCreateDeleteNotification.notificationHeader,
        numCorrelatedNotifications,
        lengthAdditionalText,
        numAdditionalInfo);
    if (rc != SA_AIS_OK)
    {
        goto error_put;
    }

    /* Allocate all object create/delete specific data fields */
    rc = ntfsv_alloc_ntf_obj_create_del(&notification_hdl_rec->ntfNotification.
        ntfObjectCreateDeleteNotification,
        numAttributes);
    if (rc != SA_AIS_OK)
    {
        goto error_put;
    }

    /* A copy to the client */
    *notification = notification_hdl_rec->ntfNotification.
        ntfObjectCreateDeleteNotification;
    goto done_give_hdl;

error_put:
    pthread_mutex_lock(&ntfa_cb.cb_lock);
    if (NCSCC_RC_SUCCESS != ntfa_notification_hdl_rec_del(&hdl_rec->notification_list, notification_hdl_rec))
    {
        TRACE("Unable to delete notifiction record");
    }
    pthread_mutex_unlock(&ntfa_cb.cb_lock);
done_give_hdl:
    ncshm_give_hdl(ntfHandle);
done:
    TRACE_LEAVE();
    return rc;
}

/*  3.14.2      saNtfAttributeChangeNotificationAllocate()  */
SaAisErrorT
    saNtfAttributeChangeNotificationAllocate(
    SaNtfHandleT ntfHandle,
    SaNtfAttributeChangeNotificationT *notification,
    SaUint16T numCorrelatedNotifications,
    SaUint16T lengthAdditionalText,
    SaUint16T numAdditionalInfo,
    SaUint16T numAttributes,
    SaInt16T variableDataSize)
{
    SaAisErrorT rc;
    ntfa_notification_hdl_rec_t *notification_hdl_rec;
    ntfa_client_hdl_rec_t *hdl_rec;
    TRACE_ENTER();
    /* Input pointer check */
    if (notification == NULL)
    {
        TRACE("NULL pointer in notification struct inparameter");
        rc = SA_AIS_ERR_INVALID_PARAM;
        goto done;
    }

    /* retrieve hdl rec */
    hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, ntfHandle);
    if (hdl_rec == NULL)
    {
        TRACE("ncshm_take_hdl failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done;
    }
    /**                 Lock ntfa_CB                 **/
    pthread_mutex_lock(&ntfa_cb.cb_lock);

    notification_hdl_rec = ntfa_notification_hdl_rec_add(&hdl_rec);
    TRACE_1("notification_hdl_rec = %u", notification_hdl_rec->notification_hdl);
    if (notification_hdl_rec == NULL)
    {
        pthread_mutex_unlock(&ntfa_cb.cb_lock);
        rc = SA_AIS_ERR_NO_MEMORY;
        goto done_give_hdl;
    }
    /**                  UnLock ntfa_CB            **/
    pthread_mutex_unlock(&ntfa_cb.cb_lock);

    notification_hdl_rec->ntfNotification.ntfAttributeChangeNotification.
        notificationHandle = (SaUint64T) notification_hdl_rec->notification_hdl;
    notification_hdl_rec->parent_hdl = hdl_rec;
    notification_hdl_rec->ntfNotificationType =
        SA_NTF_TYPE_ATTRIBUTE_CHANGE;
    notification_hdl_rec->ntfNotificationVariableDataSize = variableDataSize;

    /* Allocate all data fields in the header */
    rc = ntfsv_alloc_ntf_header(
        &notification_hdl_rec->ntfNotification.\
        ntfAttributeChangeNotification.notificationHeader,
        numCorrelatedNotifications,
        lengthAdditionalText,
        numAdditionalInfo);
    if (rc != SA_AIS_OK)
    {
        goto error_put;
    }

    /* Allocate all object create/delete specific data fields */
    rc = ntfsv_alloc_ntf_attr_change(&notification_hdl_rec->ntfNotification.
        ntfAttributeChangeNotification,
        numAttributes);
    if (rc != SA_AIS_OK)
    {
        goto error_put;
    }
    *notification = notification_hdl_rec->ntfNotification.\
        ntfAttributeChangeNotification;
    goto done_give_hdl;

error_put:
    pthread_mutex_lock(&ntfa_cb.cb_lock);
    if (NCSCC_RC_SUCCESS != ntfa_notification_hdl_rec_del(&hdl_rec->notification_list, notification_hdl_rec))
    {
        TRACE("Unable to delete notifiction record");
    }
    pthread_mutex_unlock(&ntfa_cb.cb_lock);
done_give_hdl:
    ncshm_give_hdl(ntfHandle);
done:
    TRACE_LEAVE();
    return rc;
}


/*  3.14.3      saNtfStateChangeNotificationAllocate()  */
SaAisErrorT
    saNtfStateChangeNotificationAllocate(
    SaNtfHandleT ntfHandle,
    SaNtfStateChangeNotificationT *notification,
    SaUint16T numCorrelatedNotifications,
    SaUint16T lengthAdditionalText,
    SaUint16T numAdditionalInfo,
    SaUint16T numStateChanges,
    SaInt16T variableDataSize)
{
    SaAisErrorT rc;
    ntfa_notification_hdl_rec_t *notification_hdl_rec;
    ntfa_client_hdl_rec_t *hdl_rec;
    TRACE_ENTER();

    /* Input pointer check */
    if (notification == NULL)
    {
        TRACE("NULL pointer in notification struct inparameter");
        rc = SA_AIS_ERR_INVALID_PARAM;
        goto done;
    }

    /* retrieve hdl rec */
    hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, ntfHandle);
    if (hdl_rec == NULL)
    {
        TRACE("ncshm_take_hdl failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done;
    }

    /**                 Lock ntfa_CB                 **/
    pthread_mutex_lock(&ntfa_cb.cb_lock);
    notification_hdl_rec = ntfa_notification_hdl_rec_add(&hdl_rec);
    TRACE_1("notification_hdl_rec = %u", notification_hdl_rec->notification_hdl);
    if (notification_hdl_rec == NULL)
    {
        pthread_mutex_unlock(&ntfa_cb.cb_lock);
        rc = SA_AIS_ERR_NO_MEMORY;
        goto done_give_hdl;
    }
    /**                  UnLock ntfa_CB            **/
    pthread_mutex_unlock(&ntfa_cb.cb_lock);

    notification_hdl_rec->ntfNotification.ntfStateChangeNotification.
        notificationHandle = (SaUint64T) notification_hdl_rec->notification_hdl;
    notification_hdl_rec->parent_hdl = hdl_rec;
    notification_hdl_rec->ntfNotificationType =
        SA_NTF_TYPE_STATE_CHANGE;
    notification_hdl_rec->ntfNotificationVariableDataSize = variableDataSize;

    /* Allocate all data fields in the header */
    rc = ntfsv_alloc_ntf_header(
        &notification_hdl_rec->ntfNotification.\
        ntfStateChangeNotification.notificationHeader,
        numCorrelatedNotifications,
        lengthAdditionalText,
        numAdditionalInfo);
    if (rc != SA_AIS_OK)
    {
        goto error_put;
    }

    /* Allocate all object create/delete specific data fields */
    rc = ntfsv_alloc_ntf_state_change(&notification_hdl_rec->ntfNotification.\
        ntfStateChangeNotification,
        numStateChanges);
    if (rc != SA_AIS_OK)
    {
        goto error_put;
    }
    *notification = notification_hdl_rec->ntfNotification.\
        ntfStateChangeNotification;
    goto done_give_hdl;

error_put:
    pthread_mutex_lock(&ntfa_cb.cb_lock);
    if (NCSCC_RC_SUCCESS != ntfa_notification_hdl_rec_del(&hdl_rec->notification_list, notification_hdl_rec))
    {
        TRACE("Unable to delete notifiction record");
    }
    pthread_mutex_unlock(&ntfa_cb.cb_lock);

done_give_hdl:
    ncshm_give_hdl(ntfHandle);
done:
    TRACE_LEAVE();
    return rc;
}

/*  3.14.5	saNtfSecurityAlarmNotificationAllocate()  */
SaAisErrorT saNtfSecurityAlarmNotificationAllocate(
    SaNtfHandleT ntfHandle,
    SaNtfSecurityAlarmNotificationT *notification,
    SaUint16T numCorrelatedNotifications,
    SaUint16T lengthAdditionalText,
    SaUint16T numAdditionalInfo,
    SaInt16T variableDataSize)
{
    SaAisErrorT rc;
    ntfa_notification_hdl_rec_t *notification_hdl_rec;
    ntfa_client_hdl_rec_t *hdl_rec;
    TRACE_ENTER();
    /* Input pointer check */
    if (notification == NULL)
    {
        TRACE("NULL pointer in notification struct inparameter");
        rc = SA_AIS_ERR_INVALID_PARAM;
        goto done;
    }
    /* retrieve hdl rec */
    hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, ntfHandle);
    if (hdl_rec == NULL)
    {
        TRACE("ncshm_take_hdl failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done;
    }
    /**                 Lock ntfa_CB                 **/
    pthread_mutex_lock(&ntfa_cb.cb_lock);
    notification_hdl_rec = ntfa_notification_hdl_rec_add(&hdl_rec);
    TRACE_1("notification_hdl_rec = %u", notification_hdl_rec->notification_hdl);
    if (notification_hdl_rec == NULL)
    {
        pthread_mutex_unlock(&ntfa_cb.cb_lock);
        rc = SA_AIS_ERR_NO_MEMORY;
        goto done_give_hdl;
    }
    /**                  UnLock ntfa_CB            **/
    pthread_mutex_unlock(&ntfa_cb.cb_lock);

    notification_hdl_rec->ntfNotification.ntfSecurityAlarmNotification.
        notificationHandle = (SaUint64T) notification_hdl_rec->notification_hdl;
    notification_hdl_rec->parent_hdl = hdl_rec;
    notification_hdl_rec->ntfNotificationType =
        SA_NTF_TYPE_SECURITY_ALARM;
    notification_hdl_rec->ntfNotificationVariableDataSize = variableDataSize;

    /* Allocate all data fields in the header */
    rc = ntfsv_alloc_ntf_header(
        &notification_hdl_rec->ntfNotification.\
        ntfSecurityAlarmNotification.notificationHeader,
        numCorrelatedNotifications,
        lengthAdditionalText,
        numAdditionalInfo);
    if (rc != SA_AIS_OK)
    {
        goto error_put;
    }
    /* Allocate all object create/delete specific data fields */
    rc = ntfsv_alloc_ntf_security_alarm(&notification_hdl_rec->ntfNotification.
        ntfSecurityAlarmNotification);
    if (rc != SA_AIS_OK)
    {
        goto error_put;
    }
    *notification = notification_hdl_rec->ntfNotification.
        ntfSecurityAlarmNotification;
    goto done_give_hdl;

error_put:
    pthread_mutex_lock(&ntfa_cb.cb_lock);
    if (NCSCC_RC_SUCCESS != ntfa_notification_hdl_rec_del(&hdl_rec->notification_list,
        notification_hdl_rec))
    {
        TRACE("Unable to delete notifiction record");
    }
    pthread_mutex_unlock(&ntfa_cb.cb_lock);

done_give_hdl:
    ncshm_give_hdl(ntfHandle);
done:
    TRACE_LEAVE();
    return rc;
}

/*  3.14.6	saNtfPtrValAllocate()  */
SaAisErrorT saNtfPtrValAllocate(
    SaNtfNotificationHandleT notificationHandle,
    SaUint16T dataSize,
    void **dataPtr,
    SaNtfValueT *value)
{
    return SA_AIS_ERR_NOT_SUPPORTED;
}

/*  3.14.7	saNtfArrayValAllocate()  */
SaAisErrorT saNtfArrayValAllocate(
    SaNtfNotificationHandleT notificationHandle,
    SaUint16T numElements,
    SaUint16T elementSize,
    void **arrayPtr,
    SaNtfValueT *value)
{
    return SA_AIS_ERR_NOT_SUPPORTED;
}

/*  3.15	Consumer Operations  */
/*  3.15.2	Common Operations  */
/*  3.15.2.1	saNtfLocalizedMessageGet()  */
SaAisErrorT saNtfLocalizedMessageGet(
    SaNtfNotificationHandleT notificationHandle,
    SaStringT *message)
{
    return SA_AIS_ERR_NOT_SUPPORTED;
}

/*  3.15.2.2 saNtfLocalizedMessageFree() */
SaAisErrorT saNtfLocalizedMessageFree(
    SaStringT message)
{
    return SA_AIS_ERR_NOT_SUPPORTED;
}

/*  3.15.2.3	saNtfPtrValGet()  */
SaAisErrorT saNtfPtrValGet(
    SaNtfNotificationHandleT notificationHandle,
    SaNtfValueT *value,
    void **dataPtr,
    SaUint16T *dataSize)
{
    return SA_AIS_ERR_NOT_SUPPORTED;
}

/*  3.15.2.4	saNtfArrayValGet()  */
SaAisErrorT saNtfArrayValGet(
    SaNtfNotificationHandleT notificationHandle,
    SaNtfValueT *value,
    void **arrayPtr,
    SaUint16T *numElements,
    SaUint16T *elementSize)
{
    return SA_AIS_ERR_NOT_SUPPORTED;
}

/* 3.15.2.5  saNtfObjectCreateDeleteNotificationFilterAllocate() */
SaAisErrorT saNtfObjectCreateDeleteNotificationFilterAllocate(
    SaNtfHandleT ntfHandle,
    SaNtfObjectCreateDeleteNotificationFilterT *notificationFilter,
    SaUint16T numEventTypes,
    SaUint16T numNotificationObjects,
    SaUint16T numNotifyingObjects,
    SaUint16T numNotificationClassIds,
    SaUint16T numSourceIndicators)
{
    return SA_AIS_ERR_NOT_SUPPORTED;
}

/*  3.15.2.6	saNtfAttributeChangeNotificationFilterAllocate()  */
SaAisErrorT saNtfAttributeChangeNotificationFilterAllocate(
    SaNtfHandleT ntfHandle,
    SaNtfAttributeChangeNotificationFilterT *notificationFilter,
    SaUint16T numEventTypes,
    SaUint16T numNotificationObjects,
    SaUint16T numNotifyingObjects,
    SaUint16T numNotificationClassIds,
    SaUint32T numSourceIndicators)
{
    return SA_AIS_ERR_NOT_SUPPORTED;
}

/*  3.15.2.7	saNtfStateChangeNotificationFilterAllocate()  */
SaAisErrorT saNtfStateChangeNotificationFilterAllocate(
    SaNtfHandleT ntfHandle,
    SaNtfStateChangeNotificationFilterT *notificationFilter,
    SaUint16T numEventTypes,
    SaUint16T numNotificationObjects,
    SaUint16T numNotifyingObjects,
    SaUint16T numNotificationClassIds,
    SaUint32T numSourceIndicators,
    SaUint32T numChangedStates)
{
    return SA_AIS_ERR_NOT_SUPPORTED;
}
/* END DUMMY for not implemented */

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
    SaUint32T numTrends)
{
    SaAisErrorT rc = SA_AIS_OK;
    ntfa_client_hdl_rec_t *hdl_rec;
    ntfa_filter_hdl_rec_t *filter_hdl_rec;

    TRACE_ENTER();
    if (notificationFilter == NULL)
    {
        TRACE_1("NULL pointer in notificationFilter inparameter to \
               saNtfAlarmNotificationFilterAllocate");
        rc = SA_AIS_ERR_INVALID_PARAM;
        goto done;
    }

    /* retrieve hdl rec */
    hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, ntfHandle);
    if (hdl_rec == NULL)
    {
        TRACE("ncshm_take_hdl failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done;
    }

    TRACE_1("Creating notificationFilterHandle!");

    /**                 Lock ntfa_CB                 **/
    pthread_mutex_lock(&ntfa_cb.cb_lock);


    filter_hdl_rec = ntfa_filter_hdl_rec_add(&hdl_rec);
    TRACE_1("filter_hdl_rec = %llu", filter_hdl_rec->filter_hdl);
    if (filter_hdl_rec == NULL)
    {
        pthread_mutex_unlock(&ntfa_cb.cb_lock);
        rc = SA_AIS_ERR_NO_MEMORY;
        goto done_give_hdl;
    }
    /**                  UnLock ntfa_CB            **/
    pthread_mutex_unlock(&ntfa_cb.cb_lock);


    /* Set the instance handle */
    filter_hdl_rec->ntfHandle = ntfHandle;

    /* Set filter handle in instance struct*/
    notificationFilter->notificationFilterHandle = filter_hdl_rec->filter_hdl;

    /* Set the pointers for the client */

    /* Header part */
    /* Event types */
    if (numEventTypes != 0)
    {
        filter_hdl_rec->ntfEventTypes =
            (SaNtfEventTypeT *) \
            malloc(numEventTypes * sizeof (SaNtfEventTypeT));
        if (filter_hdl_rec->ntfEventTypes == NULL)
        {
            TRACE_1("Out of memory in eventTypes field");
            rc = SA_AIS_ERR_NO_MEMORY;
            goto error_put;
        }
    }
    else
    {
        filter_hdl_rec->ntfEventTypes = NULL;
    }
    filter_hdl_rec->ntfNumEventTypes = numEventTypes;
    notificationFilter->notificationFilterHeader.eventTypes =
        filter_hdl_rec->ntfEventTypes;

    /* Notification objects */
    if (numNotificationObjects != 0)
    {

        filter_hdl_rec->ntfNotificationObjects =
            (SaNameT *) malloc(numNotificationObjects * sizeof (SaNameT));
        if (filter_hdl_rec->ntfNotificationObjects == NULL)
        {
            TRACE_1("Out of memory in notificationObjects field");
            rc = SA_AIS_ERR_NO_MEMORY;
            goto error_put_dealloc;
        }
    }
    else
    {
        filter_hdl_rec->ntfNotificationObjects = NULL;
    }
    filter_hdl_rec->ntfNumNotificationObjects =
        numNotificationObjects;
    notificationFilter->notificationFilterHeader.notificationObjects =
        filter_hdl_rec->ntfNotificationObjects;

    /* Notifying objects */
    if (numNotifyingObjects != 0)
    {

        filter_hdl_rec->ntfNotifyingObjects =
            (SaNameT *) malloc(numNotifyingObjects * sizeof (SaNameT));
        if (filter_hdl_rec->ntfNotifyingObjects == NULL)
        {
            TRACE_1("Out of memory in notifyingObjects field");
            rc = SA_AIS_ERR_NO_MEMORY;
            goto error_put_dealloc;
        }
    }
    else
    {
        filter_hdl_rec->ntfNotifyingObjects = NULL;
    }
    filter_hdl_rec->ntfNumNotifyingObjects = numNotifyingObjects;
    notificationFilter->notificationFilterHeader.notifyingObjects =
        filter_hdl_rec->ntfNotifyingObjects;

    /* Notification class Id:s */
    if (numNotificationClassIds != 0)
    {

        filter_hdl_rec->ntfNotificationClassIds =
            (SaNtfClassIdT *)
        malloc(numNotificationClassIds * sizeof (SaNtfClassIdT));
        if (filter_hdl_rec->ntfNotificationClassIds == NULL)
        {
            TRACE_1("Out of memory in notificationClassIds field");
            rc = SA_AIS_ERR_NO_MEMORY;
            goto error_put_dealloc;
        }
    }
    else
    {
        filter_hdl_rec->ntfNotificationClassIds = NULL;
    }
    filter_hdl_rec->ntfNumNotificationClassIds =
        numNotificationClassIds;
    notificationFilter->notificationFilterHeader.notificationClassIds =
        filter_hdl_rec->ntfNotificationClassIds;

    /* Body part */
    /* Probable causes */
    if (numProbableCauses != 0)
    {

        filter_hdl_rec->ntfProbableCauses =
            (SaNtfProbableCauseT *)
        malloc(numProbableCauses * sizeof (SaNtfProbableCauseT));
        if (filter_hdl_rec->ntfProbableCauses == NULL)
        {
            TRACE_1("Out of memory in probableCauses field");
            rc = SA_AIS_ERR_NO_MEMORY;
            goto error_put_dealloc;
        }
    }
    else
    {
        filter_hdl_rec->ntfProbableCauses = NULL;
    }
    filter_hdl_rec->ntfNumProbableCauses = numProbableCauses;
    notificationFilter->probableCauses =
        filter_hdl_rec->ntfProbableCauses;

    /* Percieved severities */
    if (numPerceivedSeverities != 0)
    {

        filter_hdl_rec->ntfPerceivedSeverities =
            (SaNtfSeverityT *)
        malloc(numPerceivedSeverities * sizeof (SaNtfSeverityT));
        if (filter_hdl_rec->ntfPerceivedSeverities == NULL)
        {
            TRACE_1("Out of memory in perceivedSeverities field");
            rc = SA_AIS_ERR_NO_MEMORY;
            goto error_put_dealloc;
        }
    }
    else
    {
        filter_hdl_rec->ntfPerceivedSeverities = NULL;
    }
    filter_hdl_rec->ntfNumPercievedSeverities =
        numPerceivedSeverities;
    notificationFilter->perceivedSeverities =
        filter_hdl_rec->ntfPerceivedSeverities;

    /* Severity trends */
    if (numTrends != 0)
    {

        filter_hdl_rec->ntfTrends =
            (SaNtfSeverityTrendT *)
        malloc(numTrends * sizeof (SaNtfSeverityTrendT));
        if (filter_hdl_rec->ntfTrends == NULL)
        {
            TRACE_1("Out of memory in trends field");
            rc = SA_AIS_ERR_NO_MEMORY;
            goto error_put_dealloc;
        }
    }
    else
    {
        filter_hdl_rec->ntfTrends = NULL;
    }
    filter_hdl_rec->ntfNumTrends = numTrends;
    notificationFilter->trends =
        filter_hdl_rec->ntfTrends;

    /* initialize the Client struct data */
    /* ================================= */

    /* Header part */
    notificationFilter->notificationFilterHeader.numEventTypes = numEventTypes;
    notificationFilter->notificationFilterHeader.numNotificationObjects =
        numNotificationObjects;
    notificationFilter->notificationFilterHeader.numNotifyingObjects =
        numNotifyingObjects;
    notificationFilter->notificationFilterHeader.numNotificationClassIds =
        numNotificationClassIds;

    /* Body part */
    notificationFilter->numProbableCauses = (SaUint16T) numProbableCauses;
    notificationFilter->numPerceivedSeverities =
        (SaUint16T) numPerceivedSeverities;
    notificationFilter->numTrends = (SaUint16T) numTrends;

    goto done_give_hdl;

    error_put_dealloc:
/* TODO: free here */
error_put:
    TRACE("ERROR, rc = %d!!!", rc);
done_give_hdl:
    ncshm_give_hdl(ntfHandle);
done:
    TRACE_LEAVE();
    return rc;

}

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
    SaUint32T numServiceProviders)
{
    return SA_AIS_ERR_NOT_SUPPORTED;
}

/*  3.15.2.10	saNtfNotificationFilterFree()  */
SaAisErrorT saNtfNotificationFilterFree(
    SaNtfNotificationFilterHandleT notificationFilterHandle)
{
    SaAisErrorT rc = SA_AIS_OK;
    unsigned int    client_handle;
    ntfa_filter_hdl_rec_t *filter_hdl_rec;

    ntfa_client_hdl_rec_t *client_rec;
    TRACE_ENTER();

    /**                 Lock ntfa_CB                 **/
    pthread_mutex_lock(&ntfa_cb.cb_lock);
    TRACE_1("filterHandle = %llu", notificationFilterHandle);
    /* retrieve filter hdl rec */
    filter_hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA,
        notificationFilterHandle);
    if (filter_hdl_rec == NULL)
    {
        pthread_mutex_unlock(&ntfa_cb.cb_lock);
        TRACE("ncshm_take_hdl notificationFilterHandle failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done;
    }
    client_handle =filter_hdl_rec->parent_hdl->local_hdl;
    /* retrieve client hdl rec */
    client_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA,
        client_handle);
    if (client_rec == NULL)
    {
        TRACE("ncshm_take_hdl client_handle failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done_give_hdl;
    }

    /* free the resources allocated by saNtf<ntfType>FilterAllocate */
    ntfa_filter_hdl_rec_destructor(filter_hdl_rec);


    /** Delete the resources related to the notificationFilterHandle &
     *  remove reference in the client.
    **/
    if (NCSCC_RC_SUCCESS != ntfa_filter_hdl_rec_del(&client_rec->filter_list,
        filter_hdl_rec))
    {
        TRACE("Unable to delete notifiction record");
        rc = SA_AIS_ERR_LIBRARY;
    }
    /**                  UnLockntfa_CB            **/
    pthread_mutex_unlock(&ntfa_cb.cb_lock);

    ncshm_give_hdl(client_handle);
done_give_hdl:
    ncshm_give_hdl(notificationFilterHandle);
done:
    TRACE_LEAVE();
    return rc;
}

/*  3.15.3.2	saNtfNotificationUnsubscribe()  */
/**
 * The saNtfNotificationUnsubscribe() function deletes the subscription
 * previously made with a call to saNtfNotificationSubscribe().
 *
 * @param subscriptionId
 *
 * @return SaAisErrorT
 */SaAisErrorT saNtfNotificationUnsubscribe(
    SaNtfSubscriptionIdT subscriptionId)
{
    TRACE_ENTER();
    SaAisErrorT rc = SA_AIS_ERR_NOT_EXIST;
    SaNtfHandleT ntfHandle;

    ntfa_subscriber_list_t           *listPtr;
    ntfa_subscriber_list_t           *listPtrNext;

    ntfa_client_hdl_rec_t *client_hdl_rec;

    ntfsv_msg_t                 msg, *o_msg = NULL;
    ntfsv_unsubscribe_req_t       *send_param;
    uns32                       timeout= NTFS_WAIT_TIME;

    if (NULL != subscriberNoList)
    {
        /* Check the list for entries */
        for (listPtr = subscriberNoList,
            listPtrNext = listPtr->next;
            listPtr != NULL;
            listPtr = listPtrNext,
            listPtrNext = listPtr->next)
        {
            TRACE_1("listPtr->SubscriptionId %d",
                listPtr->subscriberListSubscriptionId);
            if (listPtr->subscriberListSubscriptionId ==
                subscriptionId)
            {
                rc = SA_AIS_OK;
                break;
            }
            if (listPtrNext == NULL)
            {
                break;
            }
        }
    }

    if (rc == SA_AIS_ERR_NOT_EXIST)
    {
        goto done;
    }

    if (listPtr != NULL)
    {
        ntfHandle = listPtr->subscriberListNtfHandle;
        /* retrieve client hdl rec */
        client_hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, ntfHandle);
        if (client_hdl_rec == NULL)
        {
            TRACE_1("ncshm_take_hdl failed");
            rc = SA_AIS_ERR_BAD_HANDLE;
            goto done;
        }

        /**
         ** Populate a sync MDS message
         **/
        memset(&msg, 0, sizeof(ntfsv_msg_t));
        msg.type = NTFSV_NTFA_API_MSG;
        msg.info.api_info.type = NTFSV_UNSUBSCRIBE_REQ;
        send_param = &msg.info.api_info.param.unsubscribe;

        send_param->client_id = client_hdl_rec->ntfs_client_id;
        send_param->subscriptionId = subscriptionId;

        /* Check whether NTFS is up or not */
        if (!ntfa_cb.ntfs_up)
        {
            TRACE_1("NTFS down");
            rc = SA_AIS_ERR_TRY_AGAIN;
            goto done_give_hdl;
        }

        /* Send a sync MDS message to obtain a log stream id */
        rc = ntfa_mds_msg_sync_send(&ntfa_cb, &msg, &o_msg, timeout);
        if (rc != NCSCC_RC_SUCCESS)
        {
            if (o_msg)
                ntfa_msg_destroy(o_msg);
            rc = SA_AIS_ERR_TRY_AGAIN;
            goto done_give_hdl;
        }

        if (SA_AIS_OK != o_msg->info.api_resp_info.rc)
        {
            rc = o_msg->info.api_resp_info.rc;
            TRACE_1("Bad return status! rc = %d", rc);
            if (o_msg)
                ntfa_msg_destroy(o_msg);
            goto done_give_hdl;
        }
        TRACE_1("subscriptionId from server %u",
            o_msg->info.api_resp_info.param.unsubscribe_rsp.subscriptionId);

        if (listPtr->next != NULL)
        {
            listPtr->next->prev = listPtr->prev;
        }

        if (listPtr->prev != NULL)
        {
            listPtr->prev->next = listPtr->next;
        }
        else
        {
            if (listPtr->next != NULL)
                subscriberNoList = listPtr->next;
            else
                subscriberNoList = NULL;
        }
        TRACE_1("REMOVE: listPtr->SubscriptionId %d",
            listPtr->subscriberListSubscriptionId);
        free(listPtr);
    }

done_give_hdl:
    ncshm_give_hdl(ntfHandle);
done:
    TRACE_LEAVE();
    return rc;
}

/*  3.15.4	 Reader Operations  */
/*  3.15.4.1	saNtfNotificationReadInitialize()  */
SaAisErrorT saNtfNotificationReadInitialize(
    SaNtfSearchCriteriaT searchCriteria,
    const SaNtfNotificationTypeFilterHandlesT *notificationFilterHandles,
    SaNtfReadHandleT *readHandle)
{
    SaAisErrorT rc = SA_AIS_ERR_NOT_SUPPORTED;

    ntfa_filter_hdl_rec_t *filter_hdl_rec;
    ntfa_client_hdl_rec_t *client_hdl_rec;
    ntfa_reader_hdl_rec_t *reader_hdl_rec;

    ntfsv_msg_t                 msg, *o_msg = NULL;
    ntfsv_reader_init_req_t     *send_param;
    uns32                       timeout= NTFS_WAIT_TIME;

    TRACE_ENTER();
    if (notificationFilterHandles == NULL)
    {
        rc = SA_AIS_ERR_INVALID_PARAM;
        TRACE_1("notificationFilterHandles == NULL");
        goto done;
    }

    if ((searchCriteria.searchMode != SA_NTF_SEARCH_AT_OR_AFTER_TIME) &&
        (searchCriteria.searchMode != SA_NTF_SEARCH_ONLY_FILTER))
    {
        rc = SA_AIS_ERR_NOT_SUPPORTED;
        TRACE_1("searchMode = %d\n",
            (SaNtfSearchModeT)searchCriteria.searchMode);
        TRACE_1("searchCriteria->searchMode other than 'only filter' or 'at "
            "or after time' not supported!");
        goto done;
    }

    if (notificationFilterHandles->alarmFilterHandle != SA_NTF_FILTER_HANDLE_NULL)
    {
        TRACE_1("Getting notificationFilterHandle!");

        /* retrieve notification filter hdl rec */
        filter_hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA,
            notificationFilterHandles->alarmFilterHandle);
        if (filter_hdl_rec == NULL)
        {
            TRACE_1("ncshm_take_hdl failed");
            rc = SA_AIS_ERR_BAD_HANDLE;
            goto done;
        }
        /* retrieve client hdl rec */
        client_hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, filter_hdl_rec->ntfHandle);
        if (client_hdl_rec == NULL)
        {
            TRACE_1("ncshm_take_hdl failed");
            rc = SA_AIS_ERR_BAD_HANDLE;
            goto done_give_hdl;
        }
        /**                 Lock ntfa_CB                 **/
        pthread_mutex_lock(&ntfa_cb.cb_lock);
        reader_hdl_rec = ntfa_reader_hdl_rec_add(&client_hdl_rec);
        TRACE_1("reader_hdl_rec = %u", reader_hdl_rec->reader_hdl);
        if (reader_hdl_rec == NULL)
        {
            pthread_mutex_unlock(&ntfa_cb.cb_lock);
            rc = SA_AIS_ERR_NO_MEMORY;
            goto done_give_hdl;
        }
        /**                  UnLock ntfa_CB            **/
        pthread_mutex_unlock(&ntfa_cb.cb_lock);
        *readHandle = (SaNtfReadHandleT)reader_hdl_rec->reader_hdl;
        reader_hdl_rec->ntfHandle = filter_hdl_rec->ntfHandle;
    }
    else
    {
        TRACE_1("No filter except the alarm filter supported!");
        rc = SA_AIS_ERR_NOT_SUPPORTED;
        goto done_give_hdls;
    }

    /* TODO: Fill in eventTime and notificationId later on */

    /**
     ** Populate a sync MDS message
     **/
    memset(&msg, 0, sizeof(ntfsv_msg_t));
    msg.type = NTFSV_NTFA_API_MSG;
    msg.info.api_info.type = NTFSV_READER_INITIALIZE_REQ;
    send_param = &msg.info.api_info.param.reader_init;
    send_param->client_id = client_hdl_rec->ntfs_client_id;
    /* Fill in ipc send struct */
    send_param->searchCriteria.searchMode = searchCriteria.searchMode;

    /* Check whether NTFS is up or not */
    if (!ntfa_cb.ntfs_up)
    {
        TRACE("NTFS down");
        rc = SA_AIS_ERR_TRY_AGAIN;
        goto done_give_hdls;
    }

    /* Send a sync MDS message to obtain a log stream id */
    rc = ntfa_mds_msg_sync_send(&ntfa_cb, &msg, &o_msg, timeout);
    if (rc != NCSCC_RC_SUCCESS)
    {
        if (o_msg)
            ntfa_msg_destroy(o_msg);
        rc = SA_AIS_ERR_TRY_AGAIN;
        goto done_give_hdls;
    }

    if (SA_AIS_OK != o_msg->info.api_resp_info.rc)
    {
        rc = o_msg->info.api_resp_info.rc;
        TRACE("Bad return status!!! rc = %d", rc);
        if (o_msg)
            ntfa_msg_destroy(o_msg);
        goto done_give_hdls;
    }

    if (o_msg->info.api_resp_info.type != NTFSV_READER_INITIALIZE_RSP)
    {
        TRACE("msg type (%d) failed", (int)&o_msg->info.api_resp_info.type);
        rc = SA_AIS_ERR_LIBRARY;
        goto done_give_hdls;
    }

    /* Store the readerId returned from server */
    reader_hdl_rec->reader_id = o_msg->info.api_resp_info.param.reader_init_rsp.readerId;

done_give_hdls:
    ncshm_give_hdl(notificationFilterHandles->alarmFilterHandle);
done_give_hdl:
    ncshm_give_hdl(client_hdl_rec->local_hdl);
done:
    TRACE_LEAVE();
    return rc;
}

/*  3.15.4.2	saNtfNotificationReadFinalize()  */
SaAisErrorT saNtfNotificationReadFinalize(
    SaNtfReadHandleT readhandle)
{
    SaAisErrorT rc = SA_AIS_ERR_NOT_SUPPORTED;
    uns32 oas_rc = NCSCC_RC_FAILURE;

    ntfa_client_hdl_rec_t *client_hdl_rec;
    ntfa_reader_hdl_rec_t *reader_hdl_rec;

    ntfsv_msg_t                 msg, *o_msg = NULL;
    ntfsv_reader_finalize_req_t *send_param;
    uns32                       timeout= NTFS_WAIT_TIME;

    TRACE_ENTER();

    /* retrieve notification filter hdl rec */
    reader_hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, readhandle);
    if (reader_hdl_rec == NULL)
    {
        TRACE_1("ncshm_take_hdl failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done;
    }
    /* retrieve client hdl rec */
    client_hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, reader_hdl_rec->ntfHandle);
    if (client_hdl_rec == NULL)
    {
        TRACE_1("ncshm_take_hdl failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done_give_hdl;
    }
    TRACE_1("reader_hdl_rec = %u", reader_hdl_rec->reader_hdl);

    /**
     ** Populate a sync MDS message
     **/
    memset(&msg, 0, sizeof(ntfsv_msg_t));
    msg.type = NTFSV_NTFA_API_MSG;
    msg.info.api_info.type = NTFSV_READER_FINALIZE_REQ;
    send_param = &msg.info.api_info.param.reader_finalize;
    send_param->client_id = client_hdl_rec->ntfs_client_id;
    send_param->readerId = reader_hdl_rec->reader_id;

    /* Check whether NTFS is up or not */
    if (!ntfa_cb.ntfs_up)
    {
        TRACE("NTFS down");
        rc = SA_AIS_ERR_TRY_AGAIN;
        goto done_give_hdls;
    }

    /* Send a sync MDS message */
    rc = ntfa_mds_msg_sync_send(&ntfa_cb, &msg, &o_msg, timeout);
    if (rc != NCSCC_RC_SUCCESS)
    {
        if (o_msg)
            ntfa_msg_destroy(o_msg);
        rc = SA_AIS_ERR_TRY_AGAIN;
        goto done_give_hdls;
    }

    if (SA_AIS_OK != o_msg->info.api_resp_info.rc)
    {
        rc = o_msg->info.api_resp_info.rc;
        TRACE("Bad return status!!! rc = %d", rc);
        if (o_msg)
            ntfa_msg_destroy(o_msg);
        goto done_give_hdls;
    }

    if (o_msg->info.api_resp_info.type != NTFSV_READER_FINALIZE_RSP)
    {
        TRACE("msg type (%d) failed", (int)&o_msg->info.api_resp_info.type);
        rc = SA_AIS_ERR_LIBRARY;
        goto done_give_hdls;
    }

    if (NULL != o_msg->info.api_resp_info.param.read_next_rsp.readNotification)
    {
        free(o_msg->info.api_resp_info.param.read_next_rsp.readNotification);
    }

    oas_rc = ntfa_reader_hdl_rec_del(&client_hdl_rec->reader_list, reader_hdl_rec);
    if ( oas_rc != NCSCC_RC_SUCCESS)
    {
        oas_rc = SA_AIS_ERR_LIBRARY;
    }
    if (o_msg)
        ntfa_msg_destroy(o_msg);

done_give_hdls:
    ncshm_give_hdl(readhandle);
done_give_hdl:
    ncshm_give_hdl(client_hdl_rec->local_hdl);
done:
    TRACE_LEAVE();
    return rc;
}

/*  3.15.4.2	saNtfNotificationReadNext()  */
SaAisErrorT saNtfNotificationReadNext(
    SaNtfReadHandleT readHandle,
    SaNtfSearchDirectionT searchDirection,
    SaNtfNotificationsT *notification)
{
    SaAisErrorT rc = SA_AIS_ERR_NOT_SUPPORTED;

    ntfa_client_hdl_rec_t *client_hdl_rec;
    ntfa_reader_hdl_rec_t *reader_hdl_rec;

    ntfsv_msg_t                 msg, *o_msg = NULL;
    ntfsv_read_next_req_t *send_param;
    uns32                       timeout= NTFS_WAIT_TIME;
    ntfsv_send_not_req_t *read_not = NULL;

    TRACE_ENTER();

    /* retrieve notification filter hdl rec */
    reader_hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, readHandle);
    if (reader_hdl_rec == NULL)
    {
        TRACE_1("ncshm_take_hdl failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done;
    }
    /* retrieve client hdl rec */
    client_hdl_rec = ncshm_take_hdl(NCS_SERVICE_ID_NTFA, reader_hdl_rec->ntfHandle);
    if (client_hdl_rec == NULL)
    {
        TRACE_1("ncshm_take_hdl failed");
        rc = SA_AIS_ERR_BAD_HANDLE;
        goto done_give_hdl;
    }
    TRACE_1("reader_hdl_rec = %u", reader_hdl_rec->reader_hdl);

    /**
     ** Populate a sync MDS message
     **/
    memset(&msg, 0, sizeof(ntfsv_msg_t));
    msg.type = NTFSV_NTFA_API_MSG;
    msg.info.api_info.type = NTFSV_READ_NEXT_REQ;
    send_param = &msg.info.api_info.param.read_next;
    send_param->client_id = client_hdl_rec->ntfs_client_id;
    send_param->readerId = reader_hdl_rec->reader_id;
    send_param->searchDirection = searchDirection;
    /* Check whether NTFS is up or not */
    if (!ntfa_cb.ntfs_up)
    {
        TRACE("NTFS down");
        rc = SA_AIS_ERR_TRY_AGAIN;
        goto done_give_hdls;
    }

    /* Send a sync MDS message */
    rc = ntfa_mds_msg_sync_send(&ntfa_cb, &msg, &o_msg, timeout);
    if (rc != NCSCC_RC_SUCCESS)
    {
        if (o_msg)
            ntfa_msg_destroy(o_msg);
        rc = SA_AIS_ERR_TRY_AGAIN;
        goto done_give_hdls;
    }

    if (SA_AIS_OK != o_msg->info.api_resp_info.rc)
    {
        rc = o_msg->info.api_resp_info.rc;
        TRACE("error: response msg rc = %d", rc);
        if (o_msg)
            ntfa_msg_destroy(o_msg);
        goto done_give_hdls;
    }
    if (o_msg->info.api_resp_info.type != NTFSV_READ_NEXT_RSP)
    {
        TRACE("msg type (%d) failed", (int)&o_msg->info.api_resp_info.type);
        rc = SA_AIS_ERR_LIBRARY;
        goto done_give_hdls;
    }

    read_not = o_msg->info.api_resp_info.param.read_next_rsp.readNotification;

    /* Only alarm supported */
    if (read_not->notificationType == SA_NTF_TYPE_ALARM)
    {
        notification->notificationType =
            read_not->notificationType;
        notification->notification.alarmNotification = read_not->notification.alarm;
    }
    else
    {
        TRACE_1("Notification type (%d) is not alarm!", (int)read_not->notificationType);
        rc = SA_AIS_ERR_NOT_SUPPORTED;
    }

    if (o_msg)
        ntfa_msg_destroy(o_msg);
done_give_hdls:
    ncshm_give_hdl(readHandle);
done_give_hdl:
    ncshm_give_hdl(client_hdl_rec->local_hdl);
done:
    TRACE_LEAVE();
    return rc;
}

