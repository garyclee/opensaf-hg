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
 * the callback requests the component to assume a particular HA state for one
 * or all component service instances.
 *
 * <P><B>SAF Reference:</B> <code>SaAmfCSISetCallbackT</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public interface SetCsiCallback {

    /**
     * The Availability Management Framework invokes this callback to request
     * the component, identified by componentName, to assume a particular HA
     * state, specified by haState, for one or all component service instances.
     * The component service instances targeted by this call along with
     * additional information about them are provided by the csiDescriptor
     * parameter.
     * <P>If the haState parameter indicates the new HA state for the CSI(s) is
     * quiescing, the process must notify the Availability Management Framework
     * when the CSI(s) have been quiesced by invoking the
     * CsiManager.completedCsiQuiescing() method. When the
     * process invokes the completedCsiQuiescing() method, the process returns
     * invocation as an in parameter.
     * <P>This callback is invoked in the context of a thread of a
     * registered process issuing an dispatch() call on the handle
     * AmfHandle, which was used when registering the component,
     * identified by componentName, via the registerComponent() call. The
     * Availability Management Framework sets invocation and the process returns
     * invocation as an in parameter when it responds to the Availability
     * Management Framework using the response() method. The response()
     * method also has error as an in parameter, which, in this case, has one
     * of the following possible values defined in AisStatus:
     * <UL>
     * <LI>OK - The component executed the setCsiCallback() callback successfully.
     * <LI>ERR_FAILED_OPERATION - The component failed to assume the HA state,
     * specified by haState, for the given component service instance.
     * </UL>
     * <P>If the invoked process does not respond with the AmfHandle.response() method within a
     * configured time interval or returns an error, the Availability Management Framework
     * must engage the configured recovery policy (recoveryOnError) for the component to
     * which the process belongs.
     * <P>If the process responds to a setCSICallback() callback that specifies the
     * quiescing haState by invoking the Amfhandle.response() method with the error
     * parameter set to OK within the aforementioned time interval, and the process
     * does not subsequently respond that it has successfully quiesced within another
     * configured time interval (by invoking the completedCsiQuiescing() method),
     * the Availability Management Framework must engage the configured recovery policy
     * for the component to which the process belongs.
     *
     * @param invocation [in] This parameter designates a particular invocation
     *            of this callback method. The invoked process returns
     *            invocation when it responds to the Availability Management
     *            Framework using the response() method of AmfHandle.
     * @param componentName [in] The name of the component to which a new
     *            component service instance is assigned or for which the HA
     *            state of one or all supported component service instances is
     *            changed.
     * @param haState [in] The new HA state to be assumed by the component for
     *            the component service instance, identified by csiDescriptor,
     *            or for all component service instances already supported by
     *            the component (if CsiFlags.TARGET_ALL is set in csiFlags of the
     *            csiDescriptor parameter).
     * @param csiDescriptor [in] The descriptor with information about the
     *            component service instances targeted by this callback
     *            invocation.
     */
    void setCsiCallback( long invocation,
                         String componentName,
                         HaState haState,
                         CsiDescriptor csiDescriptor );

}


