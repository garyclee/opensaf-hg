/*      -*- OpenSAF  -*-
 *
 *  Copyright (c) 2007, Ericsson AB
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under High-Availability Operating 
 * Environment Software License Version 1.4.
 * Complete License can be accesseble from below location.
 * http://www.opensaf.org/license 
 * See the Copying file included with the OpenSAF distribution for
 * full licensing terms.
 *
 * Author(s):
 *   
 */

#include <assert.h>
#include <unistd.h>
#include <stdarg.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ais_utils.h>
#define syslog(l,arg...) printf(arg)

#define DEFAULT_MAXTRIES 10
#define DEFAULT_RETRYDELAY 200000 /* 1/5 second */
static unsigned int maxtries = DEFAULT_MAXTRIES;
static unsigned int retrydelay = DEFAULT_RETRYDELAY;

char const* strSaAisErrorT(SaAisErrorT rc)
{
    switch (rc) {
    case SA_AIS_OK: return "SA_AIS_OK";
    case SA_AIS_ERR_LIBRARY: return "SA_AIS_ERR_LIBRARY";
    case SA_AIS_ERR_VERSION: return "SA_AIS_ERR_VERSION";
    case SA_AIS_ERR_INIT: return "SA_AIS_ERR_INIT";
    case SA_AIS_ERR_TIMEOUT: return "SA_AIS_ERR_TIMEOUT";
    case SA_AIS_ERR_TRY_AGAIN: return "SA_AIS_ERR_TRY_AGAIN";
    case SA_AIS_ERR_INVALID_PARAM: return "SA_AIS_ERR_INVALID_PARAM";
    case SA_AIS_ERR_NO_MEMORY: return "SA_AIS_ERR_NO_MEMORY";
    case SA_AIS_ERR_BAD_HANDLE: return "SA_AIS_ERR_BAD_HANDLE";
    case SA_AIS_ERR_BUSY: return "SA_AIS_ERR_BUSY";
    case SA_AIS_ERR_ACCESS: return "SA_AIS_ERR_ACCESS";
    case SA_AIS_ERR_NOT_EXIST: return "SA_AIS_ERR_NOT_EXIST";
    case SA_AIS_ERR_NAME_TOO_LONG: return "SA_AIS_ERR_NAME_TOO_LONG";
    case SA_AIS_ERR_EXIST: return "SA_AIS_ERR_EXIST";
    case SA_AIS_ERR_NO_SPACE: return "SA_AIS_ERR_NO_SPACE";
    case SA_AIS_ERR_INTERRUPT: return "SA_AIS_ERR_INTERRUPT";
    case SA_AIS_ERR_NAME_NOT_FOUND: return "SA_AIS_ERR_NAME_NOT_FOUND";
    case SA_AIS_ERR_NO_RESOURCES: return "SA_AIS_ERR_NO_RESOURCES";
    case SA_AIS_ERR_NOT_SUPPORTED: return "SA_AIS_ERR_NOT_SUPPORTED";
    case SA_AIS_ERR_BAD_OPERATION: return "SA_AIS_ERR_BAD_OPERATION";
    case SA_AIS_ERR_FAILED_OPERATION: return "SA_AIS_ERR_FAILED_OPERATION";
    case SA_AIS_ERR_MESSAGE_ERROR: return "SA_AIS_ERR_MESSAGE_ERROR";
    case SA_AIS_ERR_QUEUE_FULL: return "SA_AIS_ERR_QUEUE_FULL";
    case SA_AIS_ERR_QUEUE_NOT_AVAILABLE: 
        return "SA_AIS_ERR_QUEUE_NOT_AVAILABLE";
    case SA_AIS_ERR_BAD_FLAGS: return "SA_AIS_ERR_BAD_FLAGS";
    case SA_AIS_ERR_TOO_BIG: return "SA_AIS_ERR_TOO_BIG";
    case SA_AIS_ERR_NO_SECTIONS: return "SA_AIS_ERR_NO_SECTIONS";
    }
    return "(unknown err)";
}

char const* strSaAmfHAStateT(SaAmfHAStateT s)
{
    switch (s) {
    case SA_AMF_HA_ACTIVE: return "SA_AMF_HA_ACTIVE";
    case SA_AMF_HA_STANDBY: return "SA_AMF_HA_STANDBY";
    case SA_AMF_HA_QUIESCED: return "SA_AMF_HA_QUIESCED";
    case SA_AMF_HA_QUIESCING: return "SA_AMF_HA_QUIESCING";
    }
    return "(unknown)";
}

char const* strSaNameT(SaNameT const* name)
{
    static char buf[SA_MAX_NAME_LENGTH+1];
    if (name->length == 0) return "";
    if (name->length < SA_MAX_NAME_LENGTH &&
        (name->value[name->length] == 0 || name->value[name->length-1] == 0))
        return (char const*)name->value;

    memcpy(buf, name->value, name->length);
    buf[name->length] = 0;
    return buf;
}

SaAisErrorT aisCallq(SaAisErrorT (*fn)(), ...)
{
    SaAisErrorT rc = SA_AIS_ERR_TRY_AGAIN;
    unsigned int i = 0;
    char const* fnName = "";

    va_list ap;
    va_start(ap, fn);

    if (fn == saAmfInitialize) {
        SaAmfHandleT* amfHandle = va_arg(ap, SaAmfHandleT*);
        SaAmfCallbacksT* callbacks = va_arg(ap, SaAmfCallbacksT*);
        SaVersionT* version = va_arg(ap, SaVersionT*);
        fnName = "saAmfInitialize";
        rc = saAmfInitialize(amfHandle, callbacks, version);
        while (rc == SA_AIS_ERR_TRY_AGAIN && i < maxtries) {
            usleep(retrydelay);
            i++;
            rc = saAmfInitialize(amfHandle, callbacks, version);
        }
    } else if (fn == saAmfComponentNameGet) {
        SaAmfHandleT handle = va_arg(ap, SaAmfHandleT);
        SaNameT* compName = va_arg(ap, SaNameT*);
        fnName = "saAmfComponentNameGet";
        rc = saAmfComponentNameGet(handle, compName);
        while (rc == SA_AIS_ERR_TRY_AGAIN && i < maxtries) {
            usleep(retrydelay);
            i++;
            rc = saAmfComponentNameGet(handle, compName);
        }
    } else if (fn == saAmfComponentRegister) {
        SaAmfHandleT handle = va_arg(ap, SaAmfHandleT);
        SaNameT* compName = va_arg(ap, SaNameT*);
        SaNameT* proxyCompName = va_arg(ap, SaNameT*);
        fnName = "saAmfComponentRegister";
        rc = saAmfComponentRegister(handle, compName, proxyCompName);
        while (rc == SA_AIS_ERR_TRY_AGAIN && i < maxtries) {
            usleep(retrydelay);
            i++;
            rc = saAmfComponentRegister(handle, compName, proxyCompName);
        }
    } else if (fn == saAmfDispatch) {
        SaAmfHandleT handle = va_arg(ap, SaAmfHandleT);
        SaDispatchFlagsT dispatchFlags = va_arg(ap, SaDispatchFlagsT);
        fnName = "saAmfDispatch";
        rc = saAmfDispatch(handle, dispatchFlags);
        while (rc == SA_AIS_ERR_TRY_AGAIN && i < maxtries) {
            usleep(retrydelay);
            i++;
            rc = saAmfDispatch(handle, dispatchFlags);
        }
    } else if (fn == saAmfSelectionObjectGet) {
        SaAmfHandleT handle = va_arg(ap, SaAmfHandleT);
        SaSelectionObjectT *fd = va_arg(ap, SaSelectionObjectT*);
        fnName = "saAmfSelectionObjectGet";
        rc = saAmfSelectionObjectGet(handle, fd);
        while (rc == SA_AIS_ERR_TRY_AGAIN && i < maxtries) {
            usleep(retrydelay);
            i++;
            rc = saAmfSelectionObjectGet(handle, fd);
        }
    } else if (fn == saAmfResponse) {
        SaAmfHandleT handle = va_arg(ap, SaAmfHandleT);
        SaInvocationT invocation = va_arg(ap, SaInvocationT);
        SaAisErrorT error = va_arg(ap, SaAisErrorT);
        fnName = "saAmfResponse";
        rc = saAmfResponse(handle, invocation, error);
        while (rc == SA_AIS_ERR_TRY_AGAIN && i < maxtries) {
            usleep(retrydelay);
            i++;
            rc = saAmfResponse(handle, invocation, error);
        }
    } else if (fn == saAmfHealthcheckStart) {
        SaAmfHandleT handle = va_arg(ap, SaAmfHandleT);
        SaNameT* compName = va_arg(ap, SaNameT*);
        SaAmfHealthcheckKeyT* healthcheckKey = 
            va_arg(ap, SaAmfHealthcheckKeyT*);
        SaAmfHealthcheckInvocationT invocationType = 
            va_arg(ap, SaAmfHealthcheckInvocationT);
        SaAmfRecommendedRecoveryT recommendedRecovery 
            = va_arg(ap, SaAmfRecommendedRecoveryT);
        fnName = "saAmfHealthcheckStart";
        rc = saAmfHealthcheckStart(handle, compName, healthcheckKey, 
                                   invocationType, recommendedRecovery);
        while (rc == SA_AIS_ERR_TRY_AGAIN && i < maxtries) {
            usleep(retrydelay);
            i++;
            rc = saAmfHealthcheckStart(handle, compName, healthcheckKey, 
                                       invocationType, recommendedRecovery);
        }
    } else {
        assert(0);
    }
    va_end(ap);
    if (rc != SA_AIS_OK) {
        syslog(LOG_ERR, "Failed: %s, rc = %d (%s)", 
               fnName, (int)rc, strSaAisErrorT(rc));
        exit(EXIT_FAILURE);
    }
    return rc;
}

