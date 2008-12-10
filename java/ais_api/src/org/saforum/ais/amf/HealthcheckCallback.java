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
 * the callback requests the component to perform a healthcheck.
 *
 * <P><B>SAF Reference:</B> <code>SaAmfHealthcheckCallbackT</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public interface HealthcheckCallback {

    /**
     * The Availability Management Framework requests the component, identified
     * by componentName, to perform a healthcheck specified by healthcheckKey.
     * The Availability Management Framework may ask a proxy component to
     * execute a healthcheck on one of its proxied components.
     * <P>
     * This callback is invoked in the context of a thread issuing a dispatch()
     * call on the handle, which was specified when starting the healthcheck
     * operation via the startHealthcheck() call. The Availability Management
     * Framework provides the invocation parameter that the invoked process uses
     * in the response() method to notify the Availability Management Framework
     * that it has completed the healthcheck. The component reports the result
     * of its healthcheck to the Availability Management Framework using the
     * error parameter of the response() method, which in this case has one of
     * the following values defined in AisException:
     * <UL>
     * <LI>OK - The healthcheck completed successfully.
     * <LI>ERR_FAILED_OPERATION - The component failed to successfully execute the
     * given healthcheck and has reported an error on the faulty component using
     * reportComponentError().
     * </UL>
     * <P>If the invoked process does not respond with the AmfHandle.response() method within a
     * configured time interval or returns an error, the Availability Management Framework
     * must engage the configured recovery policy (recoveryOnError) for the component to
     * which the process belongs.
     *
     * @param invocation [in] This parameter designates a particular invocation
     *            of this callback method. The invoked process returns
     *            invocation when it responds to the Availability Management
     *            Framework using the response() method of AmfHandle.
     * @param componentName [in] The name of the component that must undergo the
     *            particular healthcheck.
     * @param healthcheckKey [in] The key of the healthcheck to be executed.
     *            Using this key, the Availability Management Framework can
     *            retrieve the corresponding healthcheck parameters.
     */
    void healthcheckCallback( long invocation,
                             String componentName,
                             byte[] healthcheckKey );

}


