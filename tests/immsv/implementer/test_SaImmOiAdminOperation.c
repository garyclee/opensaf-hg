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

#include <poll.h>
#include <unistd.h>
#include <pthread.h>
#include "immtest.h"

static SaNameT testObjectName = 
    {sizeof("opensafImm=opensafImm,safApp=safImmService"), 
     "opensafImm=opensafImm,safApp=safImmService"};
static SaAisErrorT saImmOiAdminOperationCallback_response = SA_AIS_OK;
static SaImmAdminOperationIdT operationId = 0xdead;
static SaUint64T value = 0xbad;
static SaBoolT objectImplementerIsSet;
static SaAisErrorT immReturnValue;
static SaAisErrorT operationReturnValue;
static SaInvocationT userInvocation = 0xdead;

static void saImmOiAdminOperationCallback(SaImmOiHandleT immOiHandle,
    SaInvocationT invocation,
    const SaNameT *objectName,
    SaImmAdminOperationIdT opId,
    const SaImmAdminOperationParamsT_2 **params)
{   
    TRACE_ENTER2("Returning %s\n", get_saf_error(saImmOiAdminOperationCallback_response));
    assert(opId == operationId);
    assert(*((SaUint64T*) params[0]->paramBuffer) == value);

    if (saImmOiAdminOperationCallback_response != SA_AIS_ERR_TIMEOUT)
        safassert(saImmOiAdminOperationResult(immOiHandle, invocation,
            saImmOiAdminOperationCallback_response), SA_AIS_OK);
    TRACE_LEAVE();
}

static const SaImmOiCallbacksT_2 oiCallbacks =
    {.saImmOiAdminOperationCallback = saImmOiAdminOperationCallback};

static void saImmOmAdminOperationInvokeCallback(
    SaInvocationT invocation,
    SaAisErrorT opRetVal,
    SaAisErrorT error)
{
    TRACE_ENTER2("%llu (0x%llx), %u, %u\n", invocation, invocation, opRetVal, error);
    assert(invocation == userInvocation);
    operationReturnValue = opRetVal;
    immReturnValue = error;
    TRACE_LEAVE();
}

static const SaImmCallbacksT omCallbacks = {saImmOmAdminOperationInvokeCallback};

/**
 * 
 * @param arg - ptr to objectName
 * 
 * @return void*
 */
static void *objectImplementerThreadMain(void *arg)
{
    struct pollfd fds[1];
    int ret;
    const SaImmOiImplementerNameT implementerName = (SaImmOiImplementerNameT) __FUNCTION__;
    SaSelectionObjectT selObj;
    SaImmHandleT handle;
    const SaNameT *objectName = arg;

    TRACE_ENTER();
    safassert(saImmOiInitialize_2(&handle, &oiCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOiImplementerSet(handle, implementerName), SA_AIS_OK);
    safassert(saImmOiObjectImplementerSet(handle, objectName, SA_IMM_ONE), SA_AIS_OK);
    objectImplementerIsSet = SA_TRUE;
    safassert(saImmOiSelectionObjectGet(handle, &selObj), SA_AIS_OK);

    fds[0].fd = (int) selObj;
    fds[0].events = POLLIN;

    ret = poll(fds, 1, 1000);
    if (ret == 0)
    {
        TRACE("poll timeout\n");
        goto done;
    }
    if (ret == -1)
    {
        fprintf(stderr, "poll error: %s\n", strerror(errno));
        goto done;
    }

    safassert(saImmOiDispatch(handle, SA_DISPATCH_ONE), SA_AIS_OK);

 done:
    safassert(saImmOiObjectImplementerRelease(handle, objectName, SA_IMM_ONE), SA_AIS_OK);
    safassert(saImmOiFinalize(handle), SA_AIS_OK);

    TRACE_LEAVE();
    return NULL;
}

static SaAisErrorT om_admin_exec(SaAisErrorT *imm_rc, const SaNameT *objectName, SaAisErrorT in_rc)
{
    SaAisErrorT rc;
    SaImmHandleT handle;
    const SaImmAdminOwnerNameT adminOwnerName = (SaImmAdminOwnerNameT) __FUNCTION__;
    SaImmAdminOwnerHandleT ownerHandle;
    const SaNameT *objectNames[] = {objectName, NULL};
    SaImmAdminOperationParamsT_2 param = {
        "TEST",
        SA_IMM_ATTR_SAUINT64T,
        &value
    };
    const SaImmAdminOperationParamsT_2 *params[] = {&param, NULL};

    if (in_rc == SA_AIS_ERR_INVALID_PARAM)
        param.paramType = -1;

    TRACE_ENTER();
    safassert(saImmOmInitialize(&handle, NULL, &immVersion), SA_AIS_OK);
    safassert(saImmOmAdminOwnerInitialize(handle, adminOwnerName, SA_TRUE, &ownerHandle), SA_AIS_OK);
    safassert(saImmOmAdminOwnerSet(ownerHandle, objectNames, SA_IMM_SUBTREE), SA_AIS_OK);

    *imm_rc = saImmOmAdminOperationInvoke_2(
        ownerHandle, &testObjectName, 0, operationId,
        params, &rc, SA_TIME_ONE_SECOND);

    safassert(saImmOmAdminOwnerFinalize(ownerHandle), SA_AIS_OK);
    safassert(saImmOmFinalize(handle), SA_AIS_OK);

    TRACE_LEAVE();
    return rc;
}

void SaImmOiAdminOperation_01(void)
{
    int ret;
    pthread_t thread;
    SaAisErrorT imm_rc;

    TRACE_ENTER();
    objectImplementerIsSet = SA_FALSE;
    ret = pthread_create(&thread, NULL, objectImplementerThreadMain, &testObjectName);
    assert(ret == 0);

    while (!objectImplementerIsSet)
        usleep(100);

    rc = om_admin_exec(&imm_rc, &testObjectName, SA_AIS_OK);

    pthread_join(thread, NULL);

    if (imm_rc != SA_AIS_OK)
        TRACE("om_admin_exec failed: %s\n", get_saf_error(imm_rc));

    assert(imm_rc == SA_AIS_OK);
    test_validate(rc, SA_AIS_OK);
    TRACE_LEAVE();
}

void SaImmOiAdminOperation_02(void)
{
    SaAisErrorT operationReturnValue;
    SaImmHandleT handle;
    const SaImmAdminOwnerNameT adminOwnerName = (SaImmAdminOwnerNameT) __FUNCTION__;
    SaImmAdminOwnerHandleT ownerHandle;
    const SaNameT *objectNames[] = {&testObjectName, NULL};

    SaImmAdminOperationParamsT_2 param = {
        "TEST",
        SA_IMM_ATTR_SAUINT64T,
        &value
    };
    const SaImmAdminOperationParamsT_2 *params[] = {&param, NULL};

    TRACE_ENTER();
    safassert(saImmOmInitialize(&handle, NULL, &immVersion), SA_AIS_OK);

    rc = saImmOmAdminOperationInvoke_2(
        -1, &testObjectName, 0, operationId,
        params, &operationReturnValue, SA_TIME_ONE_SECOND);
    if (rc != SA_AIS_ERR_BAD_HANDLE)
        goto done;

    safassert(saImmOmAdminOwnerInitialize(handle, adminOwnerName, SA_TRUE, &ownerHandle), SA_AIS_OK);
    safassert(saImmOmAdminOwnerSet(ownerHandle, objectNames, SA_IMM_SUBTREE), SA_AIS_OK);
    safassert(saImmOmAdminOwnerFinalize(ownerHandle), SA_AIS_OK);
    rc = saImmOmAdminOperationInvoke_2(
        ownerHandle, &testObjectName, 0, operationId,
        params, &operationReturnValue, SA_TIME_ONE_SECOND);

done:
    safassert(saImmOmFinalize(handle), SA_AIS_OK);

    test_validate(rc, SA_AIS_ERR_BAD_HANDLE);
    TRACE_LEAVE();
}

void SaImmOiAdminOperation_03(void)
{
    int ret;
    pthread_t thread;
    SaAisErrorT imm_rc;

    TRACE_ENTER();

    (void) om_admin_exec(&imm_rc, &testObjectName, SA_AIS_ERR_INVALID_PARAM);
    if (imm_rc != SA_AIS_ERR_INVALID_PARAM)
        goto done;

    objectImplementerIsSet = SA_FALSE;
    ret = pthread_create(&thread, NULL, objectImplementerThreadMain, &testObjectName);
    assert(ret == 0);

    while (!objectImplementerIsSet)
        usleep(100);

    saImmOiAdminOperationCallback_response = SA_AIS_ERR_INVALID_PARAM;
    rc = om_admin_exec(&imm_rc, &testObjectName, SA_AIS_OK);

    pthread_join(thread, NULL);

    if (imm_rc != SA_AIS_OK)
    {
        TRACE("om_admin_exec failed: %s\n", get_saf_error(imm_rc));
        goto done;
    }

done:
    test_validate(rc, SA_AIS_ERR_INVALID_PARAM);
    saImmOiAdminOperationCallback_response = SA_AIS_OK;
    TRACE_LEAVE();
}

void SaImmOiAdminOperation_04(void)
{
    SaAisErrorT rc;
    SaImmHandleT handle;
    const SaImmAdminOwnerNameT adminOwnerName = (SaImmAdminOwnerNameT) __FUNCTION__;
    SaImmAdminOwnerHandleT ownerHandle;
    SaImmAdminOperationParamsT_2 param = {
        "TEST",
        SA_IMM_ATTR_SAUINT64T,
        &value
    };
    const SaImmAdminOperationParamsT_2 *params[] = {&param, NULL};

    TRACE_ENTER();
    safassert(saImmOmInitialize(&handle, NULL, &immVersion), SA_AIS_OK);
    safassert(saImmOmAdminOwnerInitialize(handle, adminOwnerName, SA_TRUE, &ownerHandle), SA_AIS_OK);

    rc = saImmOmAdminOperationInvoke_2(
        ownerHandle, &testObjectName, 0, operationId,
        params, &rc, SA_TIME_ONE_SECOND);

    safassert(saImmOmAdminOwnerFinalize(ownerHandle), SA_AIS_OK);
    safassert(saImmOmFinalize(handle), SA_AIS_OK);
    test_validate(rc, SA_AIS_ERR_BAD_OPERATION);
    TRACE_LEAVE();
}

void SaImmOiAdminOperation_05(void)
{
    int ret;
    pthread_t thread;
    SaAisErrorT imm_rc;

    TRACE_ENTER();
    objectImplementerIsSet = SA_FALSE;
    ret = pthread_create(&thread, NULL, objectImplementerThreadMain, &testObjectName);
    assert(ret == 0);

    while (!objectImplementerIsSet)
        usleep(100);

    saImmOiAdminOperationCallback_response = SA_AIS_ERR_TIMEOUT;
    rc = om_admin_exec(&imm_rc, &testObjectName, SA_AIS_OK);

    pthread_join(thread, NULL);

    if (imm_rc != SA_AIS_ERR_TIMEOUT)
        TRACE("om_admin_exec failed: %s\n", get_saf_error(imm_rc));
    test_validate(imm_rc, SA_AIS_ERR_TIMEOUT);
    saImmOiAdminOperationCallback_response = SA_AIS_OK;
    TRACE_LEAVE();
}

void SaImmOiAdminOperation_06(void)
{
    SaAisErrorT imm_rc;

    TRACE_ENTER();
    rc = om_admin_exec(&imm_rc, &testObjectName, SA_AIS_OK);
    test_validate(imm_rc, SA_AIS_ERR_NOT_EXIST);
    TRACE_LEAVE();
}

void SaImmOiAdminOperation_07(void)
{
    struct pollfd fds[1];
    int ret;
    pthread_t thread;
    SaAisErrorT rc;
    SaImmHandleT handle;
    const SaImmAdminOwnerNameT adminOwnerName = (SaImmAdminOwnerNameT) __FUNCTION__;
    SaImmAdminOwnerHandleT ownerHandle;
    const SaNameT *objectNames[] = {&testObjectName, NULL};
    SaSelectionObjectT selObj;
    SaImmAdminOperationParamsT_2 param = {
        "TEST",
        SA_IMM_ATTR_SAUINT64T,
        &value
    };
    const SaImmAdminOperationParamsT_2 *params[] = {&param, NULL};

    TRACE_ENTER();

    objectImplementerIsSet = SA_FALSE;
    ret = pthread_create(&thread, NULL, objectImplementerThreadMain, &testObjectName);
    assert(ret == 0);
    while (!objectImplementerIsSet)
        usleep(100);

    safassert(saImmOmInitialize(&handle, &omCallbacks, &immVersion), SA_AIS_OK);
    safassert(saImmOmSelectionObjectGet(handle, &selObj), SA_AIS_OK);
    safassert(saImmOmAdminOwnerInitialize(handle, adminOwnerName, SA_TRUE, &ownerHandle), SA_AIS_OK);
    safassert(saImmOmAdminOwnerSet(ownerHandle, objectNames, SA_IMM_SUBTREE), SA_AIS_OK);
    safassert(saImmOmAdminOperationInvokeAsync_2(
        ownerHandle, userInvocation, &testObjectName, 0, operationId, params), SA_AIS_OK);

    fds[0].fd = (int) selObj;
    fds[0].events = POLLIN;

    ret = poll(fds, 1, 1000);
    if (ret == 0)
    {
        TRACE("poll timeout\n");
        rc = SA_AIS_ERR_TIMEOUT;
        goto done;
    }
    if (ret == -1)
    {
        fprintf(stderr, "poll error: %s\n", strerror(errno));
        rc = SA_AIS_ERR_FAILED_OPERATION;
        goto done;
    }

    immReturnValue = SA_AIS_ERR_FAILED_OPERATION;
    safassert(saImmOmDispatch(handle, SA_DISPATCH_ONE), SA_AIS_OK);

done:
    safassert(saImmOmAdminOwnerFinalize(ownerHandle), SA_AIS_OK);
    safassert(saImmOmFinalize(handle), SA_AIS_OK);

    pthread_join(thread, NULL);

    TRACE("%s", get_saf_error(immReturnValue));
    assert(immReturnValue == SA_AIS_OK);
    test_validate(operationReturnValue, SA_AIS_OK);
    TRACE_LEAVE();
}

__attribute__ ((constructor)) static void saImmOiInitialize_2_constructor(void)
{
    test_suite_add(5, "Administrative Operations");
    test_case_add(5, SaImmOiAdminOperation_01, "SaImmOiAdminOperation - SA_AIS_OK");
    test_case_add(5, SaImmOiAdminOperation_02, "SaImmOiAdminOperation - SA_AIS_ERR_BAD_HANDLE");
    test_case_add(5, SaImmOiAdminOperation_03, "SaImmOiAdminOperation - SA_AIS_ERR_INVALID_PARAM");
    test_case_add(5, SaImmOiAdminOperation_04, "SaImmOiAdminOperation - SA_AIS_ERR_BAD_OPERATION");
    test_case_add(5, SaImmOiAdminOperation_05, "SaImmOiAdminOperation - SA_AIS_ERR_TIMEOUT");
    test_case_add(5, SaImmOiAdminOperation_06, "SaImmOiAdminOperation - SA_AIS_ERR_NOT_EXIST");
    test_case_add(5, SaImmOiAdminOperation_07, "SaImmOiAdminOperation, OM async - SA_AIS_OK");
}

