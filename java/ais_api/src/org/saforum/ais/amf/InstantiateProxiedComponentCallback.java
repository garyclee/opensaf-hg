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
 * the callback requests a proxy component to instantiate a proxied component.
 *
 * <P><B>SAF Reference:</B> <code>SaAmfProxiedComponentInstantiateCallbackT</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public interface InstantiateProxiedComponentCallback {

    /**
     * The Availability Management Framework requests a proxy component to
     * instantiate a proxied component, identified by proxiedCompName. The proxy
     * component to which this request is addressed must have registered the
     * proxied component with the Availability Management Framework before the
     * Availability Management Framework invokes this method.
     * <P>
     * This callback is invoked in the context of a thread of a registered
     * process for a proxy component issuing a dispatch() call on the handle
     * AmfHandle, which was used when registering the component,
     * identified by proxiedCompName, via the registerComponent() call. The
     * invoked process responds by invoking the response() method, supplying
     * invocation and error as input parameters; in this case, error has one of
     * the following values defined by AisStatus:
     * <UL>
     * <LI>OK - The method completed successfully.
     * <LI>ERR_FAILED_OPERATION -The proxy component failed to instantiate the
     * proxied component. It is useless for the Availability Management
     * Framework to attempt to instantiate the proxied component again.
     * <LI>ERR_TRY_AGAIN - The proxy component failed to instantiate the proxied
     * component. The Availability Management Framework might issue a further
     * attempt to instantiate the proxied component.
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
     * @param proxiedCompName [in] The name of the proxied component to be
     *            instantiated.
     */
    void instantiateProxiedComponentCallback( long invocation, String proxiedCompName );

}


