/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R2-JD-A.01.01
**   SAI-Overview-B.01.01
**   SAI-AIS-AMF-B.01.01
**
** DATE:
**   Wed Aug 6 2008
**
** LEGAL:
**   OWNERSHIP OF SPECIFICATION AND COPYRIGHTS.
**
** Copyright 2008 by the Service Availability Forum. All rights reserved.
**
** Permission to use, copy, and distribute this mapping specification for any
** purpose without fee is hereby granted, provided that this entire notice
** is included in all copies. No permission is granted for, and users are
** prohibited from, modifying or making derivative works of the mapping
** specification.
**
*******************************************************************************/

package org.saforum.ais.amf;


/**
 * This interface supports a callback by the Availability Management Framework:
 * the callback requests the component to terminate.
 *
 * <P><B>SAF Reference:</B> <code>SaAmfComponentTerminateCallbackT</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public interface TerminateComponentCallback {

    /**
     * The Availability Management Framework requests the component, identified
     * by componentName, to terminate. To terminate a proxied component, the
     * Availability Management Framework invokes this method on the proxy
     * component that is proxying the component identified by componentName. The
     * component, identified by componentName, is expected to release all
     * acquired resources and to terminate itself. The invoked process responds
     * by invoking the response() method. On return from the response()
     * method, the Availability Management Framework removes all service
     * instances associated with the component, and the component terminates.
     * <P>
     * This callback is invoked in the context of a thread of a registered
     * process issuing a dispatch() call on the handle AmfHandle, which
     * was used when registering the component, identified by componentName, via
     * the registerComponent() call. The component supplies invocation and an
     * error code as in parameters to the response() method. The error code,
     * in this case, is one of the following values defined by
     * AisStatus:
     * <UL>
     * <LI>OK - The method completed successfully.
     * <LI>ERR_FAILED_OPERATION - The component identified in componentName failed
     * to terminate.
     * </UL>
     * <P>If the invoked process does not respond with the AmfHandle.response() method within a
     * configured time interval or returns an error, the Availability Management Framework
     * must engage the configured recovery policy (recoveryOnError) for the component to
     * which the process belongs.
     *
     *
     * @param invocation [in] This parameter designates a particular invocation
     *            of this callback method. The invoked process returns
     *            invocation when it responds to the Availability Management
     *            Framework using the response() method of AmfHandle.
     * @param componentName [in] The name of the component to be terminated.
     */
    void terminateComponentCallback( long invocation, String componentName );

}


