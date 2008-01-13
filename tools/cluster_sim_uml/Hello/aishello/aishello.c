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

/*
 * aishello.c --
 *      An example SAF application. Intended as a quick
 *      introduction to SAF programming and configuration.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <poll.h>
#include <string.h>
#include <errno.h>

#include <saAmf.h>
#include "ais_utils.h"
/* #define syslog(l,arg...) printf(arg) */

/* AMF Callback declarations; */
static void healthcheckCallback(
    SaInvocationT invocation,
    const SaNameT *compName,
    SaAmfHealthcheckKeyT *healthcheckKey);
static void componentTerminateCallback(
    SaInvocationT invocation,
    const SaNameT *compName);
static void cSISetCallback(
    SaInvocationT invocation,
    const SaNameT *compName,
    SaAmfHAStateT haState,
    SaAmfCSIDescriptorT csiDescriptor);
static void cSIRemoveCallback(
    SaInvocationT invocation,
    const SaNameT *compName,
    const SaNameT *csiName,
    SaAmfCSIFlagsT csiFlags);


/* The handle is used in the call-backs (and is not passed for some
 * reason) so it has to be in file-scope. */
static SaAmfHandleT amfHandle;

/* ----------------------------------------------------------------------
 * Main program;
 */
int
main(int argc, char* argv[])
{
    SaNameT compName;
    SaVersionT version = {'B',1,1};
    SaAmfHealthcheckKeyT key1 = { "A9FD64E12C", 10 };
    SaAmfCallbacksT callbacks = {
        healthcheckCallback,
        componentTerminateCallback,
        cSISetCallback,
        cSIRemoveCallback,
        NULL, /* protectionGroupTrackCallback, */
        NULL, /* proxiedComponentInstantiateCallback, */
        NULL  /* proxiedComponentCleanupCallback */
    };
    SaSelectionObjectT fd = 0;
    struct pollfd fds[1];
    int retval;

    /* Detach and run as a daemon. This should be the normal behavior
     * of an AMF component. */
#if 1
    if (daemon(0, 0) != 0) {
        perror("daemon");
        exit(EXIT_FAILURE);
    }
#endif
    /* Since we run as a daemon, use the "syslog" for logging. SAF
     * will hopefully provide a logging API in the future. */
    /* openlog("aishello", LOG_PID, LOG_USER); */
    syslog(LOG_DEBUG, "Started ...");

    /* Init the AMF connection */
    aisCallq(saAmfInitialize, &amfHandle, &callbacks, &version);
    aisCallq(saAmfComponentNameGet, amfHandle, &compName);
    aisCallq(saAmfComponentRegister, amfHandle, &compName, NULL);
    aisCallq(saAmfSelectionObjectGet, amfHandle, &fd);
    syslog(LOG_DEBUG, "Registered as [%s]", strSaNameT(&compName));

    fds[0].fd = fd;
    fds[0].events = POLLIN;

    /* Start health-check */
    aisCallq(saAmfHealthcheckStart, amfHandle, &compName, &key1,
             SA_AMF_HEALTHCHECK_AMF_INVOKED, SA_AMF_COMPONENT_RESTART);

    /* Main loop. */
    for (;;) {
        retval = poll (fds, 1, -1); 
        if (retval > 0) {
            if (fds[0].revents & POLLERR) {
                syslog(LOG_DEBUG, "poll error, exiting... (%x)\n", fds[0].revents);
                exit(1);
            }

            /* data to be read */
            if (fds[0].revents & POLLIN) {
                aisCallq(saAmfDispatch, amfHandle, SA_DISPATCH_ALL);
            }
        } else {
            syslog (LOG_ERR, "poll failed - %s", strerror (errno));
            if (retval == -1 && errno != EINTR) {
                exit(-1);
            }
        }
    }
    return 0;
}


/* ----------------------------------------------------------------------
 * AMF callback-functions;
 */
static void healthcheckCallback(
    SaInvocationT invocation,
    const SaNameT *compName,
    SaAmfHealthcheckKeyT *healthcheckKey)
{
    syslog(LOG_DEBUG, "healthcheckCallback: ...");
    aisCallq(saAmfResponse, amfHandle, invocation, SA_AIS_OK);
}
static void componentTerminateCallback(
    SaInvocationT invocation,
    const SaNameT *compName)
{
    syslog(LOG_DEBUG, "componentTerminateCallback: exiting...");
    exit(EXIT_SUCCESS);
}
static void cSISetCallback(
    SaInvocationT invocation,
    const SaNameT *compName,
    SaAmfHAStateT haState,
    SaAmfCSIDescriptorT csiDescriptor)
{
    syslog(LOG_DEBUG, "cSISetCallback: %s, CSI=%s", 
           strSaAmfHAStateT(haState), strSaNameT(&csiDescriptor.csiName));
        if (csiDescriptor.csiFlags & SA_AMF_CSI_ADD_ONE) {
        SaAmfCSIAttributeT *attr;
                int i;
                for (i = 0; i < csiDescriptor.csiAttr.number; i++) {
                        attr = &csiDescriptor.csiAttr.attr[i];
                        syslog(LOG_DEBUG, "\tname: %s, value: %s",
                                attr->attrName, attr->attrValue);
                }
        }
    aisCallq(saAmfResponse, amfHandle, invocation, SA_AIS_OK);
}
static void cSIRemoveCallback(
    SaInvocationT invocation,
    const SaNameT *compName,
    const SaNameT *csiName,
    SaAmfCSIFlagsT csiFlags)
{
    syslog(LOG_DEBUG, "cSIRemoveCallback: CSI=%s", strSaNameT(csiName));
    aisCallq(saAmfResponse, amfHandle, invocation, SA_AIS_OK);
}
