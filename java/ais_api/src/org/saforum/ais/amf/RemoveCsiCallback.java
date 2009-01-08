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
 * the callback requests the component to remove a component service instance
 * from the set of component service instances being supported.
 *
 * <P><B>SAF Reference:</B> <code>SaAmfCSIRemoveCallbackT</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public interface RemoveCsiCallback {

    /**
     * The Availability Management Framework requests the component, identified
     * by componentName, to remove the component service instances identified by
     * csiName, from the set of component service instances being supported. If
     * the value of csiFlags is TARGET_ONE, csiName contains the
     * component service instance that must be removed. If the value of csiFlags
     * is TARGET_ALL, csiName is NULL and the method must remove all component
     * service instances. TARGET_ALL is always set for components that only
     * provide the x_active_or_x_standby capability model.
     * <P>
     * This callback is invoked in the context of a thread of a registered
     * process issuing an dispatch() call on the handle AmfHandle, which
     * was used when registering the component, identified by componentName, via
     * the registerComponent() call. The Availability Management Framework sets
     * invocation and the component returns invocation as an in parameter when
     * it responds to the Availability Management Framework using the response()
     * method. It also has error as an input parameter, which, in this case, has
     * one of the following possible values defined by AisStatus:
     * <UL>
     * <LI>OK - The component executed the removeCsiCallback() callback successfully
     * <LI>ERR_FAILED_OPERATION -The component failed to remove the given component
     * service instance.
     * </UL>
     * <P>
     * If the invoked process does not respond with the AmfHandle.response() method within a
     * configured time interval or returns an error, the Availability Management Framework
     * must engage the configured recovery policy (recoveryOnError) for the component to
     * which the process belongs.
     *
     *
     * @param invocation [in] This parameter designates a particular invocation
     *            of this callback method. The invoked process returns
     *            invocation when it responds to the Availability Management
     *            Framework using the response() method of AmfHandle.
     * @param componentName [in] The name of the component from which all
     *            component service instances or the component service instance,
     *            identified by csiName, will be removed.
     * @param csiName [in] The name of the component service instance that
     *            must be removed from the component identified by compName.
     * @param csiFlags [in] This flag specifies whether one or more component
     *            service instances are affected. It can contain one of the
     *            values TARGET_ONE or TARGET_ALL, defined in CsiDescriptor.
     */
    void removeCsiCallback( long invocation,
                            String componentName,
                            String csiName,
                            CsiDescriptor.CsiFlags csiFlags );

}


