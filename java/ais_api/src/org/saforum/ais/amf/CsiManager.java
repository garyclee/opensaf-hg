/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R2-JD-A.01.01
**   SAI-Overview-B.01.01
**   SAI-AIS-AMF-B.01.01
**
** DATE:
**   Thur May 29 2008
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

import org.saforum.ais.*;


/**
 * A CsiManager object contains API methods that enable to manage the HA state of
 * components on behalf of the component service instances that they support.
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public interface CsiManager {



    /**
     * The Availability Management Framework returns the HA state of a
     * component, identified by componentName, on behalf of the component
     * service instance, identified by csiName.
     * <P><B>SAF Reference:</B> <code>saAmfHAStateGet()</code>
     *
     * @param componentName [in] name of the component for which the information
     *            is requested.
     * @param csiName [in] name of the component service instance for which the
     *            information is requested.
     * @return the HA state that the Availability Management Framework is
     *         assigning to the component, identified by componentName, on
     *         behalf of the component service instance, identified by csiName.
     *         The HA state is active, standby, quiescing, or quiesced, as
     *         defined by the HAState enumeration type.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this
     *             CsiManager object is invalid, since it is corrupted or has
     *             already been finalized.
     * @throws AisInvalidParamException A parameter is not set correctly.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException The component, identified by componentName,
     *             has not registered with the Availability Management
     *             Framework, or the component has not been assigned the
     *             component service instance, identified by csiName.
     */
    public HaState getHaState( String componentName, String csiName )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;

    /**
     * Using this method, a component can notify the Availability Management
     * Framework that it has successfully stopped its activity related to a
     * particular component service instance or to all component service
     * instances assigned to it, following a previous request by the
     * Availability Management Framework to enter the QUIESCING HA state for
     * that particular component service instance or to all component service
     * instances.
     * <P>
     * The invocation of this API indicates that the component has now completed
     * quiescing the particular component service instance or all component
     * service instances and has transitioned to the quiesced HA state for that
     * particular component service instance or to all component service
     * instances.
     * <P>
     * It is possible that the component is unable to complete the ongoing work
     * due to, for example, a failure in the component. If possible, the
     * component should notify the Availability Management Framework of this
     * fact also using this method. The error parameter specifies whether or not
     * the component has stopped cleanly as requested.
     * <P>
     * This method may only be called by the registered process of a component
     * and the AmfHandle instance must be the same that was used when
     * the registered process registered this component via the
     * registerComponent() call.
     * <P><B>SAF Reference:</B> <code>saAmfCSIQuiescingComplete()</code>
     *
     * @param invocation [in] The invocation parameter that the Availability
     *            Management Framework assigned when it asked the component to
     *            enter the QUIESCING HA state for a particular component
     *            service instance or for all component service instances
     *            assigned to it by invoking the setCsi callback.
     * @param error [in] The component returns the status of the completion of
     *            the quiescing operation, which has one of the following
     *            values:
     *            <UL>
     *            <LI>AisStatus.OK - The component stopped successfully
     *            its activity related to a particular component service
     *            instance or to all component service instances assigned to it.
     *            <LI>AisStatus.ERR_FAILED_OPERATION - The component failed
     *            to stop its activity related to a particular component service
     *            instance or to all component service instances assigned to it.
     *            Some of the actions required during quiescing might not have
     *            been performed.
     *            </UL>
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this
     *             CsiManager object is invalid, since it is corrupted or has
     *             already been finalized.
     * @throws AisInvalidParamException A parameter is not set correctly.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     */
    public void completedCsiQuiescing( long invocation, int error )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException;

}


